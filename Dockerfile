FROM alpine:edge

RUN ["apk", "add", \
 "autoconf", \
 "automake", \
 "bash", \
 "clang", \
 "cmake", \
 "curl", \
 "diffutils", \
 "file", \
 "git", \
 "libtool", \
 "libxslt", \
 "linux-headers", \
 "make", \
 "meson", \
 "pkgconf", \
 "python3", \
 "samurai", \
 "tcl", \
 "yasm"]

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV CC=clang CXX=clang++ SYSROOT=/sysroot
ENV CFLAGS="-I$SYSROOT/usr/include -fPIC" LDFLAGS="-L$SYSROOT/usr/lib -L$SYSROOT/usr/lib64"
ENV PKG_CONFIG_PATH=$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/lib64/pkgconfig:$SYSROOT/usr/share/pkgconfig

WORKDIR /work/build

COPY linux/static-toolchain.cmake /sysroot/

RUN tar jxf <(curl -L https://github.com/kcat/openal-soft/releases/download/1.23.1/openal-soft-1.23.1.tar.bz2) \
 && cd openal-soft-1.23.1 \
 && cmake -Wno-dev \
  -DCMAKE_TOOLCHAIN_FILE=/sysroot/static-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr \
  -DLIBTYPE=STATIC \
  -GNinja \
  -B_build \
  -H. \
 && cmake --build _build \
 && cmake --install _build \
 && rm -rf /work/build

RUN tar zxf <(curl -L https://github.com/google/double-conversion/archive/refs/tags/v3.3.0.tar.gz) \
 && cd double-conversion-3.3.0 \
 && cmake -Wno-dev \
  -DCMAKE_TOOLCHAIN_FILE=/sysroot/static-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr \
  -GNinja \
  -B_build \
  -H. \
 && cmake --build _build \
 && cmake --install _build \
 && rm -rf /work/build

# Installed to /usr so autoconf can find them.
RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/util/util-macros-1.20.2.tar.gz) \
 && cd util-macros-1.20.2 \
 && ./configure --prefix="/usr" \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/proto/xorgproto-2024.1.tar.gz) \
 && cd xorgproto-2024.1 \
 && ./configure --prefix="$SYSROOT/usr" \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl https://xcb.freedesktop.org/dist/xcb-proto-1.17.0.tar.gz) \
 && cd xcb-proto-1.17.0 \
 && ./configure --prefix="$SYSROOT/usr" \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/lib/libXau-1.0.11.tar.gz) \
 && cd libXau-1.0.11 \
 && ./configure --prefix="$SYSROOT/usr" \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl https://xcb.freedesktop.org/dist/libxcb-1.17.0.tar.gz) \
 && cd libxcb-1.17.0 \
 && ./configure --prefix="$SYSROOT/usr" --enable-xkb --enable-glx --disable-shared \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

# RUN tar zxf <(curl https://xcb.freedesktop.org/dist/libpthread-stubs-0.5.tar.gz) \
#  && cd libpthread-stubs-0.5 \
#  && ./configure --prefix="$SYSROOT/usr" \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build

# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/lib/xtrans-1.5.2.tar.gz) \
#  && cd xtrans-1.5.2 \
#  && ./configure --prefix="$SYSROOT/usr" \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/lib/libX11-1.8.tar.gz) \
#  && cd libX11-1.8 \
#  && ./configure --prefix="$SYSROOT/usr" --with-sysroot="$SYSROOT" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl -L https://github.com/libffi/libffi/releases/download/v3.4.6/libffi-3.4.6.tar.gz) \
#  && cd libffi-3.4.6 \
#  && ./configure --prefix="$SYSROOT/usr" --disable-shared --enable-portable-binary --enable-pax_emutramp --disable-exec-static-tramp \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl -L https://github.com/libexpat/libexpat/releases/download/R_2_6_4/expat-2.6.4.tar.gz) \
#  && cd expat-2.6.4 \
#  && cmake \
#   -DCMAKE_TOOLCHAIN_FILE=/sysroot/static-toolchain.cmake \
#   -DCMAKE_INSTALL_PREFIX="$SYSROOT/usr" \
#   -DCMAKE_BUILD_TYPE=Release \
#   -DBUILD_SHARED_LIBS=OFF \
#   -GNinja \
#   -B_build \
#   -H. \
#  && cmake --build _build \
#  && cmake --install _build \
#  && rm -rf /work/build
# 
# RUN ["apk", "add", "python3-dev"]
# RUN tar zxf <(curl -L https://github.com/GNOME/libxml2/archive/refs/tags/v2.13.5.tar.gz) \
#  && cd libxml2-2.13.5 \
#  && autoreconf -fi \
#  && ./configure --prefix="$SYSROOT/usr" --with-sysroot="$SYSROOT" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN git clone --depth=1 https://gitlab.freedesktop.org/wayland/wayland.git \
#  && cd wayland \
#  && meson setup build/ -Ddefault_library=static -Ddocumentation=false -Dtests=false --prefix="$SYSROOT/usr" \
#  && ninja -C build/ install \
#  && rm -rf /work/build
# 
# RUN git clone --depth=1 https://gitlab.freedesktop.org/wayland/wayland-protocols.git \
#  && cd wayland-protocols \
#  && meson setup build/ -Ddefault_library=static -Dtests=false --prefix="$SYSROOT/usr" \
#  && ninja -C build/ install \
#  && rm -rf /work/build
# 
# RUN ["apk", "add", "bison"]
# RUN tar Jxf <(curl -L https://xkbcommon.org/download/libxkbcommon-1.7.0.tar.xz) \
#  && cd libxkbcommon-1.7.0 \
#  && LDFLAGS="-lXau -L$SYSROOT/usr/lib" meson setup build/ -Ddefault_library=static --prefix="$SYSROOT/usr" -Denable-tools=false \
#  && ninja -C build/ install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/proto/randrproto-1.5.0.tar.gz) \
#  && cd randrproto-1.5.0 \
#  && ./configure --prefix="$SYSROOT/usr" \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/proto/renderproto-0.11.tar.gz) \
#  && cd renderproto-0.11 \
#  && ./configure --prefix="$SYSROOT/usr" \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/lib/libXrender-0.9.11.tar.gz) \
#  && cd libXrender-0.9.11 \
#  && ./configure --prefix="$SYSROOT/usr" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/lib/libXext-1.3.6.tar.gz) \
#  && cd libXext-1.3.6 \
#  && ./configure --prefix="$SYSROOT/usr" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/lib/libXrandr-1.5.4.tar.gz) \
#  && cd libXrandr-1.5.4 \
#  && ./configure --prefix="$SYSROOT/usr" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/proto/xineramaproto-1.2.1.tar.gz) \
#  && cd xineramaproto-1.2.1 \
#  && autoreconf -fi \
#  && ./configure --prefix="$SYSROOT/usr" \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/lib/libXinerama-1.1.5.tar.gz) \
#  && cd libXinerama-1.1.5 \
#  && ./configure --prefix="$SYSROOT/usr" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/lib/libXfixes-6.0.1.tar.gz) \
#  && cd libXfixes-6.0.1 \
#  && ./configure --prefix="$SYSROOT/usr" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/lib/libXcursor-1.2.3.tar.gz) \
#  && cd libXcursor-1.2.3 \
#  && ./configure --prefix="$SYSROOT/usr" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl https://xorg.freedesktop.org/archive/individual/lib/libXi-1.8.2.tar.gz) \
#  && cd libXi-1.8.2 \
#  && ./configure --prefix="$SYSROOT/usr" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build

RUN tar zxf <(curl -L https://github.com/openssl/openssl/releases/download/openssl-3.4.0/openssl-3.4.0.tar.gz) \
 && cd openssl-3.4.0 \
 && ./Configure --prefix="$SYSROOT/usr" -no-shared -no-tests -fPIC \
 && make -j"$(nproc)" \
 && make install_sw \
 && rm -rf /work/build

RUN tar zxf <(curl -L https://github.com/unicode-org/icu/releases/download/release-72-1/icu4c-72_1-src.tgz) \
 && cd icu/source \
 && ./runConfigureICU Linux --enable-static --disable-shared --prefix="$SYSROOT/usr" \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl -L https://ftp.gnu.org/pub/gnu/gettext/gettext-0.22.5.tar.gz) \
 gettext-0.22.5/build-aux \
 gettext-0.22.5/gettext-runtime \
 gettext-0.22.5/m4 \
 gettext-0.22.5/.version \
 && cd gettext-0.22.5/gettext-runtime \
 && ./configure --prefix="$SYSROOT/usr" --disable-shared \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar Jxf <(curl -L https://download.gnome.org/sources/glib/2.83/glib-2.83.0.tar.xz) \
 && cd glib-2.83.0 \
 && meson setup build/ -Ddefault_library=static --prefix="$SYSROOT/usr" -Dtests=false -Dglib_debug=disabled \
 && ninja -C build/ install \
 && rm -rf /work/build

RUN tar Jxf <(curl -L https://dbus.freedesktop.org/releases/dbus/dbus-1.15.12.tar.xz) \
 && cd dbus-1.15.12 \
 && LDFLAGS="$LDFLAGS -lxcb" meson setup build/ -Ddefault_library=static --prefix="$SYSROOT/usr" -Dtools=false \
 && ninja -C build/ install \
 && rm -rf /work/build

RUN tar Jxf <(curl -L https://www.ffmpeg.org/releases/ffmpeg-7.1.tar.xz) \
 && cd ffmpeg-7.1 \
 && ./configure \
 --prefix="$SYSROOT/usr" \
 --disable-shared \
 --disable-debug \
 --disable-programs \
 --disable-protocols \
 --disable-doc \
 --disable-sdl2 \
 --disable-avfilter \
 --disable-filters \
 --disable-iconv \
 --disable-network \
 --disable-muxers \
 --disable-postproc \
 --disable-swresample \
 --disable-swscale-alpha \
 --disable-dwt \
 --disable-lsp \
 --disable-faan \
 --disable-vaapi \
 --disable-vdpau \
 --disable-zlib \
 --disable-xlib \
 --disable-bzlib \
 --disable-lzma \
 --disable-encoders \
 --disable-decoders \
 --disable-demuxers \
 --disable-parsers \
 --disable-bsfs \
 --enable-demuxer=h264 \
 --enable-demuxer=mjpeg \
 --enable-parser=h264 \
 --enable-parser=mjpeg \
 --enable-decoder=h264 \
 --enable-decoder=mjpeg \
 --enable-decoder=rawvideo \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar jxf <(curl -L https://github.com/libexif/libexif/releases/download/v0.6.24/libexif-0.6.24.tar.bz2) \
 && cd libexif-0.6.24 \
 && ./configure --prefix="$SYSROOT/usr" --disable-shared \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl -L https://github.com/fukuchi/libqrencode/archive/refs/tags/v4.1.1.tar.gz) \
 && cd libqrencode-4.1.1 \
 && autoreconf -fi \
 && ./configure --prefix="$SYSROOT/usr" --disable-shared --without-png \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl -L https://github.com/jedisct1/libsodium/releases/download/1.0.20-RELEASE/libsodium-1.0.20.tar.gz) \
 && cd libsodium-1.0.20 \
 && ./configure --prefix="$SYSROOT/usr" --disable-shared \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl -L https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz) \
 && cd zlib-1.3.1 \
 && ./configure --prefix="$SYSROOT/usr" \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl -L https://github.com/sqlcipher/sqlcipher/archive/refs/tags/v4.6.1.tar.gz) \
 && cd sqlcipher-4.6.1 \
 && ./configure --prefix="$SYSROOT/usr" --disable-shared \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl -L https://github.com/xiph/opus/releases/download/v1.5.2/opus-1.5.2.tar.gz) \
 && cd opus-1.5.2 \
 && ./configure --prefix="$SYSROOT/usr" --disable-shared \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

RUN tar zxf <(curl -L https://github.com/webmproject/libvpx/archive/refs/tags/v1.15.0.tar.gz) \
 && cd libvpx-1.15.0 \
 && ./configure --prefix="$SYSROOT/usr" --disable-examples --disable-unit-tests --enable-pic \
 && make -j"$(nproc)" \
 && make install \
 && rm -rf /work/build

# RUN tar zxf <(curl -L https://github.com/KhronosGroup/SPIRV-Headers/archive/refs/tags/vulkan-sdk-1.3.290.0.tar.gz) \
#  && cd SPIRV-Headers-vulkan-sdk-1.3.290.0 \
#  && cmake \
#   -DCMAKE_TOOLCHAIN_FILE=/sysroot/static-toolchain.cmake \
#   -DCMAKE_INSTALL_PREFIX="$SYSROOT/usr" \
#   -DCMAKE_BUILD_TYPE=Release \
#   -DBUILD_SHARED_LIBS=OFF \
#   -GNinja \
#   -B_build \
#   -H. \
#  && cmake --build _build \
#  && cmake --install _build \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl -L https://github.com/KhronosGroup/SPIRV-Tools/archive/refs/tags/vulkan-sdk-1.3.290.0.tar.gz) \
#  && cd SPIRV-Tools-vulkan-sdk-1.3.290.0 \
#  && cmake \
#   -DCMAKE_TOOLCHAIN_FILE=/sysroot/static-toolchain.cmake \
#   -DCMAKE_INSTALL_PREFIX="$SYSROOT/usr" \
#   -DCMAKE_BUILD_TYPE=Release \
#   -DBUILD_SHARED_LIBS=OFF \
#   -DSPIRV-Headers_SOURCE_DIR="$SYSROOT/usr" \
#   -GNinja \
#   -B_build \
#   -H. \
#  && cmake --build _build \
#  && cmake --install _build \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl -L https://github.com/KhronosGroup/glslang/archive/refs/tags/vulkan-sdk-1.3.296.0.tar.gz) \
#  && cd glslang-vulkan-sdk-1.3.296.0 \
#  && cmake \
#   -DCMAKE_TOOLCHAIN_FILE=/sysroot/static-toolchain.cmake \
#   -DCMAKE_INSTALL_PREFIX="$SYSROOT/usr" \
#   -DCMAKE_BUILD_TYPE=Release \
#   -DBUILD_SHARED_LIBS=OFF \
#   -DALLOW_EXTERNAL_SPIRV_TOOLS=ON \
#   -GNinja \
#   -B_build \
#   -H. \
#  && cmake --build _build \
#  && cmake --install _build \
#  && rm -rf /work/build
# 
# #RUN tar zxf <(curl -L https://github.com/NVIDIA/libglvnd/archive/refs/tags/v1.7.0.tar.gz) \
# # && cd libglvnd-1.7.0 \
# # && sed -i -e "s/subdir('tests')/#&/" meson.build \
# # && LDFLAGS="-lxcb -lXau -L$SYSROOT/usr/lib" meson setup build/ -Ddefault_library=static --prefix="$SYSROOT/usr" \
# # && ninja -C build/ install \
# # && rm -rf /work/build
# 
# RUN tar Jxf <(curl https://xorg.freedesktop.org/archive/individual/lib/libpciaccess-0.18.1.tar.xz) \
#  && cd libpciaccess-0.18.1 \
#  && meson setup _build --prefix="$SYSROOT/usr" --default-library=static \
#  && ninja -C _build install \
#  && rm -rf /work/build
# 
# RUN tar Jxf <(curl -L https://dri.freedesktop.org/libdrm/libdrm-2.4.123.tar.xz) \
#  && cd libdrm-2.4.123 \
#  && meson setup _build --prefix="$SYSROOT/usr" --default-library=static -Dintel=enabled -Dtests=false \
#  && ninja -C _build install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl -L https://xorg.freedesktop.org/archive/individual/lib/libxshmfence-1.3.2.tar.gz) \
#  && cd libxshmfence-1.3.2 \
#  && ./configure --prefix="$SYSROOT/usr" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN tar zxf <(curl -L https://xorg.freedesktop.org/archive/individual/lib/libXxf86vm-1.1.5.tar.gz) \
#  && cd libXxf86vm-1.1.5 \
#  && ./configure --prefix="$SYSROOT/usr" --disable-shared \
#  && make -j"$(nproc)" \
#  && make install \
#  && rm -rf /work/build
# 
# RUN apk add libclc-dev llvm-dev py3-mako py3-packaging py3-yaml spirv-llvm-translator-dev elfutils-dev flex
# RUN apk add cbindgen py3-ply rust rust-bindgen clang-dev
# ENV PATH=$SYSROOT/usr/bin:$PATH
# RUN tar Jxf <(curl -L https://mesa.freedesktop.org/archive/mesa-24.2.7.tar.xz) \
#  && cd mesa-24.2.7 \
#  && LDFLAGS="-lXau -L$SYSROOT/usr/lib" meson setup _build/ -Ddefault_library=static --prefix="$SYSROOT/usr" || (cat _build/meson-logs/meson-log.txt && false) \
#  && ninja -C _build install \
#  && rm -rf /work/build

# XXX: Hack to get Qt closer to being able to build with DBus support.
RUN sed -i -e 's/ -pthread//g' /sysroot/usr/lib/pkgconfig/dbus-1.pc

RUN apk add qt6-qtbase-dev qt6-qttools-dev

RUN tar Jxf <(curl -L https://download.qt.io/archive/qt/6.8/6.8.0/single/qt-everywhere-src-6.8.0.tar.xz)
RUN cd qt-everywhere-src-6.8.0 \
 && ./configure \
    -prefix "$SYSROOT/opt/qt" \
    -qt-freetype \
    -qt-harfbuzz \
    -qt-libjpeg \
    -qt-libpng \
    -qt-pcre \
    -qt-zlib \
    -static \
    -no-dbus \
    -no-glib \
    -openssl-linked \
    -nomake examples \
    -nomake tests \
    -submodules qtbase,qtsvg,qttools \
    -skip qtactiveqt \
    -skip qtdeclarative \
    -skip qtlanguageserver \
    -skip qtshadertools \
    -no-feature-assistant \
    -no-feature-brotli \
    -no-feature-designer \
    -no-feature-printsupport \
    -no-feature-qdoc \
    -no-feature-qtplugininfo \
    -no-feature-sql \
    -no-feature-testlib \
    -opengl desktop \
    -- \
    -DCMAKE_TOOLCHAIN_FILE=/sysroot/static-toolchain.cmake \
    -Wno-dev \
 && cmake --build . \
 && cmake --install . \
 && cd - \
 && rm -rf /work/build

RUN tar zxf <(curl -L https://github.com/TokTok/c-toxcore/releases/download/v0.2.20/c-toxcore-0.2.20.tar.gz) \
 && cd c-toxcore-0.2.20 \
 && cmake \
  -DCMAKE_TOOLCHAIN_FILE=/sysroot/static-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr \
  -DENABLE_SHARED=OFF \
  -GNinja \
  -B_build \
  -H. \
 && cmake --build _build \
 && cmake --install _build \
 && rm -rf /work/build

WORKDIR /qtox
