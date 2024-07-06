# Get abseil 20240116.2
CPMAddPackage(
  NAME abseil
  URL  "https://github.com/abseil/abseil-cpp/archive/refs/tags/20240116.2.tar.gz"
  VERSION 20240116.2
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE   
  OPTIONS "ABSL_ENABLE_INSTALL ON"
) 

# Get protobuf 27.1
CPMAddPackage(
  NAME protobuf
  URL  "https://github.com/protocolbuffers/protobuf/archive/refs/tags/v27.1.tar.gz"
  VERSION 27.1
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  OPTIONS        "protobuf_BUILD_TESTS OFF" "protobuf_INSTALL OFF" "protobuf_BUILD_LIBPROTOC ON" 
  "protobuf_BUILD_PROTOC_BINARIES OFF" "protobuf_WITH_ZLIB ON" "protobuf_MSVC_STATIC_RUNTIME OFF"
) 