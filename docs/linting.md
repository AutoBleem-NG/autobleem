# Code Linting with clang-tidy

AutoBleem uses **clang-tidy** to catch bugs and enforce modern C++ standards.

## Quick Start

**Install:**
```bash
# Ubuntu/Debian
sudo apt-get install clang-tidy

# macOS
brew install llvm
```

**Run:**
```bash
make lint
```

That's it! The linter will analyze all source files and report issues.

## Usage

### Lint Entire Codebase
```bash
make lint
```

### Lint Single File
```bash
clang-tidy -p build_sys src/code/engine/cdreader.h
```

### Lint During Build
```bash
cd build_sys
cmake -DENABLE_CLANG_TIDY=ON ..
make
```

### Auto-Fix Issues
```bash
clang-tidy -p build_sys src/code/engine/cdreader.h --fix
```
⚠️ Always review changes before committing!

## Common Fixes

### NULL → nullptr
```cpp
❌ char* ptr = NULL;
✅ char* ptr = nullptr;
```

### C-style casts → C++ casts
```cpp
❌ int x = (int)value;
✅ int x = static_cast<int>(value);
```

### Initialize pointers
```cpp
❌ class Foo {
    int* data;
};

✅ class Foo {
    int* data = nullptr;
};
```

### Use override keyword
```cpp
❌ void closeImage() { }  // In derived class
✅ void closeImage() override { }
```

## CLion Integration

**Settings → Tools → clang-tidy**
1. Enable "Use clang-tidy"
2. Check "Use .clang-tidy config file"

Warnings now appear inline as you code!

## Suppressing Warnings

For intentional code that triggers warnings:
```cpp
// NOLINTNEXTLINE(check-name)
specialCode();  // Document why this is needed
```

Use sparingly!

## Configuration

AutoBleem uses **Chromium's configuration** (37 battle-tested checks).

See detailed info:
- [CLANG_TIDY_CONFIG.md](CLANG_TIDY_CONFIG.md) - What's enabled and why
- [clang-tidy-references.md](clang-tidy-references.md) - Compare with other configs

## Resources

- [clang-tidy Official Docs](https://clang.llvm.org/extra/clang-tidy/)
- [Complete Check List](https://clang.llvm.org/extra/clang-tidy/checks/list.html)
- [Chromium's Config](https://github.com/chromium/chromium/blob/main/.clang-tidy)
