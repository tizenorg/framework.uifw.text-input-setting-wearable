ADD_CUSTOM_TARGET(w-text-input-setting.edj ALL
		COMMAND edje_cc -no-save -sd ${CMAKE_CURRENT_SOURCE_DIR} -id
		${CMAKE_CURRENT_SOURCE_DIR}/images ${CMAKE_CURRENT_SOURCE_DIR}/w-text-input-setting.edc w-text-input-setting.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/w-text-input-setting.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} w-text-input-setting.edj)
INSTALL(FILES w-text-input-setting.edj DESTINATION /usr/apps/org.tizen.${PROJECT_NAME}/res/edje)

#INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/images/ DESTINATION ${IMAGEDIR} FILES_MATCHING PATTERN "*.png")
#INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/images/ DESTINATION ${IMAGEDIR} FILES_MATCHING PATTERN "*.PNG")
