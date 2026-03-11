# GET Google Test v1.14.0
CPMAddPackage(
  NAME googletest
  URL "https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  OPTIONS "INSTALL_GTEST OFF" "BUILD_GMOCK OFF"
)
