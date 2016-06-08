CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(wifi-qs C)

# defines
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")
SET(VERSION 0.1.0)

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
	${ENGINE_SOURCES}
	viewer-popups/view-main.c
	viewer-popups/view-alerts.c
	wifi-syspopup.c
	wifi-syspopup-engine-callback.c)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

# add executable
ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

ADD_CUSTOM_TARGET(wifi-syspopup-custom.edj
	COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../../resources/images
		${CMAKE_CURRENT_SOURCE_DIR}/edcs/wifi-syspopup-custom.edc
		${CMAKE_CURRENT_SOURCE_DIR}/edcs/wifi-syspopup-custom.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/edcs/wifi-syspopup-custom.edc
)

ADD_DEPENDENCIES(${PROJECT_NAME} wifi-syspopup-custom.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)

# install image files

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../resources/images/A01-3_icon_captive.png DESTINATION /usr/share/icons)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../resources/images/noti_wifi_in_range.png DESTINATION /usr/share/icons)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../resources/images/noti_wifi_in_range_ongoing.png DESTINATION /usr/share/icons)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/edcs/wifi-syspopup-custom.edj DESTINATION /usr/apps/wifi-efl-ug/res/edje/wifi-qs)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/net.${PROJECT_NAME}.xml DESTINATION /usr/share/packages/)
