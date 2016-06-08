/*
 * Wi-Fi
 *
 * Copyright 2012 Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __WIFI_SYSPOPUP_H__
#define __WIFI_SYSPOPUP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <efl_assist.h>

#include "common.h"
#include "wlan_manager.h"
#include "common_pswd_popup.h"
#include "common_eap_connect.h"

#define PACKAGE				"wifi-qs"
#define LOCALEDIR		"/usr/share/locale"
#define WIFI_SP_ICON_PATH	"/usr/share/icons"

typedef enum {
	WIFI_DEVPKR_SUPPORT_NONE			= 0x00,
	WIFI_DEVPKR_SUPPORT_QUICKPANEL,
	WIFI_DEVPKR_SUPPORT_MAX,
} WIFI_DEVPKR_SUPPORTS;

typedef enum {
	WIFI_DEVPKR_WITH_AP_LIST		= 0x00,
	WIFI_DEVPKR_WITHOUT_AP_LIST
} WIFI_DEVPKR_TYPE;

#define MAX_PROFILE_NUM NETPM_PROFILES_PERSISTENT_MAX

typedef struct wifi_object {
	/* wifi object attributes */
	WIFI_DEVPKR_SUPPORTS wifi_devpkr_support;

	/* connection_result */
	int connection_result;

	Eina_Bool update_enabled;

	/* caller type */
	WIFI_DEVPKR_TYPE devpkr_type;

	/* window */
	Evas_Object *win_main;
	Evas_Object *conformant;
	Evas_Object *layout_main;

	Evas *evas;

	/* popups */
	Evas_Object *popup;
	pswd_popup_t *passpopup;
	eap_connect_data_t *eap_popup;
	Evas_Object *alertpopup;
	Ea_Theme_Color_Table *color_table;
	Ea_Theme_Font_Table *font_table;
} wifi_object;

typedef enum {
	ITEM_CONNECTION_MODE_NULL,
	ITEM_CONNECTION_MODE_OFF,
	ITEM_CONNECTION_MODE_CONNECTING,
	ITEM_CONNECTION_MODE_CONFIGURATION,
	ITEM_CONNECTION_MODE_MAX
} ITEM_CONNECTION_MODES;

typedef struct {
	Elm_Object_Item *it;
	ITEM_CONNECTION_MODES connection_mode;
	wifi_device_info_t *dev_info;
	gboolean highlighted;
} devpkr_gl_data_t;

void wifi_devpkr_redraw(void);
int wifi_devpkr_destroy(void);
void wifi_devpkr_enable_scan_btn(void);
void wifi_devpkr_disable_scan_btn(void);
gboolean wifi_devpkr_get_scan_status(void);

#ifdef __cplusplus
}
#endif

#endif
