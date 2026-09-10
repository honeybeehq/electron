# Electron nightly review checkpoint — 2026-09-09

This lane is **incomplete**. Of 176 frozen candidate commits, 105 have source
outcomes: 99 reused by exact Git commit identity and six new outcomes. The
remaining 71 need full source/caller/test/supersession review. Their exact SHAs
and diff digests are in [coverage.json](art-nightly-2026-09-09/coverage.json).
Collected diffs do not count as inspected source. Native verification and the
full simplification pass remain incomplete independently of source progress.

Canonical main is `1d3b4504eb3aee600a408b6b44de49b3a92d68e6`. No production
change from this checkpoint belongs on that main. Previous sealed reports
remain unchanged.

## Proven branch-only regression

Removing explicit `git add` steps from lint-staged leaves generated files out
of the commit. With the actual workflow generator and lint-staged 17.0.8,
a staged build-workflow edit returns success while its generated publish
workflow remains unstaged. The focused regression test fails on the original
configuration and passes when the three generated-output staging steps return.

The repair is `8c9973ef77e3560a4e22a0295818150e63536b36`, based on
`c3194c25fd7f00888a46d2b4a22b18f6cb23350b`, on the isolated local branch
`art/electron-staging-repair-20260909`. Equivalent removals also occur at
`b74cb86b653df0d259037870d766e86fb827d6dd`,
`9678a7f0c490c60459f18228156fe86d3795f0e8`, and
`b633e93e40869bfbbfb788c167a42096c2a5caf6`. The package configuration and
generator remain unchanged through the observed 44-x-y head
`9e304448a95e5cc8bde401ae8e48841b8f7a1a61`. No repair was pushed.

The test invokes the actual generator and dependency in a disposable Git
repository. The generators themselves are unchanged. Formatting, lint and
`git diff --check` pass. Run `node --test script/lint-staged-generated-files.test.cjs`
in the repair checkout after installing its dependencies. This verifies the
workflow generator path; native builds and the other generators were not run.
Decisive output and source object IDs are in
[checks.json](art-nightly-2026-09-09/checks.json).

## Native recovery

A local native build exists at `~/Code/electron-build/src`. Its Electron
source HEAD is `054651494a1feaa722d40394d58766d65c5d4e55`; Chromium HEAD is
`cd239548724da459c1e383582de3af19aa8889dc`. Release reports Electron 43.4.1,
Chromium 150.0.7871.224 and Node 24.18.1. Testing reports Electron 43.2.0,
Chromium 150.0.7871.129 and Node 24.18.0. Both executables launch in Node mode;
GN, Ninja, Clang and Xcode also execute. This is capability evidence, not a
matching native test pass. The independent build clone adds zero daily commits
to the frozen candidate set.

Canonical main requires Chromium 154.0.8025.0 and Node 24.19.0. The resumed
merged scopes require Chromium 155.0.8038.2 and Node 24.20.0. Existing build
outputs therefore cannot establish their behavior. Matching isolated source,
patch application, generated output and binaries are required before those
native checks can close. See
[native-capability.json](art-nightly-2026-09-09/native-capability.json) for
exact pins, executable hashes, tool results and the bounded machine discovery.

The metal Linux host accepts SSH but has no Electron tree at the checked known
locations. The Linux workstation refuses SSH; the known Windows build host
is offline. No shared source, build output or configuration was mutated, and
no large bootstrap was started. Build requirements were sent to the run root.

## Other checks and simplification

The exact new sharding scripts at `8d93c43b78f42d0efd5d9c7d9c8c9211232902c9`
pass deterministic partition, weight-selection and artifact-generator fixtures.
The exact streaming-dependency test at
`532c3159a1c64c71f96ed9b206f59289a1225f2d` fails with incompatible v3 imports
and passes with compatible v1 imports.

Removing generated-output staging is rejected as a simplification: it changes
the committed result. The per-job sharding fallback and deterministic selection
remain useful and passed a second local inspection. No behavior-preserving
simplification was applied. The full per-module inspection and second pass,
including the five prior native candidates, remain pending. Large WebContents
GC/lifecycle changes, imported merge interactions and security patch stacks
still require review; native access does not replace that source work.

Raw logs and scratch fixtures remain in Art's `reviews/2026-09-09/electron-evidence`.
The structured lane result and simplification ledger retain exact pending scope.
This checkpoint must not advance a complete-review marker.
