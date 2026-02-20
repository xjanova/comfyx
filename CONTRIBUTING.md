# Contributing to ComfyX

Thank you for considering contributing to ComfyX!

## Development Setup

### Prerequisites
- CMake 3.21+
- C++17 compatible compiler (MSVC 2022, GCC 11+, Clang 13+)
- Git

### Build from Source

```bash
git clone --recursive https://github.com/xjanova/comfyx.git
cd comfyx
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Code Style
- Follow the `.clang-format` configuration
- Use `ComfyX` namespace for all classes
- Header/source pairs in appropriate module directories

### Pull Request Process
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Commit your changes
4. Push to your fork
5. Open a Pull Request

## License
By contributing, you agree that your contributions will be licensed under the MIT License.
