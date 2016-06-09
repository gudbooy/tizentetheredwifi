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
#include "ug_wifi.h"
#include "common_utils.h"
#include "common_generate_pin.h"
#include "connection_manager.h"
#include "wlan_manager.h"
#include "view_detail.h"
#include "viewer_list.h"
#include "viewer_manager.h"
#include "appcoreWrapper.h"
#include "i18nmanager.h"

#define LIST_ITEM_CONNECTED_AP_FONT_SIZE		28

static Evas_Object* viewer_list = NULL;
static Elm_Object_Item* first_item = NULL;
static Elm_Object_Item* last_item = NULL;

static Elm_Genlist_Item_Class itc;
static Elm_Genlist_Item_Class no_wifi_device_itc;
static Elm_Genlist_Item_Class grouptitle_itc;
static Elm_Object_Item* grouptitle = NULL;
//MINI
//static Elm_Genlist_Item_Class tethergrouptitle_itc;
//static Elm_Object_Item* tethergrouptitle = NULL;

extern wifi_appdata *ug_app_state;

typedef struct SCANNED_AP_LIST {
	ug_genlist_data_t *ap_data;
	struct SCANNED_AP_LIST *next;
	gboolean found;
} scanned_ap_list_t;

static scanned_ap_list_t *scan_list_head = NULL;

static void _gl_listview_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	wifi_device_info_t *device_info = (wifi_device_info_t *)data;

	if (data == NULL) {
		__COMMON_FUNC_EXIT__;
		return;
	}

	SECURE_DEBUG_LOG(UG_NAME_NORMAL, "ssid [%s]", device_info->ssid);

	view_detail(device_info, ug_app_state->layout_main);

	__COMMON_FUNC_EXIT__;
}

char* ConvertRGBAtoHex(int r, int g, int b, int a)
{
	int hexcolor = 0;
	char* string = NULL;

	string = g_try_malloc0(sizeof(char )* 255);

	hexcolor = (r << 24) + (g << 16) + (b << 8) + a;
	sprintf(string, "%x",hexcolor );

	return string;
}

static char* _gl_listview_text_get(void *data, Evas_Object *obj, const char *part)
{
	char* det = NULL;
	char* buf =NULL;
	int r = 0, g = 0, b = 0, a = 0;

	ug_genlist_data_t* gdata = (ug_genlist_data_t*) data;
	retvm_if(NULL == gdata || NULL == gdata->device_info, NULL);

	assertm_if(NULL == gdata->device_info->ssid, "NULL!!");

	if (!strncmp(part, "elm.text.main.left.top", strlen(part))) {
		det = elm_entry_utf8_to_markup(gdata->device_info->ssid);
		assertm_if(NULL == det, "NULL!!");
		if (VIEWER_ITEM_RADIO_MODE_CONNECTED == gdata->radio_mode) {
			ea_theme_color_get("AO002",&r, &g, &b, &a,
				NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL);
			buf = g_strdup_printf("<color=#%s>%s</color>",
				ConvertRGBAtoHex(r, g, b, a), det);

			g_free(det);
			return buf;
		}
	} else if (!strncmp(part, "elm.text.sub.left.bottom", strlen(part)) &&
			gdata->device_info->ap_status_txt != NULL) {
		if (VIEWER_ITEM_RADIO_MODE_CONNECTED == gdata->radio_mode) {
			if (FALSE == gdata->highlighted) {
				det = g_strdup_printf("%s", gdata->device_info->ap_status_txt);
			} else {
				det = g_strdup_printf("%s", gdata->device_info->ap_status_txt);
			}
		} else {
			det = g_strdup(gdata->device_info->ap_status_txt);
		}
		assertm_if(NULL == det, "NULL!!");
	}
	return det;
}

static Evas_Object *_gl_listview_content_get(void *data, Evas_Object *obj, const char *part)
{
	ug_genlist_data_t *gdata = (ug_genlist_data_t *)data;
	retvm_if(NULL == gdata, NULL);

	Evas_Object *icon = NULL;
	Evas_Object *btn = NULL;

	Evas_Object *ic = elm_layout_add(obj);

	if (gdata->device_info->ap_image_path == NULL) {
		/* if there is no ap_image_path (NO AP Found situation) */
		DEBUG_LOG(UG_NAME_ERR, "Fatal: Image path is NULL");

	} else if (!strcmp(part, "elm.icon.1")) {
		elm_layout_theme_set(ic, "layout", "list/B/type.1", "default");

		icon = elm_image_add(ic);
		retvm_if(NULL == icon, NULL);

		/* for strength */
		char *temp_str = NULL;
		temp_str = g_strdup_printf("%s.png", gdata->device_info->ap_image_path);
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

	} else if (!strcmp(part, "elm.icon.2")) {
		elm_layout_theme_set(ic, "layout", "list/C/type.2", "default");

		if (VIEWER_ITEM_RADIO_MODE_CONNECTING == gdata->radio_mode) {
			icon = elm_progressbar_add(ic);

			elm_object_style_set(icon, "process_medium");
			evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_progressbar_pulse(icon, TRUE);
			elm_layout_content_set(ic, "elm.swallow.content", icon);

		} else {
			btn = elm_button_add(obj);
			elm_object_style_set(btn, "circle_custom");
			icon = elm_image_add(btn);
			elm_image_file_set(icon, CUSTOM_EDITFIELD_PATH, "wifi_icon_badge_info.png");
			elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
			elm_object_part_content_set(btn, "elm.swallow.content", icon);
			evas_object_propagate_events_set(btn, EINA_FALSE);
			evas_object_smart_callback_add(btn, "clicked", _gl_listview_more_btn_cb,gdata->device_info);
			evas_object_show(btn);
			return btn;

		}
	}
	return ic;
}

static void _gl_listview_del(void *data, Evas_Object *obj)
{
	if (data == NULL)
		return;

	ug_genlist_data_t *gdata = (ug_genlist_data_t *)data;
	retm_if(NULL == gdata || NULL == gdata->device_info);

	scanned_ap_list_t *new_node = g_try_new0(scanned_ap_list_t, 1);
	retm_if(NULL == new_node);

	/* pushing at the front of the list */
	new_node->ap_data = gdata;
	new_node->next = scan_list_head;
	new_node->found = FALSE;
	scan_list_head = new_node;
}

static char *_gl_text_available_networks_get(void *data, Evas_Object *obj,
		const char *part)
{
	if (g_strcmp0(part, "elm.text.main") == 0)
		return g_strdup(sc(PACKAGE, I18N_TYPE_Available_networks));

	return NULL;
}

static Evas_Object *_gl_content_scanning_icon_get(void *data, Evas_Object *obj,
		const char *part)
{
	__COMMON_FUNC_ENTER__;
	Evas_Object *title_progressbar = NULL;

	if (HEADER_MODE_SEARCHING == viewer_manager_header_mode_get()) {
		title_progressbar  = elm_progressbar_add(obj);
		elm_object_style_set(title_progressbar, "process_medium");
		elm_progressbar_horizontal_set(title_progressbar, EINA_TRUE);
		elm_progressbar_pulse(title_progressbar, EINA_TRUE);
	}

	__COMMON_FUNC_EXIT__;
	return title_progressbar;
}

Elm_Object_Item* viewer_list_get_first_item(void)
{
	return first_item;
}

static Elm_Object_Item* viewer_list_get_last_item(void)
{
	return last_item;
}

static char *_access_info_cb(void *data, Evas_Object *obj)
{
	char *strength = NULL;
	char *buf = NULL;

	ug_genlist_data_t *gdata = (ug_genlist_data_t *)data;
	if (!gdata) {
		return NULL;
	}

	strength = common_utils_get_rssi_text(PACKAGE, gdata->device_info->rssi);
	buf = g_strdup_printf("%s. %s. %s.", strength, gdata->device_info->ssid, gdata->device_info->ap_status_txt);

	g_free(strength);
	return buf;
}

static void _gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	HEADER_MODES header_mode = viewer_manager_header_mode_get();
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (header_mode == HEADER_MODE_ACTIVATING ||
		header_mode == HEADER_MODE_DEACTIVATING ||
		header_mode == HEADER_MODE_OFF) {
		return ;
	}

	Elm_Object_Item *first_item = viewer_list_get_first_item();
	Elm_Object_Item *last_item = viewer_list_get_last_item();

	if (first_item == NULL || last_item == NULL) {
		return;
	}

	int index = (int)elm_genlist_item_index_get(item);
	int first_item_index = (int)elm_genlist_item_index_get(first_item);
	int last_item_index = (int)elm_genlist_item_index_get(last_item);
	char buf[100] = "";

	if (first_item_index == -1) {
		int group_index = (int)elm_genlist_item_index_get(grouptitle);
		first_item_index = group_index + 1;
	}

	if (index >= first_item_index && index <= last_item_index &&
			first_item_index < last_item_index) {
		if (index == first_item_index) {
			elm_object_item_signal_emit(item, "elm,state,top", "");
		} else if (index == last_item_index) {
			elm_object_item_signal_emit(item, "elm,state,bottom", "");
		} else {
			elm_object_item_signal_emit(item, "elm,state,center", "");
		}

		Evas_Object *ao = elm_object_item_access_object_get(item);
		elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, elm_object_item_data_get(item));
		g_snprintf(buf, sizeof(buf), "%s%s",
						sc(PACKAGE, I18N_TYPE_Double_tap),
						sc(PACKAGE, I18N_TYPE_Connect_to_device));
		elm_access_info_set(ao, ELM_ACCESS_TYPE, buf);
		elm_access_info_set(ao, ELM_ACCESS_STATE, sc(PACKAGE, I18N_TYPE_Wi_Fi_network_info));
	}
}

static void _info_popup_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	pswd_popup_t *passpopup = (pswd_popup_t *)data;

	if (passpopup != NULL && passpopup->info_popup != NULL) {
		evas_object_del(passpopup->info_popup);
		passpopup->info_popup = NULL;
	}
}

static void __passwd_popup_cancel_cb(void *data,  Evas_Object *obj,
		void *event_info)
{
	__COMMON_FUNC_ENTER__;

	if (ug_app_state->passpopup == NULL) {
		return;
	}

	passwd_popup_free(ug_app_state->passpopup);
	ug_app_state->passpopup = NULL;

	__COMMON_FUNC_EXIT__;
}

static void __passwd_popup_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	wifi_ap_h ap = NULL;
	int password_len = 0;
	const char *password = NULL;
	Evas_Object *info_popup = NULL;
	wifi_security_type_e sec_type = WIFI_SECURITY_TYPE_NONE;

	if (ug_app_state->passpopup == NULL) {
		return;
	}

	ap = passwd_popup_get_ap(ug_app_state->passpopup);
	password = passwd_popup_get_txt(ug_app_state->passpopup);
	password_len = strlen(password);

	wifi_ap_get_security_type(ap, &sec_type);

	switch (sec_type) {
	case WIFI_SECURITY_TYPE_WEP:
		if (password_len != 5 && password_len != 13 &&
				password_len != 26 && password_len != 10) {
			info_popup = common_utils_show_info_ok_popup(
					ug_app_state->layout_main, PACKAGE,
					sc(PACKAGE, I18N_TYPE_Wrong_Password),
					_info_popup_ok_cb,
					ug_app_state->passpopup);
			ug_app_state->passpopup->info_popup = info_popup;

			goto popup_ok_cb_exit;
		}
		break;

	case WIFI_SECURITY_TYPE_WPA_PSK:
	case WIFI_SECURITY_TYPE_WPA2_PSK:
		if (password_len < 8 || password_len > 64) {
			info_popup = common_utils_show_info_ok_popup(
					ug_app_state->layout_main, PACKAGE,
					sc(PACKAGE, I18N_TYPE_Wrong_Password),
					_info_popup_ok_cb,
					ug_app_state->passpopup);
			ug_app_state->passpopup->info_popup = info_popup;

			goto popup_ok_cb_exit;
		}
		break;

	default:
		ERROR_LOG(UG_NAME_NORMAL, "Wrong security mode: %d", sec_type);
		passwd_popup_free(ug_app_state->passpopup);
		ug_app_state->passpopup = NULL;

		goto popup_ok_cb_exit;
	}

	bool favorite = FALSE;
	//MIN_LOG("wifi_ap_is_favorite called");
	if(wifi_ap_is_favorite(ap, &favorite) == WIFI_ERROR_NONE
		&& favorite == TRUE) {
		wlan_manager_forget(ap);
		wifi_ap_refresh(ap);
	}

	wlan_manager_connect_with_password(ap, password);

	passwd_popup_free(ug_app_state->passpopup);
	ug_app_state->passpopup = NULL;

popup_ok_cb_exit:
	g_free((gpointer)password);

	__COMMON_FUNC_EXIT__;
}

static void __passwd_popup_setting_cb(void *data,  Evas_Object *obj,
		void *event_info)
{
	__COMMON_FUNC_ENTER__;

	wifi_device_info_t *device_info = (wifi_device_info_t *)data;
	Elm_Object_Item *item = event_info;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	if (device_info == NULL || ug_app_state == NULL)
		return;

	if (ug_app_state->passpopup == NULL)
		return;

	passwd_popup_hide(ug_app_state->passpopup);
	view_detail(device_info, ug_app_state->layout_main);

	__COMMON_FUNC_EXIT__;
}

static void __wps_pbc_popup_cancel_connecting(void *data, Evas_Object *obj,
		void *event_info)
{
	__COMMON_FUNC_ENTER__;

	if (ug_app_state->passpopup == NULL) {
		return;
	}

	wifi_ap_h ap = passwd_popup_get_ap(ug_app_state->passpopup);

	int ret = wlan_manager_disconnect(ap);
	if (ret != WLAN_MANAGER_ERR_NONE) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed WPS PBC cancellation [0x%x]", ap);
	}

	passwd_popup_free(ug_app_state->passpopup);
	ug_app_state->passpopup = NULL;

	viewer_manager_header_mode_set(HEADER_MODE_ON);

	__COMMON_FUNC_EXIT__;
}

static void _wps_btn_cb(void* data, Evas_Object* obj, void* event_info)
{
	__COMMON_FUNC_ENTER__;

	if (!ug_app_state->passpopup) {
		return;
	}

	wifi_ap_h ap = passwd_popup_get_ap(ug_app_state->passpopup);
	int ret = wlan_manager_wps_connect(ap);
	if (ret == WLAN_MANAGER_ERR_NONE) {
		create_pbc_popup(ug_app_state->passpopup,
				__wps_pbc_popup_cancel_connecting, NULL,
				POPUP_WPS_BTN, NULL);
	} else {
		ERROR_LOG(UG_NAME_NORMAL, "wlan_manager_wps_connect failed");
		passwd_popup_free(ug_app_state->passpopup);
		ug_app_state->passpopup = NULL;
	}

	__COMMON_FUNC_EXIT__;
}

static void _wps_cancel_cb(void* data, Evas_Object* obj, void* event_info)
{
	__COMMON_FUNC_ENTER__;

	if (ug_app_state->passpopup == NULL) {
		return;
	}

	current_popup_free(ug_app_state->passpopup, POPUP_WPS_OPTIONS);

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
	Evas_Object *info_popup = NULL;

	if (!ug_app_state->passpopup) {
		return;
	}

	/* Generate WPS pin */
	rpin = wps_generate_pin();
	if (rpin > 0)
		g_snprintf(npin, sizeof(npin), "%08d", rpin);

	pin_len = strlen(npin);
	if (pin_len != 8) {
		info_popup = common_utils_show_info_ok_popup(
				ug_app_state->layout_main, PACKAGE,
				sc(PACKAGE, I18N_TYPE_Invalid_pin),
				_info_popup_ok_cb,
				ug_app_state->passpopup);
		ug_app_state->passpopup->info_popup = info_popup;

		__COMMON_FUNC_EXIT__;
		return;
	}

	ap = passwd_popup_get_ap(ug_app_state->passpopup);

	ret = wlan_manager_wps_pin_connect(ap, npin);
	if (ret == WLAN_MANAGER_ERR_NONE) {
		INFO_LOG(UG_NAME_NORMAL, "wlan_manager_wps_pin_connect successful");

		create_pbc_popup(ug_app_state->passpopup,
				__wps_pbc_popup_cancel_connecting, NULL,
				POPUP_WPS_PIN, npin);
	} else {
		ERROR_LOG(UG_NAME_NORMAL, "wlan_manager_wps_pin_connect failed");
		passwd_popup_free(ug_app_state->passpopup);
		ug_app_state->passpopup = NULL;
	}

	__COMMON_FUNC_EXIT__;
}

static void __wps_options_popup_cb(void* data, Evas_Object* obj, void* event_info)
{
	__COMMON_FUNC_ENTER__;
	pswd_popup_create_req_data_t popup_info;

	if (!ug_app_state->passpopup) {
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
	popup_info.ap = passwd_popup_get_ap(ug_app_state->passpopup);
	popup_info.cb_data = NULL;
	create_wps_options_popup(ug_app_state->layout_main, ug_app_state->passpopup,
			&popup_info);

	g_free(popup_info.title);

	__COMMON_FUNC_EXIT__;
}

void viewer_list_wifi_reconnect(wifi_device_info_t *device_info)
{
	Evas_Object* navi_frame = NULL;
	pswd_popup_create_req_data_t popup_info;
	wifi_security_type_e sec_type = WIFI_SECURITY_TYPE_NONE;

	if (device_info == NULL)
		return;

	wifi_ap_get_security_type(device_info->ap, &sec_type);

	switch (sec_type) {
	case WIFI_SECURITY_TYPE_WEP:
	case WIFI_SECURITY_TYPE_WPA_PSK:
	case WIFI_SECURITY_TYPE_WPA2_PSK:
		memset(&popup_info, 0, sizeof(pswd_popup_create_req_data_t));

		popup_info.title = device_info->ssid;
		popup_info.ok_cb = __passwd_popup_ok_cb;
		popup_info.cancel_cb = __passwd_popup_cancel_cb;
		popup_info.setting_cb = __passwd_popup_setting_cb;
		popup_info.show_wps_btn = device_info->wps_mode;
		popup_info.wps_btn_cb = __wps_options_popup_cb;
		popup_info.ap = device_info->ap;
		popup_info.cb_data = device_info;
		popup_info.sec_type = sec_type;

		if (ug_app_state->passpopup != NULL) {
			passwd_popup_free(ug_app_state->passpopup);
			ug_app_state->passpopup = NULL;
		}
		/* TODO: parameter with device_info */
		/* TODO: finally parameter with wifi_ap_h, WPA, EAP */
		ug_app_state->passpopup = create_passwd_popup(
				ug_app_state->layout_main, PACKAGE, &popup_info);

		if (ug_app_state->passpopup == NULL) {
			INFO_LOG(UG_NAME_ERR, "Fail to create password popup");
		}

		break;
	case WIFI_SECURITY_TYPE_EAP:
		navi_frame = viewer_manager_get_naviframe();
		struct ug_data *ugd = (struct ug_data *)ug_app_state->gadget;
		ug_app_state->eap_view = create_eap_view(ug_app_state->ug_type,
				ug_app_state->layout_main, navi_frame, PACKAGE,
				device_info, ugd->win_main);
		break;
	default:
		ERROR_LOG(UG_NAME_NORMAL, "Unknown security type [%d]", sec_type);
		return;
	}

}

void viewer_list_wifi_connect(wifi_device_info_t *device_info)
{
	__COMMON_FUNC_ENTER__;
	bool favorite = false;
	Evas_Object* navi_frame = NULL;
	pswd_popup_create_req_data_t popup_info;
	wifi_security_type_e sec_type = WIFI_SECURITY_TYPE_NONE;

	if (device_info == NULL){
		__COMMON_FUNC_EXIT__;
		return;
	}


	
/*MINI*/
	//	wifi_ap_is_favorite(device_info->ap, &favorite);

//	if (favorite == true) {
//		wlan_manager_connect(device_info->ap);
//		__COMMON_FUNC_EXIT__;
//		return;
//	}

	wifi_ap_get_security_type(device_info->ap, &sec_type);
	MIN_LOG("sec_type : %d", sec_type);
	switch (sec_type) {
	case WIFI_SECURITY_TYPE_NONE:
		MIN_LOG("HIHIHIHIHIHIH");
		wlan_manager_connect(device_info->ap);
		break;

	case WIFI_SECURITY_TYPE_WEP:
	case WIFI_SECURITY_TYPE_WPA_PSK:
	case WIFI_SECURITY_TYPE_WPA2_PSK:
		memset(&popup_info, 0, sizeof(pswd_popup_create_req_data_t));

		popup_info.title = device_info->ssid;
		popup_info.ok_cb = __passwd_popup_ok_cb;
		popup_info.cancel_cb = __passwd_popup_cancel_cb;
		popup_info.setting_cb = __passwd_popup_setting_cb;
		popup_info.show_wps_btn = device_info->wps_mode;
		popup_info.wps_btn_cb = __wps_options_popup_cb;
		
		popup_info.ap = device_info->ap;
		popup_info.cb_data = device_info;
		popup_info.sec_type = sec_type;

		if (ug_app_state->passpopup != NULL) {
			passwd_popup_free(ug_app_state->passpopup);
			ug_app_state->passpopup = NULL;
		}
		/* TODO: parameter with device_info */
		/* TODO: finally parameter with wifi_ap_h, WPA, EAP */
		ug_app_state->passpopup = create_passwd_popup(
				ug_app_state->layout_main, PACKAGE, &popup_info);

		if (ug_app_state->passpopup == NULL) {
			INFO_LOG(UG_NAME_ERR, "Fail to create password popup");
		}

		break;

	case WIFI_SECURITY_TYPE_EAP:
		navi_frame = viewer_manager_get_naviframe();
		struct ug_data *ugd = (struct ug_data *)ug_app_state->gadget;
		ug_app_state->eap_view = create_eap_view(ug_app_state->ug_type,
				ug_app_state->layout_main, navi_frame, PACKAGE,
				device_info, ugd->win_main);
		break;

	default:
		ERROR_LOG(UG_NAME_NORMAL, "Unknown security type[%d]", sec_type);
		break;
	}
	__COMMON_FUNC_EXIT__;
}
//MINI
static void __viewer_list_item_bt_tether_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;
	assertm_if(NULL == event_info, "event_info is NULL!!");
	assertm_if(NULL == obj, "obj is NULL!!"); 
//MINI Get Object From 
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	ug_genlist_data_t* gdata = NULL;
	wifi_device_info_t *device_info = NULL;

	gdata = (ug_genlist_data_t *)elm_object_item_data_get(it);
	retm_if(NULL == gdata);
	device_info = gdata->device_info;
	retm_if(NULL == device_info || NULL == device_info->ssid);
	
	if(!device_info->is_bt_tethered_device)
	{
		MIN_LOG("device_info->is_bt_tethered_device is not tethered device");
		__COMMON_FUNC_EXIT__;	
		return;
	}
	else
	{
		//send bluetooth message
		start_bt_service("BC:79:AD:F7:3D:BA");		
		MIN_LOG("Send message to host device over bluetooth");

	}


	__COMMON_FUNC_EXIT__;
}
static void __viewer_list_item_clicked_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__COMMON_FUNC_ENTER__;

	assertm_if(NULL == event_info, "event_info is NULL!!");
	assertm_if(NULL == obj, "obj is NULL!!");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	ug_genlist_data_t *gdata = NULL;
	wifi_device_info_t *device_info = NULL;

	gdata = (ug_genlist_data_t *)elm_object_item_data_get(it);
	retm_if(NULL == gdata);

	device_info = gdata->device_info;
	retm_if(NULL == device_info || NULL == device_info->ssid);

	int item_state = gdata->radio_mode;
	int current_state = 0;

	SECURE_INFO_LOG(UG_NAME_NORMAL, "ssid --- %s", device_info->ssid);
	INFO_LOG(UG_NAME_NORMAL, "ap --- 0x%x", device_info->ap);
	INFO_LOG(UG_NAME_NORMAL, "current item_state state is --- %d\n", item_state);

	switch (item_state) {
	case VIEWER_ITEM_RADIO_MODE_OFF:
		current_state = viewer_manager_header_mode_get();

		INFO_LOG(UG_NAME_NORMAL, "Clicked AP information\n");
		INFO_LOG(UG_NAME_NORMAL, "header mode [%d]", current_state);

		switch (current_state) {
		case HEADER_MODE_ON:
		case HEADER_MODE_CONNECTED:
		case HEADER_MODE_CONNECTING:
		case HEADER_MODE_SEARCHING:
			viewer_list_wifi_connect(device_info);
			break;

		case HEADER_MODE_OFF:
		case HEADER_MODE_ACTIVATING:
		case HEADER_MODE_DEACTIVATING:
		default:
			INFO_LOG(UG_NAME_NORMAL, "Ignore click");
			break;
		}
		break;

	case VIEWER_ITEM_RADIO_MODE_CONNECTING:
		wlan_manager_disconnect(device_info->ap);
		break;

	case VIEWER_ITEM_RADIO_MODE_CONNECTED:
	default:
		INFO_LOG(UG_NAME_NORMAL, "Ignore click");
		break;
	}

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	__COMMON_FUNC_EXIT__;
}

static char *viewer_list_get_device_status_txt(wifi_device_info_t *wifi_device,
		VIEWER_ITEM_RADIO_MODES mode)
{
	char *status_txt = NULL;

	/* The strings are currently hard coded. It will be replaced with string ids later */
	if (VIEWER_ITEM_RADIO_MODE_CONNECTING == mode) {
		status_txt = g_strdup(sc(PACKAGE, I18N_TYPE_Connecting));
	} else if (VIEWER_ITEM_RADIO_MODE_CONFIGURATION == mode) {
		status_txt = g_strdup(sc(PACKAGE, I18N_TYPE_Obtaining_IP_addr));
	} else if (VIEWER_ITEM_RADIO_MODE_CONNECTED == mode) {
		if (connection_manager_is_wifi_connection_used()) {
			status_txt = g_strdup(sc(PACKAGE, I18N_TYPE_Connected));
		}
	} else if (VIEWER_ITEM_RADIO_MODE_OFF == mode) {
	/*MINI*/
				status_txt = common_utils_get_ap_security_type_info_txt(PACKAGE,
			wifi_device, true);
	} else {
		status_txt = g_strdup(sc(PACKAGE, I18N_TYPE_Unknown));
		INFO_LOG(UG_NAME_NORMAL, "Invalid mode: %d", mode);
	}

	return status_txt;
}

Evas_Object* viewer_list_create(Evas_Object *win)
{
	__COMMON_FUNC_ENTER__;

	assertm_if(NULL == win, "NULL!!");
	assertm_if(NULL != viewer_list, "Err!!");

	viewer_list = elm_genlist_add(win);
	assertm_if(NULL == viewer_list, "NULL!!");
	elm_genlist_fx_mode_set(viewer_list, EINA_FALSE);

	// To use multiline textblock/entry/editfield in genlist, set height_for_width mode
	// then the item's height is calculated while the item's width fits to genlist width.
	elm_genlist_mode_set(viewer_list, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(viewer_list, EINA_TRUE);

	evas_object_size_hint_weight_set(viewer_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(viewer_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

	itc.item_style = "2line.top";
	itc.func.text_get = _gl_listview_text_get;
	itc.func.content_get = _gl_listview_content_get;
	itc.func.state_get = NULL;
	itc.func.del = _gl_listview_del;

	no_wifi_device_itc.item_style = "1line";
	no_wifi_device_itc.func.text_get = _gl_listview_text_get;
	no_wifi_device_itc.func.content_get = NULL;
	no_wifi_device_itc.func.state_get = NULL;
	no_wifi_device_itc.func.del = _gl_listview_del;

	first_item = last_item = NULL;

	evas_object_smart_callback_add(viewer_list, "realized", _gl_realized, NULL);

	__COMMON_FUNC_EXIT__;
	return viewer_list;
}

void viewer_list_title_item_del_available_networks(Elm_Object_Item *item_header)
{
	if (item_header == NULL)
		return;

	elm_object_item_del(grouptitle);

	memset(&grouptitle_itc, 0, sizeof(grouptitle_itc));
	grouptitle_itc.item_style = "multiline/1text";

	assertm_if(NULL != grouptitle, "Err!!");

	grouptitle = elm_genlist_item_insert_after(viewer_list,
			&grouptitle_itc, NULL, NULL, item_header,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);

	assertm_if(NULL == grouptitle, "NULL!!");

	elm_genlist_item_select_mode_set(grouptitle,
			ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
}

void viewer_list_title_item_update(void)
{
	if (grouptitle)
		elm_genlist_item_update(grouptitle);
}

void viewer_list_title_item_set_available_networks(Elm_Object_Item *item_header)
{
	__COMMON_FUNC_ENTER__;
	if (item_header == NULL)
		return;	
	
	elm_object_item_del(grouptitle);
	

	memset(&grouptitle_itc, 0, sizeof(grouptitle_itc));
	grouptitle_itc.item_style = "groupindex";
	grouptitle_itc.func.text_get = _gl_text_available_networks_get;
	grouptitle_itc.func.content_get = _gl_content_scanning_icon_get;

	assertm_if(NULL != grouptitle, "Err!!");

	grouptitle = elm_genlist_item_insert_after(viewer_list,
			&grouptitle_itc, NULL, NULL, item_header,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);

	assertm_if(NULL == grouptitle, "NULL!!");

	elm_genlist_item_select_mode_set(grouptitle,
			ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	__COMMON_FUNC_ENTER__;
}

int viewer_list_item_radio_mode_set(Elm_Object_Item* item,
		VIEWER_ITEM_RADIO_MODES mode)
{
	__COMMON_FUNC_ENTER__;

	if (NULL == item) {
		INFO_LOG(COMMON_NAME_ERR, "item is NULL");
		return FALSE;
	}

	ug_genlist_data_t* gdata = (ug_genlist_data_t *) elm_object_item_data_get(item);
	if (NULL == gdata || NULL == gdata->device_info) {
		INFO_LOG(COMMON_NAME_ERR, "gdata or device_info is NULL");
		return FALSE;
	}

	if (gdata->radio_mode == mode) {
		SECURE_INFO_LOG(UG_NAME_NORMAL, "[%s] is already in requested state", gdata->device_info->ssid);
		return FALSE;
	}

	SECURE_INFO_LOG(UG_NAME_NORMAL, "[%s] AP Item State Transition from [%d] --> [%d]", gdata->device_info->ssid, gdata->radio_mode, mode);
	gdata->radio_mode = mode;
	if (gdata->device_info->ap_status_txt) {
		g_free(gdata->device_info->ap_status_txt);
		gdata->device_info->ap_status_txt = viewer_list_get_device_status_txt(gdata->device_info, mode);
	}

	elm_genlist_item_update(item);

	__COMMON_FUNC_EXIT__;
	return TRUE;
}

void viewer_list_scanned_ap_list_free(void)
{
	scanned_ap_list_t *temp = scan_list_head;

	while(NULL != temp) {
		scan_list_head = scan_list_head->next;

		if (FALSE == temp->found) {
			g_free(temp->ap_data->device_info->ap_image_path);
			g_free(temp->ap_data->device_info->ap_status_txt);
			g_free(temp->ap_data->device_info->ssid);
			wifi_ap_destroy(temp->ap_data->device_info->ap);
			g_free(temp->ap_data->device_info);
			g_free(temp->ap_data);
		}
		g_free(temp);

		temp = scan_list_head;
	}
}

Elm_Object_Item *viewer_list_item_insert_after(wifi_device_info_t *wifi_device,
		Elm_Object_Item *after)
{
	__COMMON_FUNC_ENTER__;
	if(wifi_device == NULL)
		MIN_LOG("viewer_list_item_insert_after : wifi_device->NULL");
	else
		MIN_LOG("viewer_list_item_insert_after : wifi_device->ssid : %s", wifi_device->ssid);
	Elm_Object_Item* ret = NULL;
	ug_genlist_data_t* gdata = NULL;
	wifi_device_info_t *no_wifi_device = NULL;
	
	ug_genlist_data_t* g_bt_data = NULL;
	bool is_bt_device = false; 
	//MINI
	//check Bt
	retvm_if(NULL == viewer_list, NULL);
	
	Elm_Object_Item* it = elm_genlist_first_item_get(viewer_list);
	
	while(it)
	{
		ug_genlist_data_t* gtemp = elm_object_item_data_get(it);
		if(gtemp && gtemp->device_info){
			if(gtemp->device_info->is_bt_tethered_device == true)
			{
				is_bt_device = true;
				break;
			}
		}
		if (it == last_item)
		{
			it = NULL;
			break;
		}
		it = elm_genlist_item_next_get(it);
	}



	if (wifi_device != NULL) {
		wifi_device->ap_status_txt = viewer_list_get_device_status_txt(
						wifi_device,
						VIEWER_ITEM_RADIO_MODE_OFF);
	} else {
		no_wifi_device = g_try_new0(wifi_device_info_t, 1);
		if (no_wifi_device == NULL){
			__COMMON_FUNC_EXIT__;
			return NULL;
		}
		no_wifi_device->ssid = g_strdup(sc(PACKAGE, I18N_TYPE_No_Wi_Fi_AP_Found));

		wifi_device = no_wifi_device;
	}

	/* Look for wifi_device->ssid in the ap_list that is maintained and,
	 * if present retain the handle and insert, otherwise insert it right away */
	scanned_ap_list_t *temp = scan_list_head;
	while(NULL != temp) {
		if (g_strcmp0(temp->ap_data->device_info->ssid, wifi_device->ssid) == 0
				&& (temp->ap_data->device_info->security_mode ==
						wifi_device->security_mode)) {
			temp->found = TRUE;
			break;
		}
		temp = temp->next;
	}

	/* if temp is NULL it implies ssid is not found */
	if (NULL == temp) {
		gdata = g_try_new0(ug_genlist_data_t, 1);
		retvm_if(NULL == gdata, NULL);
		gdata->device_info = wifi_device;
	} else {
		gdata = temp->ap_data;
		gdata->device_info = wifi_device;
		gdata->device_info->ap = temp->ap_data->device_info->ap;
		gdata->highlighted = FALSE;
	}

	gdata->radio_mode = VIEWER_ITEM_RADIO_MODE_OFF;

	if (!after) {
		/* If the after item is NULL then insert it as first item */
		after = grouptitle;
	}
	if(!is_bt_device)
	{
		g_bt_data = g_try_new0(ug_genlist_data_t, 1); 
		retvm_if(NULL == g_bt_data, NULL);
		g_bt_data->device_info = tether_device_get_singleton();
		g_bt_data->radio_mode = VIEWER_ITEM_RADIO_MODE_OFF;

		ret = elm_genlist_item_insert_before(
				viewer_list, 
				&itc, 
				g_bt_data,
				NULL, 
				grouptitle,
				ELM_GENLIST_ITEM_NONE,
				__viewer_list_item_bt_tether_cb, 
				NULL);
		elm_genlist_item_update(ret);	
	}
	
	if (no_wifi_device == NULL) {
		MIN_LOG("no_wifi_device == NULL");
			if(g_strcmp0(wifi_device->ssid, "AndroidAP") == 0){
			MIN_LOG("BLUETOOTH ITEM INSERTED!!!!!!!!!");
			ret = elm_genlist_item_insert_before(
					viewer_list,
					&itc,
					gdata,
					NULL,
					grouptitle, 
					ELM_GENLIST_ITEM_NONE,
					NULL, 
					NULL);
					viewer_list_wifi_connect(wifi_device);
					viewer_list_item_radio_mode_set(ret, VIEWER_ITEM_RADIO_MODE_CONNECTED);	
		}else{
		ret = elm_genlist_item_insert_after(
				viewer_list, /*obj*/
				&itc,/*itc*/
				gdata,/*data*/
				NULL,/*parent*/
				after, /*after than*/
				ELM_GENLIST_ITEM_NONE, /*flags*/
				__viewer_list_item_clicked_cb,/*func*/
				NULL);/*func_data*/
		}
	} else {
//		MIN_LOG("no_wifi_decice != NULL");
		ret = elm_genlist_item_insert_after(
				viewer_list, /*obj*/
				&no_wifi_device_itc,/*itc*/
				gdata,/*data*/
				NULL,/*parent*/
				after, /*after than*/
				ELM_GENLIST_ITEM_NONE, /*flags*/
				__viewer_list_item_clicked_cb,/*func*/
				NULL);/*func_data*/
	}

	if (!ret) {
		assertm_if(NULL == ret, "NULL!!");
		g_free(gdata);
	} else {
		SECURE_DEBUG_LOG(UG_NAME_NORMAL,
				"* item add complete item [0x%x] ssid:[%s] security[%d] size:[%d]",
				ret,
				wifi_device->ssid,
				wifi_device->security_mode,
				viewer_list_item_size_get());

		if (after == grouptitle) {
			first_item = ret;
			if (!last_item) {
				last_item = ret;
			}
		} else {
			last_item = ret;
			if (!first_item) {
				first_item = ret;
			}
		}

		elm_genlist_item_update(ret);
	}
	__COMMON_FUNC_EXIT__; 
	return ret;
}

void viewer_list_item_del(Elm_Object_Item *it)
{
	if (it == NULL) {
		return;
	}

	if (it == first_item) {
		first_item = elm_genlist_item_next_get(first_item);
	} else if (it == last_item) {
		last_item = elm_genlist_item_prev_get(last_item);
	}
	elm_object_item_del(it);
}

int viewer_list_item_size_get()
{
	__COMMON_FUNC_ENTER__;
	int ret = 0;
	Elm_Object_Item *it = first_item;

	while(it) {
		ret++;
		if (it == last_item) {
			break;
		}
		it = elm_genlist_item_next_get(it);
	}

	__COMMON_FUNC_EXIT__;
	return ret;
}

void viewer_list_item_clear(void)
{
	__COMMON_FUNC_ENTER__;

	Elm_Object_Item *it = first_item;
	Elm_Object_Item *nxt = NULL;
/*	Elm_Object_Item *prev = NULL;


	if(grouptitle != NULL)
	{	
		prev = elm_genlist_item_prev_get(grouptitle);
		if(prev !=NULL)
		{
			ug_genlist_data_t* gtemp = elm_object_item_data_get(prev);
			if(gtemp && gtemp->device_info)
				if(gtemp->device_info->is_bt_tethered_device == true)
				{
					elm_object_item_del(prev);
				}
		}
	}*/

	while(it) {
		nxt = elm_genlist_item_next_get(it);
		elm_object_item_del(it);
		if (it == last_item) {
			break;
		}
		it = nxt;
	}

	first_item = last_item = NULL;
	/*MINI*/
	__COMMON_FUNC_EXIT__;
}

void viewer_list_item_enable_all(void)
{
	__COMMON_FUNC_ENTER__;

	Elm_Object_Item *it = first_item;

	while(it) {
		elm_object_item_disabled_set(it, EINA_FALSE);

		if (it == last_item) {
			break;
		}

		it = elm_genlist_item_next_get(it);
	}

	__COMMON_FUNC_EXIT__;
}

void viewer_list_item_disable_all(void)
{
	__COMMON_FUNC_ENTER__;

	Elm_Object_Item *it = first_item;

	while(it) {
		elm_object_item_disabled_set(it, EINA_TRUE);

		if (it == last_item) {
			break;
		}

		it = elm_genlist_item_next_get(it);
	}

	__COMMON_FUNC_EXIT__;
}

Elm_Object_Item* item_get_for_ap(wifi_ap_h ap)
{
	__COMMON_FUNC_ENTER__;
	if (!ap) {
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

	Elm_Object_Item *it = first_item;
	wlan_security_mode_type_t sec_mode = common_utils_get_sec_mode(type);
	while(it) {
		ug_genlist_data_t* gdata = elm_object_item_data_get(it);
		wifi_device_info_t *device_info = NULL;
		if (gdata && (device_info = gdata->device_info)) {
			if (!g_strcmp0(device_info->ssid, essid) && device_info->security_mode == sec_mode) {
				break;
			}
		}
		if (it == last_item) {
			it = NULL;
			break;
		}
		it = elm_genlist_item_next_get(it);
	}

	g_free(essid);
	__COMMON_FUNC_EXIT__;
	return it;
}

Elm_Object_Item* item_get_for_ssid(const char *ssid)
{
	Elm_Object_Item *it = first_item;
	while (it) {
		ug_genlist_data_t* gdata = elm_object_item_data_get(it);
		wifi_device_info_t *device_info = NULL;
		if (gdata && (device_info = gdata->device_info)) {
			if (!g_strcmp0(device_info->ssid, ssid)) {
				break;
			}
		}
		if (it == last_item) {
			it = NULL;
			break;
		}
		it = elm_genlist_item_next_get(it);
	}

	return it;
}
