#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="$ROOT/build-macos"
DIST="$ROOT/dist"
STAGE="$ROOT/dmg-stage"
ARCH="${TARGET_ARCH:-$(uname -m)}"
VERSION="${VERSION:-1.1.0}"

rm -rf "$BUILD" "$DIST" "$STAGE"
mkdir -p "$DIST" "$STAGE"

bash "$ROOT/packaging/macos/make-icon.sh"

cmake -S "$ROOT" -B "$BUILD" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="$ARCH"

cmake --build "$BUILD" --parallel "$(sysctl -n hw.ncpu)"

APP="$(find "$BUILD" -type d -name '*.app' -print -quit)"
if [[ -z "$APP" ]]; then
  echo "错误：编译后未找到 .app。"
  find "$BUILD" -maxdepth 4 -print
  exit 1
fi

MACDEPLOYQT="$(command -v macdeployqt || true)"
if [[ -z "$MACDEPLOYQT" ]]; then
  echo "错误：未找到 macdeployqt。Qt bin: ${QT_ROOT_DIR:-unknown}"
  exit 1
fi

"$MACDEPLOYQT" "$APP" -always-overwrite -verbose=2

# 内部测试版采用 ad-hoc 签名，避免复制 Qt Framework 后签名不一致。
codesign --force --deep --sign - "$APP"

cp -R "$APP" "$STAGE/"
ln -s /Applications "$STAGE/Applications"

DMG="$DIST/北京理工大学点对点通信-${VERSION}-macOS-${ARCH}.dmg"
hdiutil create \
  -volname "北京理工大学点对点通信" \
  -srcfolder "$STAGE" \
  -ov \
  -format UDZO \
  "$DMG"

codesign --force --sign - "$DMG" || true

echo "打包完成：$DMG"
