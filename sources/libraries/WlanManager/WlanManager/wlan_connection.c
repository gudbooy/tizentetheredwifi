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
#include "wlan_connection.h"

struct wlan_connection {
	wifi_ap_h ap;
	wifi_connected_cb callback;
	void *user_data;
};

static struct wlan_connection current_item = { NULL, NULL, NULL };
static struct wlan_connection next_item = { NULL, NULL, NULL };

static void wlan_connect_debug(wifi_ap_h ap)
{
	char *next_ssid, *ap_ssid;

	wifi_ap_get_essid(ap, &ap_ssid);

	if (next_item.ap == NULL) {
		SECURE_ERROR_LOG(UG_NAME_REQ, "%s will be connected", ap_ssid);
	} else {
		wifi_ap_get_essid(next_item.ap, &next_ssid);

		SECURE_ERROR_LOG(UG_NAME_REQ, "%s will be connected (%s replaced)",
				ap_ssid, next_ssid);

		g_free(next_ssid);
	}

	g_free(ap_ssid);
}

static gboolean wlan_is_same_with_next(wifi_ap_h ap)
{
	gboolean is_same = FALSE;
	char *next_ssid, *ap_ssid;
	wifi_security_type_e next_sec, ap_sec;

	if (next_item.ap == NULL) {
		return FALSE;
	}

	wifi_ap_get_security_type(ap, &ap_sec);
	wifi_ap_get_security_type(next_item.ap, &next_sec);

	if (ap_sec != next_sec) {
		return is_same;
	}

	wifi_ap_get_essid(ap, &ap_ssid);
	wifi_ap_get_essid(next_item.ap, &next_ssid);

	if (g_strcmp0(ap_ssid, next_ssid) == 0) {
		is_same = TRUE;
	}

	g_free(ap_ssid);
	g_free(next_ssid);

	return is_same;
}

static void wlan_go_fast_next(void)
{
	bool favorite = false;

	if (current_item.ap == NULL || next_item.ap == NULL) {
		return;
	}

	wifi_ap_is_favorite(current_item.ap, &favorite);
	if (favorite == true) {
		return;
	}

	wifi_disconnect(current_item.ap, NULL, NULL);
}

static void wlan_update_next(wifi_ap_h ap, wifi_connected_cb callback,
		void *user_data)
{
	if (wlan_is_same_with_next(ap) == TRUE) {
		wifi_ap_destroy(ap);
		g_free(user_data);

		return;
	}

	wlan_connect_debug(ap);

	if (next_item.ap != NULL) {
		wifi_ap_destroy(next_item.ap);
		g_free(next_item.user_data);
	}

	next_item.ap = ap;
	next_item.callback = callback;
	next_item.user_data = user_data;

	wlan_go_fast_next();
}

static void wlan_connect_next(void)
{
	current_item.ap = next_item.ap;
	current_item.callback = next_item.callback;
	current_item.user_data = next_item.user_data;

	if (next_item.ap == NULL) {
		return;
	}

	next_item.ap = NULL;
	next_item.callback = NULL;
	next_item.user_data = NULL;

	wifi_connect(current_item.ap, current_item.callback, current_item.user_data);
}

int wlan_connect(wifi_ap_h ap, wifi_connected_cb callback, void *user_data)
{
	if (current_item.ap == NULL) {
		current_item.ap = ap;
		current_item.callback = callback;
		current_item.user_data = user_data;

		return wifi_connect(ap, callback, user_data);
	}

	wlan_update_next(ap, callback, user_data);

	return WIFI_ERROR_NONE;
}

void wlan_validate_alt_connection(void)
{
	wifi_connection_state_e state = WIFI_CONNECTION_STATE_DISCONNECTED;

	if (current_item.ap == NULL) {
		return;
	}

	wifi_get_connection_state(&state);

	switch (state) {
	case WIFI_CONNECTION_STATE_FAILURE:
	case WIFI_CONNECTION_STATE_CONNECTED:
	case WIFI_CONNECTION_STATE_DISCONNECTED:
		wlan_connect_next();
		break;

	case WIFI_CONNECTION_STATE_ASSOCIATION:
	case WIFI_CONNECTION_STATE_CONFIGURATION:
		wlan_go_fast_next();
		break;
	default:
		break;
	}
}
