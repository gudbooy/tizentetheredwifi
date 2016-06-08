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

#ifndef __COMMON_PSWD_POPUP_H__
#define __COMMON_PSWD_POPUP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <Evas.h>
#include <wifi.h>

#include "common_utils.h"

typedef enum {
	POPUP_WPS_OPTIONS,
	POPUP_WPS_PIN,
	POPUP_WPS_BTN,
} popup_type_t;

typedef struct {
	char *title;
	Eina_Bool show_wps_btn;
	Evas_Smart_Cb ok_cb;
	Evas_Smart_Cb cancel_cb;
	Evas_Smart_Cb setting_cb;
	Evas_Smart_Cb wps_btn_cb;
	Evas_Smart_Cb wps_pin_cb;
	wifi_ap_h ap;
	void *cb_data;
	wifi_security_type_e sec_type;
} pswd_popup_create_req_data_t;

typedef struct {
	/* PBC popup related attributes */
	Evas_Object* popup;
	Evas_Object* progressbar;
	Evas_Object* timer_label;
	Ecore_Timer *timer;
	int checker;
	double value;
} pbc_popup_t;

struct pswd_popup {
	/* Password popup related attributes */
	const char *str_pkg_name;
	Evas_Object *win;
	Evas_Object *popup;
	Evas_Object *entry;
	Evas_Object *info_popup;
	Evas_Object *wps_options_popup;
	Elm_Object_Item *wps_options_item;
	pbc_popup_t *pbc_popup_data;
	wifi_ap_h ap;
	wifi_security_type_e sec_type;
};

typedef struct pswd_popup pswd_popup_t;

pswd_popup_t *create_passwd_popup(Evas_Object *win_main, const char *pkg_name,
		pswd_popup_create_req_data_t *popup_info);
void create_pbc_popup(pswd_popup_t *pswd_popup_data, Evas_Smart_Cb cancel_cb,
		void *cancel_cb_data, popup_type_t type, char *pin);
void create_wps_options_popup(Evas_Object *win_main,
		pswd_popup_t *pswd_popup_data,
		pswd_popup_create_req_data_t *popup_info);

void current_popup_free(pswd_popup_t *pswd_popup_data, popup_type_t type);
void passwd_popup_show(pswd_popup_t *pswd_popup_data);
void passwd_popup_hide(pswd_popup_t *pswd_popup_data);
void passwd_popup_free(pswd_popup_t *pswd_popup_data);

char *passwd_popup_get_txt(pswd_popup_t *pswd_popup_data);
wifi_ap_h passwd_popup_get_ap(pswd_popup_t *pswd_popup_data);

#ifdef __cplusplus
}
#endif

#endif
