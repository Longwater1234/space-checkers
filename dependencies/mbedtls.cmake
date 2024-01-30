# mbedtls v2.25.0 (Apache-2.0 License)

CPMAddPackage(
    NAME mbedtls
    GIT_REPOSITORY https://gitee.com/zhanghe666/mbedtls.git
    VERSION 2.25.0
    GIT_SHALLOW
    DOWNLOAD_ONLY ON
)

if(mbedtls_ADDED)
    file(GLOB mbedtls_SOURCES ${mbedtls_SOURCE_DIR}/library/*.c)
    add_library(mbedtls STATIC ${mbedtls_SOURCES})
    target_include_directories(mbedtls PUBLIC ${mbedtls_SOURCE_DIR}/include)
endif()