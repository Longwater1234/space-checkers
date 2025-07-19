# space - checkers

[![CMake on Windows platform](https://github.com/Longwater1234/space-checkers/actions/workflows/cmake-single-platform.yml/badge.svg?branch=main)](https://github.com/Longwater1234/space-checkers/actions/workflows/cmake-single-platform.yml)

Offline & Online Multiplayer Checkers game in C++ built with SFML 2.6, imGui, Protobuf and ixWebsockets. With very minimal dependencies
and a simple build process. All dependencies are auto-downloaded (as `.tar.gz`) and configured for you using [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake). The only dependency you need pre-installed on your OS is SFML 2.6.x (or newer).

This game can connect to both Private and Public game servers. The backend server for this game is written in Golang, and is [available on GitHub](https://github.com/Longwater1234/checkers-backend) which you can self-host! Download and Play the game from the itch.io link above.

### Main Libraries Used

- SFML 2.6.1
- imGui-SFML
- ixWebsockets
- spdlog
- Google Protobuf v30.1 (Used entirely during gameplay)
- simdjson (Used once, for parsing list of public servers)

## Requirements for Building

- C++17 (or newer) compiler.
- Pre-built [SFML 2.6.x](https://www.sfml-dev.org/download/sfml/2.6.1/) binaries.
- [CMake 3.20+](https://cmake.org/download/) or newer (GUI recommended)

### For Windows

- MS Visual Studio 2022 or newer (NOT vscode), with "**Desktop C++ Development**" bundle.
- Please download "Visual C++ 64bit" edition of SFML; ignore others.
- Move your unzipped `SFML-2.6.x` folder to its own home, example: `C:/SFML/SFML-2.6.1`.
- Edit **line 25** in [CMakeLists.txt](CMakeLists.txt#L25), to set value `SFML_HOME` to folder path you moved SFML into (from previous step)

### For macOS (x64 & arm64)

- XCode 14 or newer from AppStore (with MacOS SDK)
- Please install SFML 2.6 as **Frameworks**, not as "dylibs", as shown in [official SFML guide](https://www.sfml-dev.org/tutorials/2.6/start-osx.php).
- Download Apple Developer tools. After Xcode is installed, run this in your Terminal:

```bash
  sudo xcode-select --install
```

- After installing the CMake GUI, add its accompanying CLI to PATH, simply run the following command:

```bash
   sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install
```

### For Linux Desktop

- Latest display drivers
- Use your OS package manager (`apt` or `yum`) to install SFML 2.6 or newer.
- Alternatively, you may build SFML 2.6 from source, see [official SFML docs](https://www.sfml-dev.org/tutorials/2.6/start-linux.php).
- You are required to install latest **OpenSSL Dev** library. See example for Ubuntu / Debian below.

```bash
  sudo apt install libsfml-dev
  sudo apt install libssl-dev
```

- Luckily, Windows and macOS come with their **native SSL libs pre-installed**, so nothing more to do 😊

## Build Instructions

Please see [BUILDING.md](BUILDING.md) for detailed instructions for each platform.

## Code Contributions

Pull requests are welcome! See GitHub Issues tab to help with new Features. Just kindly remember run `./lint.sh` script before you git push. Also, for this project, header files should end with `.hpp`.

## License

[GPL v3](LICENSE) &copy; 2024, Davis T.
