# Surface embedding implementation notes

## Final API

```ts
await contents.attachToFrame(frame: WebFrameMain): Promise<void>
await contents.detachFromFrame(): Promise<void>
contents.isAttachedToFrame(): boolean
```

`did-attach-to-frame` emits `(event, frame)`. `did-detach-from-frame` emits
`(event, reason)`, where `reason` is `explicit`, `frame-destroyed`, or
`embedder-destroyed`. Destroying the target emits only the existing `destroyed`
event.

The target remains a normal, app-owned `WebContents`. It is never registered
with `guest-view-manager`, `WebViewManager`, or the classic `<webview>` maps,
and its type is not changed to `webview`.

## Deltas from the accepted sketch

The binding PM decisions override the reconnaissance sketch as follows:

- `attachToFrame` accepts a `WebFrameMain`, not an embedder/token pair. The
  native binding derives the `RenderFrameHost`, owning `WebContents`, frame
  tree node, and process directly. Malformed-token and wrong-embedder states
  therefore cannot be expressed by the public API.
- `detachFromFrame` returns `Promise<void>`. Chromium's detach remains
  synchronous, so the returned promise is resolved immediately after the
  relationship is gone.
- `did-attach-to-frame` supplies the post-swap outer-delegate `WebFrameMain`
  instead of an embedder and stale token.
- A dedicated `FrameAttachmentObserver` owns outer observation, zoom wiring,
  owner-window restoration, and automatic detach events. The classic
  `WebViewGuestDelegate` remains construction-time guest-only; its zoom reset
  was hardened independently.
- V1 enforces an active, live, same-process `about:blank` `<iframe>`, not merely
  Chromium's broader prepared-child-frame contract.
- Docked DevTools are rejected before and after frame preparation. DevTools
  opened while attached are forced to `detach` mode even if another mode was
  requested.
- Visibility uses Chromium inner-tree propagation only. No
  `guest-view-manager` visibility forwarding was added.
- The inverted classic `DetachFromOuterFrame` condition and its regression
  test are isolated in the first commit.
- Reusing the original native `WebContentsView` wrapper after final detach is
  explicitly unsupported in V1.

## Guard implementation

| Guard | Implementation |
| --- | --- |
| Feature gate | `features::kAttachUnownedInnerWebContents` is enabled in `shell/browser/feature_list.cc`; an explicit command-line disable still wins and makes the API reject before Chromium. |
| App-owned target | Only normal `kBrowserWindow`/`kBrowserView` wrappers enter this path. Classic guests, remote wrappers, background pages, and offscreen contents reject. |
| One outer / pending serialization | `GetOuterWebContents()` and `frame_attach_pending_` reject double attach. `frame_attachment_generation_` invalidates canceled or stale callbacks. |
| Frame identity | The binding reads the `WebFrameMain`'s live RFH directly. A disposed frame rejects with `ERR_FRAME_NOT_FOUND`; the primary main frame rejects separately. |
| Supported placeholder | Both before and after preparation, the RFH must be active, live, an `<iframe>`, `about:blank`, same-process with the outer main frame, and not already an outer delegate. |
| Prepare and revalidate | `PrepareForInnerWebContentsAttach` runs asynchronously. The callback rechecks target/embedder lifetime, generation, frame-tree-node identity, ownership, lifecycle, URL, process, host vacancy, and DevTools state. |
| Ownership | `GuestContentsHandle::AttachToOuterWebContents` reaches Chromium's unowned-inner path. `InspectableWebContents` remains the sole owner; the classic `attached_` ownership flag is untouched. |
| Explicit detach | `GuestContentsHandle::DetachFromOuterWebContents` runs synchronously, ancillary state is reset, and the promise resolves after `GetOuterWebContents()` is null. |
| Frame/embedder teardown | An outer observer checks the actual relationship after frame/RFH loss, outer renderer loss, or outer destruction. It emits once and leaves the target alive. |
| Target teardown | Chromium's `GuestContentsHandle` detaches during target destruction. Electron invalidates pending work and removes its outer/zoom observer without emitting a detach event. |
| Zoom | Attach copies the outer level/default factor and observes later changes. Every detach clears the inner controller's embedder pointer before removing the outer observer. The classic delegate now does the same. |
| Focus and crash focus | `focus()` and post-crash focus skip any actual inner WebContents, based on `GetOuterWebContents()` rather than `is_guest()`. |
| Keyboard | Unhandled keyboard events follow the actual outer chain before falling back to classic embedder bookkeeping or the owner window. |
| Popups/dialog ownership | The target retains its normal delegate and `setWindowOpenHandler` path. Its owner window follows the outer while attached and is restored after detach. |
| DevTools | Attach rejects docked DevTools. `openDevTools` forces `detach` mode while an outer relationship exists. |
| Fullscreen | Upward propagation uses the actual outer relationship. Downward exit walks `GetInnerWebContents()`, covering both classic and unowned inners. |
| Visibility | No Electron forwarding was added; specs exercise Chromium's hide/show propagation. |
| Crash behavior | Inner renderer loss does not change attachment state and remains reloadable. Outer loss is handled as automatic detach. |
| Native view | Documentation requires removal from the native hierarchy before attach and declares the old wrapper non-reusable after final detach. |

## APIARY-VERIFY

Two true runtime-ordering questions are marked in source:

1. Confirm that outer teardown notification ordering consistently upgrades the
   detach reason to `embedder-destroyed`, rather than observing only the hosted
   frame deletion.
2. Confirm that Chromium's post-swap outer-delegate RFH remains representable
   as a live `WebFrameMain` for the `did-attach-to-frame` event.

The exact Chromium 150 signatures for
`GuestContentsHandle::{CreateForWebContents,AttachToOuterWebContents,DetachFromOuterWebContents}`,
`RenderFrameHost::PrepareForInnerWebContentsAttach`,
`RenderFrameHost::IsActive`, and the unowned attach/detach methods were checked
against tag `150.0.7871.129`.

## Test coverage

Fifteen specs were added:

- classic owned-webview outer-frame detach regression;
- explicit attach/detach/reattach and state preservation;
- iframe removal, automatic detach, and replacement-frame reattach;
- main, disposed, navigated, self-owned, pending, already-attached, and
  occupied-host rejection paths;
- embedder destruction while the target remains usable;
- target destruction while the embedder remains usable;
- inner renderer crash/reload without detach;
- focus and keyboard delivery;
- normal `setWindowOpenHandler` popup policy;
- zoom inheritance and observer reset;
- docked DevTools rejection and forced detached mode;
- Chromium visibility and disabled-background-throttling propagation;
- HTML fullscreen propagation.

IME composition, accessibility, native z-order, drag routing, and native-view
reuse remain manual cross-platform checks; they are intentionally not brittle
automated UI tests.

## Full-tree compile and iteration runbook

Do not run this against `~/Code/electron-build` until the orchestrator confirms
that tree is ready.

1. Export this series:

   ```sh
   cd ~/Code/electron-fork-recon
   git format-patch --output-directory /tmp/apiary-surface-embedding \
     v43.2.0..apiary/surface-embedding
   ```

2. Apply it to the clean full source tree:

   ```sh
   cd ~/Code/electron-build
   git am /tmp/apiary-surface-embedding/*.patch
   ```

3. Generate and inspect the public TypeScript contract:

   ```sh
   npm run create-typescript-definitions
   rg -n 'attachToFrame|detachFromFrame|did-attach-to-frame|did-detach-from-frame' \
     electron.d.ts
   ```

4. Run formatting/lint and compile:

   ```sh
   npm run lint
   autoninja -C out/Testing electron
   ```

5. Run the focused specs, then the full suite:

   ```sh
   npm run test -- -g='classic webview detach|app-owned frame attachments'
   npm run test
   ```

6. Repeat the focused specs on macOS, Windows, and Linux. Manually verify IME,
   accessibility, z-order, drag/input routing, repeated detach/reattach,
   detached DevTools, fullscreen, hide/show throttling, and destruction in
   both orders.

This blobless checkout has no Chromium tree and no installed JavaScript
dependencies. `git diff --check` is available here; compilation, generated
typing validation, lint, and runtime specs belong to the full-tree pass above.
