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

#include "common.h"
#include "common_utils.h"
#include "wifi-syspopup.h"
#include "view-alerts.h"
#include "i18nmanager.h"

extern wifi_object* devpkr_app_state;

int view_alerts_powering_on_show(void)
{
	__COMMON_FUNC_ENTER__;
	if (WIFI_DEVPKR_SUPPORT_QUICKPANEL == devpkr_app_state->wifi_devpkr_support){
		__COMMON_FUNC_EXIT__;
		return TRUE;
	}
	if (NULL != devpkr_app_state->alertpopup) {
		evas_object_del(devpkr_app_state->alertpopup);
		devpkr_app_state->alertpopup = NULL;
	}

	devpkr_app_state->alertpopup = elm_popup_add(devpkr_app_state->layout_main);

	Evas_Object *box = elm_box_add(devpkr_app_state->alertpopup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(box);

	Evas_Object *progressbar = elm_progressbar_add(box);
	elm_object_style_set(progressbar, "process_medium");
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_show(progressbar);
	elm_box_pack_end(box, progressbar);

	Evas_Object *label = elm_label_add(box);
	elm_object_style_set(label, "popup/default");
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_text_set(label, sc(PACKAGE, I18N_TYPE_Activating));
	evas_object_show(label);
	elm_box_pack_end(box, label);

	elm_object_content_set(devpkr_app_state->alertpopup, box);
	evas_object_size_hint_weight_set(devpkr_app_state->alertpopup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(devpkr_app_state->alertpopup);

	__COMMON_FUNC_EXIT__;

	return TRUE;
}

void view_alerts_popup_show(const char *err_msg)
{
	__COMMON_FUNC_ENTER__;

	if (WIFI_DEVPKR_SUPPORT_QUICKPANEL ==
			devpkr_app_state->wifi_devpkr_support) {
		__COMMON_FUNC_EXIT__;
		return;
	}

	if (NULL != devpkr_app_state->alertpopup) {
		evas_object_del(devpkr_app_state->alertpopup);
		devpkr_app_state->alertpopup = NULL;
	}

	devpkr_app_state->alertpopup = common_utils_show_info_timeout_popup(devpkr_app_state->win_main, err_msg, 2.0f);

	__COMMON_FUNC_EXIT__;
}
