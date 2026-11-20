/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

/* global addToLibrary, SYSCALLS */
/* eslint camelcase: ["error", { properties: "never" }] */

// Make fsync synchronous, because Emscripten 5's version crashes with Asyncify and threads.
addToLibrary({
  fd_sync__async: false,
  fd_sync__proxy: "sync",
  fd_sync: (fd) => {
    try {
      const stream = SYSCALLS.getStreamFromFD(fd);
      return stream.stream_ops.fsync ? stream.stream_ops.fsync(stream) : 0;
    } catch (e) {
      if (e.name !== "ErrnoError") {
        throw e;
      }
      return e.errno;
    }
  },
});
