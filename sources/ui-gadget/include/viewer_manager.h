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

#ifndef __VIEWER_MANAGER_H__
#define __VIEWER_MANAGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>
#include <Elementary.h>

#include "wlan_manager.h"

typedef enum {
	HEADER_MODE_OFF = 0x01,
	HEADER_MODE_ON,
	HEADER_MODE_ACTIVATING,
	HEADER_MODE_CONNECTING,
	HEADER_MODE_CONNECTED,
	HEADER_MODE_DEACTIVATING,
	HEADER_MODE_SEARCHING,
	HEADER_MODE_MAX
} HEADER_MODES;

typedef enum {
	VIEWER_ITEM_RADIO_MODE_OFF = 0x01,
	VIEWER_ITEM_RADIO_MODE_CONNECTED,
	VIEWER_ITEM_RADIO_MODE_CONNECTING,
	VIEWER_ITEM_RADIO_MODE_CONFIGURATION,
	VIEWER_ITEM_RADIO_MODE_MAX
} VIEWER_ITEM_RADIO_MODES;

typedef enum {
	VIEWER_WINSET_SEARCHING,
	VIEWER_WINSET_SUB_CONTENTS,
	VIEWER_WINSET_SEARCHING_GRP_TITLE
} VIEWER_WINSETS;


Evas_Object *viewer_manager_create(Evas_Object *parent, Evas_Object *_win_main);
Eina_Bool viewer_manager_show(VIEWER_WINSETS winset);
Eina_Bool viewer_manager_hide(VIEWER_WINSETS winset);
Eina_Bool viewer_manager_refresh(void);
void hi(void);
void power_control(void);

void viewer_manager_update_setup_wizard_scan_btn(void);
void language_changed_refresh(void);
Evas_Object *viewer_manager_get_naviframe(void);

void viewer_manager_header_mode_set(HEADER_MODES new_mode);
HEADER_MODES viewer_manager_header_mode_get(void);

void viewer_manager_move_to_top(void);
Elm_Object_Item *viewer_manager_move_item_to_top(Elm_Object_Item *item);
void viewer_manager_specific_scan_response_hlr(GSList *bss_info_list,
		void *user_data);
void viewer_manager_refresh_ap_info(Elm_Object_Item *item);
void viewer_manager_update_rssi(void);
void viewer_manager_setup_wizard_button_controller();
void viewer_manager_update_item_favorite_status(wifi_ap_h ap);
wifi_device_info_t *view_list_item_device_info_create(wifi_ap_h ap);
view_manager_view_type_t viewer_manager_view_type_get(void);
void viewer_manager_request_scan(void);
void viewer_manager_cleanup_views(void);
void viewer_manager_rotate_top_setupwizard_layout(void);
Evas_Object *viewer_manager_naviframe_power_item_get(void);
Evas_Object *viewer_manager_create_bg(Evas_Object *parent, char *style);
void viewer_manager_update_hidden_btn(void);
int viewer_manager_create_scan_btn(void);

#ifdef __cplusplus
}
#endif

#endif
