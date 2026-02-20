#!/bin/bash
# Download required fonts for ComfyX
# These fonts are under SIL Open Font License

FONT_DIR="$(dirname "$0")/../assets/fonts"
mkdir -p "$FONT_DIR"

echo "Downloading NotoSans..."
curl -L -o "$FONT_DIR/NotoSans-Regular.ttf" \
  "https://github.com/google/fonts/raw/main/ofl/notosans/NotoSans%5Bwdth%2Cwght%5D.ttf" 2>/dev/null \
  || curl -L -o "$FONT_DIR/NotoSans-Regular.ttf" \
  "https://cdn.jsdelivr.net/gh/google/fonts/ofl/notosans/NotoSans-Regular.ttf" 2>/dev/null

echo "Downloading NotoSansThai..."
curl -L -o "$FONT_DIR/NotoSansThai-Regular.ttf" \
  "https://github.com/google/fonts/raw/main/ofl/notosansthai/NotoSansThai%5Bwdth%2Cwght%5D.ttf" 2>/dev/null \
  || curl -L -o "$FONT_DIR/NotoSansThai-Regular.ttf" \
  "https://cdn.jsdelivr.net/gh/google/fonts/ofl/notosansthai/NotoSansThai-Regular.ttf" 2>/dev/null

echo "Downloading JetBrainsMono..."
curl -L -o /tmp/JetBrainsMono.zip \
  "https://github.com/JetBrains/JetBrainsMono/releases/download/v2.304/JetBrainsMono-2.304.zip" 2>/dev/null
unzip -o /tmp/JetBrainsMono.zip "fonts/ttf/JetBrainsMono-Regular.ttf" -d /tmp/jbm 2>/dev/null
cp /tmp/jbm/fonts/ttf/JetBrainsMono-Regular.ttf "$FONT_DIR/" 2>/dev/null
rm -rf /tmp/JetBrainsMono.zip /tmp/jbm

echo "Fonts downloaded to $FONT_DIR"
ls -la "$FONT_DIR"
