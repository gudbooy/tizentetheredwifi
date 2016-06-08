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

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <cert-service.h>
#include <cert-svc/cpkcs12.h>
#include <cert-svc/cprimitives.h>
#include <ui-gadget.h>
#include <efl_assist.h>
#include <utilX.h>

#include "common.h"
#include "common_eap_connect.h"
#include "i18nmanager.h"
#include "common_utils.h"
#include "common_ip_info.h"

#define EAP_CONNECT_POPUP			"popup_view"
#define GENLIST_ITEM_HEIGHT				96

#define EAP_TLS_PATH			"/tmp/"
#define EAP_TLS_CA_CERT_PATH		"ca_cert.pem"
#define EAP_TLS_USER_CERT_PATH		"user_cert.pem"
#define EAP_TLS_PRIVATEKEY_PATH		"privatekey.pem"

#define EAP_TYPE_UNKNOWN		"UNKNOWN"
#define EAP_TYPE_PEAP			"PEAP"
#define EAP_TYPE_TLS				"TLS"
#define EAP_TYPE_TTLS			"TTLS"
#define EAP_TYPE_SIM				"SIM"
#define EAP_TYPE_AKA				"AKA"

#define EAP_AUTH_TYPE_NONE		"NONE"
#define EAP_AUTH_TYPE_PAP			"PAP"
#define EAP_AUTH_TYPE_MSCHAP				"MSCHAP"
#define EAP_AUTH_TYPE_MSCHAPV2			"MSCHAPV2"
#define EAP_AUTH_TYPE_GTC				"GTC"
#define EAP_AUTH_TYPE_MD5				"MD5"

typedef enum {
	EAP_SEC_TYPE_UNKNOWN  = 0,
	EAP_SEC_TYPE_PEAP,
	EAP_SEC_TYPE_TLS,
	EAP_SEC_TYPE_TTLS,
	EAP_SEC_TYPE_SIM,
	EAP_SEC_TYPE_AKA,
	EAP_SEC_TYPE_NULL
} eap_type_t;

typedef enum {
	EAP_SEC_AUTH_NONE  = 0,
	EAP_SEC_AUTH_PAP,
	EAP_SEC_AUTH_MSCHAP,
	EAP_SEC_AUTH_MSCHAPV2,
	EAP_SEC_AUTH_GTC,
	EAP_SEC_AUTH_MD5,
	EAP_SEC_AUTH_NULL
} eap_auth_t;

typedef struct {
	char *name;
	Elm_Genlist_Item_Type flags;
} _Expand_List_t;

struct eap_info_list {
	wifi_ap_h ap;
	eap_type_t eap_type;
	Elm_Object_Item *eap_method_item;
	Elm_Object_Item *eap_auth_item;
	Elm_Object_Item *user_cert_item;
	Elm_Object_Item *id_item;
	Elm_Object_Item *pswd_item;
};

static const _Expand_List_t list_eap_type[] = {
	{"UNKNOWN", ELM_GENLIST_ITEM_NONE},
	{"PEAP", ELM_GENLIST_ITEM_NONE},
	{"TLS", ELM_GENLIST_ITEM_NONE},
	{"TTLS", ELM_GENLIST_ITEM_NONE},
	{"SIM", ELM_GENLIST_ITEM_NONE},
	{"AKA", ELM_GENLIST_ITEM_NONE},
	{NULL, ELM_GENLIST_ITEM_NONE}
};

static const _Expand_List_t list_eap_auth[] = {
	{"NONE", ELM_GENLIST_ITEM_NONE},
	{"PAP", ELM_GENLIST_ITEM_NONE},
	{"MSCHAP", ELM_GENLIST_ITEM_NONE},
	{"MSCHAPV2", ELM_GENLIST_ITEM_NONE},
	{"GTC", ELM_GENLIST_ITEM_NONE},
	{"MD5", ELM_GENLIST_ITEM_NONE},
	{NULL, ELM_GENLIST_ITEM_NONE}
};

struct common_eap_connect_data {
	Elm_Object_Item *eap_type_item;
	Elm_Object_Item *eap_auth_item;
	Elm_Object_Item *eap_user_cert_item;
	Elm_Object_Item *eap_id_item;
	Elm_Object_Item *eap_pw_item;
	Evas_Object *popup;
	Evas_Object *ctxpopup;
	Evas_Object *info_popup;
	Evas_Object *genlist;
	Eina_Bool eap_done_ok;
	Evas_Object *layout;
	Evas_Object *win;
	const char *str_pkg_name;
	wifi_ap_h ap;
	ip_info_list_t *ip_info_list;
	Evas_Object* navi_frame;
	char *cert_alias;
	char *ca_cert_path;
	char *user_cert_path;
	char *privatekey_path;

	int key_status;

};

static bool g_eap_id_show_keypad = FALSE;
static Elm_Genlist_Item_Class g_eap_type_itc;
static Elm_Genlist_Item_Class g_eap_auth_itc;
static Elm_Genlist_Item_Class g_eap_user_cert_itc;
static Elm_Genlist_Item_Class g_eap_entry_itc;

static void _gl_eap_type_item_update_border(eap_connect_data_t *eap_data);
static void _create_and_update_list_items_based_on_rules(eap_type_t new_type, eap_connect_data_t *eap_data);
static void _delete_eap_entry_items(eap_connect_data_t *eap_data);
static eap_type_t __common_eap_connect_popup_get_eap_type(wifi_ap_h ap);
static eap_auth_t __common_eap_connect_popup_get_auth_type(wifi_ap_h ap);
static wifi_eap_type_e __common_eap_connect_popup_get_wlan_eap_type(eap_type_t eap_type);
static wifi_eap_auth_type_e __common_eap_connect_popup_get_wlan_auth_type(eap_auth_t auth_type);
static void _info_popup_ok_cb(void *data, Evas_Object *obj, void *event_info);
static void __cancel_cb(void *data, Evas_Object *obj, void *event_info);

static void ctxpopup_dismissed_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;

	evas_object_del(eap_data->ctxpopup);
	eap_data->ctxpopup = NULL;

	common_utils_edit_box_allow_focus_set(eap_data->eap_id_item, EINA_TRUE);
	common_utils_edit_box_allow_focus_set(eap_data->eap_pw_item, EINA_TRUE);
	ip_info_enable_all_keypads(eap_data->ip_info_list);
}

static void move_dropdown(eap_connect_data_t *eap_data, Evas_Object *obj)
{
	Evas_Coord x, y, w , h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	evas_object_move(eap_data->ctxpopup, x + (w / 2), y + h);
}

static void _gl_editbox_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(item, FALSE);
}

static void _gl_eap_type_sub_sel(void *data, Evas_Object *obj, void *event_info)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *) data;
	eap_type_t selected_item_index = EAP_SEC_TYPE_UNKNOWN;

	eap_type_t pre_index = __common_eap_connect_popup_get_eap_type(eap_data->ap);
	const char *label = elm_object_item_text_get((Elm_Object_Item *) event_info);

	if (strcmp(label, EAP_TYPE_UNKNOWN) == 0)
		selected_item_index = EAP_SEC_TYPE_UNKNOWN;
	else if (strcmp(label, EAP_TYPE_PEAP) == 0)
		selected_item_index = EAP_SEC_TYPE_PEAP;
	else if (strcmp(label, EAP_TYPE_TLS) == 0)
		selected_item_index = EAP_SEC_TYPE_TLS;
	else if (strcmp(label, EAP_TYPE_TTLS) == 0)
		selected_item_index = EAP_SEC_TYPE_TTLS;
	else if (strcmp(label, EAP_TYPE_SIM) == 0)
		selected_item_index = EAP_SEC_TYPE_SIM;
	else if (strcmp(label, EAP_TYPE_AKA) == 0)
		selected_item_index = EAP_SEC_TYPE_AKA;

	DEBUG_LOG(UG_NAME_NORMAL, "previous index = %d; selected index = %d;",
			pre_index, selected_item_index);
	if (pre_index != selected_item_index) {
		_create_and_update_list_items_based_on_rules(selected_item_index, data);
		wifi_eap_type_e type;
		wifi_ap_set_eap_type(eap_data->ap,
				__common_eap_connect_popup_get_wlan_eap_type(selected_item_index));
		wifi_ap_get_eap_type(eap_data->ap, &type);
		DEBUG_LOG(UG_NAME_NORMAL, "set to new index = %d", type);
	} else {
		DEBUG_LOG(UG_NAME_NORMAL, "pre_index == selected_item_index[%d]",
				selected_item_index);
	}

	evas_object_del(eap_data->ctxpopup);
	eap_data->ctxpopup = NULL;

	elm_genlist_item_update(eap_data->eap_type_item);

	common_utils_edit_box_allow_focus_set(eap_data->eap_id_item, EINA_TRUE);
	common_utils_edit_box_allow_focus_set(eap_data->eap_pw_item, EINA_TRUE);
	ip_info_enable_all_keypads(eap_data->ip_info_list);
}

static void _gl_eap_type_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *) data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	Evas_Object *ctxpopup;
	int i = EAP_SEC_TYPE_PEAP;

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	common_utils_edit_box_focus_set(eap_data->eap_id_item, EINA_FALSE);
	common_utils_edit_box_focus_set(eap_data->eap_pw_item, EINA_FALSE);
	ip_info_close_all_keypads(eap_data->ip_info_list);

	if (eap_data->ctxpopup != NULL) {
		evas_object_del(eap_data->ctxpopup);
	}

	ctxpopup = elm_ctxpopup_add(eap_data->win);
	eap_data->ctxpopup = ctxpopup;
	elm_object_style_set(ctxpopup, "dropdown/list");
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup,"dismissed", ctxpopup_dismissed_cb, eap_data);

	/* eap_type = __common_eap_connect_popup_get_eap_type(eap_data->ap); */
	while (list_eap_type[i].name != NULL) {
		elm_ctxpopup_item_append(ctxpopup, list_eap_type[i].name, NULL,
			_gl_eap_type_sub_sel, eap_data);
		elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_DOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN);
		i++;
	}
	move_dropdown(eap_data, obj);
	evas_object_show(ctxpopup);
	_gl_eap_type_item_update_border(eap_data);
}

static void _gl_eap_type_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}
}

static void _gl_eap_auth_sub_sel(void *data, Evas_Object *obj, void *event_info)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *) data;
	eap_auth_t selected_item_index = EAP_SEC_AUTH_NONE;

	const char *label = elm_object_item_text_get((Elm_Object_Item *) event_info);

	if (strcmp(label, EAP_AUTH_TYPE_NONE) == 0)
		selected_item_index = EAP_SEC_AUTH_NONE;
	else if (strcmp(label, EAP_AUTH_TYPE_PAP) == 0)
		selected_item_index = EAP_SEC_AUTH_PAP;
	else if (strcmp(label, EAP_AUTH_TYPE_MSCHAP) == 0)
		selected_item_index = EAP_SEC_AUTH_MSCHAP;
	else if (strcmp(label, EAP_AUTH_TYPE_MSCHAPV2) == 0)
		selected_item_index = EAP_SEC_AUTH_MSCHAPV2;
	else if (strcmp(label, EAP_AUTH_TYPE_GTC) == 0)
		selected_item_index = EAP_SEC_AUTH_GTC;
	else if (strcmp(label, EAP_AUTH_TYPE_MD5) == 0)
		selected_item_index = EAP_SEC_AUTH_MD5;

	wifi_ap_set_eap_auth_type(eap_data->ap,
		__common_eap_connect_popup_get_wlan_auth_type(selected_item_index));

	evas_object_del(eap_data->ctxpopup);
	eap_data->ctxpopup = NULL;

	elm_genlist_item_update(eap_data->eap_auth_item);

	common_utils_edit_box_allow_focus_set(eap_data->eap_id_item, EINA_TRUE);
	common_utils_edit_box_allow_focus_set(eap_data->eap_pw_item, EINA_TRUE);
	ip_info_enable_all_keypads(eap_data->ip_info_list);
}

static void _gl_eap_auth_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}
}

static void _gl_eap_auth_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *) data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	Evas_Object *ctxpopup;
	int i = 0;

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	common_utils_edit_box_focus_set(eap_data->eap_id_item, EINA_FALSE);
	common_utils_edit_box_focus_set(eap_data->eap_pw_item, EINA_FALSE);
	ip_info_close_all_keypads(eap_data->ip_info_list);

	if (eap_data->ctxpopup != NULL) {
		evas_object_del(eap_data->ctxpopup);
	}

	ctxpopup = elm_ctxpopup_add(eap_data->win);
	eap_data->ctxpopup = ctxpopup;
	elm_object_style_set(ctxpopup, "dropdown/list");
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup,"dismissed", ctxpopup_dismissed_cb, eap_data);

	while (list_eap_auth[i].name != NULL) {
		elm_ctxpopup_item_append(ctxpopup, list_eap_auth[i].name,
			NULL, _gl_eap_auth_sub_sel, eap_data);
		elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_DOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN);
		i++;
	}
	move_dropdown(eap_data, obj);
	evas_object_show(ctxpopup);
}

static char *_gl_eap_type_text_get(void *data, Evas_Object *obj, const char *part)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;

	if (!strcmp(part, "elm.text.main")) {
		return g_strdup(sc(eap_data->str_pkg_name, I18N_TYPE_EAP_method));
	}

	return NULL;
}

static Evas_Object *_gl_eap_type_content_get(void *data,
		Evas_Object *obj, const char *part)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;
	eap_type_t sel_sub_item_id = __common_eap_connect_popup_get_eap_type(eap_data->ap);
	Evas_Object *btn = NULL;
	char buf[100];

	if (!strcmp(part, "elm.icon.entry")) {
		btn = elm_button_add(obj);

		g_snprintf(buf, sizeof(buf), "<align=left>%s</align>",
				list_eap_type[sel_sub_item_id].name);

		elm_object_text_set(btn, buf);
		elm_object_style_set(btn, "dropdown");
		evas_object_propagate_events_set(btn, EINA_FALSE);
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL,
				EVAS_HINT_FILL);
		evas_object_smart_callback_add(btn, "clicked", _gl_eap_type_btn_cb,
				eap_data);

		return btn;
	}
	return NULL;
}

static char *_gl_eap_auth_text_get(void *data, Evas_Object *obj, const char *part)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;

	if (!strcmp(part, "elm.text.main")) {
		return g_strdup(sc(eap_data->str_pkg_name, I18N_TYPE_Phase_2_authentication));
	}

	return NULL;
}

static Evas_Object *_gl_eap_auth_content_get(void *data,
		Evas_Object *obj, const char *part)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;
	eap_type_t sel_sub_item_id = __common_eap_connect_popup_get_auth_type(eap_data->ap);
	Evas_Object *btn = NULL;
	char buf[100];

	if (!strcmp(part, "elm.icon.entry")) {
		btn = elm_button_add(obj);

		g_snprintf(buf, sizeof(buf), "<align=left>%s</align>",
				list_eap_auth[sel_sub_item_id].name);

		elm_object_text_set(btn, buf);
		elm_object_style_set(btn, "dropdown");
		evas_object_propagate_events_set(btn, EINA_FALSE);
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL,
				EVAS_HINT_FILL);
		evas_object_smart_callback_add(btn, "clicked", _gl_eap_auth_btn_cb,
				eap_data);

		return btn;
	}
	return NULL;
}

static char *_gl_eap_user_cert_text_get(void *data, Evas_Object *obj, const char *part)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;
	char buf[1024];

	if (!strcmp(part, "elm.text.main.left.top")) {
		snprintf(buf, 1023, "%s",
				sc(eap_data->str_pkg_name, I18N_TYPE_User_Certificate));
		return strdup(buf);
	} else if (!strcmp(part, "elm.text.sub.left.bottom")) {
		if (eap_data->cert_alias == NULL) {
			snprintf(buf, 1023, "%s",
					sc(eap_data->str_pkg_name, I18N_TYPE_Unspecified));
			return strdup(buf);
		} else {
			snprintf(buf, 1023, "%s", eap_data->cert_alias);
			return strdup(buf);
		}
	}

	return NULL;
}

static void _gl_eap_entry_key_enter_cb(void *data, Evas_Object *obj, void *event_info)
{
	common_utils_entry_info_t *entry_info = (common_utils_entry_info_t *)data;
	if (!entry_info) {
		return;
	}

	Evas_Object *entry = NULL;
	Elm_Object_Item *next_item = NULL;

	switch (entry_info->entry_id) {
	case ENTRY_TYPE_USER_ID:
	case ENTRY_TYPE_PASSWORD:
		next_item = elm_genlist_item_next_get(entry_info->item);
		while (next_item) {
			if (elm_object_item_disabled_get(next_item) == EINA_FALSE &&
				elm_genlist_item_select_mode_get(next_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
				entry = elm_object_item_part_content_get(next_item, "elm.icon.entry");
				if (entry) {
					elm_object_focus_set(entry, EINA_TRUE);
					return;
				}
			}

			next_item = elm_genlist_item_next_get(next_item);
		}
		break;
	default:
		break;
	}
}

static void _gl_eap_entry_cursor_changed_cb(void* data, Evas_Object* obj, void* event_info)
{
	common_utils_entry_info_t *entry_info = (common_utils_entry_info_t *)data;
	if (!entry_info) {
		return;
	}

	if (elm_object_focus_get(obj)) {
		if (elm_entry_is_empty(obj)) {
			elm_object_item_signal_emit(entry_info->item, "elm,state,eraser,hide", "");
		} else {
			elm_object_item_signal_emit(entry_info->item, "elm,state,eraser,show", "");
		}
	}

	if (entry_info->entry_txt) {
		g_free(entry_info->entry_txt);
		entry_info->entry_txt = NULL;
	}

	char *entry_text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));

	if (entry_text != NULL && entry_text[0] != '\0') {
		entry_info->entry_txt = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	}

	g_free(entry_text);
}

static void _gl_eap_entry_changed_cb(void* data, Evas_Object* obj, void* event_info)
{
	common_utils_entry_info_t *entry_info = (common_utils_entry_info_t *)data;
	if (!entry_info) {
		return;
	}

	if (obj == NULL) {
		return;
	}

	if (elm_object_focus_get(obj)) {
		if (elm_entry_is_empty(obj)) {
			elm_object_item_signal_emit(entry_info->item, "elm,state,eraser,hide", "");
		} else {
			elm_object_item_signal_emit(entry_info->item, "elm,state,eraser,show", "");
		}
	}
}

static void _gl_eap_entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	common_utils_entry_info_t *entry_info = (common_utils_entry_info_t *)data;
	if (!entry_info) {
		return;
	}

	if (!elm_entry_is_empty(obj)) {
		elm_object_item_signal_emit(entry_info->item, "elm,state,eraser,show", "");
	}

	elm_object_item_signal_emit(entry_info->item, "elm,state,rename,hide", "");
}

static void _gl_eap_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	common_utils_entry_info_t *entry_info = (common_utils_entry_info_t *)data;
	if (!entry_info) {
		return;
	}

	if (entry_info->entry_txt) {
		g_free(entry_info->entry_txt);
		entry_info->entry_txt = NULL;
	}

	char *entry_text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	if (entry_text != NULL && entry_text[0] != '\0')
		entry_info->entry_txt = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));

	g_free(entry_text);

	elm_object_item_signal_emit(entry_info->item, "elm,state,eraser,hide", "");
	elm_object_item_signal_emit(entry_info->item, "elm,state,rename,show", "");
}

static void _gl_eap_entry_maxlength_reached(void *data, Evas_Object *obj,
		void *event_info)
{
	common_utils_send_message_to_net_popup("Password length",
			"Lengthy Password", "notification", NULL);
}

static void _gl_eap_entry_eraser_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	common_utils_entry_info_t *entry_info = (common_utils_entry_info_t *)data;
	if (!entry_info) {
		return;
	}

	Evas_Object *entry = elm_object_item_part_content_get(entry_info->item, "elm.icon.entry");
	elm_object_focus_set(entry, EINA_TRUE);
	elm_entry_entry_set(entry, "");
}

static char *_gl_eap_entry_item_text_get(void *data, Evas_Object *obj, const char *part)
{
	common_utils_entry_info_t *entry_info = (common_utils_entry_info_t *)data;
	if (!entry_info) {
		return NULL;
	}

	if (!strcmp(part, "elm.text.main")) {
		return g_strdup(entry_info->title_txt);
	}

	return NULL;
}

static Evas_Object *_gl_eap_entry_item_content_get(void *data, Evas_Object *obj, const char *part)
{
	common_utils_entry_info_t *entry_info = (common_utils_entry_info_t *)data;
	if (!entry_info) {
		return NULL;
	}

	if (g_strcmp0(part, "elm.icon.entry") == 0) {
		Evas_Object *entry = NULL;
		char *guide_txt = NULL;
		char *accepted = NULL;
		Eina_Bool hide_entry_txt = EINA_FALSE;
		Elm_Input_Panel_Layout panel_type;

		static Elm_Entry_Filter_Limit_Size limit_filter_data;

		switch (entry_info->entry_id)
		{
		case ENTRY_TYPE_USER_ID:
			panel_type = ELM_INPUT_PANEL_LAYOUT_NORMAL;
			guide_txt = entry_info->guide_txt;
			break;
		case ENTRY_TYPE_PASSWORD:
			panel_type = ELM_INPUT_PANEL_LAYOUT_PASSWORD;
			guide_txt = entry_info->guide_txt;
			hide_entry_txt = EINA_TRUE;
			break;
		default:
			return NULL;
		}

		entry = elm_entry_add(obj);
		elm_entry_single_line_set(entry, EINA_TRUE);
		elm_entry_password_set(entry, hide_entry_txt);
		elm_entry_prediction_allow_set(entry, EINA_FALSE);
		elm_entry_autocapital_type_set(entry, ELM_AUTOCAPITAL_TYPE_NONE);

		elm_object_part_text_set(entry, "elm.guide", guide_txt);
		if (entry_info->entry_txt && (strlen(entry_info->entry_txt) > 0)) {
			elm_entry_entry_set(entry, entry_info->entry_txt);
		}

		elm_entry_input_panel_layout_set(entry, panel_type);
		elm_entry_input_panel_return_key_type_set(entry, ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_NEXT);

		limit_filter_data.max_char_count = 32;
		elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);

		Elm_Entry_Filter_Accept_Set digits_filter_data;
		memset(&digits_filter_data, 0, sizeof(Elm_Entry_Filter_Accept_Set));
		digits_filter_data.accepted = accepted;
		elm_entry_markup_filter_append(entry, elm_entry_filter_accept_set, &digits_filter_data);
		elm_entry_context_menu_disabled_set(entry, EINA_TRUE);

		Ecore_IMF_Context *imf_ctxt = elm_entry_imf_context_get(entry);
		if (imf_ctxt && entry_info->input_panel_cb) {
			ecore_imf_context_input_panel_event_callback_add(imf_ctxt,
					ECORE_IMF_INPUT_PANEL_STATE_EVENT,
					entry_info->input_panel_cb,
					entry_info->input_panel_cb_data);
			DEBUG_LOG(UG_NAME_NORMAL, "set the imf ctxt cbs");
		}

		evas_object_smart_callback_add(entry, "activated", _gl_eap_entry_key_enter_cb, entry_info);
		evas_object_smart_callback_add(entry, "cursor,changed", _gl_eap_entry_cursor_changed_cb, entry_info);
		evas_object_smart_callback_add(entry, "changed", _gl_eap_entry_changed_cb, entry_info);
		evas_object_smart_callback_add(entry, "focused", _gl_eap_entry_focused_cb, entry_info);
		evas_object_smart_callback_add(entry, "unfocused", _gl_eap_entry_unfocused_cb, entry_info);
		evas_object_smart_callback_add(entry, "maxlength,reached", _gl_eap_entry_maxlength_reached, NULL);

		if (ENTRY_TYPE_USER_ID == entry_info->entry_id) {
			if (TRUE == g_eap_id_show_keypad) {
				elm_object_focus_set(entry, EINA_TRUE);
				g_eap_id_show_keypad = FALSE;
			}
		}

		return entry;
	} else if (g_strcmp0(part, "elm.icon.eraser") == 0) {
		Evas_Object *btn = elm_button_add(obj);
		elm_object_style_set(btn, "editfield_clear");
		evas_object_smart_callback_add(btn, "clicked", _gl_eap_entry_eraser_clicked_cb, entry_info);
		return btn;
	}

	return NULL;
}

static void _gl_eap_entry_item_del(void *data, Evas_Object *obj)
{
	common_utils_entry_info_t *entry_info = (common_utils_entry_info_t *)data;
	if (entry_info == NULL) {
		return;
	}

	if (entry_info->entry_txt) {
		g_free(entry_info->entry_txt);
	}

	g_free(entry_info);
}

static void _gl_eap_type_item_update_border(eap_connect_data_t *eap_data)
{
	eap_type_t eap_type;

	eap_type = __common_eap_connect_popup_get_eap_type(eap_data->ap);

	if (eap_type == EAP_SEC_TYPE_SIM || eap_type == EAP_SEC_TYPE_AKA) {
		elm_object_item_signal_emit(eap_data->eap_type_item,
				"elm,state,normal", "");
	} else {
		elm_object_item_signal_emit(eap_data->eap_type_item,
				"elm,state,top", "");
	}
}

static void _gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = event_info;
	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;

	ip_info_items_realize(eap_data->ip_info_list);

	if (item == eap_data->eap_type_item) {
		_gl_eap_type_item_update_border(eap_data);
	} else if (item == eap_data->eap_pw_item) {
		elm_object_item_signal_emit(item, "elm,state,bottom", "");
	} else {
		elm_object_item_signal_emit(item, "elm,state,center", "");
	}
}

static void __common_eap_connect_popup_init_item_class(void *data)
{
	g_eap_type_itc.item_style = "entry.main";
	g_eap_type_itc.func.text_get = _gl_eap_type_text_get;
	g_eap_type_itc.func.content_get = _gl_eap_type_content_get;
	g_eap_type_itc.func.state_get = NULL;
	g_eap_type_itc.func.del = NULL;

	g_eap_auth_itc.item_style = "entry.main";
	g_eap_auth_itc.func.text_get = _gl_eap_auth_text_get;
	g_eap_auth_itc.func.content_get = _gl_eap_auth_content_get;
	g_eap_auth_itc.func.state_get = NULL;
	g_eap_auth_itc.func.del = NULL;

	g_eap_user_cert_itc.item_style = "2line.top";
	g_eap_user_cert_itc.func.text_get = _gl_eap_user_cert_text_get;
	g_eap_user_cert_itc.func.content_get = NULL;
	g_eap_user_cert_itc.func.state_get = NULL;
	g_eap_user_cert_itc.func.del = NULL;

	g_eap_entry_itc.item_style = "entry.main";
	g_eap_entry_itc.func.text_get = _gl_eap_entry_item_text_get;
	g_eap_entry_itc.func.content_get = _gl_eap_entry_item_content_get;
	g_eap_entry_itc.func.state_get = NULL;
	g_eap_entry_itc.func.del = _gl_eap_entry_item_del;

	return;
}

static gboolean __cert_extract_files(const char *cert_alias,
		eap_connect_data_t *eap_data)
{
	int ret;
	int validity;
	int cert_counts = 0;
	int cert_index;
	gchar *ca_cert_path = NULL;
	gchar *user_cert_path = NULL;
	gchar *privatekey_path = NULL;
	FILE *fp;
	CertSvcInstance cert_instance;
	CertSvcString cert_alias_str;
	CertSvcCertificateList cert_list;
	CertSvcCertificate user_certificate;
	CertSvcCertificate ca_certificate;
	CertSvcCertificate *selected_certificate = NULL;
	X509 *x509 = NULL;
	EVP_PKEY *privatekey = NULL;

	ret = certsvc_instance_new(&cert_instance);
	if (ret != CERTSVC_SUCCESS) {
		ERROR_LOG(UG_NAME_NORMAL, "failed to certsvc_instance_new");
		goto error;
	}

	ret = certsvc_string_new(cert_instance, cert_alias, strlen(cert_alias), &cert_alias_str);
	if (ret != CERTSVC_SUCCESS) {
		ERROR_LOG(UG_NAME_NORMAL, "failed to certsvc_string_new");
		goto error;
	}

	ret = certsvc_pkcs12_load_certificate_list(cert_instance, cert_alias_str, &cert_list);
	if (ret != CERTSVC_SUCCESS) {
		ERROR_LOG(UG_NAME_NORMAL, "failed to certsvc_pkcs12_load_certificate_list");
		goto error;
	}

	ret = certsvc_certificate_list_get_length(cert_list, &cert_counts);
	if (cert_counts < 1) {
		ERROR_LOG(UG_NAME_NORMAL, "there is no certificates");
		goto error;
	}

	INFO_LOG(UG_NAME_NORMAL, "cert counts: %d", cert_counts);
	selected_certificate = g_try_new0(CertSvcCertificate, cert_counts);
	if (selected_certificate == NULL) {
		ERROR_LOG(UG_NAME_NORMAL, "failed to allocate memory");
		goto error;
	}

	ret = certsvc_certificate_list_get_one(cert_list, 0, &user_certificate);
	if (ret != CERTSVC_SUCCESS) {
		ERROR_LOG(UG_NAME_NORMAL, "failed to certsvc_certificate_list_get_one");
		goto error;
	}
	cert_index = cert_counts - 1;

	selected_certificate[0] = user_certificate;

	ret = certsvc_certificate_dup_x509(user_certificate, &x509);

	user_cert_path = g_strdup_printf("%s%s_%s", EAP_TLS_PATH,
				cert_alias, EAP_TLS_USER_CERT_PATH);

	if ((fp = fopen(user_cert_path, "w")) == NULL) {
		goto error;
	}
	ret = PEM_write_X509(fp, x509);
	fclose(fp);
	certsvc_certificate_free_x509(x509);
	INFO_LOG(UG_NAME_NORMAL, "success to save user_cert file");

	ca_cert_path = g_strdup_printf("%s%s_%s", EAP_TLS_PATH, cert_alias,
				EAP_TLS_CA_CERT_PATH);

	while (cert_index) {
		ret = certsvc_certificate_list_get_one(cert_list, cert_index, &ca_certificate);
		if (ret != CERTSVC_SUCCESS) {
			ERROR_LOG(UG_NAME_NORMAL, "failed to certsvc_certificate_list_get_one");
			goto error;
		}

		selected_certificate[cert_counts-cert_index] = ca_certificate;
		cert_index--;

		ret = certsvc_certificate_dup_x509(ca_certificate, &x509);
		if ((fp = fopen(ca_cert_path, "a")) == NULL) {
			goto error;
		}
		ret = PEM_write_X509(fp, x509);
		fclose(fp);
		certsvc_certificate_free_x509(x509);
	}
	INFO_LOG(UG_NAME_NORMAL, "success to save ca_cert file");

	ret = certsvc_certificate_verify(selected_certificate[0], selected_certificate, cert_counts, NULL, 0, &validity);
	if (ret != CERTSVC_SUCCESS) {
		ERROR_LOG(UG_NAME_NORMAL, "failed to verify ca_certificate");
		goto error;
	}

	if (validity == 0) {
		ERROR_LOG(UG_NAME_NORMAL, "Invalid certificate");
		goto error;
	}

	ret = certsvc_pkcs12_dup_evp_pkey(cert_instance, cert_alias_str, &privatekey);

	privatekey_path = g_strdup_printf("%s%s_%s", EAP_TLS_PATH,
				cert_alias, EAP_TLS_PRIVATEKEY_PATH);

	if ((fp = fopen(privatekey_path, "w")) == NULL) {
		goto error;
	}
	ret = PEM_write_PrivateKey(fp, privatekey, NULL, NULL, 0, NULL, NULL);
	fclose(fp);
	certsvc_pkcs12_free_evp_pkey(privatekey);
	INFO_LOG(UG_NAME_NORMAL, "success to save privatekey file");

	g_free(selected_certificate);
	certsvc_instance_free(cert_instance);

	eap_data->ca_cert_path = ca_cert_path;
	eap_data->user_cert_path = user_cert_path;
	eap_data->privatekey_path = privatekey_path;

	return TRUE;

error:
	g_free(ca_cert_path);
	g_free(user_cert_path);
	g_free(privatekey_path);

	if (selected_certificate) {
		g_free(selected_certificate);
	}

	certsvc_instance_free(cert_instance);
	return FALSE;
}

static void __cert_ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	Evas_Object *base;

	if (!ug || !priv) {
		return;
	}

	base = (Evas_Object *) ug_get_layout(ug);
	if (!base) {
		return;
	}

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(base);
		break;
	default:
		break;
	}
}

static void __cert_ug_result_cb(ui_gadget_h ug, app_control_h result, void *priv)
{
	if (!ug || !priv) {
		return;
	}

	char *cert_alias = NULL;
	eap_connect_data_t *eap_data = (eap_connect_data_t *)priv;

	app_control_get_extra_data(result, "selected-cert", &cert_alias);
	if (cert_alias == NULL) {
		return;
	}

	SECURE_INFO_LOG(UG_NAME_NORMAL, "result receive from cert ug: %s", cert_alias);

	if (__cert_extract_files(cert_alias, eap_data)) {
		eap_data->cert_alias = cert_alias;

		elm_genlist_item_update(eap_data->eap_user_cert_item);
	} else {
		eap_data->info_popup = common_utils_show_info_ok_popup(
				eap_data->layout, eap_data->str_pkg_name,
				sc(eap_data->str_pkg_name, I18N_TYPE_Invalid_certificate),
				_info_popup_ok_cb, eap_data);
	}
}

static void __cert_ug_destroy_cb(ui_gadget_h ug, void *priv)
{
	if (!ug || !priv) {
		return;
	}

	ug_destroy(ug);
	ug = NULL;
}

static void _gl_eap_user_cert_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_gadget_h ug;
	Elm_Object_Item *item = NULL;

	item = (Elm_Object_Item *)event_info;
	if (item) {
		elm_genlist_item_selected_set(item, FALSE);
	}

	struct ug_cbs *cbs = (struct ug_cbs *)calloc(1, sizeof(struct ug_cbs));
	if (cbs == NULL) {
		ERROR_LOG(UG_NAME_NORMAL, "failed to calloc ug_cbs");
		return;
	}

	cbs->layout_cb = __cert_ug_layout_cb;
	cbs->result_cb = __cert_ug_result_cb;
	cbs->destroy_cb = __cert_ug_destroy_cb;
	cbs->priv = data;

	ug = ug_create(NULL, "cert-selection-ug-efl", UG_MODE_FULLVIEW, NULL, cbs);
	if (!ug) {
		ERROR_LOG(UG_NAME_NORMAL, "failed to create certificate ug");
	}

	free(cbs);
}

static void __common_eap_connect_imf_ctxt_evnt_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	if (!data) {
		return;
	}

	if (value == ECORE_IMF_INPUT_PANEL_STATE_SHOW) {
		DEBUG_LOG(UG_NAME_NORMAL, "Key pad is now open");
		elm_object_item_signal_emit(data, "elm,state,sip,shown", "");
	} else if (value == ECORE_IMF_INPUT_PANEL_STATE_HIDE) {
		DEBUG_LOG(UG_NAME_NORMAL, "Key pad is now close");
		elm_object_item_signal_emit(data, "elm,state,sip,hidden", "");
	}
	return;
}

static void __common_eap_connect_im_ctxt_evnt_resize_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	__COMMON_FUNC_ENTER__;

	if (!data) {
		return;
	}

	int rotate_angle = common_utils_get_rotate_angle(APPCORE_RM_UNKNOWN);

	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;
	eap_data->key_status = value;

	eap_view_rotate_popup(eap_data, rotate_angle);

	__COMMON_FUNC_EXIT__;
	return;
}

static void __common_eap_popup_set_imf_ctxt_evnt_cb(eap_connect_data_t *eap_data)
{
	if (!eap_data) {
		return;
	}

	if (eap_data->eap_id_item) {
		common_utils_set_edit_box_imf_panel_evnt_cb(eap_data->eap_id_item,
				__common_eap_connect_im_ctxt_evnt_resize_cb, eap_data);
	}
	if (eap_data->eap_pw_item) {
		common_utils_set_edit_box_imf_panel_evnt_cb(eap_data->eap_pw_item,
				__common_eap_connect_im_ctxt_evnt_resize_cb, eap_data);
	}

	return;
}

static void __common_eap_view_set_imf_ctxt_evnt_cb(eap_connect_data_t *eap_data)
{
	if (!eap_data) {
		return;
	}

	Elm_Object_Item *navi_it = elm_naviframe_top_item_get(eap_data->navi_frame);
	if (!navi_it) {
		return;
	}

	if (eap_data->eap_id_item) {
		common_utils_set_edit_box_imf_panel_evnt_cb(eap_data->eap_id_item,
				__common_eap_connect_imf_ctxt_evnt_cb, navi_it);
	}
	if (eap_data->eap_pw_item) {
		common_utils_set_edit_box_imf_panel_evnt_cb(eap_data->eap_pw_item,
				__common_eap_connect_imf_ctxt_evnt_cb, navi_it);
	}
}

/* This creates EAP type, Auth type, CA certificate, User certificate,
 * User Id, Anonymous Id and Password items.
 */
static void _create_and_update_list_items_based_on_rules(eap_type_t new_type,
		eap_connect_data_t *eap_data)
{
	__COMMON_FUNC_ENTER__;
	Evas_Object* view_list = eap_data->genlist;
	Elm_Object_Item *insert_after_item = NULL;
	eap_type_t pre_type;

	if (NULL == eap_data->eap_type_item) {
		/* Create EAP method/type */
		pre_type = EAP_SEC_TYPE_SIM;
		eap_data->eap_type_item = elm_genlist_item_append(
						view_list, &g_eap_type_itc,
						eap_data, NULL,
						ELM_GENLIST_ITEM_NONE,
						_gl_eap_type_sel, eap_data);
	} else {
		pre_type = __common_eap_connect_popup_get_eap_type(eap_data->ap);
	}

	switch (new_type) {
	case EAP_SEC_TYPE_PEAP:
	case EAP_SEC_TYPE_TLS:
	case EAP_SEC_TYPE_TTLS:
		if (EAP_SEC_TYPE_UNKNOWN == pre_type ||
				EAP_SEC_TYPE_SIM == pre_type ||
				EAP_SEC_TYPE_AKA == pre_type) {
			insert_after_item = eap_data->eap_type_item;
		}
		break;
	case EAP_SEC_TYPE_SIM:
	case EAP_SEC_TYPE_AKA:
		if (EAP_SEC_TYPE_PEAP == pre_type ||
				EAP_SEC_TYPE_TLS == pre_type ||
				EAP_SEC_TYPE_TTLS == pre_type) {
			_delete_eap_entry_items(eap_data);
		}
		break;
	default:
		break;
	}

	if (insert_after_item) {
		common_utils_entry_info_t *edit_box_details;

		/* Add EAP phase2 authentication */
		eap_data->eap_auth_item = elm_genlist_item_insert_after(
				view_list, &g_eap_auth_itc, eap_data, NULL,
				insert_after_item, ELM_GENLIST_ITEM_NONE,
				_gl_eap_auth_sel, eap_data);

		/* Add User certificate */
		eap_data->eap_user_cert_item = elm_genlist_item_insert_after(
				view_list, &g_eap_user_cert_itc, eap_data, NULL,
				eap_data->eap_auth_item, ELM_GENLIST_ITEM_NONE,
				_gl_eap_user_cert_sel_cb, eap_data);

		/* Add EAP ID */
		edit_box_details = g_try_new0(common_utils_entry_info_t, 1);
		if (edit_box_details == NULL) {
			return;
		}

		edit_box_details->entry_id = ENTRY_TYPE_USER_ID;
		edit_box_details->title_txt = sc(eap_data->str_pkg_name, I18N_TYPE_Identity);
		edit_box_details->guide_txt = sc(eap_data->str_pkg_name, I18N_TYPE_Enter_Identity);
		edit_box_details->item = elm_genlist_item_insert_after(
				view_list, &g_eap_entry_itc, edit_box_details,
				NULL, eap_data->eap_user_cert_item,
				ELM_GENLIST_ITEM_NONE, _gl_editbox_sel_cb, NULL);
		eap_data->eap_id_item = edit_box_details->item;
		g_eap_id_show_keypad = FALSE;

		/* Add EAP Password */
		edit_box_details = g_try_new0(common_utils_entry_info_t, 1);
		if (edit_box_details == NULL) {
			g_free(edit_box_details);
			return;
		}

		edit_box_details->entry_id = ENTRY_TYPE_PASSWORD;
		edit_box_details->title_txt = sc(eap_data->str_pkg_name, I18N_TYPE_Password);
		edit_box_details->guide_txt = sc(eap_data->str_pkg_name, I18N_TYPE_Enter_password);
		edit_box_details->item = elm_genlist_item_insert_after(
				view_list, &g_eap_entry_itc, edit_box_details,
				NULL, eap_data->eap_id_item,
				ELM_GENLIST_ITEM_NONE, _gl_editbox_sel_cb, NULL);
		eap_data->eap_pw_item = edit_box_details->item;

		if (eap_data->popup) {	/* Popup */
			__common_eap_popup_set_imf_ctxt_evnt_cb(eap_data);
		} else {				/* View */
			__common_eap_view_set_imf_ctxt_evnt_cb(eap_data);
		}
	}
	__COMMON_FUNC_EXIT__;
	return;
}

void _delete_eap_entry_items(eap_connect_data_t *eap_data)
{
	__COMMON_FUNC_ENTER__;
	elm_object_item_del(eap_data->eap_auth_item);
	eap_data->eap_auth_item = NULL;
	elm_object_item_del(eap_data->eap_user_cert_item);
	eap_data->eap_user_cert_item = NULL;
	elm_object_item_del(eap_data->eap_id_item);
	eap_data->eap_id_item = NULL;
	elm_object_item_del(eap_data->eap_pw_item);
	eap_data->eap_pw_item = NULL;
	__COMMON_FUNC_EXIT__;
	return;
}

static Evas_Object* _create_list(Evas_Object* parent, void *data)
{
	__COMMON_FUNC_ENTER__;
	assertm_if(NULL == parent, "NULL!!");

	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;
	Evas_Object* view_list = NULL;

	__common_eap_connect_popup_init_item_class(eap_data);
	eap_data->eap_done_ok = FALSE;
	eap_data->genlist = view_list = elm_genlist_add(parent);
	elm_genlist_fx_mode_set(view_list, EINA_FALSE);
	elm_genlist_realization_mode_set(view_list, TRUE);

	evas_object_size_hint_weight_set(view_list, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(view_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* Set default values. eap type = PEAP, auth type = MSCHAPv2 */
	wifi_ap_set_eap_type(eap_data->ap, WIFI_EAP_TYPE_PEAP);
	wifi_ap_set_eap_auth_type(eap_data->ap, WIFI_EAP_AUTH_TYPE_MSCHAPV2);

	/* Create the entry items */
	_create_and_update_list_items_based_on_rules(EAP_SEC_TYPE_PEAP, eap_data);

	evas_object_smart_callback_add(view_list, "realized", _gl_realized, eap_data);

	__COMMON_FUNC_EXIT__;
	return view_list;
}

static void __common_eap_connect_cleanup(eap_connect_data_t *eap_data)
{
	if (eap_data == NULL) {
		return;
	}

	ip_info_remove(eap_data->ip_info_list);
	eap_data->ip_info_list = NULL;

	if (eap_data->genlist) {
		evas_object_del(eap_data->genlist);
	}

	if (eap_data->cert_alias) {
		g_free(eap_data->cert_alias);
		eap_data->cert_alias = NULL;
	}

	if (eap_data->ca_cert_path) {
		g_free(eap_data->ca_cert_path);
		eap_data->ca_cert_path = NULL;
	}

	if (eap_data->user_cert_path) {
		g_free(eap_data->user_cert_path);
		eap_data->user_cert_path = NULL;
	}

	if (eap_data->privatekey_path) {
		g_free(eap_data->privatekey_path);
		eap_data->privatekey_path = NULL;
	}

	wifi_ap_destroy(eap_data->ap);
	eap_data->ap = NULL;

	if(eap_data->info_popup){
		evas_object_del(eap_data->info_popup);
		eap_data->info_popup = NULL;
	}

	if (!eap_data->navi_frame) {
		evas_object_hide(eap_data->popup);
		evas_object_del(eap_data->popup);
	}

	wlan_manager_enable_scan_result_update();
}

static Eina_Bool __common_eap_connect_destroy(void *data, Elm_Object_Item *it)
{
	__common_eap_connect_cleanup((eap_connect_data_t *)data);

	return EINA_TRUE;
}

static void __common_eap_connect_destroy_cb(void *data,  Evas_Object *obj,
		void *event_info)
{
	__common_eap_connect_cleanup((eap_connect_data_t *)data);
}

static void __eap_popup_keydown_cb(void *data, Evas *e, Evas_Object *obj,
		void *event_info)
{
	__COMMON_FUNC_ENTER__;

	Evas_Event_Key_Down *event = event_info;

	if (g_strcmp0(event->keyname, KEY_BACK) == 0) {
		__common_eap_connect_destroy_cb(data, obj, event_info);
	}

	__COMMON_FUNC_EXIT__;
}

static void _info_popup_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;

	if (eap_data->info_popup != NULL) {
		evas_object_del(eap_data->info_popup);
		eap_data->info_popup = NULL;
	}
}

static void __common_eap_connect_done_cb(void *data,  Evas_Object *obj,
		void *event_info)
{
	__COMMON_FUNC_ENTER__;

	char *str_id = NULL;
	char *str_pw = NULL;
	wifi_eap_type_e eap_type;
	wifi_eap_auth_type_e eap_auth_type;

	bool favorite = FALSE;

	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;

	if (eap_data->eap_done_ok == TRUE) {
		return;
	}

	eap_data->eap_done_ok = TRUE;

	wifi_ap_get_eap_type(eap_data->ap, &eap_type);
	wifi_ap_get_eap_auth_type(eap_data->ap, &eap_auth_type);

	if(wifi_ap_is_favorite(eap_data->ap, &favorite) == WIFI_ERROR_NONE &&
			favorite == TRUE) {
		wlan_manager_forget(eap_data->ap);
		wifi_ap_refresh(eap_data->ap);
	}

	wifi_ap_set_eap_ca_cert_file(eap_data->ap, "");
	wifi_ap_set_eap_client_cert_file(eap_data->ap, "");
	wifi_ap_set_eap_private_key_info(eap_data->ap, "", "");

	switch (eap_type) {
	case WIFI_EAP_TYPE_PEAP:
	case WIFI_EAP_TYPE_TTLS:
		str_id = common_utils_get_list_item_entry_txt(eap_data->eap_id_item);
		if (str_id == NULL || str_id[0] == '\0') {
			eap_data->info_popup = common_utils_show_info_ok_popup(
					eap_data->layout, eap_data->str_pkg_name,
					sc(eap_data->str_pkg_name, I18N_TYPE_Enter_Identity),
					_info_popup_ok_cb, eap_data);
			eap_data->eap_done_ok = FALSE;

			if(str_id != NULL) {
				g_free(str_id);
				str_id = NULL;
			}

			__COMMON_FUNC_EXIT__;
			return;
		}

		str_pw = common_utils_get_list_item_entry_txt(eap_data->eap_pw_item);
		if (str_pw == NULL || str_pw[0] == '\0') {
			eap_data->info_popup = common_utils_show_info_ok_popup(
					eap_data->layout, eap_data->str_pkg_name,
					sc(eap_data->str_pkg_name, I18N_TYPE_Enter_password),
					_info_popup_ok_cb, eap_data);
			eap_data->eap_done_ok = FALSE;

			if(str_id != NULL) {
				g_free(str_id);
				str_id = NULL;
			}
			if(str_pw != NULL) {
				g_free(str_pw);
				str_pw = NULL;
			}

			__COMMON_FUNC_EXIT__;
			return;
		}

		wifi_ap_set_eap_type(eap_data->ap, eap_type);
		wifi_ap_set_eap_auth_type(eap_data->ap, eap_auth_type);
		wifi_ap_set_eap_passphrase(eap_data->ap, str_id, str_pw);
		break;

	case WIFI_EAP_TYPE_TLS:
		str_id = common_utils_get_list_item_entry_txt(eap_data->eap_id_item);
		str_pw = common_utils_get_list_item_entry_txt(eap_data->eap_pw_item);

		wifi_ap_set_eap_type(eap_data->ap, eap_type);
		wifi_ap_set_eap_auth_type(eap_data->ap, eap_auth_type);
		wifi_ap_set_eap_passphrase(eap_data->ap, str_id, str_pw);
		wifi_ap_set_eap_ca_cert_file(eap_data->ap, eap_data->ca_cert_path);
		wifi_ap_set_eap_client_cert_file(eap_data->ap, eap_data->user_cert_path);
		wifi_ap_set_eap_private_key_info(eap_data->ap, eap_data->privatekey_path, NULL);
		break;

	case WIFI_EAP_TYPE_SIM:
	case WIFI_EAP_TYPE_AKA:
		break;

	default:
		ERROR_LOG(UG_NAME_NORMAL, "Unknown EAP method %d", eap_type);
		break;
	}

	/* Before we proceed to make a connection, lets save the entered IP data */
	ip_info_save_data(eap_data->ip_info_list);

	wlan_manager_connect(eap_data->ap);

	__common_eap_connect_cleanup(eap_data);

	elm_naviframe_item_pop(eap_data->navi_frame);

	if(str_id != NULL) {
		g_free(str_id);
		str_id = NULL;
	}
	if(str_pw != NULL) {
		g_free(str_pw);
		str_pw = NULL;
	}

	__COMMON_FUNC_EXIT__;
	return;
}

static gboolean __common_eap_connect_show_ime(void *data)
{
	Elm_Object_Item *list_entry_item = (Elm_Object_Item *)data;
	if (!list_entry_item) {
		return FALSE;
	}

	Evas_Object *entry = elm_object_item_part_content_get(list_entry_item, "elm.icon.entry");
	if (!entry) {
		return FALSE;
	}

	g_eap_id_show_keypad = TRUE;
	elm_genlist_item_update(list_entry_item);
	return FALSE;
}

static gboolean __common_eap_connect_load_ip_info_list_cb(void *data)
{
	eap_connect_data_t *eap_data = (eap_connect_data_t *)data;
	Elm_Object_Item *navi_it = NULL;
	Evas_Object *list = NULL;

	if (!eap_data) {
		return FALSE;
	}

	if (eap_data->navi_frame) {
		Evas_Object *layout = NULL;
		navi_it = elm_naviframe_top_item_get(eap_data->navi_frame);
		layout = elm_object_item_part_content_get(navi_it, "elm.swallow.content");
		list = elm_object_part_content_get(layout, "elm.swallow.content");
		eap_data->ip_info_list = ip_info_append_items(eap_data->ap,
						eap_data->str_pkg_name, list,
						__common_eap_connect_imf_ctxt_evnt_cb, navi_it);
	} else {
		Evas_Object *box = elm_object_content_get(eap_data->popup);
		Eina_List *box_childs = elm_box_children_get(box);
		list = eina_list_nth(box_childs, 0);
		eap_data->ip_info_list = ip_info_append_items(eap_data->ap,
						eap_data->str_pkg_name, list,
						__common_eap_connect_im_ctxt_evnt_resize_cb, eap_data);
	}

	common_util_managed_idle_add(__common_eap_connect_show_ime, eap_data->eap_id_item);

	return FALSE;
}

static void __cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	elm_naviframe_item_pop(data);

	__COMMON_FUNC_EXIT__;
}

eap_connect_data_t *create_eap_view(int ug_type,
		Evas_Object *layout_main, Evas_Object *navi_frame,
		const char *pkg_name, wifi_device_info_t *device_info,
		Evas_Object *win)
{
	__COMMON_FUNC_ENTER__;

	/* Create eap connect view */
	Elm_Object_Item *navi_it = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *list = NULL;
	Evas_Object *temp_parent = NULL;
	Evas_Object *bottom_view = NULL;
	guint id;

	if (layout_main == NULL || device_info == NULL || pkg_name == NULL) {
		return NULL;
	}

	eap_connect_data_t *eap_data = g_try_new0(eap_connect_data_t, 1);
	if (eap_data == NULL) {
		return NULL;
	}

	eap_data->str_pkg_name = pkg_name;
	eap_data->layout = layout_main;
	eap_data->win = win;

	/* Clone the WiFi AP handle */
	wifi_ap_clone(&(eap_data->ap), device_info->ap);

	eap_data->navi_frame = navi_frame;

	if (ug_type == UG_VIEW_SETUP_WIZARD) {
		layout = elm_layout_add(navi_frame);

		elm_layout_file_set(layout, SETUP_WIZARD_EDJ_PATH, "pwlock");
		elm_object_part_text_set(layout,
				"text.title", device_info->ssid);

		navi_it = elm_naviframe_item_push(navi_frame, NULL, NULL, NULL,
				layout, NULL);
		elm_naviframe_item_title_enabled_set(navi_it, EINA_FALSE, EINA_FALSE);

		int change_ang = elm_win_rotation_get(win);
		if (change_ang == 0 || change_ang == 180) {
			common_utils_contents_rotation_adjust(UG_EVENT_ROTATE_PORTRAIT);
			edje_object_signal_emit((Evas_Object *)elm_layout_edje_get(layout)
				,"location,vertical", "elm");
		} else {
			common_utils_contents_rotation_adjust(UG_EVENT_ROTATE_LANDSCAPE);
			edje_object_signal_emit((Evas_Object *)elm_layout_edje_get(layout),
				"location,horizontal", "elm");
		}
	} else {
		layout = common_utils_create_layout(navi_frame);

		navi_it = elm_naviframe_item_push(navi_frame, device_info->ssid, NULL, NULL,
				layout, NULL);
	}

	/* Create an EAP connect view list */
	list = _create_list(layout, eap_data);
	elm_object_part_content_set(layout, "elm.swallow.content", list);

	evas_object_data_set(navi_frame, SCREEN_TYPE_ID_KEY,
			(void *)VIEW_MANAGER_VIEW_TYPE_EAP);

	/* Tool bar Connect button */
#if 0
	Evas_Object *toolbar = elm_toolbar_add(navi_frame);
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);

	elm_toolbar_item_append(toolbar, NULL,
			sc(pkg_name, I18N_TYPE_Connect),
			__common_eap_connect_done_cb, eap_data);

	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
#endif

	if (ug_type == UG_VIEW_SETUP_WIZARD) {
		temp_parent = layout;
	} else {
		bottom_view = elm_layout_add(layout);
		elm_layout_file_set(bottom_view, CUSTOM_EDITFIELD_PATH, "eap_bottom");

		temp_parent = bottom_view;
	}

	Evas_Object *cancel_btn = elm_button_add(temp_parent);
	elm_object_style_set(cancel_btn, "bottom");
	elm_object_text_set(cancel_btn, sc(pkg_name, I18N_TYPE_Cancel));
	evas_object_show(cancel_btn);
	evas_object_smart_callback_add(cancel_btn, "clicked", __cancel_cb, eap_data->navi_frame);

	Evas_Object *connect_btn = elm_button_add(temp_parent);
	elm_object_style_set(connect_btn, "bottom");
	elm_object_text_set(connect_btn, sc(pkg_name, I18N_TYPE_Connect));
	evas_object_show(connect_btn);
	evas_object_smart_callback_add(connect_btn, "clicked", __common_eap_connect_done_cb, eap_data);

	if (ug_type == UG_VIEW_SETUP_WIZARD) {
		elm_object_part_content_set(layout, "button.prev", cancel_btn);
		elm_object_part_content_set(layout, "button.next", connect_btn);
	} else {
		elm_object_part_content_set(bottom_view, "cancel_btn", cancel_btn);
		elm_object_part_content_set(bottom_view, "connect_btn", connect_btn);

		elm_object_item_part_content_set(navi_it, "toolbar", bottom_view);
	}

	/* Append ip info items and add a separator */
	id = common_util_managed_idle_add(__common_eap_connect_load_ip_info_list_cb, eap_data);
	if (!id) {
		g_free(eap_data);
		return NULL;
	}

	/* Set pop callback */
	elm_naviframe_item_pop_cb_set(navi_it, __common_eap_connect_destroy, eap_data);

	/* Register imf event cbs */
	__common_eap_view_set_imf_ctxt_evnt_cb(eap_data);

	__COMMON_FUNC_EXIT__;
	return eap_data;
}

eap_connect_data_t *create_eap_popup(Evas_Object *layout, Evas_Object *win,
		const char *pkg_name, wifi_device_info_t *device_info)
{
	__COMMON_FUNC_ENTER__;

	guint id;
	Evas_Object *list = NULL;

	if (layout == NULL || win == NULL || device_info == NULL || pkg_name == NULL) {
		return NULL;
	}

	eap_connect_data_t *eap_data = g_try_new0(eap_connect_data_t, 1);
	if (eap_data == NULL) {
		return NULL;
	}

	eap_data->str_pkg_name = pkg_name;
	eap_data->layout = layout;
	eap_data->win = win;

	/* Clone the WiFi AP handle */
	wifi_ap_clone(&(eap_data->ap), device_info->ap);

	/* Create eap connect popup */
	Evas_Object *popup;
	Evas_Object *box;
	Evas_Object *btn;
	int rotate_angle;

	/* Lets disable the scan updates so that the UI is not refreshed unnecessarily */
	wlan_manager_disable_scan_result_update();

	eap_data->popup = popup = elm_popup_add(layout);
	elm_object_style_set(popup, "default");
	elm_object_part_text_set(popup, "title,text", device_info->ssid);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup");
	elm_object_text_set(btn, sc(pkg_name, I18N_TYPE_Cancel));
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked",
			__common_eap_connect_destroy_cb, eap_data);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup");
	elm_object_text_set(btn, sc(pkg_name, I18N_TYPE_Connect));
	elm_object_part_content_set(popup, "button2", btn);
	evas_object_smart_callback_add(btn, "clicked",
			__common_eap_connect_done_cb, eap_data);

	/* Create and add a box into the layout. */
	box = elm_box_add(popup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);

	rotate_angle = common_utils_get_rotate_angle(APPCORE_RM_UNKNOWN);
	if (0 == rotate_angle || 180 == rotate_angle) {
		evas_object_size_hint_min_set(box, -1,
				ELM_SCALE_SIZE(DEVICE_PICKER_POPUP_H));
	} else {
		evas_object_size_hint_min_set(box, -1,
				ELM_SCALE_SIZE(DEVICE_PICKER_POPUP_LN_H));
	}

	evas_object_name_set(box, EAP_CONNECT_POPUP);

	/* Create an EAP connect view list */
	list = _create_list(box, eap_data);

	/* Append ip info items and add a separator */
	id = common_util_managed_idle_add(__common_eap_connect_load_ip_info_list_cb, eap_data);
	if (!id) {
		g_free(eap_data);
		return NULL;
	}

	/* Pack the list into the box */
	elm_box_pack_end(box, list);
	elm_object_content_set(popup, box);
	evas_object_show(list);
	evas_object_show(box);

	evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN,
			__eap_popup_keydown_cb, eap_data);

	evas_object_show(popup);
	elm_object_focus_set(popup, EINA_TRUE);

	__common_eap_popup_set_imf_ctxt_evnt_cb(eap_data);

	__COMMON_FUNC_EXIT__;
	return eap_data;
}

static wifi_eap_type_e __common_eap_connect_popup_get_wlan_eap_type(eap_type_t eap_type)
{
	wifi_eap_type_e wlan_eap_type = WLAN_SEC_EAP_TYPE_PEAP;
	switch (eap_type) {
	case EAP_SEC_TYPE_PEAP:
		wlan_eap_type = WIFI_EAP_TYPE_PEAP;
		break;
	case EAP_SEC_TYPE_TLS:
		wlan_eap_type = WIFI_EAP_TYPE_TLS;
		break;
	case EAP_SEC_TYPE_TTLS:
		wlan_eap_type = WIFI_EAP_TYPE_TTLS;
		break;
	case EAP_SEC_TYPE_SIM:
		wlan_eap_type = WIFI_EAP_TYPE_SIM;
		break;
	case EAP_SEC_TYPE_AKA:
		wlan_eap_type = WIFI_EAP_TYPE_AKA;
		break;
	default:
		/* This case should never occur */
		ERROR_LOG(UG_NAME_NORMAL, "Err!");
		break;
	}

	return wlan_eap_type;
}

static wifi_eap_auth_type_e __common_eap_connect_popup_get_wlan_auth_type(eap_auth_t auth_type)
{
	wifi_eap_auth_type_e wlan_auth_type = WIFI_EAP_AUTH_TYPE_NONE;
	switch (auth_type) {
	case EAP_SEC_AUTH_NONE:
		wlan_auth_type = WIFI_EAP_AUTH_TYPE_NONE;
		break;
	case EAP_SEC_AUTH_PAP:
		wlan_auth_type = WIFI_EAP_AUTH_TYPE_PAP;
		break;
	case EAP_SEC_AUTH_MSCHAP:
		wlan_auth_type = WIFI_EAP_AUTH_TYPE_MSCHAP;
		break;
	case EAP_SEC_AUTH_MSCHAPV2:
		wlan_auth_type = WIFI_EAP_AUTH_TYPE_MSCHAPV2;
		break;
	case EAP_SEC_AUTH_GTC:
		wlan_auth_type = WIFI_EAP_AUTH_TYPE_GTC;
		break;
	case EAP_SEC_AUTH_MD5:
		wlan_auth_type = WIFI_EAP_AUTH_TYPE_MD5;
		break;
	default:
		/* This case should never occur */
		ERROR_LOG(UG_NAME_NORMAL, "Err!");
		break;
	}
	return wlan_auth_type;
}

static eap_type_t __common_eap_connect_popup_get_eap_type(wifi_ap_h ap)
{
	wifi_eap_type_e wlan_eap_type = 0;
	int ret = wifi_ap_get_eap_type(ap, &wlan_eap_type);
	if (WIFI_ERROR_OPERATION_FAILED == ret) {
		ret = wifi_ap_set_eap_type(ap, WIFI_EAP_TYPE_PEAP);	// Set to default
	}

	if (WIFI_ERROR_NONE != ret) {
		ERROR_LOG(UG_NAME_ERR, "Unable to get the eap type. err = %d", ret);
		return EAP_SEC_TYPE_UNKNOWN;
	}
	INFO_LOG(UG_NAME_NORMAL, "WiFi EAP type = %d", wlan_eap_type);
	switch (wlan_eap_type) {
	case WIFI_EAP_TYPE_PEAP:  /**< EAP PEAP type */
		return EAP_SEC_TYPE_PEAP;

	case WIFI_EAP_TYPE_TLS:  /**< EAP TLS type */
		return EAP_SEC_TYPE_TLS;

	case WIFI_EAP_TYPE_TTLS:  /**< EAP TTLS type */
		return EAP_SEC_TYPE_TTLS;

	case WIFI_EAP_TYPE_SIM:  /**< EAP SIM type */
		return EAP_SEC_TYPE_SIM;

	case WIFI_EAP_TYPE_AKA:  /**< EAP AKA type */
		return EAP_SEC_TYPE_AKA;

	default:
		return EAP_SEC_TYPE_PEAP;
	}
	return EAP_SEC_TYPE_PEAP;
}

static eap_auth_t __common_eap_connect_popup_get_auth_type(wifi_ap_h ap)
{
	wifi_eap_auth_type_e wlan_auth_type = 0;
	int ret = wifi_ap_get_eap_auth_type(ap, &wlan_auth_type);
	if (WIFI_ERROR_OPERATION_FAILED == ret) {
		ret = wifi_ap_set_eap_auth_type(ap, EAP_SEC_AUTH_NONE);	// Set to default
	}

	if (WIFI_ERROR_NONE != ret) {
		ERROR_LOG(UG_NAME_ERR, "Unable to get the eap auth type. err = %d", ret);
		return EAP_SEC_AUTH_NONE;
	}
	INFO_LOG(UG_NAME_NORMAL, "WiFi EAP auth type = %d", wlan_auth_type);

    switch (wlan_auth_type) {
	case WIFI_EAP_AUTH_TYPE_NONE:  /**< EAP phase2 authentication none */
		return EAP_SEC_AUTH_NONE;

	case WIFI_EAP_AUTH_TYPE_PAP:  /**< EAP phase2 authentication PAP */
		return EAP_SEC_AUTH_PAP;

	case WIFI_EAP_AUTH_TYPE_MSCHAP:  /**< EAP phase2 authentication MSCHAP */
		return EAP_SEC_AUTH_MSCHAP;

	case WIFI_EAP_AUTH_TYPE_MSCHAPV2:  /**< EAP phase2 authentication MSCHAPv2 */
		return EAP_SEC_AUTH_MSCHAPV2;

	case WIFI_EAP_AUTH_TYPE_GTC:  /**< EAP phase2 authentication GTC */
		return EAP_SEC_AUTH_GTC;

	case WIFI_EAP_AUTH_TYPE_MD5:  /**< EAP phase2 authentication MD5 */
		return EAP_SEC_AUTH_MD5;

	default:
		return EAP_SEC_AUTH_NONE;
	}
	return EAP_SEC_AUTH_NONE;
}

static char *_eap_info_get_user_cert_alias(wifi_ap_h ap)
{
	char *path = NULL;
	char *alias = NULL;
	char *filename = NULL;
	char *cert_name = NULL;
	int alias_len = 0;

	wifi_ap_get_eap_client_cert_file(ap, &path);
	if (path == NULL)
		return NULL;

	filename = strrchr(path, '/');
	if (filename == NULL) {
		ERROR_LOG(UG_NAME_ERR, "Invalid file name");
		return NULL;
	}

	filename++;
	cert_name = strstr(filename, EAP_TLS_USER_CERT_PATH);
	if (cert_name == NULL) {
		ERROR_LOG(UG_NAME_ERR, "Invalid file name");
		return NULL;
	}

	cert_name--;
	alias_len = cert_name - filename;

	alias = g_try_malloc0(alias_len + 1);
	if (alias == NULL) {
		ERROR_LOG(UG_NAME_ERR, "Invalid file name");
		g_free(path);
		return NULL;
	}

	g_strlcpy(alias, filename, alias_len + 1);
	INFO_LOG(UG_NAME_NORMAL, "Alias [%s] length [%d]", alias, alias_len);

	g_free(path);

	return alias;
}

/* This creates Auth type, ID, Anonymous Id and Password items
 * This function should be called after creating the EAP type item
 */
eap_info_list_t *eap_info_append_items(wifi_ap_h ap, Evas_Object* view_list,
		const char *str_pkg_name, imf_ctxt_panel_cb_t input_panel_cb,
		void *input_panel_cb_data)
{
	__COMMON_FUNC_ENTER__;

	eap_type_t eap_type;
	eap_auth_t auth_type;
	char *temp_str = NULL;
	Eina_Bool append_continue = TRUE;
	eap_info_list_t *eap_info_list_data = NULL;
	Elm_Object_Item* item = NULL;
	if (!view_list || !str_pkg_name || !ap) {
		ERROR_LOG(UG_NAME_ERR, "Invalid params passed!");
		return NULL;
	}

	eap_info_list_data = g_try_new0(eap_info_list_t, 1);
	if (eap_info_list_data == NULL) {
		return NULL;
	}

	eap_info_list_data->ap = ap;
	eap_type = __common_eap_connect_popup_get_eap_type(ap);
	auth_type = __common_eap_connect_popup_get_auth_type(ap);

	item = common_utils_add_2_line_txt_disabled_item(view_list,
			"2line.top",
			sc(str_pkg_name, I18N_TYPE_EAP_method),
			list_eap_type[eap_type].name);
	eap_info_list_data->eap_method_item = item;

	eap_info_list_data->eap_type = eap_type;

	switch (eap_type) {
	case EAP_SEC_TYPE_UNKNOWN:
	case EAP_SEC_TYPE_PEAP:
	case EAP_SEC_TYPE_TLS:
	case EAP_SEC_TYPE_TTLS:
		break;
	case EAP_SEC_TYPE_SIM:
	case EAP_SEC_TYPE_AKA:
	default:
		append_continue = FALSE;
		break;
	}

	if (append_continue) {
		common_utils_entry_info_t *edit_box_details;

		/* Add EAP phase2 authentication */
		item = common_utils_add_2_line_txt_disabled_item(view_list,
				"2line.top",
				sc(str_pkg_name, I18N_TYPE_Phase_2_authentication),
				list_eap_auth[auth_type].name);
		eap_info_list_data->eap_auth_item = item;

		/* Add User certificate */
		temp_str = _eap_info_get_user_cert_alias(ap);

		if (temp_str == NULL || strlen(temp_str) == 0) {
			if (temp_str != NULL) {
				g_free(temp_str);
			}
			temp_str = g_strdup(sc(str_pkg_name, I18N_TYPE_Unspecified));
		}

		item = common_utils_add_2_line_txt_disabled_item(view_list, "2line.top",
				sc(str_pkg_name, I18N_TYPE_User_Certificate), temp_str);
		eap_info_list_data->user_cert_item = item;
		g_free(temp_str);

		/* Add EAP ID */
		bool is_paswd_set;
		temp_str = NULL;
		wifi_ap_get_eap_passphrase(ap, &temp_str, &is_paswd_set);
		item = common_utils_add_2_line_txt_disabled_item(view_list, "2line.top",
				sc(str_pkg_name, I18N_TYPE_Identity), temp_str);
		eap_info_list_data->id_item = item;
		g_free(temp_str);

		/* Add EAP Password */
		g_eap_entry_itc.item_style = "entry.main";
		g_eap_entry_itc.func.text_get = _gl_eap_entry_item_text_get;
		g_eap_entry_itc.func.content_get = _gl_eap_entry_item_content_get;
		g_eap_entry_itc.func.state_get = NULL;
		g_eap_entry_itc.func.del = _gl_eap_entry_item_del;

		edit_box_details = g_try_new0(common_utils_entry_info_t, 1);
		if (edit_box_details == NULL) {
			g_free(eap_info_list_data);
			return NULL;
		}

		edit_box_details->entry_id = ENTRY_TYPE_PASSWORD;
		edit_box_details->title_txt = sc(str_pkg_name, I18N_TYPE_Password);
		edit_box_details->entry_txt = NULL;
		edit_box_details->guide_txt = sc(str_pkg_name, I18N_TYPE_Enter_password);
		edit_box_details->input_panel_cb = input_panel_cb;
		edit_box_details->input_panel_cb_data = input_panel_cb_data;
		edit_box_details->item = elm_genlist_item_append(view_list,
				&g_eap_entry_itc, edit_box_details, NULL,
				ELM_GENLIST_ITEM_NONE, _gl_editbox_sel_cb, NULL);
		elm_genlist_item_select_mode_set(edit_box_details->item,
				ELM_OBJECT_SELECT_MODE_NONE);
		eap_info_list_data->pswd_item = edit_box_details->item;
	}

	__COMMON_FUNC_EXIT__;
	return eap_info_list_data;
}

void eap_info_save_data(eap_info_list_t *eap_info_list_data)
{
	if (!eap_info_list_data) {
		ERROR_LOG(UG_NAME_ERR, "Invalid params passed!");
		return;
	}
	char *txt = common_utils_get_list_item_entry_txt(eap_info_list_data->pswd_item);

	wifi_ap_set_eap_passphrase(eap_info_list_data->ap, NULL, txt);
	g_free(txt);
	return;
}

void eap_info_remove(eap_info_list_t *eap_info_list_data)
{
	if (!eap_info_list_data) {
		ERROR_LOG(UG_NAME_ERR, "Invalid params passed!");
		return;
	}

	g_free(eap_info_list_data);
}

void eap_connect_data_free(eap_connect_data_t *eap_data)
{
	__common_eap_connect_cleanup(eap_data);
}

void eap_view_rotate_popup(eap_connect_data_t *eap_data, int rotate_angle)
{
	__COMMON_FUNC_ENTER__;

	if (NULL == eap_data || NULL == eap_data->popup) {
		return;
	}

	int value = eap_data->key_status;
	Evas_Object *box = elm_object_content_get(eap_data->popup);

	if (value == ECORE_IMF_INPUT_PANEL_STATE_SHOW) {
		DEBUG_LOG(UG_NAME_NORMAL, "Key pad is open");

		if (rotate_angle == 0 || rotate_angle == 180) {
			evas_object_size_hint_min_set(box, -1,
					ELM_SCALE_SIZE(530));
		} else {
			evas_object_size_hint_min_set(box, -1,
					ELM_SCALE_SIZE(200));
		}
	} else if (value == ECORE_IMF_INPUT_PANEL_STATE_HIDE) {
		if (rotate_angle == 0 || rotate_angle == 180) {
			evas_object_size_hint_min_set(box, -1,
					ELM_SCALE_SIZE(DEVICE_PICKER_POPUP_H));
		} else {
			evas_object_size_hint_min_set(box, -1,
					ELM_SCALE_SIZE(DEVICE_PICKER_POPUP_LN_H));
		}
	}

	__COMMON_FUNC_EXIT__;
	return;
}

void eap_info_items_realize(eap_info_list_t *eap_info_list)
{
	retm_if(NULL == eap_info_list);

	Elm_Object_Item *item = NULL;

	item = eap_info_list->eap_method_item;
	if (item != NULL) {
		elm_object_item_signal_emit(item, "elm,state,top", "");
	}
	item = eap_info_list->eap_auth_item;
	if (item != NULL) {
		elm_object_item_signal_emit(item, "elm,state,center", "");
	}
	item = eap_info_list->user_cert_item;
	if (item != NULL) {
		elm_object_item_signal_emit(item, "elm,state,center", "");
	}
	item = eap_info_list->id_item;
	if (item != NULL) {
		elm_object_item_signal_emit(item, "elm,state,center", "");
	}
	item = eap_info_list->pswd_item;
	if (item != NULL) {
		elm_object_item_signal_emit(item, "elm,state,bottom", "");
	}
}
