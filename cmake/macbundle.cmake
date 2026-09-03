# icon
set(MACOSX_BUNDLE_ICON_FILE "${CMAKE_PROJECT_NAME}.icns")
set(application_icon "${CMAKE_SOURCE_DIR}/resources/${MACOSX_BUNDLE_ICON_FILE}")
set_source_files_properties(${application_icon}
                            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

# images and fonts
file(GLOB_RECURSE assets "${CMAKE_SOURCE_DIR}/resources/*")
foreach(FILE ${assets}) 
  get_filename_component(FILENAME ${FILE} NAME)
  # skip .DS_Store files
  if (NOT FILENAME STREQUAL ".DS_Store")   
    file(RELATIVE_PATH NEW_FILE "${CMAKE_SOURCE_DIR}/" ${FILE})
    get_filename_component(PARENT_DIR ${NEW_FILE} DIRECTORY) # parent dir
    set_source_files_properties(${PARENT_DIR} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${PARENT_DIR}")
  endif()
endforeach()

add_executable(${CMAKE_PROJECT_NAME} MACOSX_BUNDLE
               ${GAME_SRC} "${CMAKE_SOURCE_DIR}/src/utils/ResourcePath.mm" 
               ${application_icon} "${assets}") 

set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES BUNDLE TRUE
             XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
             XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
             LINK_SEARCH_START_STATIC TRUE
             LINK_SEARCH_END_STATIC   TRUE
             BUILD_RPATH "@executable_path/../Frameworks"
             INSTALL_RPATH "@executable_path/../Frameworks"
             XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks"
             MACOSX_BUNDLE_BUNDLE_NAME "${CMAKE_PROJECT_NAME}"
             MACOSX_BUNDLE_GUI_IDENTIFIER "com.davistiba.${CMAKE_PROJECT_NAME}"
             MACOSX_BUNDLE_COPYRIGHT "(c) 2024, Davis Tibbz"
             MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
             MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
             RESOURCE "${assets}")

# FreeType is SFML's only dynamic framework dependency on macOS
find_library(FREETYPE_FW freetype PATHS
  "${sfml_SOURCE_DIR}/extlibs/libs-osx/Frameworks"
  "${CMAKE_BINARY_DIR}/_deps/sfml-src/extlibs/libs-osx/Frameworks"
  /Library/Frameworks
  ~/Library/Frameworks
)

if(CMAKE_GENERATOR STREQUAL "Xcode")
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    XCODE_EMBED_FRAMEWORKS "${FREETYPE_FW}"
    XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
  )
else()
  find_program(DITTO_CMD ditto)
  find_program(CODESIGN_CMD codesign)

  add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_CONTENT_DIR:${CMAKE_PROJECT_NAME}>/Frameworks"
    COMMAND ${DITTO_CMD} "${FREETYPE_FW}" "$<TARGET_BUNDLE_CONTENT_DIR:${CMAKE_PROJECT_NAME}>/Frameworks/freetype.framework"
    COMMAND ${CODESIGN_CMD} --force --deep -s - "$<TARGET_BUNDLE_CONTENT_DIR:${CMAKE_PROJECT_NAME}>/Frameworks/freetype.framework"
    COMMAND ${CODESIGN_CMD} --force -s - "$<TARGET_BUNDLE_DIR:${CMAKE_PROJECT_NAME}>"
    COMMENT "Embedding freetype.framework and signing ${CMAKE_PROJECT_NAME}.app"
  )
endif()