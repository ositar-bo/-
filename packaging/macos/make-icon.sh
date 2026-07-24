#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SVG="$ROOT/assets/icons/app.svg"
ICONSET="$ROOT/packaging/macos/AppIcon.iconset"
ICNS="$ROOT/packaging/macos/AppIcon.icns"

command -v rsvg-convert >/dev/null 2>&1 || {
  echo "错误：未找到 rsvg-convert，请先安装 librsvg。"
  exit 1
}

rm -rf "$ICONSET" "$ICNS"
mkdir -p "$ICONSET"

for size in 16 32 128 256 512; do
  rsvg-convert -w "$size" -h "$size" "$SVG" -o "$ICONSET/icon_${size}x${size}.png"
  double=$((size * 2))
  rsvg-convert -w "$double" -h "$double" "$SVG" -o "$ICONSET/icon_${size}x${size}@2x.png"
done

iconutil -c icns "$ICONSET" -o "$ICNS"
echo "已生成图标：$ICNS"
