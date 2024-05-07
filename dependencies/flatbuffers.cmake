# GET flatbuffers v24.3.25
CPMAddPackage(
  NAME flatbuffers
  URL "https://github.com/google/flatbuffers/archive/refs/tags/v24.3.25.tar.gz"
  VERSION v24.3.25
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  OPTIONS "FLATBUFFERS_BUILD_FLATC OFF"
) 