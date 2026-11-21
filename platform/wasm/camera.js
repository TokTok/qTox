/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

/* global addToLibrary, HEAPU8, qtoxCamera */
/* eslint camelcase: ["error", { properties: "never" }] */

// Give qTox camera frames as JPEG images, because its video code reads them like a MJPEG camera.
addToLibrary({
  $qtoxCamera: {
    video: null,
    canvas: null,
    failed: false,
    stop: (video) => video?.srcObject?.getTracks().forEach((t) => t.stop()),
  },

  qtox_camera_start__deps: ["$qtoxCamera"],
  qtox_camera_start__proxy: "sync",
  qtox_camera_start: (width, height) => {
    const camera = qtoxCamera;
    const video = document.createElement("video");
    video.muted = true;
    camera.video = video;
    camera.canvas = document.createElement("canvas");
    camera.failed = false;
    navigator.mediaDevices
      .getUserMedia({
        video:
          width > 0
            ? { width: { ideal: width }, height: { ideal: height } }
            : true,
      })
      .then((stream) => {
        video.srcObject = stream;
        // Stop it again if qTox closed the camera while the browser was asking.
        if (camera.video !== video) {
          camera.stop(video);
          return null;
        }
        return video.play();
      })
      .catch((err) => {
        console.error("Couldn't open the camera:", err);
        camera.failed = true;
      });
  },

  // Returns the JPEG size, 0 if there is no frame yet, or -1 on failure.
  // If the size is bigger than `size`, nothing was copied.
  qtox_camera_frame__deps: ["$qtoxCamera"],
  qtox_camera_frame__proxy: "sync",
  qtox_camera_frame: (buf, size) => {
    const { video, canvas, failed } = qtoxCamera;
    if (failed) {
      return -1;
    }
    if (video.readyState < video.HAVE_CURRENT_DATA) {
      return 0;
    }
    canvas.width = video.videoWidth;
    canvas.height = video.videoHeight;
    canvas.getContext("2d").drawImage(video, 0, 0);
    const url = canvas.toDataURL("image/jpeg");
    const jpeg = atob(url.slice(url.indexOf(",") + 1));
    if (jpeg.length <= size) {
      HEAPU8.set(
        Uint8Array.from(jpeg, (c) => c.charCodeAt(0)),
        buf,
      );
    }
    return jpeg.length;
  },

  qtox_camera_stop__deps: ["$qtoxCamera"],
  qtox_camera_stop__proxy: "sync",
  qtox_camera_stop: () => {
    qtoxCamera.stop(qtoxCamera.video);
    qtoxCamera.video = null;
  },
});
