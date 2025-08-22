# 🚀 SmartFileCmd Quick Start

Get your AI-powered file manager running in 5 minutes!

## ⚡ Quick Install

```bash
# Clone the repository
git clone <repository-url>
cd smartfilecli

# Run the installer
./install.sh

# Set your Gemini API key
export GEMINI_API_KEY="your-api-key-here"

# Test it works
smartfilecli "create a test folder in Downloads" --dry-run
```

## 🎯 Your First Commands

```bash
# Clean up Downloads (safe preview)
smartfilecli "delete all .zip files in Downloads" --dry-run

# Organize documents
smartfilecli "move all .docx files from Downloads to Documents"

# Create project folders
smartfilecli "create a new folder called Archive in Documents"
```

## 🔑 Get Your API Key

1. Go to [Google AI Studio](https://makersuite.google.com/app/apikey)
2. Create a new API key
3. Set it: `export GEMINI_API_KEY="your-key-here"`
4. Add to your shell profile for persistence

## 🛡️ Safety First

- **Always use `--dry-run` first** to see what would happen
- **The system asks for confirmation** before deleting files
- **Subdirectory files are protected** by default
- **System directories are blocked** for safety

## 🚨 Troubleshooting

**"Command not found"**
```bash
# Run from project directory
./smartfilecli "your command"

# Or add to PATH
export PATH="$PWD:$PATH"
```

**"API key not set"**
```bash
export GEMINI_API_KEY="your-key-here"
```

**"Backend not found"**
```bash
# Rebuild the backend
mkdir build && cd build
cmake .. && make
cd ..
```

## 💡 Pro Tips

- Use natural language: "remove all temporary files from Downloads"
- Be specific: "move all .jpg images to Pictures folder"
- Test first: Always use `--dry-run` for new operations
- Check results: The system shows exactly what happened

## 🎉 You're Ready!

Your AI-powered file manager is now working! Try some commands and see the magic happen.

**Need help?** Run `smartfilecli --help` or check the full README.md 