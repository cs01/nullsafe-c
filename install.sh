#!/bin/bash

set -e

REPO="cs01/llvm-project"
INSTALL_DIR="${INSTALL_DIR:-$HOME/.local/null-safe-clang}"

if [ -z "$VERSION" ]; then
    echo "Fetching latest release version..."
    VERSION=$(curl -s https://api.github.com/repos/$REPO/releases | \
        jq -r '[.[] | select(.tag_name | startswith("v")) | select(.prerelease == false)] | first | .tag_name')

    if [ -z "$VERSION" ]; then
        echo "Failed to fetch latest version. Set VERSION manually:"
        echo "  VERSION=v0.1.6 $0"
        exit 1
    fi
    echo "Latest version: $VERSION"
fi

echo "Detecting platform..."
OS=$(uname -s)
ARCH=$(uname -m)

if [ "$OS" = "Linux" ] && [ "$ARCH" = "x86_64" ]; then
    RELEASE_FILE="clang-nullsafe-linux-x86_64.tar.gz"
    echo "Detected: Linux x86_64"
elif [ "$OS" = "Darwin" ]; then
    RELEASE_FILE="clang-nullsafe-macos-universal.tar.gz"
    echo "Detected: macOS (Universal)"

    if [ ! -f "/opt/homebrew/opt/zstd/lib/libzstd.1.dylib" ] && [ ! -f "/usr/local/opt/zstd/lib/libzstd.1.dylib" ]; then
        echo ""
        echo "Missing required dependency: zstd"
        echo ""
        echo "Please install it with Homebrew first:"
        echo "    brew install zstd"
        echo ""
        echo "Then run this installer again."
        exit 1
    fi
else
    echo "Unsupported platform: $OS $ARCH"
    echo ""
    echo "For WebAssembly/Playground, visit:"
    echo "    https://cs01.github.io/llvm-project/"
    echo ""
    echo "Or download manually from:"
    echo "    https://github.com/$REPO/releases/tag/$VERSION"
    exit 1
fi

URL="https://github.com/$REPO/releases/download/$VERSION/$RELEASE_FILE"

echo "Downloading Null-Safe Clang $VERSION..."
echo "   $URL"

mkdir -p "$INSTALL_DIR"
cd "$INSTALL_DIR"

if command -v curl >/dev/null 2>&1; then
    curl -L "$URL" | tar xz --strip-components=0
elif command -v wget >/dev/null 2>&1; then
    wget -O - "$URL" | tar xz --strip-components=0
else
    echo "Neither curl nor wget found. Please install one of them."
    exit 1
fi

if [ "$OS" = "Darwin" ]; then
    echo "Removing macOS quarantine attribute..."
    xattr -dr com.apple.quarantine "$INSTALL_DIR/bin" 2>/dev/null || true

    if ! command -v xcrun >/dev/null 2>&1; then
        echo ""
        echo "Xcode Command Line Tools not found"
        echo ""
        echo "Please install them first:"
        echo "    xcode-select --install"
        echo ""
        echo "Then run this installer again."
        exit 1
    fi

    SDK_PATH=$(xcrun --show-sdk-path 2>/dev/null || echo "")
    if [ -n "$SDK_PATH" ]; then
        echo "Configuring macOS SDK integration..."

        mv "$INSTALL_DIR/bin/clang" "$INSTALL_DIR/bin/clang.real"
        mv "$INSTALL_DIR/bin/clang++" "$INSTALL_DIR/bin/clang++.real" 2>/dev/null || true

        cat > "$INSTALL_DIR/bin/clang" << 'WRAPPER_EOF'
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_PATH="$(xcrun --show-sdk-path 2>/dev/null || echo "")"
if [ -n "$SDK_PATH" ]; then
    exec "$SCRIPT_DIR/clang.real" -isysroot "$SDK_PATH" "$@"
else
    exec "$SCRIPT_DIR/clang.real" "$@"
fi
WRAPPER_EOF

        cat > "$INSTALL_DIR/bin/clang++" << 'WRAPPER_EOF'
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_PATH="$(xcrun --show-sdk-path 2>/dev/null || echo "")"
if [ -n "$SDK_PATH" ]; then
    exec "$SCRIPT_DIR/clang++.real" -isysroot "$SDK_PATH" "$@"
else
    exec "$SCRIPT_DIR/clang++.real" "$@"
fi
WRAPPER_EOF

        chmod +x "$INSTALL_DIR/bin/clang" "$INSTALL_DIR/bin/clang++"
    fi
fi

echo ""
echo "Successfully installed to: $INSTALL_DIR"
echo ""
echo "To use Null-Safe Clang, add this to your shell profile (~/.bashrc, ~/.zshrc, etc.):"
echo ""
echo "    export PATH=\"$INSTALL_DIR/bin:\$PATH\""
echo ""
echo "Then reload your shell or run:"
echo ""
echo "    source ~/.zshrc  # or ~/.bashrc"
echo ""
echo "Verify installation:"
echo "    clang --version"
echo ""
echo "Try it out:"
echo "    echo 'void f(int* p) { *p = 42; }' | clang -x c - -fsyntax-only"
echo ""
echo "Try the interactive WebAssembly playground:"
echo "    https://cs01.github.io/llvm-project/"
