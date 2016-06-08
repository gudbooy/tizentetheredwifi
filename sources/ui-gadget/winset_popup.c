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

#include <tethering.h>
#include <efl_assist.h>

#include "common.h"
#include "ug_wifi.h"
#include "i18nmanager.h"
#include "common_utils.h"
#include "winset_popup.h"
#include "wlan_manager.h"
#include "viewer_manager.h"

struct popup_manager_object {
	/* General popup attributes */
	Evas_Object* win;
	Evas_Object *popup_user_prompt;
	char *str_pkg_name;
	void *tethering_handle;
	int type;
};

static void __wifi_tethering_deactivated_cb(tethering_error_e error,
		tethering_type_e type, tethering_disabled_cause_e code, void *data)
{
	__COMMON_FUNC_ENTER__;

	if (data) {
		popup_manager_object_t *manager_object = (popup_manager_object_t *)data;
		tethering_h handle = (tethering_h)manager_object->tethering_handle;
		tethering_destroy(handle);
		manager_object->tethering_handle = NULL;
	}

	if (error != TETHERING_ERROR_NONE) {
		INFO_LOG(COMMON_NAME_LIB, "Error occurred [%d]\n", error);
		viewer_manager_header_mode_set(HEADER_MODE_OFF);
	} else {
		INFO_LOG(COMMON_NAME_LIB, "TYPE = %d", type);
		if (type == TETHERING_TYPE_WIFI ||
				type == TETHERING_TYPE_RESERVED) {
			INFO_LOG(COMMON_NAME_LIB, "OK\n");
			/* Tethering is now disabled. All OK to switch on Wi-Fi */
			power_control();
		} else {
			viewer_manager_header_mode_set(HEADER_MODE_OFF);
		}
	}

	__COMMON_FUNC_EXIT__;
}

static gboolean __wifi_tethering_deativate(popup_manager_object_t *manager_object)
{
	__COMMON_FUNC_ENTER__;

	tethering_error_e ret = TETHERING_ERROR_NONE;
	tethering_h handle = NULL;

	if (manager_object == NULL) {
		INFO_LOG(COMMON_NAME_LIB, "popup manager_object is NULL \n", ret);
		return FALSE;
	}

	if (manager_object->tethering_handle) {
		tethering_destroy(manager_object->tethering_handle);
		manager_object->tethering_handle = NULL;
	}

	ret = tethering_create(&handle);
	if (ret != TETHERING_ERROR_NONE) {
		INFO_LOG(COMMON_NAME_LIB, "Failed to tethering_create() [%d]\n", ret);
		return FALSE;
	}

	manager_object->tethering_handle = handle;
	ret = tethering_set_disabled_cb(handle, manager_object->type, __wifi_tethering_deactivated_cb, manager_object);
	if (ret != TETHERING_ERROR_NONE) {
		INFO_LOG(COMMON_NAME_LIB, "Failed to tethering_set_disabled_cb() [%d]\n", ret);
		goto exit;
	}

	ret = tethering_disable(handle, manager_object->type);
	if (ret != TETHERING_ERROR_NONE) {
		INFO_LOG(COMMON_NAME_LIB, "Failed to tethering_disable() [%d]\n", ret);
		goto exit;
	}

	__COMMON_FUNC_EXIT__;
	return TRUE;

exit:
	tethering_destroy(handle);
	manager_object->tethering_handle = NULL;

	__COMMON_FUNC_EXIT__;
	return FALSE;
}

static void __wifi_tethering_off_ok_cb(void* data, Evas_Object* obj,
		void* event_info)
{
	__COMMON_FUNC_ENTER__;

	popup_manager_object_t *manager_object = (popup_manager_object_t *)data;
	if (manager_object == NULL) {
		return;
	}

	INFO_LOG(UG_NAME_NORMAL, "Response OK");
	if (manager_object && NULL != manager_object->popup_user_prompt) {
		evas_object_hide(manager_object->popup_user_prompt);
		evas_object_del(manager_object->popup_user_prompt);
		manager_object->popup_user_prompt = NULL;
	}

	if (manager_object->type == TETHERING_TYPE_WIFI ||
			manager_object->type == TETHERING_TYPE_RESERVED) {
		if (FALSE != __wifi_tethering_deativate(manager_object)) {
			INFO_LOG(UG_NAME_NORMAL, "Successfully de-activate Wi-Fi tethering");

			viewer_manager_header_mode_set(HEADER_MODE_ACTIVATING);
		} else {
			INFO_LOG(UG_NAME_NORMAL, "Fail to de-activate Wi-Fi tethering");
		}
	}

	__COMMON_FUNC_EXIT__;
}

static void __wifi_tethering_off_no_cb(void* data, Evas_Object* obj,
		void* event_info)
{
	__COMMON_FUNC_ENTER__;

	popup_manager_object_t *manager_object = (popup_manager_object_t *)data;

	INFO_LOG(UG_NAME_NORMAL, "Response CANCEL");

	if (manager_object && NULL != manager_object->popup_user_prompt) {
		evas_object_hide(manager_object->popup_user_prompt);
		evas_object_del(manager_object->popup_user_prompt);
		manager_object->popup_user_prompt = NULL;
		viewer_manager_header_mode_set(HEADER_MODE_OFF);
	}

	__COMMON_FUNC_EXIT__;
}

popup_manager_object_t *winset_popup_manager_create(Evas_Object* win,
		const char *str_pkg_name)
{
	popup_manager_object_t *manager_object;
	manager_object = g_new0(popup_manager_object_t, 1);
	manager_object->win = win;
	manager_object->str_pkg_name = (char *)str_pkg_name;
	manager_object->tethering_handle = NULL;
	manager_object->type = 0;

	return manager_object;
}

void winset_popup_mode_set(popup_manager_object_t *manager_object,
		POPUP_MODE_OPTIONS option, void *input_data)
{
	__COMMON_FUNC_ENTER__;

	popup_btn_info_t popup_btn_data;

	if (manager_object == NULL) {
		return;
	}

	if (manager_object->tethering_handle) {
		tethering_destroy(manager_object->tethering_handle);
		manager_object->tethering_handle = NULL;
	}

	INFO_LOG(UG_NAME_NORMAL, "option = %d", option);

	memset(&popup_btn_data, 0, sizeof(popup_btn_data));

	switch (option) {
	case POPUP_OPTION_POWER_ON_FAILED_TETHERING_OCCUPIED:
		if (NULL != manager_object->popup_user_prompt) {
			break;
		}

		manager_object->type = TETHERING_TYPE_WIFI;
		popup_btn_data.info_txt = sc(manager_object->str_pkg_name, I18N_TYPE_WiFi_network_will_disable_tethering);
		popup_btn_data.btn1_txt = sc(manager_object->str_pkg_name, I18N_TYPE_Cancel);
		popup_btn_data.btn1_cb = __wifi_tethering_off_no_cb;
		popup_btn_data.btn1_data = popup_btn_data.btn2_data = manager_object;
		popup_btn_data.btn2_txt = sc(manager_object->str_pkg_name, I18N_TYPE_Ok);
		popup_btn_data.btn2_cb = __wifi_tethering_off_ok_cb;

		manager_object->popup_user_prompt =
				common_utils_show_info_popup(manager_object->win, &popup_btn_data);

		break;

	case POPUP_OPTION_POWER_ON_FAILED_TETHERING_AP_OCCUPIED:
		if (NULL != manager_object->popup_user_prompt) {
			break;
		}

		manager_object->type = TETHERING_TYPE_RESERVED;
		popup_btn_data.info_txt = sc(manager_object->str_pkg_name, I18N_TYPE_WiFi_network_will_disable_tethering);
		popup_btn_data.btn1_cb = __wifi_tethering_off_no_cb;
		popup_btn_data.btn1_txt = sc(manager_object->str_pkg_name, I18N_TYPE_Cancel);
		popup_btn_data.btn1_data = popup_btn_data.btn2_data = manager_object;
		popup_btn_data.btn2_cb = __wifi_tethering_off_ok_cb;
		popup_btn_data.btn2_txt = sc(manager_object->str_pkg_name, I18N_TYPE_Ok);

		manager_object->popup_user_prompt =
				common_utils_show_info_popup(manager_object->win, &popup_btn_data);

		break;

	default:
		break;
	}

	__COMMON_FUNC_EXIT__;
}

gboolean winset_popup_manager_destroy(popup_manager_object_t *manager_object)
{
	if (manager_object == NULL) {
		return FALSE;
	}

	if (manager_object->tethering_handle) {
		tethering_destroy(manager_object->tethering_handle);
		manager_object->tethering_handle = NULL;
	}

	g_free(manager_object);

	return TRUE;
}

gboolean winset_popup_hide_popup(popup_manager_object_t *manager_object)
{
	if (manager_object == NULL) {
		return FALSE;
	}

	if (manager_object->popup_user_prompt != NULL) {
		evas_object_hide(manager_object->popup_user_prompt);
		evas_object_del(manager_object->popup_user_prompt);
		manager_object->popup_user_prompt = NULL;
	}

	return TRUE;
}
