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

#ifndef __WIFI_WLAN_MANAGER_H__
#define __WIFI_WLAN_MANAGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>
#include <wifi.h>

#include "connman-request.h"
#include "connman-response.h"

typedef enum {
	WLAN_MANAGER_ERR_NONE = 0x00,
	WLAN_MANAGER_ERR_UNKNOWN,
	WLAN_MANAGER_ERR_INVALID_PARAM,
	WLAN_MANAGER_ERR_ALREADY_REGISTERED,
	WLAN_MANAGER_ERR_CONNECT_PASSWORD_NEEDED,
	WLAN_MANAGER_ERR_CONNECT_EAP_SEC_TYPE,
	WLAN_MANAGER_ERR_WIFI_TETHERING_OCCUPIED,
	WLAN_MANAGER_ERR_WIFI_AP_TETHERING_OCCUPIED,
	WLAN_MANAGER_ERR_NOSERVICE,
	WLAN_MANAGER_ERR_IN_PROGRESS,
} WLAN_MANAGER_ERR_TYPE;

typedef enum {
	SIGNAL_STRENGTH_TYPE_EXCELLENT,
	SIGNAL_STRENGTH_TYPE_GOOD,
	SIGNAL_STRENGTH_TYPE_WEAK,
	SIGNAL_STRENGTH_TYPE_VERY_WEAK,
	SIGNAL_STRENGTH_TYPE_NULL
} STRENGTH_TYPES;

/*
 * JOIN TYPE, SECURITY TYPE and EAP_TYPE
 *
 * Determine it`s security type
 * It should be merged into one enumerations
*/
typedef enum {
	SECURITY_TYPE_NONE = 0x01,
	SECURITY_TYPE_WEP,
	SECURITY_TYPE_WPA_PSK,
	SECURITY_TYPE_WPA2_PSK,

	SECURITY_TYPE_WPA_EAP,
	SECURITY_TYPE_WPA2_EAP,

	SECURITY_TYPE_MAX,
	SECURITY_TYPE_NULL
} SECURITY_TYPES;

typedef enum {
	WLAN_MANAGER_EAP_TYPE_NONE, /* only use it`s WPA number */
	WLAN_MANAGER_EAP_TYPE_TLS,
	WLAN_MANAGER_EAP_TYPE_TTLS_PAP,         /** PAP is tunneled protocol */
	WLAN_MANAGER_EAP_TYPE_TTLS_CHAP,             /** CHAP is tunneled protocol */
	WLAN_MANAGER_EAP_TYPE_TTLS_MSCHAP,           /** MSCHAP is tunneled protocol */
	WLAN_MANAGER_EAP_TYPE_TTLS_MSCHAPV2,         /** MSCHAPV2 is tunneled protocol */

	/** Define EAP-TTLS and tunneled EAP sub types here */
	WLAN_MANAGER_EAP_TYPE_TTLS_EAP_GTC,     /** EAP_GTC is tunneled protocol */
	WLAN_MANAGER_EAP_TYPE_TTLS_EAP_MD5,          /** EAP_MD5 is tunneled protocol */
	WLAN_MANAGER_EAP_TYPE_TTLS_EAP_MSCHAPV2,     /** EAP_MSCHAPV2 is tunneled protocol */

	/** Define EAP-PEAP version 0 */
	WLAN_MANAGER_EAP_TYPE_PEAP0_MSCHAPV2,   /** MSCHAPV2 is tunneled protocol, PEAP version is 0 */
	WLAN_MANAGER_EAP_TYPE_PEAP0_MD5,             /** MD5 is tunneled protocol, PEAP version is 0 */
	WLAN_MANAGER_EAP_TYPE_PEAP0_GTC,             /** GTC is tunneled protocol, PEAP version is 0 */

	/** Define EAP-PEAP version 1 */
	WLAN_MANAGER_EAP_TYPE_PEAP1_MSCHAPV2,   /** MSCHAPV2 is tunneled protocol, PEAP version is 1 */
	WLAN_MANAGER_EAP_TYPE_PEAP1_MD5,             /** MD5 is tunneled protocol, PEAP version is 1 */
	WLAN_MANAGER_EAP_TYPE_PEAP1_GTC,             /** GTC is tunneled protocol, PEAP version is 1 */
	WLAN_MANAGER_EAP_TYPE_MAX,
	WLAN_MANAGER_EAP_TYPE_ERROR
} WLAN_MANAGER_EAP_TYPES;

/*
 * RESPONSES
 */
typedef enum {
	WLAN_MANAGER_RESPONSE_TYPE_NONE								= 0x00,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTING						= 0x01,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_OK					= 0x02,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_FAIL_IN_PROGRESS		= 0x03,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_FAIL_ABORTED			= 0x04,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_FAIL_ALREADY_EXIST	= 0x05,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_FAIL_TIMEOUT			= 0x06,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_FAIL_UNKNOWN			= 0x07,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_FAIL_UNKNOWN_METHOD	= 0x08,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_CONNECT_FAILED		= 0x09,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_INVALID_KEY			= 0x0A,
	WLAN_MANAGER_RESPONSE_TYPE_DISCONNECTION_OK					= 0x0B,
	WLAN_MANAGER_RESPONSE_TYPE_POWER_ON_OK						= 0x0C,
	WLAN_MANAGER_RESPONSE_TYPE_POWER_ON_NOT_SUPPORTED			= 0x0D,
	WLAN_MANAGER_RESPONSE_TYPE_POWER_ON_RESTRICTED				= 0x0E,
	WLAN_MANAGER_RESPONSE_TYPE_POWER_OFF_OK						= 0x0F,
	WLAN_MANAGER_RESPONSE_TYPE_SCAN_OK							= 0x10,
	WLAN_MANAGER_RESPONSE_TYPE_WPS_ENROLL_OK					= 0x11,
	WLAN_MANAGER_RESPONSE_TYPE_WPS_ENROLL_FAIL					= 0x12,
	WLAN_MANAGER_RESPONSE_TYPE_CANCEL_WPS_ENROLL_OK				= 0x13,
	WLAN_MANAGER_RESPONSE_TYPE_CANCEL_WPS_ENROLL_FAIL			= 0x14,
	WLAN_MANAGER_RESPONSE_TYPE_CONNECTION_IND					= 0x15,
	WLAN_MANAGER_RESPONSE_TYPE_DISCONNECTION_IND				= 0x16,
	WLAN_MANAGER_RESPONSE_TYPE_SCAN_RESULT_IND					= 0x17,
	WLAN_MANAGER_RESPONSE_TYPE_MAC_ID_IND						= 0x18,
	WLAN_MANAGER_RESPONSE_TYPE_SPECIFIC_SCAN_OK					= 0x19,
	WLAN_MANAGER_RESPONSE_TYPE_SPECIFIC_SCAN_FAIL				= 0x1A,
	WLAN_MANAGER_RESPONSE_TYPE_SPECIFIC_SCAN_IND				= 0x1B,
	WLAN_MANAGER_RESPONSE_TYPE_UPDATE_WIFI_RSSI					= 0x1C,
	WLAN_MANAGER_RESPONSE_TYPE_CONFIGURATION						= 0x1D,
	WLAN_MANAGER_RESPONSE_TYPE_MAX								= 0x1E,
} WLAN_MANAGER_RESPONSE_TYPES;

#define WLAN_PROXY_LEN_MAX 64
#define MAX_BT_UUID 30

typedef struct _bluetooth_info_t {
char uuid[MAX_BT_UUID];
char* bt_server_address;
char* remote_server_name;
int gSocketFd;
} bluetooth_info_t;

typedef struct _wifi_device_info_t {
	wifi_ap_h ap;
	char *ssid;
	char *ap_status_txt;
	char *ap_image_path;
	int ipconfigtype;
	int rssi;
	wlan_security_mode_type_t security_mode;
	bool wps_mode;
	/*MINI*/
	bool is_bt_tethered_device;
	bluetooth_info_t *bt_info;
} wifi_device_info_t;

typedef struct {
	const char *password;
	char *category;
	char *subcategory;
	char *username;
	char *userpassword;
	char *ca_cert_filename;
	char *client_cert_filename;
	char *private_key;
	char *private_key_password;
	int wlan_eap_type;
} wlan_manager_password_data;

/* it should be implement. */
typedef enum {
	WLAN_MANAGER_ERROR			= 0x01,
	WLAN_MANAGER_OFF			= 0x02,
	WLAN_MANAGER_UNCONNECTED	= 0x03,
	WLAN_MANAGER_CONNECTED		= 0x04,
	WLAN_MANAGER_CONNECTING		= 0x05,
	WLAN_MANAGER_MAX			= 0x06,
} WLAN_MANAGER_STATES;

typedef void (*wlan_manager_ui_refresh_func_t)(void);

typedef struct {
	WLAN_MANAGER_RESPONSE_TYPES event_type;
	wifi_ap_h ap;
	wifi_rssi_level_e rssi_level;
	GSList *bss_info_list;
} wlan_mgr_event_info_t;

typedef void (*wlan_event_handler)(wlan_mgr_event_info_t *event_info, void *user_data);

/** It should be hide to others */
typedef struct wlan_manager_object {
	wlan_event_handler message_func;
	wlan_manager_ui_refresh_func_t refresh_func;
	gboolean b_scan_blocked;
	gboolean b_ui_refresh;
} wlan_manager_object;

/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////
//MINI
wifi_device_info_t *tether_device_get_singleton(void);

wlan_manager_object *wlan_manager_get_singleton(void);
void *wlan_manager_create(void);
int wlan_manager_destroy(void);
int wlan_manager_start(void);

wifi_ap_h wlan_manager_get_ap_with_state(int ap_state);
int wlan_manager_state_get(void);
void wlan_manager_set_message_callback(wlan_event_handler func);
void wlan_manager_set_refresh_callback(wlan_manager_ui_refresh_func_t func);

void wlan_manager_enable_scan_result_update(void);
void wlan_manager_disable_scan_result_update(void);

// request
int wlan_manager_connect(wifi_ap_h ap);
int wlan_manager_connect_with_password(wifi_ap_h ap, const char *pass_phrase);
int wlan_manager_disconnect(wifi_ap_h ap);
int wlan_manager_wps_connect(wifi_ap_h ap);
int wlan_manager_wps_pin_connect(wifi_ap_h ap, const char *pin);
int wlan_manager_power_on(void);
int wlan_manager_power_off(void);
int wlan_manager_scan(void);
int wlan_manager_forget(wifi_ap_h ap);
int wlan_manager_scan_with_ssid(const char *ssid, void *data);

int wlan_manager_profile_modify_by_device_info(net_profile_info_t *profiles);

STRENGTH_TYPES wlan_manager_get_signal_strength(int rssi);

//// profile refresh /////////////////////////////////////////////
void wlan_manager_scanned_profile_refresh(void);

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_REQUEST_HANDLER_H__ */
