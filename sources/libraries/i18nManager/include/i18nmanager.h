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

#ifndef __I18NMANAGER_H__
#define __I18NMANAGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define sc(pkg_name, I18N_TYPE) i18n_manager_get_text(pkg_name, I18N_TYPE)

typedef enum {
	I18N_MODE_SELF_MADE, /* self made, reserve to add platform string convert service */
	I18N_MODE_NO_NEEDED, /* No need to convert other language */
	I18N_MODE_MAX
} I18N_MODES;

typedef enum {
	I18N_TYPE_Ok,
	I18N_TYPE_Find,
	I18N_TYPE_Cancel,
	I18N_TYPE_Wi_Fi,
	I18N_TYPE_Name,
	I18N_TYPE_Unknown,
	I18N_TYPE_Scan,
	I18N_TYPE_Next,
	I18N_TYPE_Prev,
	I18N_TYPE_On,
	I18N_TYPE_Off,
	I18N_TYPE_Maximum_Number_Of_Characters_PD_Reached,
	I18N_TYPE_Connected_To_Wi_Fi_Network,
	I18N_TYPE_Wrong_Password,
	I18N_TYPE_No_Wi_Fi_AP_Found,
	I18N_TYPE_Available_networks,
	I18N_TYPE_Open,
	I18N_TYPE_Secured,
	I18N_TYPE_WPS_Available,
	I18N_TYPE_EAP,
	I18N_TYPE_Saved,
	I18N_TYPE_Connecting,
	I18N_TYPE_Obtaining_IP_addr,
	I18N_TYPE_Connected,
	I18N_TYPE_Find_Hidden_Network,
	I18N_TYPE_Password,
	I18N_TYPE_Show_password,
	I18N_TYPE_Wi_Fi_network_info,
	I18N_TYPE_Connect,
	I18N_TYPE_WPS,
	I18N_TYPE_WPS_Button,
	I18N_TYPE_WPS_PIN,
	I18N_TYPE_Press_WPS_On_Your_Wi_Fi_Access_Point,
	I18N_TYPE_Enter_PIN_number_on_your_WIFI_access_point,
	I18N_TYPE_Static_IP,
	I18N_TYPE_IP_address,
	I18N_TYPE_MAC_addr,
	I18N_TYPE_Proxy_address,
	I18N_TYPE_Proxy_port,
	I18N_TYPE_Subnet_mask,
	I18N_TYPE_Gateway_address,
	I18N_TYPE_DNS_1,
	I18N_TYPE_DNS_2,
	I18N_TYPE_Forget,
	I18N_TYPE_Forget_Network,
	I18N_TYPE_EAP_method,
	I18N_TYPE_Phase_2_authentication,
	I18N_TYPE_User_Certificate,
	I18N_TYPE_Unspecified,
	I18N_TYPE_Identity,
	I18N_TYPE_Enter_Identity,
	I18N_TYPE_Enter_password,
	I18N_TYPE_Ssid,
	I18N_TYPE_A_Wi_Fi_Network_Has_Been_Detected,
	I18N_TYPE_Advanced_setting,
	I18N_TYPE_Network_notification,
	I18N_TYPE_Network_notify_me_later,
	I18N_TYPE_Keep_WIFI_on_during_sleep,
	I18N_TYPE_Always,
	I18N_TYPE_Plugged,
	I18N_TYPE_Donot_Use,
	I18N_TYPE_Increases_Data_Usage,
	I18N_TYPE_Sort_by,
	I18N_TYPE_Alphabetical,
	I18N_TYPE_Signal_Strength,
	I18N_TYPE_Advanced,
	I18N_TYPE_Current_Network_Will_Be_Disconnected,
	I18N_TYPE_Select_WPS_Method,
	I18N_TYPE_Invalid_pin,
	I18N_TYPE_Invalid_certificate,
	I18N_TYPE_Skip,
	I18N_TYPE_WiFi_network_detected_connect,

	I18N_TYPE_Button,
	I18N_TYPE_Double_tap,
	I18N_TYPE_Connect_to_device,


	I18N_TYPE_Excellent,
	I18N_TYPE_Good,
	I18N_TYPE_Weak,
	I18N_TYPE_Activating,
	I18N_TYPE_WiFi_network_will_disable_tethering,
	I18N_TYPE_Scanning,
	I18N_TYPE_Help_tap_the_network_to_connect,
	I18N_TYPE_Help_automatically_connected,
	I18N_TYPE_Help_successfully_connected,
	I18N_TYPE_Help_connect_to_secured_network,
	I18N_TYPE_Help_tap_to_scan,
	I18N_TYPE_Help_no_wifi_networks,
	I18N_TYPE_Help_invalid_action,
	I18N_TYPE_Enter_user_id,
	I18N_TYPE_Enter_user_password,

} I18N_TYPES;

char* i18n_manager_get_text(const char *pkg_name, I18N_TYPES type);

#ifdef __cplusplus
}
#endif

#endif
