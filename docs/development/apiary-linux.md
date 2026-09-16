# Apiary Linux fork distribution

Apiary consumes `honeybeehq/electron`, on the release-bearing
`apiary/surface-embedding-43.4.1` branch. The fork's `main` tracks a different
Electron/Chromium version and is not the base for a 43.4.1 Linux build.
`feature/linux-support` starts from release `v43.4.1-apiary.3` (`de883c8a07`).

The existing frame-attachment, page-freezing, media-capture and background
visibility-lock implementations are shared across platforms. Linux needs the
same native tests and a published distribution; stock Electron is not an
equivalent replacement. The satellite apiaryd runtime is independent of Electron,
but building or running the Apiary desktop in a satellite checkout needs this zip.

## Build and verify

Use a dedicated Linux gclient tree and named Electron build-tools config. Follow
[Linux prerequisites](build-instructions-linux.md) and
[GN setup](build-instructions-gn.md). An existing tree can be preserved with
`e worktree add apiary-linux-support /path/to/new-tree --source old-config --no-sync`.
Check out the intended fork commit in the new `src/electron` before syncing.

Set `override_electron_version="43.4.1"` in the testing config's GN args:
otherwise `git describe` can put the `-apiary.N` release suffix into the binary.
For release builds, `build/args/apiary-linux.gn` imports upstream's release
profile and pins that runtime version. Select it with
`e init apiary-linux-release --root=/path/to/new-tree -i apiary-linux -o Release --target-cpu=x64 --remote-build=none`.

Run as a normal user with Chromium's sandbox available. On a headless builder,
install Xvfb and use its X11 display. Do not disable the sandbox in the build recipe.

```sh
e --config=apiary-linux-support sync
e --config=apiary-linux-support build --no-remote --target electron:electron_dist_zip
cd /path/to/new-tree/src/electron
xvfb-run -a ../out/Testing/electron script/apiary/smoke.cjs 43.4.1 --ozone-platform=x11
ELECTRON_OUT_DIR=Testing xvfb-run -a node script/spec-runner.js --runners=main \
  --files=spec/api-web-contents-spec.ts --grep='app-owned frame attachments|classic webview detach'
python3 -B -m unittest discover -s script/apiary -p 'test_*.py'
python3 script/apiary/stage_linux_dist.py ../out/Testing/dist.zip \
  --version 43.4.1 --arch x64 --output /path/to/new-linux-assets
```

`Testing` is useful for native verification because DCHECKs remain enabled.
Use a release config and its corresponding output directory for shipping;
run the smoke and native specs on that exact binary too. The stage helper uses
the upstream zip manifest, verifies the internal version and ELF architecture,
checks executable permissions and CRCs, and copies the standard Electron archive
unchanged. It refuses an existing staging directory. Supported staging targets
are x64 and arm64; arm64 needs its own native verification before publication.

Keep native Electron switches after the smoke script and its version argument:
Electron retains switches in `process.argv`, so putting them before the script
shifts the smoke's positional version argument. The spec runner's `--files`
paths are relative to the Electron repository, including the `spec/` prefix.
For these frame/lifecycle-only tests, a builder without the general runner's
Python D-Bus mocks can run the same specs directly:

```sh
xvfb-run -a ../out/Testing/electron spec --ozone-platform=x11 \
  --files=spec/api-web-contents-spec.ts --grep='app-owned frame attachments|classic webview detach'
```

## Release contract

Stage `electron-v43.4.1-linux-x64.zip` and, when verified,
`electron-v43.4.1-linux-arm64.zip`. The npm/runtime version remains `43.4.1`;
only the release tag carries `-apiary.N`.

The helper writes a platform-specific `SHASUMS256-linux-<arch>.txt` fragment.
Assemble **one** `SHASUMS256.txt` containing every platform asset in the release,
including macOS. Never upload a Linux-only checksum file over a shared release
manifest. Verify all archives against that combined manifest before publishing.
Use a new release revision, keep all platform builds on the same reviewed source,
and record the source SHA, GN args and smoke/spec results in release evidence.
The helper does not publish, alter release tags, or change Apiary's pin.

After publication, update Apiary's `electron_custom_dir` and verify a clean Linux
`pnpm install --frozen-lockfile`, `pnpm electron:fetch`, `pnpm build` and
`pnpm dev:agent`. Complex keyboard/focus and graphics checks still need manual
Wayland/Hyprland and X11 testing, including embedded-browser focus, overlays,
retained pages and terminal Control chords.

## CI availability

As checked on 2026-09-16, Actions are disabled on `honeybeehq/electron` and no
runners are registered there. Inherited upstream publish workflows are gated to
`electron/electron` and require Electron's own build infrastructure. The Apiary
repository's `apiary-ci` runners are not automatically available to this fork.
Provision a dedicated builder and enable only the fork build workflow when
introducing CI; do not turn on the inherited publish workflows as a shortcut.
