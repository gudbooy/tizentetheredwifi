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

#include "common.h"
#include "common_pswd_popup.h"
#include "common_generate_pin.h"
#include "i18nmanager.h"
#include "ug_wifi.h"

#define EDJ_GRP_OUTER_LAYOUT_LANDSCAPE "outer_layout_landscape"
#define EDJ_GRP_OUTER_LAYOUT_2_GL_ITEMS "outer_layout_2_gl_items"
#define EDJ_GRP_OUTER_LAYOUT_1_GL_ITEMS "outer_layout_1_gl_items"
#define EDJ_GRP_OUTER_LAYOUT_0_GL_ITEMS "outer_layout_0_gl_items"
#define EDJ_GRP_POPUP_BUTTON_LAYOUT "popup_button_layout"
#define EDJ_GRP_POPUP_PBC_BUTTON_LAYOUT "popup_pbc_button_layout"
#define EDJ_GRP_POPUP_WPS_PIN_LAYOUT "popup_wps_pin_layout"

#define PBC_TIMEOUT_MSG_STR	"one click connection failed"
#define MAX_PBC_TIMEOUT_SECS	120	// Time in seconds
#define PASSWORD_LENGTH		64

static Elm_Genlist_Item_Class g_wps_itc;
static Elm_Genlist_Item_Class g_info_itc;
static Elm_Genlist_Item_Class g_check_box_itc;
static Elm_Genlist_Item_Class g_pswd_entry_itc;
static gboolean wps_options_btn_hl = FALSE;


static void __popup_entry_changed_cb(void* data, Evas_Object* obj, void* event_info)
{
	Evas_Object *btn_ok = NULL;
	pswd_popup_t *pswd_popup_data = NULL;
	char* entry_str = NULL;
	unsigned short int passwd_length = 0;

	if (!data) {
		return;
	}

	pswd_popup_data = (pswd_popup_t *) data;
	entry_str = passwd_popup_get_txt(pswd_popup_data);
	if (entry_str != NULL)
		passwd_length = strlen(entry_str);

	btn_ok = elm_object_part_content_get(pswd_popup_data->popup, "button2");

	if (pswd_popup_data->sec_type == WIFI_SECURITY_TYPE_WEP) {
		if (passwd_length > 0) {
			elm_object_disabled_set(btn_ok, EINA_FALSE);
			elm_entry_input_panel_return_key_disabled_set(obj,
					EINA_FALSE);
		} else {
			elm_object_disabled_set(btn_ok, EINA_TRUE);
			elm_entry_input_panel_return_key_disabled_set(obj,
					EINA_TRUE);
		}
	} else if (pswd_popup_data->sec_type == WIFI_SECURITY_TYPE_WPA2_PSK ||
			pswd_popup_data->sec_type == WIFI_SECURITY_TYPE_WPA_PSK) {
		if (passwd_length > 7) {
			elm_object_disabled_set(btn_ok, EINA_FALSE);
			elm_entry_input_panel_return_key_disabled_set(obj,
					EINA_FALSE);
		} else {
			elm_object_disabled_set(btn_ok, EINA_TRUE);
			elm_entry_input_panel_return_key_disabled_set(obj,
					EINA_TRUE);
		}
	}

	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (elm_object_focus_get(obj)) {
			if (elm_entry_is_empty(obj)) {
				elm_object_signal_emit(obj,
						"elm,state,clear,hidden", "");
			} else {
				elm_object_signal_emit(obj,
						"elm,state,clear,visible", "");
			}
		}
	}

	if (entry_str)
		g_free(entry_str);
}

static void __popup_entry_maxlength_reached(void *data, Evas_Object *obj,
		void *event_info)
{
	common_utils_send_message_to_net_popup("Password length",
			"Lengthy Password", "notification", NULL);
}

static void __common_pbc_popup_destroy(pbc_popup_t *pbc_popup_data)
{
	__COMMON_FUNC_ENTER__;
	if (!pbc_popup_data) {
		return;
	}

	if (pbc_popup_data->checker == 0) {
		pbc_popup_data->checker = 1;

		if (pbc_popup_data->timer != NULL) {
			ecore_timer_del(pbc_popup_data->timer);
			pbc_popup_data->timer = NULL;
		}
		if (pbc_popup_data->popup != NULL) {
			evas_object_hide(pbc_popup_data->popup);
			evas_object_del(pbc_popup_data->popup);
			pbc_popup_data->popup = NULL;
		}
		g_free(pbc_popup_data);
	}
	__COMMON_FUNC_EXIT__;
	return;
}

static Eina_Bool _fn_pb_timer_bar(void *data)
{
	double val = 120.0;
	pswd_popup_t *pswd_popup_data = (pswd_popup_t *)data;
	pbc_popup_t *pbc_popup_data = pswd_popup_data->pbc_popup_data;
	if (!pbc_popup_data || pbc_popup_data->timer == NULL || pbc_popup_data->progressbar == NULL) {
		return ECORE_CALLBACK_CANCEL;
	}
	const double diff = (double)1/(double)MAX_PBC_TIMEOUT_SECS;
	pbc_popup_data->value = elm_progressbar_value_get(pbc_popup_data->progressbar);
	pbc_popup_data->value += diff;
	if (pbc_popup_data->value >= 1) {
		if (pbc_popup_data->checker == 0) {
			__COMMON_FUNC_ENTER__;
			common_utils_show_info_timeout_popup(pswd_popup_data->win, PBC_TIMEOUT_MSG_STR, 3.0f);
			Evas_Object *cancel_btn = elm_object_part_content_get(pbc_popup_data->popup, "button1");
			evas_object_smart_callback_call(cancel_btn, "clicked", NULL);
			__COMMON_FUNC_EXIT__;
		}
		return ECORE_CALLBACK_CANCEL;
	}

	val = (float)(val * (1 - pbc_popup_data->value));
	int remain_mins = (int)(val);
	int remain_secs = 0;
	remain_secs = remain_mins % 60;
	remain_mins /= 60;

	char *remaining_time_str = g_strdup_printf("<font_size=40><align=center>%02d:%02d</align></font_size>", remain_mins, remain_secs);
	elm_object_text_set(pbc_popup_data->timer_label, remaining_time_str);
	//INFO_LOG(UG_NAME_NORMAL, "pbc_popup_data->value = %lf; remain_mins = %d; remain_secs = %d; remaining_time_str = %s", pbc_popup_data->value, remain_mins, remain_secs, remaining_time_str);
	g_free(remaining_time_str);

	elm_progressbar_value_set(pbc_popup_data->progressbar, pbc_popup_data->value);

	return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _enable_scan_updates_cb(void *data)
{
	/* Lets enable the scan updates */
	wlan_manager_enable_scan_result_update();

	/* Reset the ecore timer handle */
	common_util_manager_ecore_scan_update_timer_reset();

	return ECORE_CALLBACK_CANCEL;
}

void create_pbc_popup(pswd_popup_t *pswd_popup_data, Evas_Smart_Cb cancel_cb,
		void *cancel_cb_data, popup_type_t type, char *pin)
{
	if (!pswd_popup_data) {
		return;
	}
	Evas_Object *popup = NULL;
	Evas_Object *progressbar = NULL;
	Evas_Object *timer_label = NULL;
	Evas_Object *layout;
	Evas_Object *icon = NULL;
	char *icon_path = NULL;
	char buf[200] = "";

	pbc_popup_t *pbc_popup_data = NULL;
	pbc_popup_data = g_new0(pbc_popup_t, 1);

	popup_btn_info_t popup_btn_data;
	memset(&popup_btn_data, 0, sizeof(popup_btn_data));
	popup_btn_data.btn1_txt = sc(pswd_popup_data->str_pkg_name, I18N_TYPE_Cancel);
	popup_btn_data.btn1_cb = cancel_cb;
	popup_btn_data.btn1_data = cancel_cb_data;
	popup = common_utils_show_info_popup(pswd_popup_data->win, &popup_btn_data);

	icon = elm_icon_add(popup);
	common_utils_get_wps_icon(&icon_path, FALSE);
	elm_image_file_set(icon, CUSTOM_EDITFIELD_PATH, icon_path);
	elm_object_part_content_set(popup, "title,icon", icon);
	g_free(icon_path);

	if (type == POPUP_WPS_BTN) {
		elm_object_part_text_set(popup, "title,text",
				sc(pswd_popup_data->str_pkg_name, I18N_TYPE_WPS_Button));
	} else if (type == POPUP_WPS_PIN) {
		elm_object_part_text_set(popup, "title,text",
				sc(pswd_popup_data->str_pkg_name, I18N_TYPE_WPS_PIN));

	}

	layout = elm_layout_add(popup);
	if (layout == NULL) {
		return;
	}

	if (type == POPUP_WPS_BTN) {
		elm_layout_file_set(layout, CUSTOM_EDITFIELD_PATH,
				EDJ_GRP_POPUP_PBC_BUTTON_LAYOUT);
		g_snprintf(buf, sizeof(buf),
				_(sc(pswd_popup_data->str_pkg_name,
					I18N_TYPE_Press_WPS_On_Your_Wi_Fi_Access_Point)), 2);
		elm_object_part_text_set(layout, "elm.text.description", buf);
	} else if (type == POPUP_WPS_PIN) {
		elm_layout_file_set(layout, CUSTOM_EDITFIELD_PATH,
				EDJ_GRP_POPUP_WPS_PIN_LAYOUT);
		g_snprintf(buf, sizeof(buf),
				_(sc(pswd_popup_data->str_pkg_name,
					I18N_TYPE_Enter_PIN_number_on_your_WIFI_access_point)),
					pin, 2);
		elm_object_part_text_set(layout, "elm.text.description", buf);
	}
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	progressbar = elm_progressbar_add(layout);
	elm_object_style_set(progressbar, "list_progress");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(progressbar, 0.0);

	timer_label = elm_label_add(layout);
	elm_object_style_set(timer_label, "label3");
	elm_label_line_wrap_set(timer_label, ELM_WRAP_MIXED);
	elm_object_text_set(timer_label, _("<font_size=40><align=center>02:00</align></font_size>"));
	evas_object_size_hint_weight_set(timer_label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(timer_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(timer_label);

	elm_object_part_content_set(layout, "slider", progressbar);
	elm_object_part_content_set(layout, "timer_label", timer_label);

	pbc_popup_data->checker = 0;
	pbc_popup_data->value = 0.0;
	pbc_popup_data->progressbar = progressbar;
	pbc_popup_data->timer_label = timer_label;
	pbc_popup_data->popup = popup;
	pbc_popup_data->timer = ecore_timer_add(1.0, _fn_pb_timer_bar, pswd_popup_data);
	evas_object_show(progressbar);

	evas_object_show(popup);
	elm_object_content_set(popup, layout);
	pswd_popup_data->pbc_popup_data = pbc_popup_data;

	evas_object_hide(pswd_popup_data->popup);

	return;
}

static char *_gl_wps_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text.main.left")) {
		char buf[1024];
		snprintf(buf, 1023, "%s", sc(PACKAGE, (int)data));
		return strdup(buf);
	}
	return NULL;
}

static void _gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	if (!event_info)
		return;

	int index = (int)elm_object_item_data_get(event_info);

	if (index == I18N_TYPE_WPS_PIN) {
		elm_object_item_signal_emit(event_info, "elm,state,bottomline,hide", "");
	}
}

void create_wps_options_popup(Evas_Object *win_main,
		pswd_popup_t *pswd_popup_data,
		pswd_popup_create_req_data_t *popup_info)
{
	if (!win_main || !popup_info || !pswd_popup_data) {
		return;
	}

	Evas_Object *popup = NULL;
	Evas_Object *box;
	Evas_Object *genlist = NULL;
	static Elm_Genlist_Item_Class wps_itc;
	Evas_Object *icon = NULL;
	char *icon_path = NULL;

	popup = elm_popup_add(win_main);
	elm_object_part_text_set(popup, "title,text", popup_info->title);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	icon = elm_icon_add(popup);
	common_utils_get_wps_icon(&icon_path, FALSE);
	elm_image_file_set(icon, CUSTOM_EDITFIELD_PATH, icon_path);
	elm_object_part_content_set(popup, "title,icon", icon);
	g_free(icon_path);

	box = elm_box_add(popup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(popup);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "realized", _gl_realized, NULL);

	wps_itc.item_style = "1line";
	wps_itc.func.text_get = _gl_wps_text_get;
	wps_itc.func.content_get = NULL;
	wps_itc.func.state_get = NULL;
	wps_itc.func.del = NULL;

	elm_genlist_item_append(genlist, &wps_itc,
			(void*)I18N_TYPE_WPS_Button, NULL,
			ELM_GENLIST_ITEM_NONE, popup_info->wps_btn_cb, popup_info->cb_data);
	elm_genlist_item_append(genlist, &wps_itc,
			(void*)I18N_TYPE_WPS_PIN, NULL,
			ELM_GENLIST_ITEM_NONE, popup_info->wps_pin_cb, popup_info->cb_data);

	elm_box_pack_end(box, genlist);
	evas_object_show(genlist);

	ea_object_event_callback_add(popup, EA_CALLBACK_BACK,
			popup_info->cancel_cb, popup_info->cb_data);
	elm_object_part_content_set(popup, "elm.swallow.content" , box);
	evas_object_size_hint_min_set(box, -1, (ELM_SCALE_SIZE(192)));
	elm_object_content_set(popup, box);
	evas_object_show(popup);
	elm_object_focus_set(popup, EINA_TRUE);

	pswd_popup_data->wps_options_popup = popup;

	evas_object_hide(pswd_popup_data->popup);
}

static char *_wifi_network_info_item_text_get(void *data, Evas_Object *obj,
		const char *part)
{
	__COMMON_FUNC_ENTER__;
	if (!strcmp(part, "elm.text.main.left")) {
		char buf[1024];
		snprintf(buf, 1023, "%s", sc(PACKAGE, I18N_TYPE_Wi_Fi_network_info));
		return strdup(buf);
	}
	return NULL;
}

static Evas_Object *_passwd_popup_wps_item_content_get(void *data,
		Evas_Object *obj, const char *part)
{
	__COMMON_FUNC_ENTER__;
	Evas_Object *icon = NULL;
	char *icon_path = NULL;
	Evas_Object *ic = NULL;

	ic = elm_layout_add(obj);
	retvm_if(NULL == ic, NULL);

	if (!strcmp(part, "elm.icon.left")) {
		elm_layout_file_set(ic, CUSTOM_EDITFIELD_PATH,
				"popup_image_list_layout");
		elm_object_part_text_set(ic, "elm.text", sc(PACKAGE, I18N_TYPE_WPS));
		evas_object_size_hint_weight_set(ic,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		/* image */
		icon = elm_image_add(ic);
		common_utils_get_wps_icon(&icon_path, wps_options_btn_hl);
		elm_image_file_set(icon, CUSTOM_EDITFIELD_PATH, icon_path);
		g_free(icon_path);
		elm_image_aspect_fixed_set(icon, EINA_FALSE);
		elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);

		if (wps_options_btn_hl == 1) {
			ea_theme_object_color_set(icon, "AO001iP");
		} else {
			ea_theme_object_color_set(icon, "AO001");
		}

		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(ic, "elm.swallow.content", icon);

		__COMMON_FUNC_EXIT__;

	}
	return ic;
}

static void _passwd_popup_wps_item_highlighted(void *data, Evas_Object *obj,
		void *ei)
{
	retm_if(NULL == ei);
	retm_if(NULL == data);

	Elm_Object_Item *item = ei;
	Eina_Bool is_focus_ui = EINA_FALSE;

	wps_options_btn_hl = TRUE;
	pswd_popup_t *pswd_popup_data = (pswd_popup_t *)data;

	if (item == pswd_popup_data->wps_options_item) {
		is_focus_ui = elm_win_focus_highlight_enabled_get(ug_get_window());
		if (!is_focus_ui)
			elm_object_focus_set(pswd_popup_data->entry, EINA_FALSE);
		elm_genlist_item_fields_update(item, "elm.icon",
				ELM_GENLIST_ITEM_FIELD_CONTENT);
		elm_genlist_item_fields_update(item, "elm.text",
				ELM_GENLIST_ITEM_FIELD_TEXT);
	}
}

static void _passwd_popup_wps_item_unhighlighted(void *data, Evas_Object *obj,
		void *ei)
{
	retm_if(NULL == ei);
	retm_if(NULL == data);

	wps_options_btn_hl = FALSE;
	Elm_Object_Item *item = ei;
	pswd_popup_t *pswd_popup_data = (pswd_popup_t *)data;

	if (item == pswd_popup_data->wps_options_item) {
		elm_genlist_item_fields_update(item, "elm.icon",
				ELM_GENLIST_ITEM_FIELD_CONTENT);
		elm_genlist_item_fields_update(item, "elm.text",
				ELM_GENLIST_ITEM_FIELD_TEXT);
	}
}

static int __rotate_popup_cb(enum appcore_rm rotmode, void *data,
		Eina_Bool is_wps, Eina_Bool is_setting)
{
	Evas_Object *layout = data;

	switch (rotmode) {
		case APPCORE_RM_LANDSCAPE_NORMAL:
		case APPCORE_RM_LANDSCAPE_REVERSE:
			elm_layout_file_set(layout, CUSTOM_EDITFIELD_PATH,
				EDJ_GRP_OUTER_LAYOUT_LANDSCAPE);
			INFO_LOG(UG_NAME_NORMAL, "Drawing landscape layout");
			break;

		case APPCORE_RM_PORTRAIT_NORMAL:
		case APPCORE_RM_PORTRAIT_REVERSE:
			INFO_LOG(UG_NAME_NORMAL, "Drawing portrait layout");
			if (is_wps) {
				if (is_setting) {
					elm_layout_file_set(layout,
						CUSTOM_EDITFIELD_PATH,
						EDJ_GRP_OUTER_LAYOUT_2_GL_ITEMS);
				} else {
					elm_layout_file_set(layout,
						CUSTOM_EDITFIELD_PATH,
						EDJ_GRP_OUTER_LAYOUT_1_GL_ITEMS);
				}
			} else {
				if (is_setting) {
					elm_layout_file_set(layout,
						CUSTOM_EDITFIELD_PATH,
						EDJ_GRP_OUTER_LAYOUT_1_GL_ITEMS);
				} else {
					elm_layout_file_set(layout,
						CUSTOM_EDITFIELD_PATH,
						EDJ_GRP_OUTER_LAYOUT_0_GL_ITEMS);
				}
			}
			break;

		case APPCORE_RM_UNKNOWN:
			break;
	}
	return 0;
}

static Evas_Object *__create_passwd_popup_main_layout(Evas_Object *parent,
		Eina_Bool is_wps, Eina_Bool is_setting)
{
	Evas_Object *layout = NULL;
	Eina_Bool is_portrait = EINA_FALSE;

	layout = elm_layout_add(parent);
	if (layout == NULL)
		return NULL;

	is_portrait = common_utils_is_portrait_mode();
	INFO_LOG(UG_NAME_NORMAL, "Is portrait - [%d]", is_portrait);

	if (is_portrait == EINA_TRUE) {
		INFO_LOG(UG_NAME_NORMAL, "Drawing portrait layout");

		if (is_wps) {
			if (is_setting) {
				elm_layout_file_set(layout, CUSTOM_EDITFIELD_PATH,
					EDJ_GRP_OUTER_LAYOUT_2_GL_ITEMS);
			} else {
				elm_layout_file_set(layout, CUSTOM_EDITFIELD_PATH,
					EDJ_GRP_OUTER_LAYOUT_1_GL_ITEMS);
			}
		} else {
			if (is_setting) {
				elm_layout_file_set(layout, CUSTOM_EDITFIELD_PATH,
					EDJ_GRP_OUTER_LAYOUT_1_GL_ITEMS);
			} else {
				elm_layout_file_set(layout, CUSTOM_EDITFIELD_PATH,
					EDJ_GRP_OUTER_LAYOUT_0_GL_ITEMS);
			}
		}
	} else {
		INFO_LOG(UG_NAME_NORMAL, "Drawing landscape layout");

		elm_layout_file_set(layout, CUSTOM_EDITFIELD_PATH,
				EDJ_GRP_OUTER_LAYOUT_LANDSCAPE);
	}
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	return layout;
}

static void _entry_edit_mode_show_cb(void *data, Evas *e, Evas_Object *obj,
		void *event_info)
{
	Eina_Bool is_focus_ui = EINA_FALSE;

	evas_object_event_callback_del(obj, EVAS_CALLBACK_SHOW,
			_entry_edit_mode_show_cb);

	/* Get focus UI state */
	is_focus_ui = elm_win_focus_highlight_enabled_get(ug_get_window());
	INFO_LOG(UG_NAME_NORMAL, "Focus UI state [%d]", is_focus_ui);

		elm_object_focus_set(obj, EINA_TRUE);
}

static void __popup_entry_activated_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_focus_set(obj, EINA_FALSE);
}

static Evas_Object* _gl_pswd_entry_item_content_get(void *data,
		Evas_Object *obj, const char *part)
{
	if (obj == NULL || data == NULL) {
		return NULL;
	}

	int return_key_type;
	Evas_Object *entry = NULL;
	Evas_Object *layout = NULL;

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	pswd_popup_t *pswd_popup_data = (pswd_popup_t *)data;

	if (!strcmp(part, "elm.icon.entry")) {
		layout = elm_layout_add(obj);
		elm_layout_file_set(layout, CUSTOM_EDITFIELD_PATH, "entry_style");
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		entry = elm_entry_add(layout);
		elm_entry_password_set(entry, EINA_TRUE);
		ea_entry_selection_back_event_allow_set(entry, EINA_TRUE);
		evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
		if (!strcmp(pswd_popup_data->str_pkg_name, "wifi-qs")) {
			elm_entry_input_panel_imdata_set(entry,
					"type=systempopup", 16);
		}
		elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
		elm_entry_input_panel_layout_set(entry,
				ELM_INPUT_PANEL_LAYOUT_PASSWORD);
		elm_object_part_text_set(entry, "elm.guide",
			sc(PACKAGE, I18N_TYPE_Password));
		elm_object_signal_emit(entry, "elm,action,hide,search_icon", "");

		limit_filter_data.max_char_count = PASSWORD_LENGTH;
		elm_entry_markup_filter_append(entry,
		elm_entry_filter_limit_size, &limit_filter_data);

		return_key_type = ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE;
		elm_entry_input_panel_return_key_type_set(entry, return_key_type);
		elm_entry_input_panel_return_key_disabled_set(entry, EINA_TRUE);
		evas_object_smart_callback_add(entry, "activated",
				__popup_entry_activated_cb, data);
		evas_object_smart_callback_add(entry, "changed",
				__popup_entry_changed_cb, data);
		evas_object_smart_callback_add(entry, "preedit,changed",
				__popup_entry_changed_cb, data);
		evas_object_smart_callback_add(entry, "maxlength,reached",
				__popup_entry_maxlength_reached, NULL);
		evas_object_event_callback_add(entry, EVAS_CALLBACK_SHOW,
				_entry_edit_mode_show_cb, NULL);

		pswd_popup_data->entry = entry;

		elm_object_part_content_set(layout, "entry_part", entry);

		return layout;
	}

	return NULL;
}

static void _chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	if (obj == NULL) {
		return;
	}

	Eina_Bool state = elm_check_state_get(obj);
	if (state) {
		elm_entry_password_set((Evas_Object *)data, EINA_FALSE);
	} else {
		elm_entry_password_set((Evas_Object *)data, EINA_TRUE);
	}
}
static char *_gl_pswd_check_box_item_text_get(void *data, Evas_Object *obj,
		const char *part)
{
	__COMMON_FUNC_ENTER__;

	if (!strcmp(part, "elm.text.main.left")) {
		char buf[1024];
		snprintf(buf, 1023, "%s", sc(PACKAGE, I18N_TYPE_Show_password));
		return strdup(buf);
	}
	return NULL;

}

static Evas_Object *_gl_pswd_check_box_item_content_get(void *data,
		Evas_Object *obj, const char *part)
{
	Evas_Object *layout = NULL; //Evas_Object * ic = NULL;
	Evas_Object *check = NULL; //Evas_Object *c = NULL;
	pswd_popup_t *pswd_popup_data = (pswd_popup_t *)data;
	Eina_Bool is_focus_ui = EINA_FALSE;

	layout = elm_layout_add(obj); //ic = elm_layout_add(obj);

	if(!strcmp(part, "elm.icon.right")) {
		elm_layout_theme_set(layout, "layout", "list/C/type.2", "default");

		check = elm_check_add(obj);
		evas_object_propagate_events_set(check, EINA_FALSE);

		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_content_set(layout, "elm.swallow.content", check);
		evas_object_smart_callback_add(check, "changed",
				_chk_changed_cb, pswd_popup_data->entry);

		/* Get focus UI state */
		is_focus_ui = elm_win_focus_highlight_enabled_get(ug_get_window());
		INFO_LOG(UG_NAME_NORMAL, "Focus UI state [%d]", is_focus_ui);

		if (is_focus_ui == EINA_FALSE)
			elm_object_focus_allow_set(check, EINA_FALSE);

		return layout;
	}
	return NULL;
}

static void _gl_pswd_check_box_sel(void *data, Evas_Object *obj, void *ei)
{
	__COMMON_FUNC_ENTER__;
	Elm_Object_Item *item = NULL;

	item = (Elm_Object_Item *)ei;
	if (item == NULL) {
		return;
	}

	pswd_popup_t *pswd_popup_data = (pswd_popup_t *)data;

	Evas_Object *content = elm_object_item_part_content_get(ei, "elm.icon.right");
	Evas_Object *ck = elm_object_part_content_get(content, "elm.swallow.content");

	elm_genlist_item_selected_set(item, EINA_FALSE);

	Eina_Bool state = elm_check_state_get(ck);
	elm_check_state_set(ck, !state);
	if (pswd_popup_data) {
		_chk_changed_cb(pswd_popup_data->entry, ck, NULL);
	}
}

pswd_popup_t *create_passwd_popup(Evas_Object *win_main,
		const char *pkg_name, pswd_popup_create_req_data_t *popup_info)
{
	Evas_Object *passpopup = NULL;
	Evas_Object *eo = NULL;
	Evas_Object *ao = NULL;
	Evas_Object *outer_ly = NULL;
	Evas_Object *genlist = NULL;
	Evas_Object *btn_ok = NULL;
	Eina_Bool is_setting = EINA_FALSE;
	Eina_Bool is_focus_ui = EINA_FALSE;

	__COMMON_FUNC_ENTER__;

	if (!win_main || !popup_info || !pkg_name) {
		return NULL;
	}

	pswd_popup_t *pswd_popup_data = g_new0(pswd_popup_t, 1);

	if (popup_info->ap) {
		if (WIFI_ERROR_NONE !=
				wifi_ap_clone(&(pswd_popup_data->ap), popup_info->ap)) {
			g_free(pswd_popup_data);

			return NULL;
		}
	} else {
		/* Hidden Wi-Fi network does not have handle */
	}

	/* Get focus UI state */
	is_focus_ui = elm_win_focus_highlight_enabled_get(ug_get_window());
	INFO_LOG(UG_NAME_NORMAL, "Focus UI state [%d]", is_focus_ui);

	wps_options_btn_hl = FALSE;
	/* Lets disable the scan updates so that the UI is not refreshed un necessarily */
	wlan_manager_disable_scan_result_update();

	popup_btn_info_t popup_btn_data;
	memset(&popup_btn_data, 0, sizeof(popup_btn_data));

	popup_btn_data.title_txt = popup_info->title;
	popup_btn_data.btn1_cb = popup_info->cancel_cb;
	popup_btn_data.btn1_data = popup_info->cb_data;
	popup_btn_data.btn1_txt = sc(pkg_name, I18N_TYPE_Cancel);
	popup_btn_data.btn2_cb = popup_info->ok_cb;
	popup_btn_data.btn2_data = popup_info->cb_data;
	popup_btn_data.btn2_txt = sc(pkg_name, I18N_TYPE_Connect);
	popup_btn_data.popup_with_entry = true;

	passpopup = common_utils_show_info_popup(win_main, &popup_btn_data);

	if (!passpopup) {
		ERROR_LOG(UG_NAME_ERR, "Could not initialize popup");
		return NULL;
	}

	pswd_popup_data->win = win_main;
	pswd_popup_data->str_pkg_name = pkg_name;
	pswd_popup_data->popup = passpopup;
	pswd_popup_data->sec_type = popup_info->sec_type;

	if (popup_info->setting_cb != NULL)
		is_setting = EINA_TRUE;

	/* Hide the Okay button until the password is entered */
	btn_ok = elm_object_part_content_get(passpopup, "button2");
	elm_object_disabled_set(btn_ok, TRUE);

	eo = elm_layout_edje_get(passpopup);
	const Evas_Object *po = edje_object_part_object_get(eo, "elm.text.title");
	ao = elm_access_object_get(po);
	elm_access_info_set(ao, ELM_ACCESS_INFO, popup_info->title);

	outer_ly = __create_passwd_popup_main_layout(passpopup,
			popup_info->show_wps_btn, is_setting);

	common_utils_set_rotate_cb(__rotate_popup_cb, outer_ly,
			popup_info->show_wps_btn, is_setting);

	genlist = elm_genlist_add(outer_ly);
	evas_object_size_hint_weight_set(genlist,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* Entry genlist item */
	g_pswd_entry_itc.item_style = "entry.icon";
	g_pswd_entry_itc.func.text_get = NULL;
	g_pswd_entry_itc.func.content_get = _gl_pswd_entry_item_content_get;
	g_check_box_itc.func.state_get = NULL;
	g_check_box_itc.func.del = NULL;

	elm_genlist_item_append(genlist, &g_pswd_entry_itc, pswd_popup_data,
			NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	/* Checkbox genlist item */
	g_check_box_itc.item_style = "1line";
	g_check_box_itc.func.text_get = _gl_pswd_check_box_item_text_get;
	g_check_box_itc.func.content_get = _gl_pswd_check_box_item_content_get;
	g_check_box_itc.func.state_get = NULL;
	g_check_box_itc.func.del = NULL;

	elm_genlist_item_append(genlist, &g_check_box_itc, pswd_popup_data,
			NULL, ELM_GENLIST_ITEM_NONE,
			_gl_pswd_check_box_sel, pswd_popup_data);

	if (is_setting) {
		/* Network info genlist item */
		g_info_itc.item_style = "1line";
		g_info_itc.func.text_get = _wifi_network_info_item_text_get;
		g_info_itc.func.content_get = NULL;
		g_info_itc.func.state_get = NULL;
		g_info_itc.func.del = NULL;

		elm_genlist_item_append(genlist, &g_info_itc, pkg_name, NULL,
				ELM_GENLIST_ITEM_NONE, popup_info->setting_cb,
				popup_info->cb_data);
	}

	if (popup_info->show_wps_btn) {
		/* WPS options genlist item */
		g_wps_itc.item_style = "1line";
		g_wps_itc.func.text_get = NULL;
		g_wps_itc.func.content_get = _passwd_popup_wps_item_content_get;
		g_wps_itc.func.state_get = NULL;
		g_wps_itc.func.del = NULL;

		evas_object_smart_callback_add(genlist, "highlighted",
				_passwd_popup_wps_item_highlighted, pswd_popup_data);
		evas_object_smart_callback_add(genlist, "unhighlighted",
				_passwd_popup_wps_item_unhighlighted, pswd_popup_data);
		pswd_popup_data->wps_options_item = elm_genlist_item_append(genlist,
				&g_wps_itc, NULL, NULL, ELM_GENLIST_ITEM_NONE,
				popup_info->wps_btn_cb, popup_info->cb_data);
	}

	elm_genlist_realization_mode_set(genlist, EINA_TRUE);
	evas_object_show(genlist);
	elm_object_part_content_set(outer_ly, "elm.swallow.layout", genlist);
	evas_object_show(outer_ly);

	elm_object_content_set(passpopup, outer_ly);
	evas_object_show(passpopup);

		elm_object_focus_set(pswd_popup_data->entry, EINA_TRUE);

	__COMMON_FUNC_EXIT__;
	return pswd_popup_data;
}

char *passwd_popup_get_txt(pswd_popup_t *pswd_popup_data)
{
	if (pswd_popup_data) {
		Evas_Object *entry = pswd_popup_data->entry;
		return elm_entry_markup_to_utf8(elm_entry_entry_get(entry));
	}

	return NULL;
}

wifi_ap_h passwd_popup_get_ap(pswd_popup_t *pswd_popup_data)
{
	if (pswd_popup_data) {
		return pswd_popup_data->ap;
	}

	return NULL;
}

void current_popup_free(pswd_popup_t *pswd_popup_data, popup_type_t type)
{
	if (pswd_popup_data == NULL) {
		return;
	}

	int rotation = -1;
	char buf[1024];
	Evas_Object *curr_popup = NULL;

	if (type == POPUP_WPS_OPTIONS) {
		if (pswd_popup_data->wps_options_popup) {
			evas_object_hide(pswd_popup_data->wps_options_popup);
			evas_object_del(pswd_popup_data->wps_options_popup);
			pswd_popup_data->wps_options_popup = NULL;
		}
		evas_object_show(pswd_popup_data->popup);
		curr_popup = pswd_popup_data->popup;
	}

	rotation = common_utils_get_rotate_angle(APPCORE_RM_UNKNOWN);
	if (rotation != -1) {
		g_snprintf(buf, sizeof(buf), "elm,state,orient,%d", rotation);
		SECURE_INFO_LOG(UG_NAME_NORMAL, "Rotation signal - %s", buf);
		elm_layout_signal_emit(curr_popup, buf, "elm");
	}
}

void passwd_popup_hide(pswd_popup_t *pswd_popup_data)
{
	if (pswd_popup_data == NULL) {
		return;
	}

	if (pswd_popup_data->popup != NULL) {
		evas_object_hide(pswd_popup_data->popup);
	}
}

void passwd_popup_show(pswd_popup_t *pswd_popup_data)
{
	if (pswd_popup_data == NULL) {
		return;
	}

	if (pswd_popup_data->popup != NULL) {
		evas_object_show(pswd_popup_data->popup);
		elm_object_focus_set(pswd_popup_data->popup, EINA_TRUE);
	}
}

void passwd_popup_free(pswd_popup_t *pswd_popup_data)
{
	__COMMON_FUNC_ENTER__;

	if (pswd_popup_data == NULL) {
		return;
	}

	if (pswd_popup_data->info_popup != NULL) {
		evas_object_del(pswd_popup_data->info_popup);
		pswd_popup_data->info_popup = NULL;
		INFO_LOG(UG_NAME_NORMAL, "info popup deleted");
	}

	if (pswd_popup_data->pbc_popup_data) {
		__common_pbc_popup_destroy(pswd_popup_data->pbc_popup_data);
		pswd_popup_data->pbc_popup_data = NULL;
		INFO_LOG(UG_NAME_NORMAL, "wps pbc popup deleted");
	}

	if (pswd_popup_data->popup != NULL) {
		evas_object_del(pswd_popup_data->popup);
		pswd_popup_data->popup = NULL;
		INFO_LOG(UG_NAME_NORMAL, "password popup deleted");
	}

	if (pswd_popup_data->wps_options_popup != NULL) {
		evas_object_del(pswd_popup_data->wps_options_popup);
		pswd_popup_data->wps_options_popup = NULL;
		INFO_LOG(UG_NAME_NORMAL, "wps option popup deleted");
	}

	/* Hidden Wi-Fi network does not have handle */
	if (pswd_popup_data->ap)
		wifi_ap_destroy(pswd_popup_data->ap);

	g_free(pswd_popup_data);

	common_utils_set_rotate_cb(NULL, NULL, EINA_FALSE, EINA_FALSE);

	/* A delay is needed to get the smooth Input panel closing animation effect */
	common_util_managed_ecore_scan_update_timer_add(0.1,
			_enable_scan_updates_cb, NULL);

	__COMMON_FUNC_EXIT__;
}
