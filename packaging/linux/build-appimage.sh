#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="$ROOT/build-linux"
APPDIR="$ROOT/AppDir"
DIST="$ROOT/dist"
rm -rf "$BUILD" "$APPDIR" "$DIST"
mkdir -p "$DIST"
cmake -S "$ROOT" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "$BUILD" -j"$(nproc)"
DESTDIR="$APPDIR" cmake --install "$BUILD"
LINUXDEPLOY="$ROOT/.tools/linuxdeploy-x86_64.AppImage"
QT_PLUGIN="$ROOT/.tools/linuxdeploy-plugin-qt-x86_64.AppImage"
mkdir -p "$ROOT/.tools"
[ -x "$LINUXDEPLOY" ] || { wget -qO "$LINUXDEPLOY" https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage; chmod +x "$LINUXDEPLOY"; }
[ -x "$QT_PLUGIN" ] || { wget -qO "$QT_PLUGIN" https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage; chmod +x "$QT_PLUGIN"; }
export QMAKE="${QMAKE:-$(command -v qmake6 || command -v qmake)}"
export LDAI_OUTPUT="北京理工大学点对点通信-${VERSION:-1.1.0}-x86_64.AppImage"
cd "$DIST"
"$LINUXDEPLOY" --appdir "$APPDIR" --desktop-file "$APPDIR/usr/share/applications/cn.edu.bit.p2pcomm.desktop" --icon-file "$APPDIR/usr/share/icons/hicolor/scalable/apps/cn.edu.bit.p2pcomm.svg" --plugin qt --output appimage
