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

int connman_request_register(void)
{
	int ret = net_register_client(network_evt_cb, NULL);
	switch (ret) {
		case NET_ERR_NONE:
			return WLAN_MANAGER_ERR_NONE;
		case NET_ERR_APP_ALREADY_REGISTERED:
			return WLAN_MANAGER_ERR_ALREADY_REGISTERED;
		default:
			return WLAN_MANAGER_ERR_UNKNOWN;
	}
}

int connman_request_deregister(void)
{
	int ret = net_deregister_client();
	switch (ret) {
		case NET_ERR_NONE:
			return WLAN_MANAGER_ERR_NONE;
		default:
			return WLAN_MANAGER_ERR_UNKNOWN;
	}
}

int connman_request_specific_scan(const char *ssid)
{
	int ret = net_specific_scan_wifi(ssid);
	INFO_LOG(COMMON_NAME_LIB,"net_specific_scan_wifi ret: %d", ret);

	switch (ret) {
		case NET_ERR_NONE:
		case NET_ERR_IN_PROGRESS:
			return WLAN_MANAGER_ERR_NONE;
		default:
			return WLAN_MANAGER_ERR_UNKNOWN;
	}
}

int connman_request_scan_mode_set(net_wifi_background_scan_mode_t scan_mode)
{
	int ret = net_wifi_set_background_scan_mode(scan_mode);

	switch (ret) {
	case NET_ERR_NONE:
		return WLAN_MANAGER_ERR_NONE;

	default:
		return WLAN_MANAGER_ERR_UNKNOWN;
	}
}
