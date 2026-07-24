#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="$ROOT/build-macos"
DIST="$ROOT/dist"
rm -rf "$BUILD" "$DIST"
mkdir -p "$DIST"
"$ROOT/packaging/macos/make-icon.sh"
cmake -S "$ROOT" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" -j"$(sysctl -n hw.ncpu)"
APP="$(find "$BUILD" -maxdepth 2 -name '*.app' -print -quit)"
[ -n "$APP" ] || { echo '未找到 .app'; exit 1; }
macdeployqt "$APP" -dmg -always-overwrite
cp "${APP%.app}.dmg" "$DIST/北京理工大学点对点通信-${VERSION:-1.1.0}-macOS.dmg"
