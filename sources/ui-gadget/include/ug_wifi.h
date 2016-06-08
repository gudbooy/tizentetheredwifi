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

#ifndef __NEW_WIFI_H__
#define __NEW_WIFI_H__

#define TARGET

#ifdef __cplusplus
extern "C"
{
#endif

#include <ui-gadget-module.h>
#include <efl_assist.h>

#include "common.h"
#include "common_pswd_popup.h"
#include "view_ime_hidden.h"
#include "common_eap_connect.h"
#include "winset_popup.h"
#include "wlan_manager.h"
#include "viewer_manager.h"

#define PACKAGE					"ug-wifi-efl-UG"

#define UG_CALLER "caller"
#define UG_VIEWTYPE "viewtype"
#define UG_MAIN_MESSAGE_DESTROY 1

typedef struct {
	/* ui gadget object */
	void* gadget;
	ui_gadget_h ug;

	//Basic Evas_Objects
	Evas_Object *layout_main;
	Evas *evas;
	pswd_popup_t *passpopup;

	UG_TYPE ug_type;
	Eina_Bool bAlive;

	char *lbutton_setup_wizard_prev;
	char *rbutton_setup_wizard_next;
	char *rbutton_setup_wizard_skip;
#if defined TIZEN_TETHERING_ENABLE
	popup_manager_object_t *popup_manager;
#endif
	eap_connect_data_t *eap_view;
	Ea_Theme_Color_Table *color_table;
	Ea_Theme_Font_Table *font_table;
} wifi_appdata ;

struct ug_data {
	Evas_Object *base;
	Evas_Object *win_main;
	ui_gadget_h ug;
};

int wifi_exit(void);
bool wifi_is_scan_required(void);

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_H__ */
