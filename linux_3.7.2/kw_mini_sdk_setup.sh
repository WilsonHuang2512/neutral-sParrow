#!/bin/bash
# =============================================================================
# KW-MINI SDK 3.7.1 - Linux GUI Setup Script
# Tested on Ubuntu 20.04
# =============================================================================
# USAGE:
#   1. Copy this script into the SDK's GUI folder:
#      cp kw_mini_sdk_setup.sh <SDK_PATH>/sdk_3.7.1_linux/GUI/
#   2. Run it once:
#      cd <SDK_PATH>/sdk_3.7.1_linux/GUI/
#      chmod +x kw_mini_sdk_setup.sh
#      ./kw_mini_sdk_setup.sh
#   3. Launch the GUI:
#      ./open_cam3d_gui.sh
# =============================================================================

set -e

GUI_DIR="$(cd "$(dirname "$0")" && pwd)"
SDK_DIR="$(dirname "$GUI_DIR")"

echo "============================================="
echo " KW-MINI SDK GUI Setup"
echo " GUI dir: $GUI_DIR"
echo "============================================="

# -----------------------------------------------------------------------------
# STEP 1 - Install required system packages
# -----------------------------------------------------------------------------
echo ""
echo "[1/4] Installing system dependencies..."
sudo apt-get update -q
sudo apt-get install -y \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-xinerama0 \
    libxcb-xkb1 \
    libxkbcommon-x11-0 \
    libxcb-shape0 \
    libqt5xml5

echo "  [OK] System packages installed."

# -----------------------------------------------------------------------------
# STEP 2 - Copy missing libraries into GUI folder
# -----------------------------------------------------------------------------
echo ""
echo "[2/4] Copying missing SDK libraries into GUI folder..."

# libenumerate.so is only in CPP/ and C/ — copy from CPP preferentially
if [ ! -f "$GUI_DIR/libenumerate.so" ]; then
    if [ -f "$SDK_DIR/CPP/libenumerate.so" ]; then
        cp "$SDK_DIR/CPP/libenumerate.so" "$GUI_DIR/"
        echo "  [OK] Copied libenumerate.so from CPP/"
    elif [ -f "$SDK_DIR/C/libenumerate.so" ]; then
        cp "$SDK_DIR/C/libenumerate.so" "$GUI_DIR/"
        echo "  [OK] Copied libenumerate.so from C/"
    else
        echo "  [ERROR] libenumerate.so not found in CPP/ or C/ — check your SDK package."
        exit 1
    fi
else
    echo "  [OK] libenumerate.so already present."
fi

# -----------------------------------------------------------------------------
# STEP 3 - Remove bundled Qt5 libs that conflict with system Qt5
# -----------------------------------------------------------------------------
echo ""
echo "[3/4] Disabling bundled Qt5 libs to avoid version mismatch..."

for lib in libQt5Core.so.5 libQt5Gui.so.5 libQt5Widgets.so.5 libQt5Xml.so.5; do
    if [ -f "$GUI_DIR/$lib" ]; then
        mv "$GUI_DIR/$lib" "$GUI_DIR/${lib}.bak"
        echo "  [OK] Moved $lib -> ${lib}.bak"
    fi
done

# Note: we keep libicudata/libicui18n/libicuuc .so.56 because the binary
# is hard-linked against ICU 56 and cannot use system ICU 66.
# They coexist fine since the sonames differ.

# -----------------------------------------------------------------------------
# STEP 4 - Patch the launch script to set QT_QPA_PLATFORM_PLUGIN_PATH
# -----------------------------------------------------------------------------
echo ""
echo "[4/4] Patching open_cam3d_gui.sh to set Qt platform plugin path..."

QT_PLUGIN_PATH=$(find /usr -name "libqxcb.so" 2>/dev/null | head -1 | xargs dirname)

if [ -z "$QT_PLUGIN_PATH" ]; then
    echo "  [ERROR] libqxcb.so not found. Try: sudo apt-get install libqt5gui5"
    exit 1
fi

echo "  Found Qt platform plugin at: $QT_PLUGIN_PATH"

# Only patch if not already patched
if grep -q "QT_QPA_PLATFORM_PLUGIN_PATH" "$GUI_DIR/open_cam3d_gui.sh"; then
    echo "  [OK] Script already patched, skipping."
else
    # Insert the export line before the final exec line
    sed -i "s|export LD_LIBRARY_PATH|export LD_LIBRARY_PATH\nexport QT_QPA_PLATFORM_PLUGIN_PATH=$QT_PLUGIN_PATH|" \
        "$GUI_DIR/open_cam3d_gui.sh"
    echo "  [OK] Patched open_cam3d_gui.sh"
fi

# -----------------------------------------------------------------------------
# Done
# -----------------------------------------------------------------------------
echo ""
echo "============================================="
echo " Setup complete! Run the GUI with:"
echo "   cd $GUI_DIR"
echo "   ./open_cam3d_gui.sh"
echo "============================================="
