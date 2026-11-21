/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

/* global Module, SOCKFS */

// Join queued TCP data, because toxcore waits for whole packets and they can span WebSocket messages.
Module.preRun = [].concat(Module.preRun ?? [], () => {
  const SOCK_STREAM = 1;
  const ops = SOCKFS.websocket_sock_ops;

  const joinQueue = (sock) => {
    const queue = sock.recv_queue;
    if (sock.type !== SOCK_STREAM || queue.length < 2) {
      return;
    }
    const data = new Uint8Array(
      queue.reduce((size, q) => size + q.data.length, 0),
    );
    let offset = 0;
    for (const q of queue) {
      data.set(q.data, offset);
      offset += q.data.length;
    }
    queue.splice(0, queue.length, { ...queue[0], data });
  };

  for (const name of ["ioctl", "recvmsg"]) {
    const op = ops[name];
    ops[name] = (sock, ...args) => {
      joinQueue(sock);
      return op.call(ops, sock, ...args);
    };
  }
});
