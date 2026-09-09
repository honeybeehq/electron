# Art nightly review: honeybeehq/electron, 2026-09-08

Status: **incomplete**. One branch-only regression was reproduced and repaired locally. All 99 assigned SHAs now have source-diff, caller/test and simplification inspection outcomes, including the two previously pending merges. These are source outcomes, not native test passes. Native execution and several behavior-preservation proofs remain outstanding.

## Repository and frozen scope

- Source origin: `https://github.com/trmdy/electron.git`.
- Owned baseline, `origin/main`: `572508b35dc048e63beae87d34076e18d2f57044`.
- `upstream` is `https://github.com/electron/electron.git`.
- Frozen `upstream/main`: `5c577105a068492d54c5c21bf4d3910039a1607d`.
- Frozen `upstream/42-x-y`: `22de0a3a06cfd3174fc6c59c3810ab5e1c91bf1a`.
- Frozen `upstream/43-x-y`: `9f1fa686e3db88ced2ed9acbb3a30a8fc003f982`.
- Frozen `upstream/44-x-y`: `07e460719c75b2ec5ee4893f7d2192ef31c7b8c2`.

Only the historical documentation candidate is an ancestor of frozen origin/main. Branch names quoted inside upstream merge commit messages do not identify this checkout's origin/main. The [frozen ref scope](art-nightly-2026-09-08/frozen-ref-scope.json) records the actual refs and SHAs used for ancestry checks.

GitHub now redirects the historical `trmdy/electron` origin URL to canonical **honeybeehq/electron**, repository ID `1314572776`. The earlier claim that these were different repositories was incorrect. [Live identity proof](art-nightly-2026-09-08/canonical-identity.json) preserves the alias and frozen baseline. The September 7 report was preserved and not reused as source-review proof for this lane. Root will integrate this full report over its docs-only short report at `c34e83b192976e8e178f0c6e578aba3c041e5d86`; that short report covered its original clone/main scope, while this ledger preserves all 99 assigned upstream and branch candidates. Ancestry identifies where a candidate landed; it does not establish that an older main lacks the bug a branch fixes.

The [candidate ledger](art-nightly-2026-09-08/candidate-ledger.json) contains each exact SHA, subject, changed paths, review notes, changed test paths, normalized patch identity, later candidate touches and containing frozen refs. [Diff hashes](art-nightly-2026-09-08/diff-manifest.json) identify the first-parent changes. Reproduce a diff with `git diff SHA^1 SHA`. Equal patch IDs permit reuse of the normalized edit inspection, not assumptions about whole-tree behavior or native test results.

## Reproduced regression and isolated repair

**High: stream-json 3.5.0 breaks the clang-tidy driver at module load.**

Source: `0216074fd29d02c486245699df1ec3ba688c94c8`, an upstream dependency branch. The commit upgrades stream-json from 1.9.1 to 3.5.0, while `script/run-clang-tidy.ts` still imports `stream-json/filters/Ignore` and `stream-json/streamers/StreamArray`. With the new installed package, those extensionless subpaths do not resolve. The driver cannot reach its compilation-database processing.

Repair: `532c3159a1c64c71f96ed9b206f59289a1225f2d`, on isolated local branch `art/electron-dependency-repair-20260908`, based on the exact offending SHA. It restores stream-json 1.9.1 and its compatible lock entries, retains the fast-uri 3.1.6 update, and adds `script/run-clang-tidy-dependencies.test.cjs`. No repair was applied to origin/main, whose manifest already uses stream-json 1.9.1.

The focused test derives the stream-package imports from the real driver and requires each using installed packages. It failed with v3 and passed with v1. This proves dependency resolution; it does not prove a full clang-tidy run against Chromium.

- [Red output](art-nightly-2026-09-08/dependency-v3.log): `stream-json/filters/Ignore` fails to load, exit 1.
- [Green output](art-nightly-2026-09-08/dependency-v1.log): one test passed, exit 0.
- Repair JavaScript syntax, oxfmt, oxlint and `git diff --check` passed.

To reproduce from the repair checkout, install stream-json 3.5.0 and 1.9.1 in two temporary prefixes with lifecycle scripts disabled. Run `NODE_PATH=<prefix>/node_modules node --test script/run-clang-tidy-dependencies.test.cjs` against each. The prefixes are test fixtures and do not replace repository dependencies.

## Additional checks and review evidence

The [sharding fixture](art-nightly-2026-09-08/check-sharding.cjs) runs the actual scripts at `42260acba8df81154a6feab8d159a19264804d4c` in a temporary filesystem. It passed deterministic complete partitioning, exact/fallback/median table selection, maximum retry timing, sanitizer inference, MAS separation, Wayland exclusion, and finite nonnegative shipped weights. It uses an isolated glob 11.0.3 installation. The CI workflow exports matching `ARTIFACT_KEY` values before invoking the sharder. This fixture is not an Electron integration test.

The [updater conservation check](art-nightly-2026-09-08/updater-test-conservation.json) found the same 27 detected test titles before and after each of nine exact split/backport commits. Canonical lifecycle/policy tests and helper were read, old/new line multisets compared, and branch helper differences inspected. Backports use Express where the routed-server helper is absent; the 42 branch additionally introduces template cloning and signing. Title conservation does not prove cancellation, retries, app signatures, or ShipIt cleanup.

The [patch rebase payload check](art-nightly-2026-09-08/patch-rebase-payloads.json) establishes identical ordered nested addition/deletion payloads for all 60 patches in `993fe372e60b412214463893dcc9419804245d97` and all four in `0420db4ffb7e9251a288593e857da8c8c34759f7`. Remaining context changes were inspected. This proves edit identity, not application against the missing Chromium checkout.

All four merge resolutions were inspected using `git show --remerge-diff`. The incoming commit sets and first-parent scope were also inspected. The two older merges import substantial additional code: js2c entrypoints/cache, linked bindings, crashpad propagation, File System Access grants, requesting-frame attribution, native patches, and corresponding specs. Normalized residual production/spec edits and removed patch payloads were examined; the [nan lock check](art-nightly-2026-09-08/nan-lock-structure.json) records the structured lock changes. The supplemental pass completed both integrated source inspections: `148a462376c079e2ca0b9a61e29a14f718bf73b2` (172 paths) and `5f46fcf9a9612670f60e9cd92e4016efea8a3384` (69 paths). [Per-path blob identities](art-nightly-2026-09-08/merge-path-identity.json), [41 exact source retrieval receipts](art-nightly-2026-09-08/merge-source-access.json), and [caller/contract notes](art-nightly-2026-09-08/merge-inspection.json) make the supplemental evidence portable. Missing local third-party files were resolved through pinned remote source retrieval; no source inspection remains pending. Native patch application and compatibility execution remain unverified.

## Suspicions awaiting native reproduction

These are not verified bugs or completed fixes.

1. `fb5f8e20490298416f6702382ef578c00aebd384` and debugger variants: `DestroyOverlay()` retains `last_bounds_`; `EnsureOverlay()` creates a new widget, and `UpdateOverlay()` only calls `SetBounds()` when bounds differ. Detach and reattach a contents view at identical screen bounds to check whether the replacement overlay remains correctly positioned and visible. The debugger file is absent from frozen origin/main.
2. The download-origin test imported by `5f46fcf9a9612670f60e9cd92e4016efea8a3384` calls `preventDefault()` in `will-download`, resolves a promise carrying the item, then reads `getInitiatorOrigin()` after awaiting. `Session::OnDownloadCreated` calls `Cancel/Remove` after that event, and `DownloadItem::OnDownloadDestroyed` clears the pointer checked by the getter. Pinned Chromium `DownloadItemImpl::Remove` calls its delegate synchronously, and `DownloadManagerImpl::DownloadRemoved` erases the owned item. Run the exact spec to establish its lifetime failure before repairing the test. The getter/test is absent from frozen origin/main.
3. `5c577105a068492d54c5c21bf4d3910039a1607d` replaces the custom clipboard writer with `WriteRawDataForTest` and moves GPU initialization behavior. The removed writer's 100-format cap and early GPU initialization failures need explicit before/after characterization. The smaller implementation is not assumed to preserve those contracts.

4. In both imported merges, `FrameForLockWidget` takes the first frame whose widget matches. Pinned Chromium `RenderFrameHostImpl::GetRenderWidgetHost` walks to an ancestor owning the widget, so this cannot uniquely identify every requesting document. Reproduce same-process, cross-origin iframe pointer/keyboard-lock attribution; existing OOPIF tests do not establish that case. This is a source concern, not a demonstrated permission bypass.
5. Linux `lib/node/init.ts` appends crashpad variables to normalized `envPairs`. Existing entries can therefore duplicate the new names; pinned Node forwards the array to native spawn. Run an Electron child with pre-existing handler variables to characterize which values reach crashpad before changing propagation.

## Radically-simplify pass

The [simplification ledger](art-nightly-2026-09-08/simplification.json) records the required skill path/digest, exact baseline and commit scope, inspected modules, 14 concrete candidates, contracts, proposed replacements, dispositions, and remaining work. No simplification was committed; the dependency fix is a separate regression repair.

Safe nonchanges include retaining incremental compilation-database parsing, rechecking liveness after JavaScript conversions/events, preserving per-frame security checks, retaining sequence-owned cleanup and native model ownership, and preserving transient updater retry identities. Replacing those with a shared default or deleting apparently duplicate checks changes supported behavior. The per-job sharder remains small enough that an additional generic selector would add indirection.

A second consideration of the recorded candidates found no additional transformation with sufficient preservation proof. The second source pass now covers both integrated merges. The full behavior-preservation gate remains incomplete because concrete native characterization is outstanding. Deferred candidates include mouse observer registration, clipboard writer equivalence, overlay bounds state, native compatibility patches, and imported merge architecture. They remain deferred rather than being labeled safe merely because no change was made.

## Environment, validation and handoff

The owned worktree and source checkout have no installed repository dependencies, complete matching Chromium/Node source tree, generated build outputs or built Electron app. GN, Ninja and autoninja are absent from PATH. The available system compiler is not a substitute for Electron's pinned native toolchain.

The repository markdown-lint command failed before linting because `minimist` is missing. Standalone markdownlint with the repository configuration and Electron custom rules passed. The committed sharding fixture also passed JavaScript syntax, oxfmt and oxlint checks. [Check results](art-nightly-2026-09-08/checks.json) distinguish these from the failed repository lint launcher. No native build, Electron spec suite, macOS signing/updater run, Windows/MSIX test or Linux portal test is claimed as passed. GUI reattachment, fullscreen, drag, zoom, clipboard and menu checks still require the matching application on the relevant platform.

`gh run list --repo trmdy/electron --commit 572508b35dc048e63beae87d34076e18d2f57044` returned no runs. There is no CI pass to report. Local repair/report commits were not pushed; root Art owns publication and subsequent exact-SHA CI reconciliation.

Portable files are covered by [SHA-256 hashes](art-nightly-2026-09-08/evidence-sha256.json). Machine-level result and simplification JSON files remain under Art's `reviews/2026-09-08/` directory. No historical root artifact, live runtime state, remote branch, release or deployment was changed. No child agents were launched.
