/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

// Emscripten turns each TCP connection into a WebSocket to ws://address:port,
// but Tox relays don't speak WebSocket. This gateway passes the bytes on.
(() => {
  const gateway = "wss://tox-gateway.onrender.com";
  const NativeWebSocket = window.WebSocket;
  window.WebSocket = class extends NativeWebSocket {
    constructor(url, protocols) {
      const relay = /^ws:\/\/(.+):(\d+)\/$/.exec(url);
      if (relay === null) {
        super(url, protocols);
      } else {
        // The gateway accepts no subprotocol, so we don't ask for Emscripten's.
        super(`${gateway}/${relay[1]}/${relay[2]}`);
      }
    }
  };
})();
