#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build/Release"
APPDIR="$SCRIPT_DIR/build/AppDir"
TOOLS_DIR="$SCRIPT_DIR/build/appimage-tools"
OUTPUT_DIR="$SCRIPT_DIR/dist"

LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT="$TOOLS_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"
APPIMAGETOOL="$TOOLS_DIR/appimagetool-x86_64.AppImage"

mkdir -p "$TOOLS_DIR" "$OUTPUT_DIR"

download_tool() {
    local url="$1"
    local dest="$2"
    if [[ ! -f "$dest" ]]; then
        echo "Downloading $(basename "$dest")..."
        curl -fsSL -o "$dest" "$url"
        chmod +x "$dest"
    fi
}

download_tool \
    "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" \
    "$LINUXDEPLOY"

download_tool \
    "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage" \
    "$LINUXDEPLOY_QT"

download_tool \
    "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage" \
    "$APPIMAGETOOL"

echo "Building vinyl in Release mode..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "$BUILD_DIR" --parallel

echo "Staging AppDir..."
rm -rf "$APPDIR"
DESTDIR="$APPDIR" cmake --install "$BUILD_DIR"

# Ensure standard FHS dirs exist regardless of what cmake --install created
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"

# Create .desktop file
cat > "$APPDIR/usr/share/applications/vinyl.desktop" <<'EOF'
[Desktop Entry]
Name=Vinyl
Comment=MPRIS music visualizer overlay
Exec=vinyl_app
Icon=vinyl
Type=Application
Categories=AudioVideo;Audio;Player;
EOF

# Regenerate icon if not present (requires Python3 + Pillow)
if [[ ! -f "$SCRIPT_DIR/assets/vinyl_icon.png" ]]; then
    python3 "$SCRIPT_DIR/assets/gen_icon.py"
fi

# Install icon at all standard sizes (linuxdeploy expects at least one)
cp "$SCRIPT_DIR/assets/vinyl_icon.png" \
    "$APPDIR/usr/share/icons/hicolor/256x256/apps/vinyl.png"

echo "Bundling Qt dependencies..."
QMAKE="$(which qmake6 || which qmake)" \
QMLDIR="$SCRIPT_DIR/src/qml" \
    "$LINUXDEPLOY" \
    --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/vinyl_app" \
    --desktop-file "$APPDIR/usr/share/applications/vinyl.desktop" \
    --icon-file "$SCRIPT_DIR/assets/vinyl_icon.png" \
    --plugin qt

echo "Bundling QML modules (linuxdeploy-plugin-qt skips Qt6 optional modules)..."
QML_SRC=/usr/lib/x86_64-linux-gnu/qt6/qml
QML_DEST="$APPDIR/usr/qml"
mkdir -p "$QML_DEST"

# Recursively copy entire QML module trees needed at runtime
for module in QtQuick QtQml; do
    cp -r "$QML_SRC/$module" "$QML_DEST/"
done

# Fix rpath on every QML plugin .so so it can find usr/lib relative to itself
find "$QML_DEST" -name "*.so" | while read -r so; do
    rel="${so#$QML_DEST/}"
    depth=$(echo "$rel" | tr -cd '/' | wc -c)
    # depth slashes = depth+1 dirs to traverse up to usr/, then into lib
    rpath=$(printf '../%.0s' $(seq 1 $((depth + 1))))lib
    patchelf --set-rpath "\$ORIGIN/$rpath" "$so" 2>/dev/null || true
done

# Copy Qt libs needed by those QML plugins (skip any already present)
QT_LIB_SRC=/usr/lib/x86_64-linux-gnu
for lib in \
    libQt6Quick.so.6 \
    libQt6QuickControls2.so.6 \
    libQt6QuickTemplates2.so.6 \
    libQt6QuickEffects.so.6 \
    libQt6QmlMeta.so.6 \
    libQt6QmlModels.so.6 \
    libQt6QmlWorkerScript.so.6 \
    libQt6OpenGL.so.6
do
    src="$QT_LIB_SRC/$lib"
    dest="$APPDIR/usr/lib/$lib"
    if [[ -f "$src" && ! -f "$dest" ]]; then
        cp "$src" "$dest"
        patchelf --set-rpath '$ORIGIN' "$dest"
    fi
done

echo "Packaging AppImage..."
VERSION=$(grep -m1 'project.*VERSION' "$SCRIPT_DIR/CMakeLists.txt" | grep -oP '[\d]+\.[\d]+\.[\d]+' || echo "1.0")
ARCH=x86_64 "$APPIMAGETOOL" "$APPDIR" "$OUTPUT_DIR/Vinyl-$VERSION-x86_64.AppImage"

echo ""
echo "Done: $OUTPUT_DIR/Vinyl-$VERSION-x86_64.AppImage"
