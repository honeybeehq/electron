// Run with the built Electron executable under Xvfb, never with system Node.
const assert = require('node:assert/strict');
const { app, BrowserWindow, WebContentsView } = require('electron');

const expectedVersion = process.argv[2];
const timeout = setTimeout(() => {
  console.error('Apiary fork smoke timed out');
  app.exit(1);
}, 30000);

app
  .whenReady()
  .then(async () => {
    assert.equal(process.versions.electron, expectedVersion);
    const window = new BrowserWindow({ show: false });
    const view = new WebContentsView();
    const contents = view.webContents;
    for (const method of [
      'attachToFrame',
      'detachFromFrame',
      'isAttachedToFrame',
      'setVisibility',
      'setPageFrozen',
      'hasActiveMediaCapture',
      'lockBackgroundVisibility',
      'unlockBackgroundVisibility',
      'isBackgroundVisibilityLocked'
    ]) {
      assert.equal(typeof contents[method], 'function', `Missing fork capability: ${method}`);
    }
    await window.loadURL('data:text/html,<iframe name="guest" src="about:blank"></iframe>');
    await contents.loadURL('data:text/html,<title>linux-fork-smoke</title>');
    await contents.attachToFrame(window.webContents.mainFrame.frames[0]);
    assert.equal(contents.isAttachedToFrame(), true);
    assert.equal(await contents.executeJavaScript('document.title'), 'linux-fork-smoke');
    contents.lockBackgroundVisibility();
    assert.equal(contents.isBackgroundVisibilityLocked(), true);
    assert.throws(() => contents.setVisibility('visible'), /ERR_VISIBILITY_LOCKED/);
    contents.setPageFrozen(true);
    contents.setPageFrozen(false);
    contents.unlockBackgroundVisibility('hidden');
    assert.equal(contents.isBackgroundVisibilityLocked(), false);
    assert.equal(contents.hasActiveMediaCapture(), false);
    await contents.detachFromFrame();
    assert.equal(contents.isAttachedToFrame(), false);
    contents.close();
    window.destroy();
    clearTimeout(timeout);
    console.log(
      JSON.stringify({ ok: true, version: process.versions.electron, platform: process.platform, arch: process.arch })
    );
    app.quit();
  })
  .catch((error) => {
    console.error(error);
    app.exit(1);
  });
