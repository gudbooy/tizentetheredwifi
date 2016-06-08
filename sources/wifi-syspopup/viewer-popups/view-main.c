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

#include <utilX.h>
#include "common.h"
#include "view-main.h"
#include "common_pswd_popup.h"
#include "common_generate_pin.h"
#include "common_utils.h"
#include "view-alerts.h"
#include "i18nmanager.h"
#include "common_eap_connect.h"

#define QS_POPUP_CONNECTION_STATE	"qs_popup_connection_state"
#define WIFI_DEVPKR_EDJ "/usr/apps/wifi-efl-ug/res/edje/wifi-qs/wifi-syspopup-custom.edj"
#define WIFI_SYSPOPUP_EMPTY_GRP "devpkr_no_wifi_networks"

extern wifi_object* devpkr_app_state;

static Evas_Object* list = NULL;
static Elm_Genlist_Item_Class itc;
static int profiles_list_size = 0;
static Elm_Genlist_Item_Class grouptitle_itc;
static Elm_Object_Item *grouptitle = NULL;

static GList *wifi_device_list = NULL;

int view_main_get_profile_count(void)
{
	return profiles_list_size;
}

static ITEM_CONNECTION_MODES view_main_state_get(void)
{
	ITEM_CONNECTION_MODES state;

	state = (ITEM_CONNECTION_MODES)evas_object_data_get(
			list, QS_POPUP_CONNECTION_STATE);

	return state;
}

static void view_main_state_set(ITEM_CONNECTION_MODES state)
{
	evas_object_data_set(list, QS_POPUP_CONNECTION_STATE, (const void *)state);
}

static void __popup_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	wifi_ap_h ap = NULL;
	int password_len = 0;
	const char* password = NULL;
	wifi_security_type_e sec_type = WIFI_SECURITY_TYPE_NONE;

	if (devpkr_app_state->passpopup == NULL) {
		return;
	}

	ap = passwd_popup_get_ap(devpkr_app_state->passpopup);
	password = passwd_popup_get_txt(devpkr_app_state->passpopup);
	password_len = strlen(password);

	wifi_ap_get_security_type(ap, &sec_type);

	switch (sec_type) {
	case WIFI_SECURITY_TYPE_WEP:
		if (password_len != 5 && password_len != 13 &&
				password_len != 26 && password_len != 10) {
			view_alerts_popup_show(sc(PACKAGE, I18N_TYPE_Wrong_Password));
			goto popup_ok_exit;
		}
		break;

	case WIFI_SECURITY_TYPE_WPA_PSK:
	case WIFI_SECURITY_TYPE_WPA2_PSK:
		if (password_len < 8 || password_len > 64) {
			view_alerts_popup_show(sc(PACKAGE, I18N_TYPE_Wrong_Password));
			goto popup_ok_exit;
		}
		break;

	default:
		ERROR_LOG(SP_NAME_ERR, "Wrong security mode: %d", sec_type);
		passwd_popup_free(devpkr_app_state->passpopup);
		break;
	}

	wlan_manager_connect_with_password(ap, password);

	passwd_popup_free(devpkr_app_state->passpopup);
	devpkr_app_state->passpopup = NULL;

popup_ok_exit:
	g_free((gpointer)password);

	__COMMON_FUNC_EXIT__;
}

static void __popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	if (devpkr_app_state->passpopup == NULL) {
		return;
	}

	passwd_popup_free(devpkr_app_state->passpopup);
	devpkr_app_state->passpopup = NULL;

	__COMMON_FUNC_EXIT__;
}

static void __wps_pbc_popup_cancel_connecting(void *data, Evas_Object *obj,
		void *event_info)
{
	if (devpkr_app_state->passpopup == NULL) {
		return;
	}

	wifi_ap_h ap = passwd_popup_get_ap(devpkr_app_state->passpopup);

	int ret = wlan_manager_disconnect(ap);
	if (ret != WLAN_MANAGER_ERR_NONE) {
		ERROR_LOG(SP_NAME_ERR, "Failed WPS PBC cancellation [0x%x]", ap);
	}

	passwd_popup_free(devpkr_app_state->passpopup);
	devpkr_app_state->passpopup = NULL;
}

static void __pbc_popup_keydown_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	Evas_Event_Key_Down *event = event_info;

	if (g_strcmp0(event->keyname, KEY_BACK) == 0) {
		__wps_pbc_popup_cancel_connecting(data, obj, event_info);
	}

	__COMMON_FUNC_EXIT__;
}

static void _wps_btn_cb(void* data, Evas_Object* obj, void* event_info)
{
	__COMMON_FUNC_ENTER__;

	if (!devpkr_app_state->passpopup) {
		return;
	}

	wifi_ap_h ap = passwd_popup_get_ap(devpkr_app_state->passpopup);
	int ret = wlan_manager_wps_connect(ap);
	if (ret == WLAN_MANAGER_ERR_NONE) {
		create_pbc_popup(devpkr_app_state->passpopup,
				__wps_pbc_popup_cancel_connecting, NULL,
				POPUP_WPS_BTN, NULL);
		evas_object_event_callback_add(
				devpkr_app_state->passpopup->pbc_popup_data->popup,
				EVAS_CALLBACK_KEY_DOWN,
				__pbc_popup_keydown_cb, NULL);
	} else {
		ERROR_LOG(SP_NAME_ERR, "wlan_manager_wps_connect failed");
		wifi_ap_destroy(ap);

		passwd_popup_free(devpkr_app_state->passpopup);
		devpkr_app_state->passpopup = NULL;
	}

	__COMMON_FUNC_EXIT__;
}

static void _wps_cancel_cb(void* data, Evas_Object* obj, void* event_info)
{
	__COMMON_FUNC_ENTER__;

	if (devpkr_app_state->passpopup == NULL) {
		return;
	}

	current_popup_free(devpkr_app_state->passpopup, POPUP_WPS_OPTIONS);

	__COMMON_FUNC_EXIT__;
}

static void _wps_pin_cb(void* data, Evas_Object* obj, void* event_info)
{
	__COMMON_FUNC_ENTER__;

	unsigned int rpin = 0;
	char npin[9] = { '\0' };
	int pin_len = 0;
	int ret = WLAN_MANAGER_ERR_NONE;
	wifi_ap_h ap = NULL;

	if (!devpkr_app_state->passpopup) {
		return;
	}

	/* Generate WPS pin */
	rpin = wps_generate_pin();
	if (rpin > 0)
		g_snprintf(npin, sizeof(npin), "%08d", rpin);

	pin_len = strlen(npin);
	if (pin_len != 8) {
		view_alerts_popup_show(sc(PACKAGE, I18N_TYPE_Invalid_pin));

		__COMMON_FUNC_EXIT__;
		return;
	}

	ap = passwd_popup_get_ap(devpkr_app_state->passpopup);

	ret = wlan_manager_wps_pin_connect(ap, npin);
	if (ret == WLAN_MANAGER_ERR_NONE) {
		INFO_LOG(UG_NAME_NORMAL, "wlan_manager_wps_pin_connect successful");

		create_pbc_popup(devpkr_app_state->passpopup,
				__wps_pbc_popup_cancel_connecting, NULL,
				POPUP_WPS_PIN, npin);
		evas_object_event_callback_add(
				devpkr_app_state->passpopup->pbc_popup_data->popup,
				EVAS_CALLBACK_KEY_DOWN,
				__pbc_popup_keydown_cb, NULL);
	} else {
		ERROR_LOG(UG_NAME_NORMAL, "wlan_manager_wps_pin_connect failed");

		passwd_popup_free(devpkr_app_state->passpopup);
		devpkr_app_state->passpopup = NULL;
	}

	__COMMON_FUNC_EXIT__;
}

static void __popup_wps_options_cb(void* data, Evas_Object* obj, void* event_info)
{
	__COMMON_FUNC_ENTER__;
	pswd_popup_create_req_data_t popup_info;

	if (!devpkr_app_state->passpopup) {
		return;
	}

	if (!event_info) {
		return;
	}

	Elm_Object_Item *item = event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);

	memset(&popup_info, 0, sizeof(pswd_popup_create_req_data_t));

	popup_info.title = g_strdup(sc(PACKAGE, I18N_TYPE_Select_WPS_Method));
	popup_info.ok_cb = NULL;
	popup_info.cancel_cb = _wps_cancel_cb;
	popup_info.show_wps_btn = EINA_FALSE;
	popup_info.wps_btn_cb = _wps_btn_cb;
	popup_info.wps_pin_cb = _wps_pin_cb;
	popup_info.ap = passwd_popup_get_ap(devpkr_app_state->passpopup);
	popup_info.cb_data = NULL;
	create_wps_options_popup(devpkr_app_state->layout_main,
			devpkr_app_state->passpopup, &popup_info);

	__COMMON_FUNC_EXIT__;
}

void view_main_wifi_reconnect(devpkr_gl_data_t *gdata)
{
	wifi_device_info_t *device_info;
	pswd_popup_create_req_data_t popup_info;
	wifi_security_type_e sec_type = WIFI_SECURITY_TYPE_NONE;

	retm_if(NULL == gdata);

	device_info = gdata->dev_info;
	retm_if(NULL == device_info);

	wifi_ap_get_security_type(device_info->ap, &sec_type);

	switch (sec_type) {
	case WIFI_SECURITY_TYPE_WEP:
	case WIFI_SECURITY_TYPE_WPA_PSK:
	case WIFI_SECURITY_TYPE_WPA2_PSK:
		memset(&popup_info, 0, sizeof(pswd_popup_create_req_data_t));

		popup_info.title = gdata->dev_info->ssid;
		popup_info.ok_cb = __popup_ok_cb;
		popup_info.cancel_cb = __popup_cancel_cb;
		popup_info.show_wps_btn = gdata->dev_info->wps_mode;
		popup_info.wps_btn_cb = __popup_wps_options_cb;
		popup_info.ap = gdata->dev_info->ap;
		popup_info.cb_data = NULL;
		popup_info.sec_type = sec_type;

		if (devpkr_app_state->passpopup != NULL) {
			passwd_popup_free(devpkr_app_state->passpopup);
			devpkr_app_state->passpopup = NULL;
		}

		devpkr_app_state->passpopup = create_passwd_popup(
				devpkr_app_state->layout_main, PACKAGE,
				&popup_info);
		if (devpkr_app_state->passpopup == NULL) {
			ERROR_LOG(SP_NAME_NORMAL, "Password popup creation failed");
		}
		break;

	case WIFI_SECURITY_TYPE_EAP:
		devpkr_app_state->eap_popup = create_eap_popup(
				devpkr_app_state->layout_main, devpkr_app_state->win_main,
				PACKAGE, gdata->dev_info);
		break;

	default:
		ERROR_LOG(SP_NAME_NORMAL, "Unknown security type [%d]", sec_type);
		break;
	}
}

void view_main_wifi_connect(devpkr_gl_data_t *gdata)
{
	__COMMON_FUNC_ENTER__;
	bool favorite = false;
	wifi_device_info_t *device_info;
	pswd_popup_create_req_data_t popup_info;
	wifi_security_type_e sec_type = WIFI_SECURITY_TYPE_NONE;

	retm_if(NULL == gdata);

	device_info = gdata->dev_info;
	retm_if(NULL == device_info);
	MIN_LOG("wifi_ap_is_favorite called");
	wifi_ap_is_favorite(device_info->ap, &favorite);

	if (favorite == true) {
		wlan_manager_connect(device_info->ap);
		__COMMON_FUNC_EXIT__;
		return;
	}

	wifi_ap_get_security_type(device_info->ap, &sec_type);

	switch (sec_type) {
	case WIFI_SECURITY_TYPE_NONE:
		wlan_manager_connect(device_info->ap);
		break;

	case WIFI_SECURITY_TYPE_WEP:
	case WIFI_SECURITY_TYPE_WPA_PSK:
	case WIFI_SECURITY_TYPE_WPA2_PSK:
		memset(&popup_info, 0, sizeof(pswd_popup_create_req_data_t));

		popup_info.title = gdata->dev_info->ssid;
		popup_info.ok_cb = __popup_ok_cb;
		popup_info.cancel_cb = __popup_cancel_cb;
		popup_info.show_wps_btn = gdata->dev_info->wps_mode;
		popup_info.wps_btn_cb = __popup_wps_options_cb;
		popup_info.ap = gdata->dev_info->ap;
		popup_info.cb_data = NULL;
		popup_info.sec_type = sec_type;

		if (devpkr_app_state->passpopup != NULL) {
			passwd_popup_free(devpkr_app_state->passpopup);
			devpkr_app_state->passpopup = NULL;
		}

		devpkr_app_state->passpopup = create_passwd_popup(
				devpkr_app_state->layout_main, PACKAGE,
				&popup_info);
		if (devpkr_app_state->passpopup == NULL) {
			ERROR_LOG(SP_NAME_NORMAL, "Password popup creation failed");
		}
		break;

	case WIFI_SECURITY_TYPE_EAP:
		devpkr_app_state->eap_popup = create_eap_popup(
				devpkr_app_state->layout_main, devpkr_app_state->win_main,
				PACKAGE, gdata->dev_info);
		break;

	default:
		ERROR_LOG(SP_NAME_NORMAL, "Unknown security type [%d]", sec_type);
		break;
	}

		__COMMON_FUNC_EXIT__;
}

Elm_Object_Item *view_main_item_get_for_ap(wifi_ap_h ap)
{
	__COMMON_FUNC_ENTER__;
	if (!ap || !list) {
		__COMMON_FUNC_EXIT__;
		return NULL;
	}

	char *essid = NULL;
	wifi_security_type_e type = WIFI_SECURITY_TYPE_NONE;

	if (WIFI_ERROR_NONE != wifi_ap_get_essid(ap, &essid)) {
		__COMMON_FUNC_EXIT__;
		return NULL;
	}
	if (WIFI_ERROR_NONE != wifi_ap_get_security_type(ap, &type)) {
		g_free(essid);
		__COMMON_FUNC_EXIT__;
		return NULL;
	}

	Elm_Object_Item *it = elm_genlist_first_item_get(list);
	wlan_security_mode_type_t sec_mode = common_utils_get_sec_mode(type);
	while (it) {
		devpkr_gl_data_t* gdata = elm_object_item_data_get(it);
		wifi_device_info_t *device_info = NULL;
		if (gdata && (device_info = gdata->dev_info)) {
			if (!g_strcmp0(device_info->ssid, essid) && device_info->security_mode == sec_mode) {
				break;
			}
		}

		it = elm_genlist_item_next_get(it);
	}

	g_free(essid);
	__COMMON_FUNC_EXIT__;
	return it;
}

#if 0
/* Unused function */
Elm_Object_Item *__view_main_get_item_in_mode(ITEM_CONNECTION_MODES mode)
{
	Elm_Object_Item* it = NULL;
	it = elm_genlist_first_item_get(list);
	__COMMON_FUNC_ENTER__;
	while (it) {
		devpkr_gl_data_t *gdata = (devpkr_gl_data_t *)elm_object_item_data_get(it);
		if (gdata && gdata->connection_mode == mode) {
			SECURE_INFO_LOG( SP_NAME_NORMAL, "Found Item [%s] in mode[%d]", gdata->dev_info->ssid, mode);
			__COMMON_FUNC_EXIT__;
			return it;
		}
		it = elm_genlist_item_next_get(it);
	}

	__COMMON_FUNC_EXIT__;
	return NULL;
}
#endif

static void __gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	assertm_if(NULL == data, "data is NULL!!");
	assertm_if(NULL == obj, "obj is NULL!!");
	assertm_if(NULL == event_info, "event_info is NULL!!");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	ITEM_CONNECTION_MODES state = view_main_state_get();
	devpkr_gl_data_t *gdata = (devpkr_gl_data_t *)data;

	switch (state) {
	case ITEM_CONNECTION_MODE_OFF:
	case ITEM_CONNECTION_MODE_CONNECTING:
		view_main_wifi_connect(gdata);
		break;

	default:
		break;
	}

	elm_genlist_item_selected_set(item, EINA_FALSE);

	__COMMON_FUNC_EXIT__;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	char *ret = NULL;
	assertm_if(NULL == data, "data param is NULL!!");
	assertm_if(NULL == obj, "obj param is NULL!!");
	assertm_if(NULL == part, "part param is NULL!!");

	devpkr_gl_data_t *gdata = (devpkr_gl_data_t *) data;
	retvm_if(NULL == gdata, NULL);

	if (!strncmp(part, "elm.text.main.left.top", strlen(part))) {
		ret = gdata->dev_info->ssid;
		if (ret == NULL) {
			ERROR_LOG(SP_NAME_NORMAL, "ssid name is NULL!!");
		}
	} else if (!strncmp(part, "elm.text.sub.left.bottom", strlen(part))) {
		if (ITEM_CONNECTION_MODE_CONNECTING == gdata->connection_mode) {
			ret = sc(PACKAGE, I18N_TYPE_Connecting);
		} else if (ITEM_CONNECTION_MODE_CONFIGURATION == gdata->connection_mode) {
			ret = sc(PACKAGE, I18N_TYPE_Obtaining_IP_addr);
		} else {
			ret = gdata->dev_info->ap_status_txt;
		}

		if (ret == NULL) {
			ERROR_LOG(SP_NAME_NORMAL, "ap_status_txt is NULL!!");
		}
	}

	return g_strdup(ret);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	if (data == NULL) {
		return NULL;
	}

	devpkr_gl_data_t *gdata = (devpkr_gl_data_t *) data;

	Evas_Object* icon = NULL;

	if (!strncmp(part, "elm.icon.1", strlen(part))) {
		char *temp_str = NULL;
		Evas_Object *ic = elm_layout_add(obj);

		icon = elm_image_add(ic);
		elm_layout_theme_set(ic, "layout", "list/B/type.1", "default");

		temp_str = g_strdup_printf("%s.png", gdata->dev_info->ap_image_path);
		elm_image_file_set(icon, CUSTOM_EDITFIELD_PATH, temp_str);
		g_free(temp_str);

		if (gdata->highlighted == TRUE) {
			ea_theme_object_color_set(icon, "AO001P");
		} else {
			ea_theme_object_color_set(icon, "AO001");
		}
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_content_set(ic, "elm.swallow.content", icon);

		return ic;
	} else if (!strncmp(part, "elm.icon.2", strlen(part))) {
		switch (gdata->connection_mode) {
		case ITEM_CONNECTION_MODE_OFF:
			break;

		case ITEM_CONNECTION_MODE_CONNECTING:
			icon = elm_progressbar_add(obj);
			elm_object_style_set(icon, "process_medium");
			evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_progressbar_pulse(icon, TRUE);
			break;

		default:
			break;
		}
	}

	return icon;
}

static void _gl_list_del(void* data, Evas_Object* obj)
{
	if (data == NULL) {
		return;
	}

	devpkr_gl_data_t* gdata = (devpkr_gl_data_t *) data;

	if (gdata->dev_info) {
		SECURE_DEBUG_LOG(UG_NAME_NORMAL, "del target ssid: [%s]", gdata->dev_info->ssid);
		g_free(gdata->dev_info->ap_image_path);
		g_free(gdata->dev_info->ap_status_txt);
		g_free(gdata->dev_info->ssid);
		wifi_ap_destroy(gdata->dev_info->ap);
		g_free(gdata->dev_info);
	}

	g_free(gdata);

	return;
}

static void _gl_highlighted(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	if (item) {
		devpkr_gl_data_t *gdata = (devpkr_gl_data_t *)elm_object_item_data_get(item);
		if (gdata) {
			gdata->highlighted = TRUE;
			elm_genlist_item_fields_update(item, "elm.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);
		}
	}
}

static void _gl_unhighlighted(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	if (item) {
		devpkr_gl_data_t *gdata = (devpkr_gl_data_t *)elm_object_item_data_get(item);
		if (gdata) {
			gdata->highlighted = FALSE;
			elm_genlist_item_fields_update(item, "elm.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);
		}
	}
}

static Evas_Object *_create_genlist(Evas_Object* parent)
{
	__COMMON_FUNC_ENTER__;
	assertm_if(NULL == parent, "parent is NULL!!");

	list = elm_genlist_add(parent);
	assertm_if(NULL == list, "list allocation fail!!");
	elm_genlist_fx_mode_set(list, EINA_FALSE);
	elm_genlist_mode_set(list, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(list, EINA_TRUE);

	evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_smart_callback_add(list, "highlighted", _gl_highlighted, NULL);
	evas_object_smart_callback_add(list, "unhighlighted", _gl_unhighlighted, NULL);

	itc.item_style = "2line.top";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = _gl_content_get;
	itc.func.state_get = NULL;
	itc.func.del = _gl_list_del;

	__COMMON_FUNC_EXIT__;

	return list;
}

static void view_main_scan_ui_clear(void)
{
	__COMMON_FUNC_ENTER__;

	if (list == NULL) {
		return;
	}
	elm_genlist_clear(list);

	__COMMON_FUNC_EXIT__;
}

void view_main_item_state_set(wifi_ap_h ap, ITEM_CONNECTION_MODES state)
{
	__COMMON_FUNC_ENTER__;

	char *item_ssid = NULL;
	wifi_security_type_e sec_type;
	wlan_security_mode_type_t item_sec_mode;
	Elm_Object_Item* it = NULL;

	it = elm_genlist_first_item_get(list);
	if (!it ||
		!ap ||
		(WIFI_ERROR_NONE != wifi_ap_get_essid(ap, &item_ssid)) ||
		(WIFI_ERROR_NONE != wifi_ap_get_security_type(ap, &sec_type))) {
		ERROR_LOG(SP_NAME_NORMAL, "Invalid params");
		if(item_ssid != NULL) {
			g_free(item_ssid);
			item_ssid = NULL;
		}
		__COMMON_FUNC_EXIT__;
		return;
	}
	item_sec_mode = common_utils_get_sec_mode(sec_type);
	SECURE_INFO_LOG(SP_NAME_NORMAL, "item state set for AP[%s] with sec mode[%d]", item_ssid, item_sec_mode);
	while (it) {
		devpkr_gl_data_t *gdata = (devpkr_gl_data_t *)elm_object_item_data_get(it);
		if (gdata != NULL) {
			SECURE_INFO_LOG(SP_NAME_NORMAL, "gdata AP[%s] with sec mode[%d]",
					gdata->dev_info->ssid, gdata->dev_info->security_mode);
		}

		if (gdata && gdata->dev_info->security_mode == item_sec_mode &&
			!g_strcmp0(gdata->dev_info->ssid, item_ssid)) {
			if (gdata->connection_mode != state) {
				gdata->connection_mode = state;
				INFO_LOG(SP_NAME_NORMAL, "State transition from [%d] --> [%d]", view_main_state_get(), state);
				view_main_state_set(state);
				elm_genlist_item_update(it);
			}
			break;
		}

		it = elm_genlist_item_next_get(it);
	}
	g_free(item_ssid);
	__COMMON_FUNC_EXIT__;
	return;
}

static wifi_device_info_t *view_main_item_device_info_create(wifi_ap_h ap)
{
	wifi_device_info_t *wifi_device = g_try_new0(wifi_device_info_t, 1);
	wifi_security_type_e sec_type;

	if (WIFI_ERROR_NONE != wifi_ap_clone(&(wifi_device->ap), ap)) {
		g_free(wifi_device);
		return NULL;
	} else if (WIFI_ERROR_NONE != wifi_ap_get_essid(ap, &(wifi_device->ssid))) {
		g_free(wifi_device);
		return NULL;
	} else if (WIFI_ERROR_NONE != wifi_ap_get_rssi(ap, &(wifi_device->rssi))) {
		g_free(wifi_device->ssid);
		g_free(wifi_device);
		return NULL;
	} else if (WIFI_ERROR_NONE != wifi_ap_get_security_type(ap, &sec_type)) {
		g_free(wifi_device->ssid);
		g_free(wifi_device);
		return NULL;
	} else if (WIFI_ERROR_NONE != wifi_ap_is_wps_supported(ap, &(wifi_device->wps_mode))) {
		g_free(wifi_device->ssid);
		g_free(wifi_device);
		return NULL;
	}

	wifi_device->security_mode = common_utils_get_sec_mode(sec_type);
	/*MINI*/
	wifi_device->ap_status_txt = common_utils_get_ap_security_type_info_txt(PACKAGE,
	wifi_device, true);
	wifi_device->is_bt_tethered_device = false;
	common_utils_get_device_icon(wifi_device, &wifi_device->ap_image_path);

	return wifi_device;
}

static bool view_main_wifi_insert_found_ap(wifi_device_info_t *wifi_device)
{
	devpkr_gl_data_t *gdata = g_try_new0(devpkr_gl_data_t, 1);
	wifi_connection_state_e state;

	assertm_if(NULL == list, "list is NULL");

	if (gdata == NULL)
		return false;

	gdata->dev_info = wifi_device;
	if (gdata->dev_info == NULL) {
		g_free(gdata);
		return true;
	}

	wifi_ap_get_connection_state(wifi_device->ap, &state);

	if (WIFI_CONNECTION_STATE_ASSOCIATION == state ||
			WIFI_CONNECTION_STATE_CONFIGURATION == state) {
		gdata->connection_mode = ITEM_CONNECTION_MODE_CONNECTING;
		gdata->it = elm_genlist_item_append(list, &itc, gdata,
				NULL, ELM_GENLIST_ITEM_NONE, __gl_sel,
				gdata);
		view_main_state_set(ITEM_CONNECTION_MODE_CONNECTING);

		return true;
	}

	gdata->connection_mode = ITEM_CONNECTION_MODE_OFF;

	gdata->it = elm_genlist_item_append(list, &itc, gdata, NULL,
			ELM_GENLIST_ITEM_NONE, __gl_sel, gdata);

	return true;
}

static gint compare(gconstpointer a, gconstpointer b)
{
	bool favorite1 = false, favorite2 = false;
	wifi_connection_state_e state1 = 0, state2 = 0;

	wifi_device_info_t *wifi_device1 = (wifi_device_info_t*)a;
	wifi_device_info_t *wifi_device2 = (wifi_device_info_t*)b;

	wifi_ap_get_connection_state(wifi_device1->ap, &state1);
	wifi_ap_get_connection_state(wifi_device2->ap, &state2);

	if (state1 != state2) {
		if (state1 == WIFI_CONNECTION_STATE_CONNECTED)
			return -1;
		if (state2 == WIFI_CONNECTION_STATE_CONNECTED)
			return 1;

		if (state1 == WIFI_CONNECTION_STATE_CONFIGURATION)
			return -1;
		if (state2 == WIFI_CONNECTION_STATE_CONFIGURATION)
			return 1;

		if (state1 == WIFI_CONNECTION_STATE_ASSOCIATION)
			return -1;
		if (state2 == WIFI_CONNECTION_STATE_ASSOCIATION)
			return 1;
	}

	wifi_ap_is_favorite(wifi_device1->ap, &favorite1);
	wifi_ap_is_favorite(wifi_device2->ap, &favorite2);

	if (favorite1 != favorite2) {
		if (favorite1 == true)
			return -1;
		if (favorite2 == true)
			return 1;
	}

	return strcasecmp((const char *)wifi_device1->ssid,
			(const char *)wifi_device2->ssid);
}

static bool view_main_wifi_found_ap_cb(wifi_ap_h ap, void* user_data)
{
	int *profile_size = (int *)user_data;
	wifi_device_info_t *wifi_device = NULL;

	wifi_device = view_main_item_device_info_create(ap);
	if (wifi_device == NULL)
		return true;

	wifi_device_list = g_list_insert_sorted(wifi_device_list, wifi_device, compare);
	(*profile_size)++;

	return true;
}

static Evas_Object *_gl_content_title_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *title_progressbar = NULL;

	if (FALSE == wifi_devpkr_get_scan_status())
		return NULL;

	title_progressbar = elm_progressbar_add(obj);
	elm_object_style_set(title_progressbar, "process_small");
	elm_progressbar_horizontal_set(title_progressbar, EINA_TRUE);
	elm_progressbar_pulse(title_progressbar, EINA_TRUE);

	return title_progressbar;
}

static char* _gl_text_title_get(void *data, Evas_Object *obj,const char *part)
{
	if (g_strcmp0(part, "elm.text.main") == 0) {
		if (TRUE == wifi_devpkr_get_scan_status())
			return (char*) g_strdup(sc(PACKAGE, I18N_TYPE_Scanning));
		else
			return (char*) g_strdup(sc(PACKAGE, I18N_TYPE_Available_networks));
	}

	return NULL;
}

static void view_main_add_group_title(void)
{
	grouptitle_itc.item_style = "groupindex";
	grouptitle_itc.func.text_get = _gl_text_title_get;
	grouptitle_itc.func.content_get = _gl_content_title_get;

	grouptitle = elm_genlist_item_append(list,
			&grouptitle_itc,
			NULL,
			NULL,
			ELM_GENLIST_ITEM_NONE,
			NULL,
			NULL);
	assertm_if(NULL == grouptitle, "NULL!!");

	elm_genlist_item_select_mode_set(grouptitle,
			ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
}

void view_main_update_group_title(gboolean is_bg_scan)
{
	Evas_Object *box = NULL;
	Evas_Object *main_list = NULL;

	if (list != NULL) {
		if (!is_bg_scan) {
			Elm_Object_Item *it = elm_genlist_first_item_get(list);

			while (it) {
				elm_object_item_disabled_set(it, EINA_TRUE);
				it = elm_genlist_item_next_get(it);
			}
		}

		elm_genlist_item_update(grouptitle);
	} else {
		box = elm_object_content_get(devpkr_app_state->popup);

		main_list = _create_genlist(box);
		view_main_add_group_title();
		elm_box_pack_start(box, main_list);

		evas_object_show(main_list);
		evas_object_show(box);

		wifi_devpkr_redraw();

		evas_object_show(devpkr_app_state->popup);
	}

	return;
}

static void view_main_create_empty_layout(void)
{
	__COMMON_FUNC_ENTER__;

	Evas_Object *box = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *prev_box = NULL;

	prev_box = elm_object_content_get(devpkr_app_state->popup);
	if (prev_box != NULL) {
		evas_object_del(prev_box);
		list = NULL;
		grouptitle = NULL;
	}

	box = elm_box_add(devpkr_app_state->popup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	layout = elm_layout_add(devpkr_app_state->popup);
	elm_layout_file_set(layout, WIFI_DEVPKR_EDJ, WIFI_SYSPOPUP_EMPTY_GRP);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_domain_translatable_part_text_set(layout, "text", PACKAGE,
		sc(PACKAGE, I18N_TYPE_No_Wi_Fi_AP_Found));

	elm_box_pack_end(box, layout);
	evas_object_show(layout);
	evas_object_show(box);
	elm_object_content_set(devpkr_app_state->popup, box);

	__COMMON_FUNC_EXIT__;
}

void view_main_create_main_list(void)
{
	__COMMON_FUNC_ENTER__;

	Evas_Object *box = NULL;
	Evas_Object *main_list = NULL;
	Evas_Object *prev_box = NULL;

	prev_box = elm_object_content_get(devpkr_app_state->popup);
	if (prev_box != NULL) {
		evas_object_del(prev_box);
		list = NULL;
		grouptitle = NULL;
	}

	box = elm_box_add(devpkr_app_state->popup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	main_list = _create_genlist(box);
	view_main_add_group_title();

	elm_box_pack_end(box, main_list);
	evas_object_show(main_list);
	evas_object_show(box);
	elm_object_content_set(devpkr_app_state->popup, box);

	__COMMON_FUNC_EXIT__;
}

gboolean view_main_show(void *data)
{
	__COMMON_FUNC_ENTER__;

	int i;
	wifi_device_info_t *wifi_device = NULL;
	GList* list_of_device = NULL;

	int state = wlan_manager_state_get();
	if (WLAN_MANAGER_ERROR == state || WLAN_MANAGER_OFF == state) {
		INFO_LOG(SP_NAME_NORMAL, "Wi-Fi state is OFF");
		view_main_create_empty_layout();
		goto exit;
	} else if (WLAN_MANAGER_CONNECTED == state) {
		__COMMON_FUNC_EXIT__;
		return FALSE;
	}

	wifi_devpkr_enable_scan_btn();

	/* If previous profile list exists then just clear the genlist */
	if (profiles_list_size) {
		view_main_scan_ui_clear();
		view_main_add_group_title();
	} else {
		view_main_create_main_list();
	}

	view_main_state_set(ITEM_CONNECTION_MODE_OFF);

	profiles_list_size = 0;

	wifi_foreach_found_aps(view_main_wifi_found_ap_cb, &profiles_list_size);
	INFO_LOG(SP_NAME_NORMAL, "profiles list count [%d]\n", profiles_list_size);

	list_of_device = wifi_device_list;
	for (i = 0; i < profiles_list_size && list_of_device != NULL; i++) {
		wifi_device = (wifi_device_info_t*)list_of_device->data;

		view_main_wifi_insert_found_ap(wifi_device);

		list_of_device = list_of_device->next;
	}

	if (wifi_device_list != NULL) {
		g_list_free(wifi_device_list);
		wifi_device_list = NULL;
	}

	if (profiles_list_size <= 0)
		view_main_create_empty_layout();
	else
		evas_object_show(list);

exit:
	wifi_devpkr_redraw();

	evas_object_show(devpkr_app_state->popup);
	evas_object_show(devpkr_app_state->win_main);

	__COMMON_FUNC_EXIT__;
	return FALSE;
}
