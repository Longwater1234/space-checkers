# space - checkers

[![CMake on Windows platform](https://github.com/Longwater1234/space-checkers/actions/workflows/cmake-single-platform.yml/badge.svg?branch=main)](https://github.com/Longwater1234/space-checkers/actions/workflows/cmake-single-platform.yml)
|
![GitHub License](https://img.shields.io/github/license/longwater1234/space-checkers)
|
[![Itch.io](https://img.shields.io/badge/Itch-%23FF0B34.svg?style=for-the-badge&logo=Itch.io&logoColor=white)](https://longwater1234.itch.io/spacecheckers)

Offline & Online Multiplayer Checkers game in C++ built with SFML 2.6, imGui, Protobuf and ixWebsockets. With very minimal dependencies
and a simple build process. All dependencies are auto-downloaded (as `.tar.gz`) and configured for you using [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake).

This game can connect securely to both Private and Public game servers. The backend server for Online Mode is written in Golang, and is [available on GitHub](https://github.com/Longwater1234/checkers-backend) which you can self-host! Download and Play the pre-built game from the itch.io link above.

### Main Libraries Used

- SFML 2.6.2
- imGui-SFML
- ixWebsockets
- spdlog
- mbedtls
- libcpr (curl for C++17)
- Google Protobuf v33 (Used entirely during gameplay)
- simdjson (Used once, for parsing list of public servers)

## Requirements for Building

- C++17 (or newer) compiler.
- [CMake 3.20+](https://cmake.org/download/) or newer (GUI recommended)
- Internet connection during initial build (all dependencies, including SFML 2.6, are auto-downloaded and built via [CPM.cmake](dependencies/CPM.cmake)).

### For Windows (10 or later)

- MS Visual Studio 2022 or newer (NOT vscode), with "**Desktop C++ Development**" bundle.

### For macOS (x64 & arm64)

- Xcode 14 or newer from AppStore (or Command Line Tools).
- After Xcode is installed, run this in your Terminal:

```bash
  sudo xcode-select --install
```

- After installing the CMake GUI, add its accompanying CLI to PATH as shown:

```bash
   sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install
```

### For Linux Desktop

- Latest display drivers and C++ development packages for X11/OpenGL.
- Open your terminal and install sfml: 

```bash
  sudo apt install libsfml-dev
```

## Build Instructions

Please see [BUILDING.md](BUILDING.md) for detailed instructions for each platform.

## Code Contributions

Pull requests are welcome! See GitHub Issues tab to help with new Features. Just kindly remember run `./lint.sh` script before you git push, and your header files should end with `.hpp`.

## License

[BSD v3](LICENSE) &copy; 2024, Davis T.
