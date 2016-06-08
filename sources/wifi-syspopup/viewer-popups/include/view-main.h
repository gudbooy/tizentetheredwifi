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

#ifndef __WIFI_SYSPOPUP_VIEW_MAIN_H__
#define __WIFI_SYSPOPUP_VIEW_MAIN_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <Evas.h>
#include <Eina.h>

#include "wlan_manager.h"
#include "wifi-syspopup.h"

void view_main_create_main_list(void);
void view_main_item_state_set(wifi_ap_h ap, ITEM_CONNECTION_MODES state);
gboolean view_main_show(void *data);
void view_main_wifi_connect(devpkr_gl_data_t *gdata);
void view_main_wifi_reconnect(devpkr_gl_data_t *gdata);
Elm_Object_Item *view_main_item_get_for_ap(wifi_ap_h ap);
int view_main_get_profile_count(void);
void view_main_update_group_title(gboolean is_bg_scan);

#ifdef __cplusplus
}
#endif

#endif
