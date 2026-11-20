/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

// jshint esversion: 8

// Qt's threads need SharedArrayBuffer, which browsers only allow on pages
// served with COOP/COEP headers. GitHub Pages can't send them, so the page
// registers this script as a service worker that adds them to every response.
if (typeof window === "undefined") {
  self.addEventListener("install", () => self.skipWaiting());
  self.addEventListener("activate", (event) =>
    event.waitUntil(self.clients.claim()),
  );
  self.addEventListener("fetch", (event) => {
    event.respondWith(
      fetch(event.request).then((response) => {
        // Opaque responses can't be copied.
        if (response.status === 0) {
          return response;
        }
        const headers = new Headers(response.headers);
        headers.set("Cross-Origin-Embedder-Policy", "require-corp");
        headers.set("Cross-Origin-Opener-Policy", "same-origin");
        return new Response(response.body, {
          status: response.status,
          statusText: response.statusText,
          headers,
        });
      }),
    );
  });
} else if (!window.crossOriginIsolated && navigator.serviceWorker) {
  // The page needs to load again through the worker to get the headers.
  navigator.serviceWorker.addEventListener("controllerchange", () =>
    location.reload(),
  );
  navigator.serviceWorker
    .register(document.currentScript.src)
    .then((registration) => {
      if (registration.active && !navigator.serviceWorker.controller) {
        location.reload();
      }
    });
}
