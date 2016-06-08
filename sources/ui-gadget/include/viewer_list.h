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

#ifndef __VIEWER_LIST_H__
#define __VIEWER_LIST_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "viewer_manager.h"

typedef struct {
	wifi_device_info_t *device_info;
	VIEWER_ITEM_RADIO_MODES radio_mode;
	gboolean highlighted;
} ug_genlist_data_t;

Evas_Object *viewer_list_create(Evas_Object *win);

//////// list item add / remove ///////////////////////////////////////////////////////////////
void viewer_list_title_item_set_available_networks(Elm_Object_Item *item_header);
void viewer_list_title_item_del_available_networks(Elm_Object_Item *item_header);
void viewer_list_title_item_update(void);

int viewer_list_item_radio_mode_set(Elm_Object_Item* item,
		VIEWER_ITEM_RADIO_MODES mode);
Elm_Object_Item *viewer_list_item_insert_after(wifi_device_info_t *wifi_device,
		Elm_Object_Item *after);
Elm_Object_Item *viewer_list_get_first_item(void);
void viewer_list_item_clear(void);
////////////////////////////////////////////////////////////////////////////////////////////////

//////// item iteration /////////////////////////////////////////////////////////////////////////
int viewer_list_item_size_get(void);
void viewer_list_item_del(Elm_Object_Item *item);
Elm_Object_Item *viewer_list_item_first_get(Evas_Object* list);
Elm_Object_Item *viewer_list_item_next_get(const Elm_Object_Item* current);
Elm_Object_Item *viewer_list_item_at_index(int index);
/////////////////////////////////////////////////////////////////////////////////////////////////

//////// item control /////////////////////////////////////////////////////////////////////////
void viewer_list_item_enable_all(void);
void viewer_list_item_disable_all(void);
//////////////////////////////////////////////////////////////////////////////////////////////////

Elm_Object_Item *item_get_for_ap(wifi_ap_h ap);
Elm_Object_Item *item_get_for_ssid(const char *ssid);

void viewer_list_wifi_connect(wifi_device_info_t *device_info);
void viewer_list_wifi_reconnect(wifi_device_info_t *device_info);

void viewer_list_scanned_ap_list_free(void);
char* ConvertRGBAtoHex(int r, int g, int b, int a);
#ifdef __cplusplus
}
#endif

#endif
