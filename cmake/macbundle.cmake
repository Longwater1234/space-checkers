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
             BUILD_WITH_INSTALL_RPATH TRUE
             INSTALL_RPATH "@executable_path/../Frameworks"
             BUILD_RPATH "@executable_path/../Frameworks"
             XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks"
             MACOSX_BUNDLE_BUNDLE_NAME "${CMAKE_PROJECT_NAME}"
             MACOSX_BUNDLE_GUI_IDENTIFIER "com.davistiba.${CMAKE_PROJECT_NAME}"
             MACOSX_BUNDLE_COPYRIGHT "(c) 2024, Davis Tibbz"
             MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
             MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
             RESOURCE "${assets}")

# Copy freetype.framework into the app bundle (unsigned)
if(sfml_SOURCE_DIR)
  set(FREETYPE_FW_SRC "${sfml_SOURCE_DIR}/extlibs/libs-osx/Frameworks/freetype.framework")
elseif(SFML_SOURCE_DIR)
  set(FREETYPE_FW_SRC "${SFML_SOURCE_DIR}/extlibs/libs-osx/Frameworks/freetype.framework")
elseif(CPM_PACKAGE_sfml_SOURCE_DIR)
  set(FREETYPE_FW_SRC "${CPM_PACKAGE_sfml_SOURCE_DIR}/extlibs/libs-osx/Frameworks/freetype.framework")
elseif(FREETYPE_LIBRARY AND EXISTS "${FREETYPE_LIBRARY}")
  set(FREETYPE_FW_SRC "${FREETYPE_LIBRARY}")
endif()

if(FREETYPE_FW_SRC AND EXISTS "${FREETYPE_FW_SRC}")
  add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/../Frameworks
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${FREETYPE_FW_SRC}"
            $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/../Frameworks/freetype.framework
    COMMENT "Copying freetype.framework to bundle Frameworks (unsigned)"
  )
endif()