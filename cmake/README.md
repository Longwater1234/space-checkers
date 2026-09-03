## What's 'macbundle.cmake' for?

This contains the CMake configuration to build the macOS GUI `.app` bundle. It:

1. Embeds the application icon (`SpaceCheckers.icns`) into the app bundle.
2. Copies all static assets and game resources into `Contents/Resources/` at build time.
3. Automatically embeds `freetype.framework` (SFML's macOS dynamic dependency) into `Contents/Frameworks/` and configures the binary `@rpath`.
4. Automatically ad-hoc code signs the bundle and embedded frameworks (`codesign -s -`) so it runs cleanly on Apple Silicon and modern macOS.

No manual Xcode build phases or manual file copying are required!
