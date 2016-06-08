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

#include "ug_wifi.h"
#include "viewer_list.h"
#include "wlan_manager.h"
#include "winset_popup.h"
#include "viewer_manager.h"
#include "wlan_connection.h"
#include "wifi-engine-callback.h"

extern wifi_appdata *ug_app_state;

void wlan_engine_callback(wlan_mgr_event_info_t *event_info, void *user_data)
{
	__COMMON_FUNC_ENTER__;

	if (event_info == NULL) {
		__COMMON_FUNC_EXIT__;
		return;
	}

	Elm_Object_Item *target_item = NULL;
	ug_genlist_data_t *gdata = NULL;
	int ug_state = 0;
	wifi_device_info_t *wifi_device = NULL;

	int header_state = -1;
	header_state = viewer_manager_header_mode_get();

	INFO_LOG(UG_NAME_NORMAL, "header state [%d]", header_state);
	INFO_LOG(UG_NAME_NORMAL, "event type [%d]", event_info->event_type);
	INFO_LOG(UG_NAME_NORMAL, "ap [0x%x]", event_info->ap);

	if (event_info->ap) {	/* Is it a response with AP handle? */

		/* All responses with profile names should have an associated genlist item
		 * Verify if the genlist item exists and associated genlist item data exists
		 */
		target_item = item_get_for_ap(event_info->ap);
		if (!target_item ||
			!(gdata = (ug_genlist_data_t *)elm_object_item_data_get(target_item))) {
			/* This is a case where the profile name exists but no
			 * associated genlist item OR genlist item data exists.
			 * This condition can come when an AP action(Example connect)
			 * is triggered and by the time the response came the genlist is cleared.
			 */
			ERROR_LOG(UG_NAME_RESP,"Error!!! Target item[0x%x] is NULL OR item data[0x%x] is NULL", target_item, gdata);
			if (event_info->event_type == WLAN_MANAGER_RESPONSE_TYPE_CONNECTING ||
				event_info->event_type == WLAN_MANAGER_RESPONSE_TYPE_CONFIGURATION ||
				event_info->event_type == WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_OK) {
				/* This situation comes during hidden AP connecting/connected event.
				* Anyways its always better to add the connecting/connected AP */
				wifi_device = view_list_item_device_info_create(event_info->ap);

				target_item = viewer_list_item_insert_after(wifi_device, NULL);

				if (!target_item ||
					!(gdata = (ug_genlist_data_t *)elm_object_item_data_get(target_item))) {
					ERROR_LOG(UG_NAME_RESP, "Error!!! Fatal: Unable to add a connecting/connected item with item data[0x%x].", gdata);

					goto exit;
				}
			} else {
				ERROR_LOG(UG_NAME_RESP, "Fatal: target_item or gdata is NULL");

				goto exit;
			}

		}
	}

	/* All OK to process the response */
	switch (event_info->event_type) {
	case WLAN_MANAGER_RESPONSE_TYPE_POWER_ON_OK:
		viewer_manager_show(VIEWER_WINSET_SEARCHING);
		viewer_manager_show(VIEWER_WINSET_SUB_CONTENTS);
		viewer_manager_header_mode_set(HEADER_MODE_SEARCHING);
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_POWER_OFF_OK:
		viewer_manager_cleanup_views();
		viewer_list_scanned_ap_list_free();
		viewer_list_item_clear();
		viewer_manager_header_mode_set(HEADER_MODE_OFF);
		viewer_manager_hide(VIEWER_WINSET_SUB_CONTENTS);

		break;

	case WLAN_MANAGER_RESPONSE_TYPE_POWER_ON_NOT_SUPPORTED:
		viewer_manager_header_mode_set(HEADER_MODE_OFF);
		common_utils_send_message_to_net_popup("Network connection popup", "not support", "notification", NULL);
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_POWER_ON_RESTRICTED:
		viewer_manager_header_mode_set(HEADER_MODE_OFF);
		common_utils_send_message_to_net_popup("Network connection popup", "wifi restricted", "popup", NULL);
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_SCAN_OK:
		/* Manual scan complete response */
		if (HEADER_MODE_DEACTIVATING == header_state ||
				HEADER_MODE_OFF == header_state) {
			break;
		}

		if (ug_app_state->ug_type == UG_VIEW_SETUP_WIZARD) {
			viewer_manager_update_hidden_btn();
			viewer_manager_update_setup_wizard_scan_btn();
		}

		viewer_manager_hide(VIEWER_WINSET_SEARCHING);
		/* fall through */
	case WLAN_MANAGER_RESPONSE_TYPE_SCAN_RESULT_IND:
		/* Auto scan complete response */

		vconf_get_int(VCONFKEY_WIFI_UG_RUN_STATE, &ug_state);
		if (ug_state == VCONFKEY_WIFI_UG_RUN_STATE_ON_FOREGROUND) {
			wlan_manager_scanned_profile_refresh();
		} else {
			INFO_LOG(UG_NAME_NORMAL, "Skipping refresh. UG state - [%d]",
					ug_state);
		}

		break;

	case WLAN_MANAGER_RESPONSE_TYPE_CONNECTING:
		viewer_manager_header_mode_set(HEADER_MODE_CONNECTING);

		viewer_manager_refresh_ap_info(target_item);
		viewer_list_item_radio_mode_set(target_item,
				VIEWER_ITEM_RADIO_MODE_CONNECTING);
		viewer_manager_move_item_to_top(target_item);
		viewer_manager_move_to_top();
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_CONFIGURATION:
		viewer_manager_header_mode_set(HEADER_MODE_CONNECTING);

		viewer_manager_refresh_ap_info(target_item);
		viewer_list_item_radio_mode_set(target_item,
				VIEWER_ITEM_RADIO_MODE_CONFIGURATION);
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_OK:
	case WLAN_MANAGER_RESPONSE_TYPE_WPS_ENROLL_OK:
		viewer_manager_header_mode_set(HEADER_MODE_CONNECTED);

		viewer_manager_refresh_ap_info(target_item);
		viewer_list_item_radio_mode_set(target_item,
				VIEWER_ITEM_RADIO_MODE_CONNECTED);

		if (ug_app_state->passpopup &&
				ug_app_state->passpopup->pbc_popup_data) {
			/* This is needed to remove the PBC timer popup */
			passwd_popup_free(ug_app_state->passpopup);
			ug_app_state->passpopup = NULL;
		}
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_CONNECT_FAILED:
		viewer_manager_header_mode_set(HEADER_MODE_ON);
		viewer_list_item_radio_mode_set(target_item, VIEWER_ITEM_RADIO_MODE_OFF);
		if (gdata)
			viewer_list_wifi_reconnect(gdata->device_info);
		return;

	case WLAN_MANAGER_RESPONSE_TYPE_WPS_ENROLL_FAIL:
		viewer_manager_header_mode_set(HEADER_MODE_ON);
		viewer_list_item_radio_mode_set(target_item, VIEWER_ITEM_RADIO_MODE_OFF);

		if (ug_app_state->passpopup &&
				ug_app_state->passpopup->pbc_popup_data) {
			passwd_popup_free(ug_app_state->passpopup);
			ug_app_state->passpopup = NULL;
		}
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_DISCONNECTION_OK:
		viewer_manager_refresh_ap_info(target_item);
		viewer_list_item_radio_mode_set(target_item, VIEWER_ITEM_RADIO_MODE_OFF);
		viewer_manager_header_mode_set(HEADER_MODE_ON);

		break;

	case WLAN_MANAGER_RESPONSE_TYPE_CANCEL_WPS_ENROLL_OK:
		viewer_list_item_radio_mode_set(target_item, VIEWER_ITEM_RADIO_MODE_OFF);
		viewer_manager_header_mode_set(HEADER_MODE_ON);
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_CANCEL_WPS_ENROLL_FAIL:
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_MAC_ID_IND:
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_SPECIFIC_SCAN_FAIL:
	case WLAN_MANAGER_RESPONSE_TYPE_SPECIFIC_SCAN_OK:
		viewer_manager_specific_scan_response_hlr(
						event_info->bss_info_list, user_data);
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_UPDATE_WIFI_RSSI:
		viewer_manager_update_rssi();
		break;

	case WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_INVALID_KEY:
		common_utils_send_message_to_net_popup(
				"Network connection popup", "wrong password",
				"toast_popup", NULL);

		if (gdata == NULL) {
			return;
		}

		viewer_list_wifi_connect(gdata->device_info);
		return;

	case WLAN_MANAGER_RESPONSE_TYPE_NONE:
	default:
		break;
	}

exit:
	wlan_validate_alt_connection();

	__COMMON_FUNC_EXIT__;
}

void wlan_engine_refresh_callback(void)
{
	__COMMON_FUNC_ENTER__;

	viewer_manager_refresh();

	__COMMON_FUNC_EXIT__;
}
