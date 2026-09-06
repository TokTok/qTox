FROM ghcr.io/toktok/qtox:wasm-builder AS builder

COPY . .
RUN platform/wasm/build.sh

FROM alpine:3.21 AS venv

RUN apk add curl \
  gcc \
  linux-headers \
  musl-dev \
  py3-pip \
  py3-virtualenv \
  python3-dev

# Create a virtual environment for qtwasmserver.
# hadolint ignore=DL3013
RUN python3 -m venv /opt/buildhome/venv \
 && . /opt/buildhome/venv/bin/activate \
 && CFLAGS=-Wno-int-conversion pip3 install brotli httpcompressionserver netifaces \
 && curl -L https://raw.githubusercontent.com/qt/qtbase/refs/heads/dev/util/wasm/qtwasmserver/qtwasmserver.py -o /opt/buildhome/venv/bin/qtwasmserver.py \
 && chmod +x /opt/buildhome/venv/bin/qtwasmserver.py

FROM alpine:3.21

RUN apk add python3

COPY --from=venv /opt/buildhome/venv /opt/buildhome/venv
COPY --from=builder /qtox/_site /_site

ENV PORT=8000
WORKDIR /_site
CMD . /opt/buildhome/venv/bin/activate \
 && qtwasmserver.py \
 --cross-origin-isolation \
 --all-interfaces \
 --compress-never \
 --port "$PORT"
