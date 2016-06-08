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

#ifndef __COMMON_IP_INFO_H__
#define __COMMON_IP_INFO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <Evas.h>

#include "wlan_manager.h"
#include "common_utils.h"

#define DEFAULT_GUIDE_PROXY_IP "proxy.example.com"
#define DEFAULT_GUIDE_PROXY_PORT "8080"

typedef struct ip_info_list ip_info_list_t;

ip_info_list_t *ip_info_append_items(wifi_ap_h ap, const char *pkg_name,
		Evas_Object *genlist,
		imf_ctxt_panel_cb_t input_panel_cb, void *input_panel_cb_data);
void ip_info_save_data(ip_info_list_t *ip_info_list);
void ip_info_remove(ip_info_list_t *ip_info_list);
void ip_info_close_all_keypads(ip_info_list_t *ip_info_list);
void ip_info_enable_all_keypads(ip_info_list_t *ip_info_list);
void ip_info_items_realize(ip_info_list_t *ip_info_list);

#ifdef __cplusplus
}
#endif

#endif
