#!/bin/bash
# Null-Safe Clang installer
# Auto-detects your platform and installs the latest release

set -e

REPO="cs01/llvm-project"
INSTALL_DIR="${INSTALL_DIR:-$HOME/.local/null-safe-clang}"

echo "🔍 Detecting platform..."
OS=$(uname -s)
ARCH=$(uname -m)

# Determine the correct release file
if [ "$OS" = "Linux" ] && [ "$ARCH" = "x86_64" ]; then
    RELEASE_FILE="clang-nullsafe-linux-x86_64.tar.gz"
    echo "✓ Detected: Linux x86_64"
elif [ "$OS" = "Darwin" ]; then
    RELEASE_FILE="clang-nullsafe-macos-universal.tar.gz"
    echo "✓ Detected: macOS (Universal)"
else
    echo "❌ Unsupported platform: $OS $ARCH"
    echo "Please download manually from:"
    echo "https://github.com/$REPO/releases/latest"
    exit 1
fi

# Download URL
URL="https://github.com/$REPO/releases/latest/download/$RELEASE_FILE"

echo "📦 Downloading Null-Safe Clang..."
echo "   $URL"

# Create install directory
mkdir -p "$INSTALL_DIR"
cd "$INSTALL_DIR"

# Download and extract
if command -v curl >/dev/null 2>&1; then
    curl -L "$URL" | tar xz --strip-components=0
elif command -v wget >/dev/null 2>&1; then
    wget -O - "$URL" | tar xz --strip-components=0
else
    echo "❌ Neither curl nor wget found. Please install one of them."
    exit 1
fi

echo "✓ Installed to: $INSTALL_DIR"
echo ""
echo "📝 To use Null-Safe Clang, add this to your shell profile (~/.bashrc, ~/.zshrc, etc.):"
echo ""
echo "    export PATH=\"$INSTALL_DIR/bin:\$PATH\""
echo ""
echo "Then reload your shell or run:"
echo ""
echo "    source ~/.bashrc  # or ~/.zshrc"
echo ""
echo "Verify installation:"
echo "    clang --version"
echo ""
echo "Try it out:"
echo "    echo 'void f(int* p) { *p = 42; }' | clang -x c - -fsyntax-only"
echo ""
echo "🎮 Try the interactive playground:"
echo "    https://cs01.github.io/llvm-project/"
