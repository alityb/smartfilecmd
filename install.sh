#!/bin/bash
# SmartFileCmd Installation Script

set -e

echo "🚀 Installing SmartFileCmd..."

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Check if we're in the right directory
if [ ! -f "$SCRIPT_DIR/smartfilecli" ]; then
    echo "❌ Error: Please run this script from the SmartFileCmd project directory"
    exit 1
fi

# Build the C++ backend
echo "🔨 Building C++ backend..."
if [ ! -d "build" ]; then
    mkdir -p build
fi

cd build
if command -v cmake &> /dev/null; then
    cmake ..
    make
    echo "✅ C++ backend built successfully"
else
    echo "⚠️  CMake not found, building manually..."
    g++ -std=c++20 -O2 -o smartfilecmd \
        ../cpp_backend/*.cpp -I.. \
        -lstdc++fs 2>/dev/null || \
    g++ -std=c++20 -O2 -o smartfilecmd \
        ../cpp_backend/*.cpp -I..
    echo "✅ C++ backend built manually"
fi
cd ..

# Install Python dependencies
echo "🐍 Installing Python dependencies..."
pip install -r requirements.txt

# Create global symlink
echo "🔗 Creating global command..."
if [ -w /usr/local/bin ]; then
    sudo ln -sf "$SCRIPT_DIR/smartfilecli" /usr/local/bin/smartfilecli
    echo "✅ Installed to /usr/local/bin/smartfilecli"
elif [ -w ~/.local/bin ]; then
    mkdir -p ~/.local/bin
    ln -sf "$SCRIPT_DIR/smartfilecli" ~/.local/bin/smartfilecli
    echo "✅ Installed to ~/.local/bin/smartfilecli"
    echo "💡 Add ~/.local/bin to your PATH if not already there:"
    echo "   echo 'export PATH=\"~/.local/bin:\$PATH\"' >> ~/.bashrc"
    echo "   source ~/.bashrc"
else
    echo "⚠️  Could not create global symlink. You can run with:"
    echo "   ./smartfilecli \"your command\""
fi

# Set up environment
echo "🔑 Setting up environment..."
if [ ! -f ~/.smartfilecli ]; then
    cat > ~/.smartfilecli << EOF
# SmartFileCmd Environment Configuration
# Set your Gemini API key here:
# export GEMINI_API_KEY="your-api-key-here"

# Or add to your shell profile:
# echo 'export GEMINI_API_KEY="your-api-key-here"' >> ~/.bashrc
EOF
    echo "✅ Created ~/.smartfilecli configuration file"
fi

echo ""
echo "🎉 SmartFileCmd installed successfully!"
echo ""
echo "📖 Next steps:"
echo "1. Get a Gemini API key from: https://makersuite.google.com/app/apikey"
echo "2. Set your API key: export GEMINI_API_KEY=\"your-key-here\""
echo "3. Test the installation: smartfilecli \"create a test folder in Downloads\" --dry-run"
echo ""
echo "💡 Usage examples:"
echo "   smartfilecli \"delete all .png files in Downloads\" --dry-run"
echo "   smartfilecli \"move all .docx files to Documents\""
echo "   smartfilecli \"create a new folder called Projects in Documents\""
echo ""
echo "🔧 For help: smartfilecli --help" 