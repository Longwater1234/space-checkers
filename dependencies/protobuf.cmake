# Get abseil 20250127
CPMAddPackage(
  NAME abseil
  URL  "https://github.com/abseil/abseil-cpp/archive/refs/tags/20260107.1.tar.gz"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE   
  OPTIONS "ABSL_ENABLE_INSTALL ON" "ABSL_PROPAGATE_CXX_STD ON"
) 

# Get protobuf 30.1
CPMAddPackage(
  NAME protobuf
  URL  "https://github.com/protocolbuffers/protobuf/archive/refs/tags/v33.5.tar.gz" 
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  OPTIONS    "protobuf_BUILD_TESTS OFF" "protobuf_INSTALL OFF" "protobuf_BUILD_PROTOBUF_BINARIES ON"
   "protobuf_BUILD_LIBUPB OFF" "protobuf_BUILD_PROTOC_BINARIES OFF" "protobuf_MSVC_STATIC_RUNTIME OFF"
) 