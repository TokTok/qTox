/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

/* global Module, FS, IDBFS, addRunDependency, removeRunDependency */

// Keep the home directory in IndexedDB, because the browser forgets the rest on reload.
Module.preRun = [].concat(Module.preRun ?? [], () => {
  FS.mount(IDBFS, { autoPersist: true }, "/home/web_user");
  addRunDependency("idbfs");
  FS.syncfs(true, (err) => {
    if (err) {
      console.error("Couldn't load files from IndexedDB:", err);
    }
    removeRunDependency("idbfs");
  });
});
