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
#include "wlan_manager.h"

struct _wifi_cb_s {
	wifi_specific_scan_finished_cb specific_scan_cb;
	void *specific_scan_user_data;
};

static struct _wifi_cb_s wifi_callbacks = {NULL, NULL};

int wifi_set_specific_scan_cb(wifi_specific_scan_finished_cb cb, void *data)
{
	__COMMON_FUNC_ENTER__;

	if (!cb) {
		__COMMON_FUNC_EXIT__;
		return WIFI_ERROR_INVALID_PARAMETER;
	}

	wifi_callbacks.specific_scan_cb = cb;
	wifi_callbacks.specific_scan_user_data = data;

	__COMMON_FUNC_EXIT__;
	return WIFI_ERROR_NONE;
}

int wifi_unset_specific_scan_cb(void)
{
	__COMMON_FUNC_ENTER__;

	if (wifi_callbacks.specific_scan_cb == NULL) {
		__COMMON_FUNC_EXIT__;
		return WIFI_ERROR_INVALID_OPERATION;
	}

	wifi_callbacks.specific_scan_cb = NULL;
	wifi_callbacks.specific_scan_user_data = NULL;

	__COMMON_FUNC_EXIT__;
	return WIFI_ERROR_NONE;
}

void network_evt_cb(const net_event_info_t *net_event, void *user_data)
{
	__COMMON_FUNC_ENTER__;

	switch (net_event->Event) {
	case NET_EVENT_SPECIFIC_SCAN_RSP:
		INFO_LOG(COMMON_NAME_LIB, "NET_EVENT_SPECIFIC_SCAN_RSP");

		if (wifi_callbacks.specific_scan_cb) {
			if (net_event->Error != NET_ERR_NONE) {
				wifi_callbacks.specific_scan_cb(
						WIFI_ERROR_OPERATION_FAILED,
						NULL,
						wifi_callbacks.specific_scan_user_data);
			} else {
				INFO_LOG(COMMON_NAME_LIB, "Specific scan request successful");
			}
		} else {
			ERROR_LOG(COMMON_NAME_LIB, "Specific scan cb is not set !!!");
		}
		break;

	case NET_EVENT_SPECIFIC_SCAN_IND:
		INFO_LOG(COMMON_NAME_LIB, "SSID scan results (%d found)", (int)net_event->Datalength);

		if (wifi_callbacks.specific_scan_cb) {
			if (net_event->Error == NET_ERR_NONE) {
				wifi_callbacks.specific_scan_cb(WIFI_ERROR_NONE,
						net_event->Data,
						wifi_callbacks.specific_scan_user_data);
			} else {
				wifi_callbacks.specific_scan_cb(WIFI_ERROR_OPERATION_FAILED,
						NULL,
						wifi_callbacks.specific_scan_user_data);
			}
		}
		break;

	default:
		break;
	}

	__COMMON_FUNC_EXIT__;
}
