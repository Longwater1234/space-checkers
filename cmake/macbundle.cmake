# icon
set(MACOSX_BUNDLE_ICON_FILE "${CMAKE_PROJECT_NAME}.icns")
set(application_icon "${CMAKE_SOURCE_DIR}/resources/${MACOSX_BUNDLE_ICON_FILE}")
set_source_files_properties(${application_icon}
                            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

# images
file(GLOB_RECURSE my_images "${CMAKE_SOURCE_DIR}/resources/*")
foreach(FILE ${my_images}) 
  get_filename_component(FILENAME ${FILE} DIRECTORY)
  # SKIP .DS_Store files
  if (NOT FILENAME STREQUAL ".DS_Store")   
    file(RELATIVE_PATH NEW_FILE "${CMAKE_SOURCE_DIR}/" ${FILE})
    get_filename_component(NEW_FILE_PATH ${NEW_FILE} DIRECTORY)
    set_source_files_properties(${FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${NEW_FILE_PATH}")
  endif()
endforeach()

add_executable(${CMAKE_PROJECT_NAME} MACOSX_BUNDLE
               ${GAME_SRC} "${CMAKE_SOURCE_DIR}/src/utils/ResourcePath.mm" 
               ${application_icon} "${my_images}")

set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES BUNDLE TRUE
             XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
             XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
             XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks"
             MACOSX_BUNDLE_BUNDLE_NAME "${CMAKE_PROJECT_NAME}"
             MACOSX_BUNDLE_GUI_IDENTIFIER "com.davistiba.${CMAKE_PROJECT_NAME}"
             MACOSX_BUNDLE_COPYRIGHT "(c) 2024, Davis Tibbz"
             MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
             MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
             RESOURCE "${my_images}")