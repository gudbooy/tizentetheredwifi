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

#include <vconf.h>
#include <vconf-keys.h>
#include <efl_assist.h>

#include "ug_wifi.h"
#include "view_advanced.h"
#include "i18nmanager.h"
#include "viewer_manager.h"
#include "winset_popup.h"
#include "common_utils.h"

#define VCONF_SLEEP_POLICY "file/private/wifi/sleep_policy"

struct _private_data {
	Evas_Object *list;
	Elm_Object_Item *item_network_noti;
	Elm_Object_Item *item_keep_wifi_switch;
	Evas_Object *keep_wifi_radio_group;
	Evas_Object *keep_wifi_popup;

	Elm_Object_Item *item_sort_by;
	Evas_Object *sort_by_radio_group;
	Evas_Object *sort_by_popup;
};

struct _private_data g_pd;

/* Prototype */
static char *_gl_text_get(void *data, Evas_Object *obj,	const char *part);
static char *_gl_network_notification_text_get(void *data, Evas_Object *obj,	const char *part);
static char *_gl_sort_by_text_get(void *data, Evas_Object *obj, const char *part);
static char *_gl_sort_by_sub_text_get(void *data, Evas_Object *obj, const char *part);
static char *_gl_never_text_get(void *data, Evas_Object *obj,const char *part);
static char *_gl_keep_wifi_on_during_sleep_text_get(void *data, Evas_Object *obj,const char *part);
static Evas_Object *_gl_content_get_network_noti(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_gl_content_get_keep_wifi_sub(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_gl_content_get_sort_by_sub(void *data,	Evas_Object *obj, const char *part);

/* Global variables for elm_genlist itc */
static Elm_Genlist_Item_Class itc_network_noti = {
		.item_style = "multiline_sub.main.1icon",
		.func.text_get = _gl_network_notification_text_get,
		.func.content_get = _gl_content_get_network_noti
};

static Elm_Genlist_Item_Class itc_keep_wifi = {
		.item_style = "2line.top",
		.func.text_get = _gl_keep_wifi_on_during_sleep_text_get,
};

static Elm_Genlist_Item_Class itc_keep_wifi_sub = {
		.item_style = "1line",
		.func.text_get = _gl_text_get,
		.func.content_get = _gl_content_get_keep_wifi_sub,
};

static Elm_Genlist_Item_Class itc_keep_wifi_sub_never = {
		.item_style = "2line.top",
		.func.text_get = _gl_never_text_get,
		.func.content_get = _gl_content_get_keep_wifi_sub,
};

static Elm_Genlist_Item_Class itc_sort_by = {
		.item_style = "2line.top",
		.func.text_get = _gl_sort_by_text_get,
};

static Elm_Genlist_Item_Class itc_sort_by_sub = {
		.item_style = "1line",
		.func.text_get = _gl_sort_by_sub_text_get,
		.func.content_get = _gl_content_get_sort_by_sub,
};

static int _convert_wifi_keep_value_to_vconf(int i18n_key)
{
	switch (i18n_key) {
	case I18N_TYPE_Always:
		return 0;

	case I18N_TYPE_Plugged:
		return 1;

	case I18N_TYPE_Donot_Use:
		return 2;
	}

	return -1;
}

static int _convert_vconf_to_wifi_keep_value(int vconf_value)
{
	switch (vconf_value) {
	case 0:
		return I18N_TYPE_Always;

	case 1:
		return I18N_TYPE_Plugged;

	case 2:
		return I18N_TYPE_Donot_Use;
	}

	return -1;
}

int _convert_sort_by_value_to_vconf(int i18n_key)
{
	switch (i18n_key) {
	case I18N_TYPE_Alphabetical:
		return 0;

	case I18N_TYPE_Signal_Strength:
		return 1;
	}

	return -1;
}

int _convert_vconf_to_sort_by_value(int vconf_value)
{
	switch (vconf_value) {
	case 0:
		return I18N_TYPE_Alphabetical;

	case 1:
		return I18N_TYPE_Signal_Strength;
	}

	return -1;
}

static char *_gl_sort_by_text_get(void *data, Evas_Object *obj,
		const char *part)
{
	char buf[1024];
	if (!g_strcmp0(part, "elm.text.main.left.top")||!g_strcmp0(part, "elm.text.main.left")) {
		if ((int) data != 0) {
			snprintf(buf, 1023, "%s", sc(PACKAGE, (int) data));
			return strdup(buf);
		}
	} else if (!g_strcmp0(part, "elm.text.sub.left.bottom")) {
		int value;

		value = _convert_vconf_to_sort_by_value(
				common_util_get_system_registry(VCONF_SORT_BY));
		if (value >= 0) {
			snprintf(buf, 1023, "%s", sc(PACKAGE, value));
			return strdup(buf);
		}
	}

	return NULL;
}

static char *_gl_sort_by_sub_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	if (!g_strcmp0(part, "elm.text.main.left")) {
		if ((int) data != 0) {
			snprintf(buf, 1023, "%s", sc(PACKAGE, (int) data));
			return strdup(buf);
		}
	}
	return NULL;
}

static char *_gl_network_notification_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];

	if (!strcmp(part, "elm.text.main")) {
		snprintf(buf, 1023, "%s", sc(PACKAGE, I18N_TYPE_Network_notification));
		return strdup(buf);
	} else if (!strcmp(part, "elm.text.multiline")) {
		snprintf(buf, 1023, "%s", sc(PACKAGE, I18N_TYPE_Network_notify_me_later));
		return strdup(buf);
	}
	return NULL;
}

static char *_gl_text_get(void *data, Evas_Object *obj,
		const char *part)
{
	char buf[1024];
	if (!strcmp(part, "elm.text.main.left")) {
		if ((int) data != 0) {
			snprintf(buf, 1023, "%s", sc(PACKAGE, (int) data));
			return strdup(buf);
		}
	}

	return NULL;
}

static char *_gl_keep_wifi_on_during_sleep_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];

	if(!strcmp(part, "elm.text.main.left.top")) {
		snprintf(buf, 1023, "%s", sc(PACKAGE, I18N_TYPE_Keep_WIFI_on_during_sleep));
		return strdup(buf);
	}

	if(!strcmp(part, "elm.text.sub.left.bottom")) {
		int value;
		value = _convert_vconf_to_wifi_keep_value(
				common_util_get_system_registry(VCONF_SLEEP_POLICY));
		if (value >= 0) {
			if (value == I18N_TYPE_Donot_Use) {
				snprintf(buf, 1023, "%s", sc(PACKAGE, I18N_TYPE_Donot_Use));
				return strdup(buf);
			} else {
				snprintf(buf, 1023, "%s", sc(PACKAGE, value));
				return strdup(buf);
			}
		}
	}
	return NULL;
}
static char *_gl_never_text_get(void *data, Evas_Object *obj,
		const char *part)
{
	if (!g_strcmp0(part, "elm.text.main.left.top")) {
		return g_strdup(sc(PACKAGE, I18N_TYPE_Donot_Use));
	} else if (!g_strcmp0(part, "elm.text.sub.left.bottom")) {
		return g_strdup(sc(PACKAGE, I18N_TYPE_Increases_Data_Usage));
	}

	return NULL;
}

static void _gl_changed_network_noti(void *data, Evas_Object *obj,
		void *event_info)
{
	int value;
	const char *object_type;

	__COMMON_FUNC_ENTER__;

	object_type = evas_object_type_get(obj);
	if (!object_type) {
		INFO_LOG(UG_NAME_SCAN, "object_type is NULL");
		return;
	}

	value = common_util_get_system_registry(VCONFKEY_WIFI_ENABLE_QS);

	if (g_strcmp0(object_type, "elm_check") == 0) {
		Eina_Bool check_enable = elm_check_state_get(obj);

		if (check_enable == TRUE) {
			value = VCONFKEY_WIFI_QS_ENABLE;
		} else {
			value = VCONFKEY_WIFI_QS_DISABLE;
		}
	} else if (g_strcmp0(object_type, "elm_genlist") == 0) {
		if (value == VCONFKEY_WIFI_QS_ENABLE) {
			value = VCONFKEY_WIFI_QS_DISABLE;
		} else {
			value = VCONFKEY_WIFI_QS_ENABLE;
		}
	}

	common_util_set_system_registry(VCONFKEY_WIFI_ENABLE_QS, value);
	if (value == VCONFKEY_WIFI_QS_DISABLE)
		common_utils_send_message_to_net_popup(NULL, NULL,
					"del_found_ap_noti", NULL);

	elm_genlist_item_update(g_pd.item_network_noti);
	elm_genlist_item_selected_set(g_pd.item_network_noti, EINA_FALSE);

	__COMMON_FUNC_EXIT__;
}

static Evas_Object *_gl_content_get_network_noti(void *data,
		Evas_Object *obj, const char *part)
{
	Evas_Object *ao = NULL;
	Evas_Object *toggle_btn;
	Evas_Object *content = NULL;
	int ret;
	char buf[100];

	content = elm_layout_add(obj);

	if (!strncmp(part, "elm.icon", strlen(part))) {
		elm_layout_theme_set(content, "layout", "list/C/type.3", "default");
		toggle_btn = elm_check_add(obj);
		evas_object_size_hint_align_set(toggle_btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(toggle_btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_content_set(content, "elm.swallow.content",toggle_btn);

		elm_object_style_set(toggle_btn, "on&off");
		evas_object_propagate_events_set(toggle_btn, EINA_FALSE);

		ao = elm_object_item_access_object_get(g_pd.item_network_noti);
		if (ao) {
			g_snprintf(buf, sizeof(buf), "%s%s%s",
					sc(PACKAGE, I18N_TYPE_On),
					sc(PACKAGE, I18N_TYPE_Off),
					sc(PACKAGE, I18N_TYPE_Button));
			elm_access_info_set(ao, ELM_ACCESS_TYPE, buf);
		}

		ret = common_util_get_system_registry(VCONFKEY_WIFI_ENABLE_QS);
		switch (ret) {
			case 1:
				if (ao)
					elm_access_info_set(ao, ELM_ACCESS_STATE, sc(PACKAGE, I18N_TYPE_On));

				elm_check_state_set(toggle_btn, EINA_TRUE);
		evas_object_smart_callback_add(toggle_btn, "changed", _gl_changed_network_noti, NULL);
				break;
			case 0:
				if (ao)
					elm_access_info_set(ao, ELM_ACCESS_STATE, sc(PACKAGE, I18N_TYPE_Off));

				elm_check_state_set(toggle_btn, EINA_FALSE);
		evas_object_smart_callback_add(toggle_btn, "changed", _gl_changed_network_noti, NULL);
				break;
			default:
				ERROR_LOG(COMMON_NAME_ERR, "Setting fail!!");
				break;
		}
	}
	return content;
}

static void _gl_changed_keep_wifi_sub(void *data, Evas_Object *obj,
		void *event_info)
{
	Elm_Object_Item *item;
	const char *object_type;
	int value;
	Elm_Object_Item *parent = NULL;

	__COMMON_FUNC_ENTER__;

	item = (Elm_Object_Item *) event_info;
	object_type = evas_object_type_get(obj);
	if (!object_type) {
		INFO_LOG(UG_NAME_SCAN, "object_type is NULL");
		return;
	}

	value = _convert_wifi_keep_value_to_vconf((int) data);

	if (value >= 0) {
		common_util_set_system_registry(VCONF_SLEEP_POLICY, value);
	}

	if (g_strcmp0(object_type, "elm_genlist") == 0) {
		elm_radio_value_set(g_pd.keep_wifi_radio_group, (int) data);
	}

	if (item != NULL) {
		elm_genlist_item_update(item);
		elm_genlist_item_selected_set(item, EINA_FALSE);

		parent = elm_genlist_item_parent_get(item);
		elm_genlist_item_update(parent);
	}

	elm_genlist_item_update(g_pd.item_keep_wifi_switch);

	evas_object_del(g_pd.keep_wifi_popup);
	__COMMON_FUNC_EXIT__;
}

static Evas_Object *_gl_content_get_keep_wifi_sub(void *data,
		Evas_Object *obj, const char *part)
{
	__COMMON_FUNC_ENTER__;
	Evas_Object *radio;
	int value;
	Evas_Object *content = elm_layout_add(obj);

	if (!g_pd.keep_wifi_radio_group) {
		g_pd.keep_wifi_radio_group = elm_radio_add(obj);
		elm_radio_state_value_set (g_pd.keep_wifi_radio_group, -1);
	}

	if (!strcmp(part, "elm.icon.2")) {
		elm_layout_theme_set(content, "layout", "list/C/type.2", "default");
		radio = elm_radio_add(content);
		elm_access_object_unregister(radio);
		elm_radio_state_value_set(radio, (int) data);
		elm_radio_group_add(radio, g_pd.keep_wifi_radio_group);

		elm_layout_content_set(content, "elm.swallow.content", radio);

		value = _convert_vconf_to_wifi_keep_value(
				common_util_get_system_registry(VCONF_SLEEP_POLICY));
		if (value == (int) data) {
			elm_radio_value_set(g_pd.keep_wifi_radio_group, (int) data);
		}

		evas_object_smart_callback_add(radio, "changed", _gl_changed_keep_wifi_sub, data);
	}

	return content;
	__COMMON_FUNC_EXIT__;
}

static void _gl_changed_sort_by_sub(void *data, Evas_Object *obj,
		void *event_info)
{
	Elm_Object_Item *item;
	const char *object_type;
	int value;
	Elm_Object_Item *parent = NULL;

	__COMMON_FUNC_ENTER__;

	item = (Elm_Object_Item *) event_info;
	object_type = evas_object_type_get(obj);
	if (!object_type) {
		INFO_LOG(UG_NAME_SCAN, "object_type is NULL");
		return;
	}

	value = _convert_sort_by_value_to_vconf((int) data);
	if (value >= 0) {
		common_util_set_system_registry(VCONF_SORT_BY, value);
	}

	if (g_strcmp0(object_type, "elm_genlist") == 0) {
		elm_radio_value_set(g_pd.sort_by_radio_group, (int) data);
	}

	if (item != NULL) {
		elm_genlist_item_update(item);
		elm_genlist_item_selected_set(item, EINA_FALSE);

		parent = elm_genlist_item_parent_get(item);
		elm_genlist_item_update(parent);
	}

	elm_genlist_item_update(g_pd.item_sort_by);

	evas_object_del(g_pd.sort_by_popup);

	__COMMON_FUNC_EXIT__;
}

static Evas_Object *_gl_content_get_sort_by_sub(void *data,
		Evas_Object *obj, const char *part)
{
	__COMMON_FUNC_ENTER__;

	Evas_Object *radio;
	int value;
	Evas_Object *content = elm_layout_add(obj);

	if (!g_pd.sort_by_radio_group) {
		g_pd.sort_by_radio_group = elm_radio_add(obj);
		elm_radio_state_value_set (g_pd.sort_by_radio_group, -1);
	}

	 if (!strcmp(part, "elm.icon.2")) {
		 elm_layout_theme_set(content, "layout", "list/C/type.2", "default");
		 radio = elm_radio_add(content);
		 elm_access_object_unregister(radio);
		 elm_radio_state_value_set(radio, (int) data);
		 elm_radio_group_add(radio, g_pd.sort_by_radio_group);

		 elm_layout_content_set(content, "elm.swallow.content", radio);

		 value = _convert_vconf_to_sort_by_value(
				 common_util_get_system_registry(VCONF_SORT_BY));

		 if (value == (int) data) {
			 elm_radio_value_set(g_pd.sort_by_radio_group, (int) data);
		 }

		 evas_object_smart_callback_add(radio, "changed", _gl_changed_sort_by_sub, data);
	 }

	return content;

	__COMMON_FUNC_EXIT__;
}

static void _gl_keep_wifi(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;
	Evas_Object *genlist;
	Evas_Object *box;
	Evas_Object *win = data;

	g_pd.keep_wifi_popup = elm_popup_add(win);
	ea_object_event_callback_add(g_pd.keep_wifi_popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	elm_object_part_text_set(g_pd.keep_wifi_popup, "title,text", sc(PACKAGE, I18N_TYPE_Keep_WIFI_on_during_sleep));
	evas_object_size_hint_weight_set(g_pd.keep_wifi_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(g_pd.keep_wifi_popup, "block,clicked", _gl_changed_keep_wifi_sub, win);

	/* box */
	box = elm_box_add(g_pd.keep_wifi_popup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* genlist */
	genlist = elm_genlist_add(box);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* radio */
	g_pd.keep_wifi_radio_group = elm_radio_add(genlist);
	evas_object_data_set(genlist, "radio", g_pd.keep_wifi_radio_group);

	elm_genlist_item_append(genlist, &itc_keep_wifi_sub,
			(const void *) I18N_TYPE_Always, NULL, ELM_GENLIST_ITEM_NONE,
			_gl_changed_keep_wifi_sub, (const void *)I18N_TYPE_Always);

	elm_genlist_item_append(genlist, &itc_keep_wifi_sub,
			(const void *) I18N_TYPE_Plugged, NULL, ELM_GENLIST_ITEM_NONE,
			_gl_changed_keep_wifi_sub, (const void *)I18N_TYPE_Plugged);

	elm_genlist_item_append(genlist, &itc_keep_wifi_sub_never,
			(const void *) I18N_TYPE_Donot_Use, NULL, ELM_GENLIST_ITEM_NONE,
			_gl_changed_keep_wifi_sub, (const void *)I18N_TYPE_Donot_Use);

	evas_object_show(genlist);

	elm_box_pack_end(box, genlist);
	evas_object_size_hint_min_set(box, -1, ELM_SCALE_SIZE(288));
	elm_object_content_set(g_pd.keep_wifi_popup, box);
	evas_object_show(g_pd.keep_wifi_popup);

	elm_genlist_item_selected_set(g_pd.item_keep_wifi_switch, EINA_FALSE);
	__COMMON_FUNC_EXIT__;
}

static void _gl_sort_by(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;
	Evas_Object *genlist = NULL;
	Evas_Object *box = NULL;
	Evas_Object *win = data;

	g_pd.sort_by_popup = elm_popup_add(win);
	ea_object_event_callback_add(g_pd.sort_by_popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	elm_object_part_text_set(g_pd.sort_by_popup, "title,text", sc(PACKAGE, I18N_TYPE_Sort_by));
	evas_object_size_hint_weight_set(g_pd.sort_by_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(g_pd.sort_by_popup, "block,clicked", _gl_changed_sort_by_sub, win);

	/* box */
	box = elm_box_add(g_pd.sort_by_popup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* genlist */
	genlist = elm_genlist_add(box);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* radio */
	g_pd.sort_by_radio_group = elm_radio_add(genlist);
	evas_object_data_set(genlist, "radio", g_pd.sort_by_radio_group);

	elm_genlist_item_append(genlist, &itc_sort_by_sub,
			(const void *) I18N_TYPE_Alphabetical, NULL, ELM_GENLIST_ITEM_NONE,
			_gl_changed_sort_by_sub, (const void *)I18N_TYPE_Alphabetical);

	elm_genlist_item_append(genlist, &itc_sort_by_sub,
			(const void *) I18N_TYPE_Signal_Strength, NULL, ELM_GENLIST_ITEM_NONE,
			_gl_changed_sort_by_sub, (const void *)I18N_TYPE_Signal_Strength);

	evas_object_show(genlist);

	elm_box_pack_end(box, genlist);
	evas_object_size_hint_min_set(box, -1, ELM_SCALE_SIZE(192));
	elm_object_content_set(g_pd.sort_by_popup, box);
	evas_object_show(g_pd.sort_by_popup);

	elm_genlist_item_selected_set(g_pd.item_sort_by, EINA_FALSE);
	__COMMON_FUNC_EXIT__;
}

static Evas_Object *_create_list(Evas_Object *parent)
{
	Evas_Object *gl;
	int wifi_state = 0;

	vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);
	INFO_LOG(UG_NAME_NORMAL, "Wi-Fi state %d", wifi_state);

	gl = elm_genlist_add(parent);
	assertm_if(NULL == gl, "NULL!!");

	elm_object_style_set(gl, "dialogue");
	elm_genlist_mode_set(gl, ELM_LIST_LIMIT);

	evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_genlist_mode_set(gl, ELM_LIST_COMPRESS);

	/* Network Notification */
	g_pd.item_network_noti = elm_genlist_item_append(gl, &itc_network_noti,
			(const void *) I18N_TYPE_Network_notification, NULL,
			ELM_GENLIST_ITEM_NONE, _gl_changed_network_noti, NULL);

	/* Keep WI-FI on during sleep */
	g_pd.item_keep_wifi_switch = elm_genlist_item_append(gl, &itc_keep_wifi,
			(const void *) I18N_TYPE_Keep_WIFI_on_during_sleep, NULL,
			ELM_GENLIST_ITEM_NONE, _gl_keep_wifi, elm_object_top_widget_get(parent));

	/* Sort By */
	g_pd.item_sort_by= elm_genlist_item_append(gl, &itc_sort_by,
			(const void *) I18N_TYPE_Sort_by, NULL,
			ELM_GENLIST_ITEM_NONE, _gl_sort_by, elm_object_top_widget_get(parent));

	evas_object_show(gl);

	return gl;
}

static void __vconf_noti_change_cb(keynode_t *node, void *user_data)
{
	int state = 0;

	vconf_get_int(VCONFKEY_WIFI_ENABLE_QS, &state);
	INFO_LOG(UG_NAME_NORMAL, "New notification option - %d", state);

	if (g_pd.item_network_noti != NULL) {
		elm_genlist_item_update(g_pd.item_network_noti);
	}
}

static void __vconf_sleep_change_cb(keynode_t *node, void *user_data)
{
	int state = 0;

	vconf_get_int(VCONF_SLEEP_POLICY, &state);
	INFO_LOG(UG_NAME_NORMAL, "New sleep policy - %d", state);

	if (g_pd.item_keep_wifi_switch != NULL) {
		elm_genlist_item_update(g_pd.item_keep_wifi_switch);
	}
}

static void __vconf_sort_change_cb(keynode_t *node, void *user_data)
{
	int state = 0;

	vconf_get_int(VCONF_SORT_BY, &state);
	INFO_LOG(UG_NAME_NORMAL, "New sort by option - %d", state);

	if (g_pd.item_sort_by != NULL) {
		elm_genlist_item_update(g_pd.item_sort_by);
	}
}

static Eina_Bool __back_key_cb(void *data, Elm_Object_Item *it)
{
	__COMMON_FUNC_ENTER__;

	/* Delete vconf listeners for the different options */
	vconf_ignore_key_changed(VCONFKEY_WIFI_ENABLE_QS, __vconf_noti_change_cb);
	vconf_ignore_key_changed(VCONF_SLEEP_POLICY, __vconf_sleep_change_cb);
	vconf_ignore_key_changed(VCONF_SORT_BY, __vconf_sort_change_cb);

	__COMMON_FUNC_EXIT__;

	return EINA_TRUE;
}

void view_advanced(void)
{
	Evas_Object *layout = NULL;
	Evas_Object *navi_frame = NULL;
	Elm_Object_Item *navi_it = NULL;
	char title[100];

	__COMMON_FUNC_ENTER__;

	memset (&g_pd, 0, sizeof(struct _private_data));

	navi_frame = viewer_manager_get_naviframe();
	if (navi_frame == NULL) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed to get naviframe");
		return;
	}

	layout = common_utils_create_layout(navi_frame);
	evas_object_show(layout);

	g_pd.list = _create_list(layout);
	assertm_if(NULL == g_pd.list, "_create_list failed");

	elm_object_part_content_set(layout, "elm.swallow.content", g_pd.list);

	g_strlcpy(title, sc(PACKAGE, I18N_TYPE_Advanced_setting), sizeof(title));
	navi_it = elm_naviframe_item_push(navi_frame, title, NULL, NULL,
			layout, NULL);

	evas_object_data_set(navi_frame, SCREEN_TYPE_ID_KEY,
			(void *) VIEW_MANAGER_VIEW_TYPE_ADVANCED);

	elm_naviframe_item_pop_cb_set(navi_it, __back_key_cb, NULL);

	/* Add vconf listeners for the different options */
	vconf_notify_key_changed(VCONFKEY_WIFI_ENABLE_QS, __vconf_noti_change_cb,
				NULL);
	vconf_notify_key_changed(VCONF_SLEEP_POLICY, __vconf_sleep_change_cb,
				NULL);
	vconf_notify_key_changed(VCONF_SORT_BY, __vconf_sort_change_cb,
				NULL);

	__COMMON_FUNC_EXIT__;
}
