# Get abseil 20250127
CPMAddPackage(
  NAME abseil
  URL  "https://github.com/abseil/abseil-cpp/archive/refs/tags/20250127.0.tar.gz"
  VERSION 220250127
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE   
  OPTIONS "ABSL_ENABLE_INSTALL ON" "ABSL_PROPAGATE_CXX_STD ON"
) 

# Get protobuf 30.1
CPMAddPackage(
  NAME protobuf
  URL  "https://github.com/protocolbuffers/protobuf/archive/refs/tags/v30.1.tar.gz" 
  VERSION 30.1
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  OPTIONS        "protobuf_BUILD_TESTS OFF" "protobuf_INSTALL OFF" "protobuf_BUILD_LIBPROTOC ON" 
  "protobuf_BUILD_PROTOC_BINARIES OFF" "protobuf_WITH_ZLIB ON" "protobuf_MSVC_STATIC_RUNTIME OFF"
) 