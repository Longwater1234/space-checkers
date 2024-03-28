# GET latest fmt v10.2.1, for spdlog use ONLY
CPMAddPackage(
  NAME fmt
  URL "https://github.com/fmtlib/fmt/archive/refs/tags/10.2.1.tar.gz"
  VERSION 10.2.1
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# GET spdlog v1.13, but use separate (latest) fmtlib
CPMAddPackage(
  NAME spdlog
  URL "https://github.com/gabime/spdlog/archive/refs/tags/v1.13.0.tar.gz"
  VERSION 1.13.0
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  OPTIONS "SPDLOG_FMT_EXTERNAL ON"
) 
