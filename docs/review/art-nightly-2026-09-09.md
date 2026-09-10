# Electron nightly review — 2026-09-09

All **176 frozen commits have source outcomes**: 99 reused with exact Git object
proof and 77 newly reviewed. Full diffs, callers, tests, merge provenance and
later supersession were inspected. Native verification remains **incomplete**;
this report must not advance a complete-review marker.

Canonical main remains `1d3b4504eb3aee600a408b6b44de49b3a92d68e6`.
Both discovered production defects are branch-only. Root owns integration;
no repair was pushed or applied to canonical main.

## Proven branch repairs

Removing explicit generated-output staging makes lint-staged report success
while omitting the generated publish workflow from the commit. Repair
`8c9973ef77e3560a4e22a0295818150e63536b36` restores the three staging steps.
The real workflow generator and lint-staged 17.0.8 fixture fail before and pass
after repair. Docs and DEPS staging follow the same inspected mechanism but
were not individually exercised. Formatting, lint and the focused test pass.

ANGLE scratch pixel-unpack buffers leak on early error returns after allocation.
The helper persists at the observed 42/43/44 branch heads. A local scope guard
uses StateManagerGL cleanup on every exit; explicit reset preserves cleanup
before successful work submission. The guard avoids double deletion and retains
binding invalidation. The exact helper and GL error macros produce failing
baseline checks and passing repaired checks: seven release and nine assertion
cases, covering allocation, unpack-state, upload and pre-allocation failures.

The repairs are `158c2dd495c973d749e79df8c852303d817ccba5` (43),
`9c5ae3e4f74984884852cef25f8414562decd3fa` (42), and
`0c92f8a1283010361f842f9231e326ad817647b7` (44). Each lives on its own
isolated branch. The 43 affected file was assembled from pinned ANGLE source
and its Electron patch sequence; 42/44 have byte-identical affected helpers
and passed helper patch application. These checks use fake GL callbacks;
full patch-stack, renderer, platform and ABI verification remain outstanding.

Run `python3 docs/review/art-nightly-2026-09-09/check-scratch.py` with Clang
available. The committed fixture, exact function/macro excerpts, pinned
StateManagerGL deletion excerpt and correspondence receipts are beside it.

## Source and simplification coverage

[Coverage](art-nightly-2026-09-09/coverage.json) records all 176 exact SHAs.
[Source families](art-nightly-2026-09-09/source-families.json) retain the new
review outcomes. Security stacks include 112 patch entries across 38 families;
Node rolls preserve 124 compared nested patch payloads. Three merge reviews
separate inherited parent content from explicitly inspected residual edits.
Ancestry alone is not treated as semantic supersession or native proof.

The mandatory simplification inspection and second source pass cover 450 module
entries. [The ledger](art-nightly-2026-09-09/simplification.json) records
contracts, rejected removals and deferred native candidates. Removing staging
breaks behavior; replacing success-only scratch cleanup with one local owner
repairs a leak. Neither repair is claimed as behavior-preserving simplification.
No native preservation gate was waived. Prior sealed artifacts remain unchanged.

Exact sharding and streaming dependency fixtures also pass as recorded in
[checks](art-nightly-2026-09-09/checks.json). Shared report changes retain the
99-source-reviewed/native-incomplete distinction and were proved by exact blob
identity and portable digest recomputation with root.

## Native capability and remaining work

A local build exists at `~/Code/electron-build/src`. Release identifies Electron
43.4.1 / Chromium 150.0.7871.224 / Node 24.18.1; Testing identifies Electron
43.2.0 / Chromium 150.0.7871.129 / Node 24.18.0. Both launch in Node mode.
GN, Ninja, Clang and Xcode execute. This is capability evidence only: the
binaries do not have exact source/build identity for the reviewed scenarios.

[Scenario gaps](art-nightly-2026-09-09/scenario-native-gaps.json) pin 12 target
SHAs, DEPS, available or missing local objects, platform requirements and why
existing binaries cannot close each scenario. Main Chromium 154 and Node 24.19
source objects exist; transitive checkout and toolchain closure remain unknown.
The [isolated baseline plan](art-nightly-2026-09-09/native-one-test-plan.json)
is feasible but unexecuted. Root declined its unrelated 250–550 GiB bootstrap;
it would not close deferred branch lifecycle, overlay or cppgc scenarios.

The metal Linux host accepts SSH but has no build at the bounded known paths;
the Linux workstation refused SSH and the Windows host was offline. Discovery
claims apply only to inspected locations. Shared builds were left untouched.
[Capability evidence](art-nightly-2026-09-09/native-capability.json) retains
machine, executable and tool identities. No bulk bootstrap was performed.

Raw logs and full diffs stay in Art's `reviews/2026-09-09/electron-evidence`.
The lane result and simplification ledger are in `reviews/2026-09-09`.
