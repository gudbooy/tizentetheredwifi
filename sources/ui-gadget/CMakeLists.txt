CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(wifi-efl-UG C)

# defines
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")
SET(VERSION 0.1.0)
SET(RESDIR "${PREFIX}/res")
SET(TABLEDIR "/usr/apps/wifi-efl-ug/shared/res/tables")

IF(TIZEN_TETHERING_ENABLE)
	SET(TETHERING_POPUP_SRCS winset_popup.c)
ENDIF(TIZEN_TETHERING_ENABLE)

# source files
SET(SRCS
	../libraries/i18nManager/i18nmanager.c
	../libraries/appcoreWrapper/appcoreWrapper.c
	../libraries/WlanManager/WlanManager/connection_manager.c
	../libraries/WlanManager/WlanManager/wlan_manager.c
	../libraries/WlanManager/WlanManager/wlan_connection.c
	../libraries/Common/common_utils.c
	../libraries/Common/common_ip_info.c
	../libraries/Common/common_pswd_popup.c
	../libraries/Common/common_eap_connect.c
	../libraries/Common/common_generate_pin.c
	../libraries/Common/common_bt.c
	${ENGINE_SOURCES}
	${TETHERING_POPUP_SRCS}
	bt_tethered_service.c
	wifi-efl-UG.c
	viewers-layout/viewer_manager.c
	viewers-layout/wifi_viewer_list.c
	viewers-layout/view_detail.c
	viewers-layout/view_advanced.c
	viewers-layout/view_ime_hidden.c
	wifi-wlan-callbacks.c)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

# add library
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 0.1.0)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

ADD_CUSTOM_TARGET(custom_editfield.edj
	COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../../resources/images
		${CMAKE_CURRENT_SOURCE_DIR}/edcs/custom_editfield.edc
		${CMAKE_CURRENT_SOURCE_DIR}/edcs/custom_editfield.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/edcs/custom_editfield.edc
)

ADD_DEPENDENCIES(${PROJECT_NAME} custom_editfield.edj)

ADD_CUSTOM_TARGET(setup_wizard.edj
	COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../../resources/images
		${CMAKE_CURRENT_SOURCE_DIR}/edcs/setup_wizard.edc
		${CMAKE_CURRENT_SOURCE_DIR}/edcs/setup_wizard.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/edcs/setup_wizard.edc
)

ADD_DEPENDENCIES(${PROJECT_NAME} setup_wizard.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/wifi-efl-ug/lib/ug/)

# install image files
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/edcs/custom_editfield.edj DESTINATION /usr/apps/wifi-efl-ug/res/edje/wifi-efl-UG)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/edcs/setup_wizard.edj DESTINATION /usr/apps/wifi-efl-ug/res/edje/wifi-efl-UG)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../wifi-efl-ug.xml DESTINATION /usr/share/packages/)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../tables/ DESTINATION ${TABLEDIR} FILES_MATCHING PATTERN "*.xml")
