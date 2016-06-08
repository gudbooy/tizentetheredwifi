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

#include <vconf-keys.h>

#include "common.h"
#include "connection_manager.h"
#include "wlan_manager.h"
#include "common_utils.h"
#include "wlan_connection.h"
#include "wifi-engine-callback.h"

typedef enum {
	WLAN_MANAGER_REQ_TYPE_ACTIVATE,
	WLAN_MANAGER_REQ_TYPE_DEACTIVATE,
	WLAN_MANAGER_REQ_TYPE_SCAN,
	WLAN_MANAGER_REQ_TYPE_SPECIFIC_SCAN,
	WLAN_MANAGER_REQ_TYPE_SCAN_RESULT,
	WLAN_MANAGER_REQ_TYPE_CONNECT,
	WLAN_MANAGER_REQ_TYPE_WPS_CONNECT,
	WLAN_MANAGER_REQ_TYPE_BG_SCAN_START,
} WLAN_MANAGER_REQUEST_TYPES;

typedef struct {
	WLAN_MANAGER_REQUEST_TYPES req_type;
	wifi_ap_h ap;
	void *user_data;
} wlan_mgr_req_data_t;

typedef struct {
	int state;
	wifi_ap_h *ap;
} ap_state_info_t;

wlan_mgr_req_data_t scan_req_data;

static void wlan_manager_register_cbs(void);
static void wlan_manager_deregister_cbs(void);

static wlan_manager_object *manager_object = NULL;
static wifi_device_info_t *tether_device = NULL;

wifi_device_info_t *tether_device_get_singleton(void)
{
	if(NULL == tether_device){
		tether_device = (wifi_device_info_t*)g_new0(wifi_device_info_t, 1);
		tether_device->ap = NULL;
		tether_device->ssid = "Blutooth";
		tether_device->ap_status_txt = NULL;
		tether_device->ap_image_path = NULL;
		tether_device->is_bt_tethered_device = true;
		tether_device->bt_info = g_new0(bluetooth_info_t, 1);
	}
	return tether_device;
}

wlan_manager_object *wlan_manager_get_singleton(void)
{
	if (NULL == manager_object) {
		manager_object = g_new0(wlan_manager_object, 1);
		manager_object->message_func = NULL;
		manager_object->refresh_func = NULL;
	}

	return manager_object;
}

void wlan_manager_set_refresh_callback(wlan_manager_ui_refresh_func_t func)
{
	manager_object->refresh_func = func;
}

void *wlan_manager_create(void)
{
	wlan_manager_get_singleton();

	return NULL;
}

int wlan_manager_destroy(void)
{
	int ret = WLAN_MANAGER_ERR_NONE;

	if (manager_object == NULL) {
		ERROR_LOG(COMMON_NAME_ERR, "Wlan manager object already destroyed");
		return ret;
	}

	common_util_unsubscribe_scanning_signal();

	wifi_unset_device_state_changed_cb();

	wlan_manager_deregister_cbs();

	connection_manager_destroy();

	ret = wifi_deinitialize();

	if (manager_object != NULL) {
		g_free(manager_object);
		manager_object = NULL;
	}

	return ret;
}

int wlan_manager_start(void)
{
	__COMMON_FUNC_ENTER__;

	int ret = WLAN_MANAGER_ERR_NONE;

	switch (wifi_initialize()) {
	case WIFI_ERROR_NONE:
		/* Register the callbacks */
		wlan_manager_register_cbs();
		connection_manager_create();
		break;

	case WIFI_ERROR_INVALID_OPERATION:
		/* Register the callbacks */
		wlan_manager_register_cbs();
		ret = WLAN_MANAGER_ERR_ALREADY_REGISTERED;
		break;

	default:
		ret = WLAN_MANAGER_ERR_UNKNOWN;
		break;
	}

	__COMMON_FUNC_EXIT__;

	return ret;
}

int wlan_manager_forget(wifi_ap_h ap)
{
	wifi_forget_ap(ap);

	return WLAN_MANAGER_ERR_NONE;
}

static void wlan_manager_device_state_changed_cb(
		wifi_device_state_e state, void *user_data)
{
	__COMMON_FUNC_ENTER__;

	wlan_mgr_event_info_t event_info;
	memset(&event_info, 0, sizeof(event_info));

	switch (state) {
	case WIFI_DEVICE_STATE_ACTIVATED:
		connman_request_scan_mode_set(WIFI_BGSCAN_MODE_PERIODIC);

		wlan_manager_enable_scan_result_update();
		wlan_manager_register_cbs();
		event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_POWER_ON_OK;
		break;

	case WIFI_DEVICE_STATE_DEACTIVATED:
		wlan_manager_deregister_cbs();
		event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_POWER_OFF_OK;
		break;

	default:
		return;
	}

	manager_object->message_func(&event_info, user_data);

	__COMMON_FUNC_EXIT__;
}

static void wlan_manager_connection_state_changed_cb(
		wifi_connection_state_e state, wifi_ap_h ap, void *user_data)
{
	__COMMON_FUNC_ENTER__;

	wlan_mgr_event_info_t event_info;
	memset(&event_info, 0, sizeof(event_info));

	event_info.ap = ap;

	switch (state) {
	case WIFI_CONNECTION_STATE_FAILURE:
	case WIFI_CONNECTION_STATE_DISCONNECTED:  /**< Disconnected state */
		event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_DISCONNECTION_OK;
		break;
	case WIFI_CONNECTION_STATE_ASSOCIATION:  /**< Association state */
		event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_CONNECTING;
		break;
	case WIFI_CONNECTION_STATE_CONFIGURATION:  /**< Configuration state */
		event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_CONFIGURATION;
		break;
	case WIFI_CONNECTION_STATE_CONNECTED:  /**< Connected state */
		event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_OK;
		break;
	default:
		return;
	}

	manager_object->message_func(&event_info, user_data);

	__COMMON_FUNC_EXIT__;
}

static void wlan_manager_rssi_level_changed_cb(
		wifi_rssi_level_e rssi_level, void *user_data)
{
	__COMMON_FUNC_ENTER__;

	wlan_mgr_event_info_t event_info;

	memset(&event_info, 0, sizeof(event_info));
	event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_UPDATE_WIFI_RSSI;
	event_info.rssi_level = rssi_level;

	manager_object->message_func(&event_info, user_data);

	__COMMON_FUNC_EXIT__;
}

static void wlan_manager_specific_scan_finished_cb(
		wifi_error_e error_code, GSList *bss_info_list, void *user_data)
{
	__COMMON_FUNC_ENTER__;

	wlan_mgr_event_info_t event_info;
	memset(&event_info, 0, sizeof(event_info));

	wifi_unset_specific_scan_cb();

	if (WIFI_ERROR_NONE == error_code) {
		event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_SPECIFIC_SCAN_OK;
		event_info.bss_info_list = bss_info_list;
	} else {
		event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_SPECIFIC_SCAN_FAIL;
	}

	manager_object->message_func(&event_info, user_data);

	__COMMON_FUNC_EXIT__;
}

static void wlan_manager_network_event_cb(
		wifi_error_e error_code, void *user_data)
{
	__COMMON_FUNC_ENTER__;

	wlan_mgr_req_data_t *req_data = (wlan_mgr_req_data_t *)user_data;
	if (req_data == NULL) {
		ERROR_LOG(UG_NAME_ERR, "Request data is NULL !!!");

		__COMMON_FUNC_EXIT__;
		return;
	}

	wlan_mgr_event_info_t event_info;
	memset(&event_info, 0, sizeof(event_info));

	switch (req_data->req_type) {
	case WLAN_MANAGER_REQ_TYPE_ACTIVATE:
		/* fall through */
	case WLAN_MANAGER_REQ_TYPE_DEACTIVATE:
		/* We will send POWER_ON_OK / POWER_OFF_OK response when we receive
		 * device state change. Lets just clean up the request data now.
		 */
		goto exit;
		/* fall through */
	case WLAN_MANAGER_REQ_TYPE_SCAN:
		if (WIFI_ERROR_NONE == error_code) {
			event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_SCAN_OK;
		} else {
			goto exit;
		}

		break;

	case WLAN_MANAGER_REQ_TYPE_SPECIFIC_SCAN:
		if (WIFI_ERROR_NONE == error_code) {
			event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_SPECIFIC_SCAN_OK;
		} else {
			event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_SPECIFIC_SCAN_FAIL;
		}

		break;

	case WLAN_MANAGER_REQ_TYPE_SCAN_RESULT:
		if (WIFI_ERROR_NONE == error_code) {
			event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_SCAN_RESULT_IND;
			manager_object->message_func(&event_info, req_data->user_data);
		}

		return;	// The request data is static. So returning here.

	case WLAN_MANAGER_REQ_TYPE_CONNECT:
		event_info.ap = req_data->ap;

		if (WIFI_ERROR_NONE != error_code) {
			if (error_code == WIFI_ERROR_INVALID_KEY) {
				event_info.event_type =
						WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_INVALID_KEY;
			} else {
				event_info.event_type =
						WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_CONNECT_FAILED;
			}
		} else {
			goto exit;
		}

		break;

	case WLAN_MANAGER_REQ_TYPE_WPS_CONNECT:
		event_info.ap = req_data->ap;

		if (WIFI_ERROR_NONE != error_code) {
			event_info.event_type = WLAN_MANAGER_RESPONSE_TYPE_WPS_ENROLL_FAIL;
		} else {
			goto exit;
		}

		break;

	default:
		goto exit;
	}

	manager_object->message_func(&event_info, req_data->user_data);

exit:
	if (req_data != NULL) {
		wifi_ap_destroy(req_data->ap);
		g_free(req_data);
	}

	__COMMON_FUNC_EXIT__;
}

static void wlan_manager_register_cbs(void)
{
	__COMMON_FUNC_ENTER__;

	wifi_set_device_state_changed_cb(wlan_manager_device_state_changed_cb, NULL);
	wifi_set_connection_state_changed_cb(wlan_manager_connection_state_changed_cb, NULL);
	wifi_set_rssi_level_changed_cb(wlan_manager_rssi_level_changed_cb, NULL);

	memset(&scan_req_data, 0, sizeof(scan_req_data));
	scan_req_data.req_type = WLAN_MANAGER_REQ_TYPE_SCAN_RESULT;
	wifi_set_background_scan_cb(wlan_manager_network_event_cb, &scan_req_data);

	connman_request_register();

	__COMMON_FUNC_EXIT__;
}

static void wlan_manager_deregister_cbs(void)
{
	__COMMON_FUNC_ENTER__;

	/* NOTE:
	 * We don't deregister the device state change cb here.
	 * We need to continue to listen to the device state change, because it is
	 * possible that the WiFi could be powered on / off from multiple entry
	 * points like example: Quick panel WiFi icon, Wi-Fi UG, Setting Menu.
	 *
	 * We will deregister the device state change cb only on wlan manager
	 * detsroy.
	 */


	wifi_unset_background_scan_cb();
	wifi_unset_connection_state_changed_cb();
	wifi_unset_rssi_level_changed_cb();
	wifi_unset_specific_scan_cb();
	connman_request_deregister();

	__COMMON_FUNC_EXIT__;
}

void wlan_manager_set_message_callback(wlan_event_handler func)
{
	manager_object->message_func = func;
}

static bool wifi_found_ap_with_state_cb(wifi_ap_h ap, void* user_data)
{
	wifi_connection_state_e state;
	ap_state_info_t *ap_state_info = (ap_state_info_t *)user_data;
	bool found_match = false;

	if (WIFI_ERROR_NONE != wifi_ap_get_connection_state(ap, &state))
		return true; // continue with next AP

	switch (ap_state_info->state) {
	case WLAN_MANAGER_UNCONNECTED:
		if (WIFI_CONNECTION_STATE_DISCONNECTED == state ||
				WIFI_CONNECTION_STATE_FAILURE == state) {
			/* Found a match, so terminate the loop */
			found_match = true;
		}
		break;
	case WLAN_MANAGER_CONNECTING:
		if (WIFI_CONNECTION_STATE_ASSOCIATION == state ||
			WIFI_CONNECTION_STATE_CONFIGURATION == state) {
			/* Found a match, so terminate the loop */
			found_match = true;
		}
		break;
	case WLAN_MANAGER_CONNECTED:
		if (WIFI_CONNECTION_STATE_CONNECTED == state) {
			/* Found a match, so terminate the loop */
			found_match = true;
		}
		break;
	default:
		ERROR_LOG(COMMON_NAME_ERR, "Unknown Wi-Fi state: %d", ap_state_info->state);
		return false;
	}

	if (true == found_match) {
		if (ap_state_info->ap) {
			wifi_ap_clone(ap_state_info->ap, ap);
		}
		INFO_LOG(COMMON_NAME_LIB, "Found an AP[0x%x] in the state %d", ap, ap_state_info->state);
		return false;	// found the match, so terminate the loop.
	}
	return true;
}

wifi_ap_h wlan_manager_get_ap_with_state(int ap_state)
{
	ap_state_info_t ap_state_info;
	wifi_ap_h ap = NULL;

	ap_state_info.state = ap_state;
	ap_state_info.ap = &ap;

	wifi_foreach_found_aps(wifi_found_ap_with_state_cb, &ap_state_info);

	return ap;
}

int wlan_manager_state_get(void)
{
	int ret_val = 0;
	wifi_connection_state_e connection_state;
	bool activated;

	if (WIFI_ERROR_NONE != wifi_is_activated(&activated)) {
		return WLAN_MANAGER_ERROR;
	} else if (false == activated) {
		INFO_LOG(COMMON_NAME_LIB, "STATE: WIFI_OFF");

		return WLAN_MANAGER_OFF;
	}

	ret_val = wifi_get_connection_state(&connection_state);
	if (WIFI_ERROR_NONE != ret_val) {
		return WLAN_MANAGER_ERROR;
	}

	switch (connection_state) {
	case WIFI_CONNECTION_STATE_FAILURE:
	case WIFI_CONNECTION_STATE_DISCONNECTED:
		INFO_LOG(COMMON_NAME_LIB, "STATE: WIFI_DISCONNECTED");

		ret_val = WLAN_MANAGER_UNCONNECTED;
		break;
	case WIFI_CONNECTION_STATE_ASSOCIATION:
	case WIFI_CONNECTION_STATE_CONFIGURATION:
		INFO_LOG(COMMON_NAME_LIB, "STATE: WIFI_CONNECTING");

		ret_val = WLAN_MANAGER_CONNECTING;
		break;
	case WIFI_CONNECTION_STATE_CONNECTED:
		INFO_LOG(COMMON_NAME_LIB, "STATE: WIFI_CONNECTED");

		ret_val = WLAN_MANAGER_CONNECTED;
		break;
	default:
		ERROR_LOG(COMMON_NAME_ERR, "Unknown state: %d", connection_state);

		ret_val = WLAN_MANAGER_ERROR;
		break;
	}

	return ret_val;
}

int wlan_manager_power_on(void)
{
	__COMMON_FUNC_ENTER__;

	INFO_LOG(UG_NAME_REQ, "power on");

	int ret = 0;
#if defined TIZEN_TETHERING_ENABLE
	int tethering =
			common_util_get_system_registry("memory/mobile_hotspot/mode");
	if (tethering < 0) {
		INFO_LOG(COMMON_NAME_LIB, "Fail to get tethering state");
	} else if (VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI & tethering) {
		INFO_LOG(COMMON_NAME_LIB, "Wi-Fi tethering is ON");

		__COMMON_FUNC_EXIT__;
		return WLAN_MANAGER_ERR_WIFI_TETHERING_OCCUPIED;
	} else if (VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI_AP & tethering) {
		INFO_LOG(COMMON_NAME_LIB, "Wi-Fi AP tethering is ON");

		__COMMON_FUNC_EXIT__;
		return WLAN_MANAGER_ERR_WIFI_AP_TETHERING_OCCUPIED;
	}
#endif

	wlan_mgr_req_data_t *req_data = g_new0(wlan_mgr_req_data_t, 1);
	req_data->req_type = WLAN_MANAGER_REQ_TYPE_ACTIVATE;
	ret = wifi_activate(wlan_manager_network_event_cb, req_data);
	if (WIFI_ERROR_NONE != ret && WIFI_ERROR_ALREADY_EXISTS != ret) {
		ERROR_LOG(UG_NAME_REQ, "Power on request. Error Reason [%d]", ret);

		g_free(req_data);

		__COMMON_FUNC_EXIT__;
		return WLAN_MANAGER_ERR_UNKNOWN;
	}

	__COMMON_FUNC_EXIT__;
	return WLAN_MANAGER_ERR_NONE;
}

int wlan_manager_power_off(void)
{
	__COMMON_FUNC_ENTER__;

	wlan_mgr_req_data_t *req_data = g_new0(wlan_mgr_req_data_t, 1);
	req_data->req_type = WLAN_MANAGER_REQ_TYPE_DEACTIVATE;

	int ret = wifi_deactivate(wlan_manager_network_event_cb, req_data);
	if (WIFI_ERROR_NONE != ret && WIFI_ERROR_ALREADY_EXISTS != ret) {
		ERROR_LOG(UG_NAME_REQ, "Power off request. Error Reason [%d]", ret);

		g_free(req_data);

		__COMMON_FUNC_EXIT__;
		return WLAN_MANAGER_ERR_UNKNOWN;
	}

	__COMMON_FUNC_EXIT__;
	return WLAN_MANAGER_ERR_NONE;
}

int wlan_manager_wps_connect(wifi_ap_h ap)
{
	__COMMON_FUNC_ENTER__;

	wlan_mgr_req_data_t *req_data = g_new0(wlan_mgr_req_data_t, 1);
	req_data->req_type = WLAN_MANAGER_REQ_TYPE_WPS_CONNECT;
	wifi_ap_clone(&(req_data->ap), ap);

	int ret = wifi_connect_by_wps_pbc(req_data->ap,
			wlan_manager_network_event_cb, req_data);
	if (WIFI_ERROR_NONE != ret) {
		ERROR_LOG(UG_NAME_REQ, "WPS Connect failed. Error Reason [%d]", ret);

		wifi_ap_destroy(req_data->ap);

		g_free(req_data);

		__COMMON_FUNC_EXIT__;
		return WLAN_MANAGER_ERR_UNKNOWN;
	}

	__COMMON_FUNC_EXIT__;
	return WLAN_MANAGER_ERR_NONE;
}

int wlan_manager_wps_pin_connect(wifi_ap_h ap, const char *pin)
{
	__COMMON_FUNC_ENTER__;

	wlan_mgr_req_data_t *req_data = g_new0(wlan_mgr_req_data_t, 1);
	req_data->req_type = WLAN_MANAGER_REQ_TYPE_WPS_CONNECT;
	wifi_ap_clone(&(req_data->ap), ap);

	int ret = wifi_connect_by_wps_pin(req_data->ap, pin,
			wlan_manager_network_event_cb, req_data);
	if (WIFI_ERROR_NONE != ret) {
		ERROR_LOG(UG_NAME_REQ, "WPS Connect failed. Error Reason [%d]", ret);

		wifi_ap_destroy(req_data->ap);

		g_free(req_data);

		__COMMON_FUNC_EXIT__;
		return WLAN_MANAGER_ERR_UNKNOWN;
	}

	__COMMON_FUNC_EXIT__;
	return WLAN_MANAGER_ERR_NONE;
}

int wlan_manager_connect(wifi_ap_h ap)
{
	__COMMON_FUNC_ENTER__;

	int ret;

	if (ap == NULL) {
		__COMMON_FUNC_EXIT__;
		return WLAN_MANAGER_ERR_NOSERVICE;
	}

	wlan_mgr_req_data_t *req_data = g_new0(wlan_mgr_req_data_t, 1);
	req_data->req_type = WLAN_MANAGER_REQ_TYPE_CONNECT;
	wifi_ap_clone(&(req_data->ap), ap);

	ret = wlan_connect(req_data->ap, wlan_manager_network_event_cb, req_data);
	if (ret != WIFI_ERROR_NONE) {
		ERROR_LOG(UG_NAME_REQ, "Connect failed. Error Reason [%d]", ret);

		wifi_ap_destroy(req_data->ap);
		g_free(req_data);
	}

	__COMMON_FUNC_EXIT__;
	return ret;
}

int wlan_manager_connect_with_password(wifi_ap_h ap, const char *pass_phrase)
{
	__COMMON_FUNC_ENTER__;

	int ret;

	if (ap == NULL) {
		return WLAN_MANAGER_ERR_INVALID_PARAM;
	}

	ret = wifi_ap_set_passphrase(ap, pass_phrase);
	if (ret != WIFI_ERROR_NONE) {
		__COMMON_FUNC_EXIT__;
		return ret;
	}

	wlan_mgr_req_data_t *req_data = g_new0(wlan_mgr_req_data_t, 1);
	req_data->req_type = WLAN_MANAGER_REQ_TYPE_CONNECT;
	wifi_ap_clone(&(req_data->ap), ap);

	ret = wlan_connect(req_data->ap, wlan_manager_network_event_cb, req_data);
	if (ret != WIFI_ERROR_NONE) {
		ERROR_LOG(UG_NAME_REQ, "Connect failed. Error Reason [%d]", ret);

		wifi_ap_destroy(req_data->ap);
		g_free(req_data);
	}

	__COMMON_FUNC_EXIT__;
	return ret;
}

int wlan_manager_disconnect(wifi_ap_h ap)
{
	__COMMON_FUNC_ENTER__;

	int ret;

	INFO_LOG(UG_NAME_REQ, "Request disconnection for ap [0x%x]", ap);

	ret = wifi_disconnect(ap, NULL, NULL);
	if (WIFI_ERROR_NONE != ret) {
		ERROR_LOG(UG_NAME_REQ, "Disconnect failed. Error Reason [%d]", ret);

		__COMMON_FUNC_EXIT__;
		return WLAN_MANAGER_ERR_UNKNOWN;
	}

	__COMMON_FUNC_EXIT__;
	return WLAN_MANAGER_ERR_NONE;
}

int wlan_manager_scan(void)
{
	__COMMON_FUNC_ENTER__;

	wlan_mgr_req_data_t *req_data = g_new0(wlan_mgr_req_data_t, 1);
	req_data->req_type = WLAN_MANAGER_REQ_TYPE_SCAN;

	int ret = wifi_scan(wlan_manager_network_event_cb, req_data);
	if (ret != WLAN_MANAGER_ERR_NONE) {
		ERROR_LOG(UG_NAME_REQ, "Scan failed. Ret: %d", ret);

		g_free(req_data);

		__COMMON_FUNC_EXIT__;
		return ret;
	}

	/* Since the scan request was success,
	 * lets reset the ui refresh and scan update blocked flags.
	 */
	manager_object->b_scan_blocked = FALSE;
	manager_object->b_ui_refresh = FALSE;

	__COMMON_FUNC_EXIT__;
	return ret;
}

int wlan_manager_scan_with_ssid(const char *ssid, void *user_data)
{
	__COMMON_FUNC_ENTER__;

	wifi_set_specific_scan_cb(wlan_manager_specific_scan_finished_cb, user_data);

	int ret = connman_request_specific_scan(ssid);
	if (ret != WLAN_MANAGER_ERR_NONE) {
		ERROR_LOG(UG_NAME_REQ, "Specific Scan failed. Ret: %d", ret);

		__COMMON_FUNC_EXIT__;
		return ret;
	}

	/* Since the scan request was success,
	 * lets reset the ui refresh and scan update blocked flags.
	 */
	manager_object->b_scan_blocked = FALSE;
	manager_object->b_ui_refresh = FALSE;

	__COMMON_FUNC_EXIT__;
	return ret;
}

void wlan_manager_scanned_profile_refresh(void)
{
	__COMMON_FUNC_ENTER__;

	if (FALSE == manager_object->b_scan_blocked)
		manager_object->refresh_func();
	else
		manager_object->b_ui_refresh = TRUE;

	__COMMON_FUNC_EXIT__;
}

STRENGTH_TYPES wlan_manager_get_signal_strength(int rssi)
{
	/* Wi-Fi Signal Strength Display (dB / ConnMan normalized value)
	 *
	 * Excellent :	-63 ~		/ 57 ~
	 * Good:		-74 ~ -64	/ 46 ~ 56
	 * Weak:		-82 ~ -75	/ 38 ~ 45
	 * Very weak:		~ -83	/    ~ 37
	 */
	if (rssi >= -63) {
		return SIGNAL_STRENGTH_TYPE_EXCELLENT;
	} else if (rssi >= -74) {
		return SIGNAL_STRENGTH_TYPE_GOOD;
	} else if (rssi >= -82) {
		return SIGNAL_STRENGTH_TYPE_WEAK;
	} else {
		return SIGNAL_STRENGTH_TYPE_VERY_WEAK;
	}
}

static gboolean _refresh_ui(void *data)
{
	manager_object->refresh_func();

	manager_object->b_scan_blocked = FALSE;
	manager_object->b_ui_refresh = FALSE;

	return FALSE;
}

void wlan_manager_enable_scan_result_update(void)
{
	__COMMON_FUNC_ENTER__;

	if (TRUE == manager_object->b_scan_blocked) {
		if (TRUE == manager_object->b_ui_refresh) {
			DEBUG_LOG(COMMON_NAME_LIB, "Refresh the UI with last scan update");

			/* Delayed rendering in order to get smooth effect of popup close */
			common_util_managed_idle_add(_refresh_ui, NULL);
		} else {
			manager_object->b_scan_blocked = FALSE;
		}
	}

	__COMMON_FUNC_EXIT__;
}

void wlan_manager_disable_scan_result_update(void)
{
	__COMMON_FUNC_ENTER__;

	manager_object->b_scan_blocked = TRUE;

	__COMMON_FUNC_EXIT__;
}
