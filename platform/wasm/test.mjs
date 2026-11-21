/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

// Creates a profile in the web build, waits for it to connect, then reloads
// the page and logs in again from what the browser kept.
//
// Usage: node platform/wasm/test.mjs [SITE_DIR]
import { readFile } from "node:fs/promises";
import { createServer } from "node:http";
import { extname, join, normalize } from "node:path";
import puppeteer from "puppeteer-core";

const site = process.argv[2] || "_site";
const chrome = process.env.CHROME || "/usr/bin/google-chrome";

const types = {
  ".html": "text/html",
  ".js": "text/javascript",
  ".svg": "image/svg+xml",
  ".wasm": "application/wasm",
};

// Send the headers that threads need, so the service worker doesn't have to reload the page.
const headers = (path) => ({
  "Content-Type": types[extname(path)] || "application/octet-stream",
  "Cross-Origin-Embedder-Policy": "require-corp",
  "Cross-Origin-Opener-Policy": "same-origin",
});

const serve = async (req, res) => {
  const url = new URL(req.url, "http://localhost");
  const path = normalize(url.pathname === "/" ? "/index.html" : url.pathname);
  try {
    const body = await readFile(join(site, path));
    res.writeHead(200, headers(path));
    res.end(body);
  } catch {
    res.writeHead(404);
    res.end();
  }
};

const sleep = (seconds) =>
  new Promise((resolve) => setTimeout(resolve, seconds * 1000));

// Resolves when qTox logs a line containing `text`.
const waitForLog = (page, text, seconds) =>
  new Promise((resolve, reject) => {
    let timer = null;
    const onLog = (msg) => {
      if (msg.text().includes(text)) {
        clearTimeout(timer);
        page.off("console", onLog);
        resolve();
      }
    };
    timer = setTimeout(() => {
      page.off("console", onLog);
      reject(new Error(`Timed out waiting for "${text}"`));
    }, seconds * 1000);
    page.on("console", onLog);
  });

// Loads the page and waits for the login screen.
const load = async (page, go) => {
  const started = waitForLog(page, "Process ID:", 120);
  await go();
  await started;
  await sleep(2);
};

const test = async (page, url) => {
  await load(page, () => page.goto(url));

  // Create a profile on the login screen.
  await page.mouse.click(679, 291);
  await page.keyboard.type("ci");
  const loggedIn = waitForLog(page, "Starting up", 60);
  const connected = waitForLog(page, "Connected to a TCP relay", 180);
  await page.mouse.click(690, 438);
  await loggedIn;
  await connected;

  // Load the same profile after a reload.
  await load(page, () => page.reload());
  const loaded = waitForLog(
    page,
    'Loading tox save "~/.config/tox/ci.tox"',
    60,
  );
  await page.mouse.click(716, 438);
  await loaded;
};

const main = async () => {
  const server = createServer(serve).listen(0, "127.0.0.1");
  await new Promise((resolve) => server.once("listening", resolve));
  const browser = await puppeteer.launch({
    executablePath: chrome,
    args: ["--no-sandbox"],
  });
  const errors = [];
  try {
    const page = await browser.newPage();
    await page.setViewport({ width: 1100, height: 800 });
    page.on("console", (msg) => console.log(msg.text()));
    page.on("pageerror", (err) => errors.push(err.message));
    await test(page, `http://127.0.0.1:${server.address().port}/`);
  } finally {
    await browser.close();
    server.close();
  }
  if (errors.length > 0) {
    throw new Error(`Page errors: ${errors.join("; ")}`);
  }
};

await main();
