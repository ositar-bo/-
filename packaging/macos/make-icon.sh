#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ICONSET="$ROOT/packaging/macos/AppIcon.iconset"
rm -rf "$ICONSET"; mkdir -p "$ICONSET"
for S in 16 32 128 256 512; do
  qlmanage -t -s "$S" -o /tmp "$ROOT/assets/icons/app.svg" >/dev/null 2>&1 || true
  SRC="/tmp/app.svg.png"
  sips -z "$S" "$S" "$SRC" --out "$ICONSET/icon_${S}x${S}.png" >/dev/null
  D=$((S*2)); sips -z "$D" "$D" "$SRC" --out "$ICONSET/icon_${S}x${S}@2x.png" >/dev/null
done
iconutil -c icns "$ICONSET" -o "$ROOT/packaging/macos/AppIcon.icns"
