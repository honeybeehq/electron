const assert = require('node:assert/strict');
const cp = require('node:child_process');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const root = fs.mkdtempSync(path.join(os.tmpdir(), 'electron-shard-review-'));
const sha = '42260acba8df81154a6feab8d159a19264804d4c';
fs.mkdirSync(path.join(root, 'script'));
fs.mkdirSync(path.join(root, 'spec'));
for (const file of ['split-tests.js', 'gen-spec-weights.js']) {
  fs.writeFileSync(path.join(root, 'script', file), cp.execFileSync('git', ['show', `${sha}:script/${file}`]));
}
for (const [file, count] of [
  ['a', 9],
  ['b', 4],
  ['c', 2],
  ['d', 1]
]) {
  fs.writeFileSync(path.join(root, `spec/${file}-spec.ts`), 'it('.repeat(count));
}
const run = (script, args, key = 'linux_x64') =>
  cp
    .execFileSync(process.execPath, [path.join(root, 'script', script), ...args], {
      cwd: root,
      env: { ...process.env, ARTIFACT_KEY: key }
    })
    .toString()
    .trim();
const weightsPath = path.join(root, 'script/spec-weights.json');
const table = (weights) => fs.writeFileSync(weightsPath, JSON.stringify(weights));
const a = 'spec/a-spec.ts',
  b = 'spec/b-spec.ts',
  c = 'spec/c-spec.ts',
  d = 'spec/d-spec.ts';
const assertPartition = (key) => {
  const shards = [1, 2, 3].map((n) =>
    run('split-tests.js', [String(n), '3'], key)
      .split(' ')
      .filter(Boolean)
  );
  assert.deepEqual(shards.flat().sort(), [a, b, c, d]);
  assert.deepEqual(
    shards,
    [1, 2, 3].map((n) =>
      run('split-tests.js', [String(n), '3'], key)
        .split(' ')
        .filter(Boolean)
    )
  );
};
assertPartition('linux_x64');
table({ linux_x64: { [a]: 10, [b]: 9, [c]: 3, [d]: 2 }, darwin_arm64: { [a]: 1, [b]: 2, [c]: 3, [d]: 20 } });
assert.equal(run('split-tests.js', ['1', '2']), `${a} ${d}`);
assert.equal(run('split-tests.js', ['1', '2'], 'mas_arm64'), d);
assert.equal(run('split-tests.js', ['1', '2'], 'linux_x64_asan'), `${a} ${d}`);
for (const key of ['linux_x64', 'mas_arm64', 'win_x64', 'linux_arm64_ubsan']) assertPartition(key);
table({ linux_x64: { [a]: 10, [b]: 0 } });
assertPartition('linux_x64');
const artifacts = path.join(root, 'artifacts');
const fixture = (name, obj) => {
  const dir = path.join(artifacts, name);
  fs.mkdirSync(dir, { recursive: true });
  fs.writeFileSync(path.join(dir, 'spec-timings.json'), JSON.stringify(obj));
};
fixture('test_artifacts_linux_x64_asan_x11_1', { platform: 'linux', arch: 'x64', files: { [a]: 12.3, [b]: 4 } });
fixture('test_artifacts_linux_x64_asan_x11_2', {
  platform: 'linux',
  arch: 'x64',
  sanitizer: 'asan',
  files: { [a]: 14.8 }
});
fixture('test_artifacts_linux_x64_wayland_1', { platform: 'linux', arch: 'x64', files: { [a]: 999 } });
fixture('test_artifacts_mas_arm64_1', {
  platform: 'darwin',
  arch: 'arm64',
  mas: true,
  sanitizer: null,
  files: { [b]: 2.8 }
});
run('gen-spec-weights.js', [artifacts]);
assert.deepEqual(JSON.parse(fs.readFileSync(weightsPath)), {
  linux_x64_asan: { [a]: 15, [b]: 4 },
  mas_arm64: { [b]: 3 }
});
const actual = JSON.parse(cp.execFileSync('git', ['show', `${sha}:script/spec-weights.json`]));
for (const [key, values] of Object.entries(actual)) {
  assert.match(key, /^(darwin|mas|linux|win)_(x64|arm64)(_(asan|ubsan))?$/);
  for (const [spec, weight] of Object.entries(values)) {
    assert.match(spec, /^spec\/.+-spec\.ts$/);
    assert.ok(Number.isFinite(weight) && weight >= 0);
  }
}
console.log(
  `PASS: actual scripts at ${sha}: deterministic complete partition, exact/fallback/median tables, retry max, sanitizer inference, MAS separation, Wayland exclusion, shipped weight schema. Fixture retained: ${root}`
);
