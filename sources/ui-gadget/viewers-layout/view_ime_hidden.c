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

#include <feedback.h>
#include <efl_assist.h>

#include "ug_wifi.h"
#include "view_ime_hidden.h"
#include "i18nmanager.h"
#include "wlan_manager.h"
#include "viewer_manager.h"
#include "viewer_list.h"
#include "common_utils.h"

struct hiddep_ap_popup_data {
	Evas_Object *win;
	const char *str_pkg_name;
	Evas_Object *popup;
	Evas_Object *entry;
	Evas_Object *ok_btn;
};

#define EDJ_GRP_HIDDEN_AP_ENTRY_LAYOUT "hidden_ap_entry_layout"

static hiddep_ap_popup_data_t *g_hidden_ap_popup_data = NULL;

static Eina_Bool _enable_scan_updates_cb(void *data)
{
	/* Lets enable the scan updates */
	wlan_manager_enable_scan_result_update();

	/* Reset the ecore timer handle */
	common_util_manager_ecore_scan_update_timer_reset();

	return ECORE_CALLBACK_CANCEL;
}

static void __popup_entry_changed_cb(void* data, Evas_Object* obj, void* event_info)
{
	if (g_hidden_ap_popup_data == NULL) {
		return;
	}

	hiddep_ap_popup_data_t *popup_data = g_hidden_ap_popup_data;
	Evas_Object *ok_btn = popup_data->ok_btn;

	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (elm_object_focus_get(obj)) {
			if (elm_entry_is_empty(obj)) {
				elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
				elm_object_disabled_set(ok_btn, EINA_TRUE);
				elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
			} else {
				elm_object_signal_emit(obj, "elm,state,clear,visible", "");
				elm_object_disabled_set(ok_btn, EINA_FALSE);
				elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
			}
		}
	}
}

static void __popup_entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (!elm_entry_is_empty(obj)) {
			elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		} else {
			elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
		}
	}
	elm_object_signal_emit(obj, "elm,state,focus,on", "");
}

static void __popup_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
	}
	elm_object_signal_emit(obj, "elm,state,focus,off", "");
}

static void __popup_entry_maxlength_reached(void *data, Evas_Object *obj,
		void *event_info)
{
		common_utils_send_message_to_net_popup("Password length",
				"Lengthy Password", "notification", NULL);
}

static void _password_entry_eraser_clicked_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	elm_entry_entry_set(data, "");
	feedback_initialize();
	feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_SIP);
	feedback_deinitialize();
}

hiddep_ap_popup_data_t *view_hidden_ap_popup_data_get(void)
{
	return g_hidden_ap_popup_data;
}

void view_hidden_ap_popup_delete(void)
{
	if (g_hidden_ap_popup_data == NULL)
		return;

	if (g_hidden_ap_popup_data->popup) {
		evas_object_del(g_hidden_ap_popup_data->popup);
		g_hidden_ap_popup_data->popup = NULL;
	}

	g_free(g_hidden_ap_popup_data);
	g_hidden_ap_popup_data = NULL;
}

void view_hidden_ap_popup_destroy(void)
{
	view_hidden_ap_popup_delete();

	/* A delay is needed to get the smooth Input panel closing animation effect */
	common_util_managed_ecore_scan_update_timer_add(0.1,
			_enable_scan_updates_cb, NULL);
}

static void view_hidden_ap_popup_ok_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	hiddep_ap_popup_data_t *popup_data = g_hidden_ap_popup_data;
	Elm_Object_Item *item = NULL;
	char *entry_txt = NULL;
	Evas_Object *entry = NULL;
	Evas_Object *layout = NULL;
	int err;

	if (popup_data == NULL)
		return;

	layout = elm_object_content_get(popup_data->popup);
	entry = elm_object_part_content_get(layout, "elm.swallow.content");
	entry_txt = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));

	item = item_get_for_ssid(entry_txt);
	if (item != NULL) {
		ug_genlist_data_t *gdata = elm_object_item_data_get(item);
		viewer_list_wifi_connect(gdata->device_info);

		view_hidden_ap_popup_destroy();
		g_free(entry_txt);

		return;
	}

	err = wlan_manager_scan_with_ssid(entry_txt, entry_txt);
	if (err != WLAN_MANAGER_ERR_NONE) {
		common_utils_send_message_to_net_popup("Network connection popup",
				"no ap found", "notification", NULL);

		view_hidden_ap_popup_destroy();
		g_free(entry_txt);

		return;
	}

	viewer_manager_show(VIEWER_WINSET_SEARCHING);
	viewer_manager_header_mode_set(HEADER_MODE_SEARCHING);

	evas_object_del(popup_data->popup);
	popup_data->popup = NULL;

	/* A delay is needed to get the smooth Input panel closing animation effect */
	common_util_managed_ecore_scan_update_timer_add(0.1,
			_enable_scan_updates_cb, NULL);
}

static void view_hidden_ap_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	view_hidden_ap_popup_destroy();
}

void view_hidden_ap_popup_create(Evas_Object *win_main, const char *str_pkg_name)
{
	__COMMON_FUNC_ENTER__;

	Evas_Object *passpopup = NULL;
	Evas_Object *button = NULL;
	Evas_Object *entry = NULL;
	Evas_Object *layout = NULL;
	popup_btn_info_t popup_data;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	int return_key_type;

	if (win_main == NULL) {
		INFO_LOG(UG_NAME_ERR, "Invalid argument passed.");
		return;
	}

	if (g_hidden_ap_popup_data != NULL)
		g_free(g_hidden_ap_popup_data);

	g_hidden_ap_popup_data = g_try_new0(hiddep_ap_popup_data_t, 1);
	if (g_hidden_ap_popup_data == NULL) {
		INFO_LOG(UG_NAME_ERR, "Memory allocation error.");
		return;
	}

	/* Lets disable the scan updates so that the UI is not refreshed un necessarily */
	wlan_manager_disable_scan_result_update();

	memset(&popup_data, 0, sizeof(popup_data));
	popup_data.title_txt = sc(str_pkg_name, I18N_TYPE_Find_Hidden_Network);
	popup_data.btn1_cb = view_hidden_ap_popup_cancel_cb;
	popup_data.btn1_data = NULL;
	popup_data.btn1_txt = sc(str_pkg_name, I18N_TYPE_Cancel);
	popup_data.btn2_cb = view_hidden_ap_popup_ok_cb;
	popup_data.btn2_data = NULL;
	popup_data.btn2_txt = sc(str_pkg_name, I18N_TYPE_Find);
	popup_data.popup_with_entry = true;

	passpopup = common_utils_show_info_popup(win_main, &popup_data);
	g_hidden_ap_popup_data->ok_btn = popup_data.btn;
	elm_object_disabled_set(g_hidden_ap_popup_data->ok_btn, EINA_TRUE);

	layout = elm_layout_add(passpopup);
	if (layout == NULL)
		return;

	elm_layout_theme_set(layout, "layout", "editfield", "singleline");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	entry = elm_entry_add(layout);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_object_part_text_set(entry, "elm.guide", sc(str_pkg_name, I18N_TYPE_Ssid));

	elm_object_part_content_set(layout, "elm.swallow.content", entry);
	limit_filter_data.max_char_count = 32;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size,
			&limit_filter_data);

	return_key_type = ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE;
	elm_entry_input_panel_return_key_type_set(entry, return_key_type);
	elm_entry_input_panel_return_key_disabled_set(entry, EINA_TRUE);

	g_hidden_ap_popup_data->win = win_main;
	g_hidden_ap_popup_data->str_pkg_name = str_pkg_name;
	g_hidden_ap_popup_data->popup = passpopup;
	g_hidden_ap_popup_data->entry = entry;

	evas_object_smart_callback_add(entry, "activated",
			view_hidden_ap_popup_ok_cb, NULL);
	evas_object_smart_callback_add(entry, "changed",
			__popup_entry_changed_cb, NULL);
	evas_object_smart_callback_add(entry, "preedit,changed",
			__popup_entry_changed_cb, NULL);
	evas_object_smart_callback_add(entry, "focused",
			__popup_entry_focused_cb, NULL);
	evas_object_smart_callback_add(entry, "unfocused",
			__popup_entry_unfocused_cb, NULL);
	evas_object_smart_callback_add(entry, "maxlength,reached",
			__popup_entry_maxlength_reached, NULL);
	evas_object_show(entry);

	button = elm_button_add(entry);
	elm_object_style_set(button, "editfield_clear");
	elm_object_focus_allow_set(button, EINA_FALSE);
	elm_object_part_content_set(entry, "elm.swallow.clear", button);
	evas_object_smart_callback_add(button, "clicked",
			_password_entry_eraser_clicked_cb, entry);

	evas_object_show(layout);
	elm_object_content_set(passpopup, layout);
	evas_object_show(passpopup);
	elm_object_focus_set(passpopup, EINA_TRUE);
	elm_object_focus_set(entry, EINA_TRUE);

	__COMMON_FUNC_EXIT__;
}
