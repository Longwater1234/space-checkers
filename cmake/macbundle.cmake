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

# Locate SFML and dependent frameworks to embed
set(FRAMEWORKS_TO_EMBED "")
foreach(FW sfml-graphics sfml-window sfml-system freetype)
  find_library(${FW}_PATH ${FW} PATHS /Library/Frameworks ~/Library/Frameworks)
  if(${FW}_PATH)
    list(APPEND FRAMEWORKS_TO_EMBED "${${FW}_PATH}")
  else()
    message(WARNING "Framework '${FW}' was not found; it may not be copied into the bundle.")
  endif()
endforeach()

# Embed frameworks automatically into the app bundle
if(CMAKE_GENERATOR STREQUAL "Xcode")
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    XCODE_EMBED_FRAMEWORKS "${FRAMEWORKS_TO_EMBED}"
    XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
    XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY ON
  )
else()
  find_program(DITTO_CMD ditto)
  find_program(CODESIGN_CMD codesign)

  set(COPY_FRAMEWORK_COMMANDS "")
  foreach(FW_PATH IN LISTS FRAMEWORKS_TO_EMBED)
    get_filename_component(FW_NAME "${FW_PATH}" NAME)
    if(DITTO_CMD)
      list(APPEND COPY_FRAMEWORK_COMMANDS
        COMMAND ${DITTO_CMD} "${FW_PATH}"
                "$<TARGET_BUNDLE_CONTENT_DIR:${CMAKE_PROJECT_NAME}>/Frameworks/${FW_NAME}"
      )
    else()
      list(APPEND COPY_FRAMEWORK_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${FW_PATH}"
                "$<TARGET_BUNDLE_CONTENT_DIR:${CMAKE_PROJECT_NAME}>/Frameworks/${FW_NAME}"
      )
    endif()

    if(CODESIGN_CMD)
      list(APPEND COPY_FRAMEWORK_COMMANDS
        COMMAND ${CODESIGN_CMD} --force --deep -s -
                "$<TARGET_BUNDLE_CONTENT_DIR:${CMAKE_PROJECT_NAME}>/Frameworks/${FW_NAME}"
      )
    endif()
  endforeach()

  set(SIGN_BUNDLE_COMMAND "")
  if(CODESIGN_CMD)
    set(SIGN_BUNDLE_COMMAND
      COMMAND ${CODESIGN_CMD} --force -s -
              "$<TARGET_BUNDLE_DIR:${CMAKE_PROJECT_NAME}>"
    )
  endif()

  add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_CONTENT_DIR:${CMAKE_PROJECT_NAME}>/Frameworks"
    ${COPY_FRAMEWORK_COMMANDS}
    ${SIGN_BUNDLE_COMMAND}
    COMMENT "Embedding and signing SFML frameworks into ${CMAKE_PROJECT_NAME}.app"
  )
endif()