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

#include "i18nmanager.h"
#include <libintl.h>
#include <stdio.h>
#include <string.h>

static char* apply_i18n(const char *pkg_name, I18N_MODES mode, const char *string)
{
	switch (mode) {
	case I18N_MODE_SELF_MADE:
		return (char*) dgettext(pkg_name, string);
	case I18N_MODE_NO_NEEDED:
		return (char*) gettext(string);
	default:
		return "(debug)applying_i18n_failed";
	}
}

char* i18n_manager_get_text(const char *pkg_name, I18N_TYPES type)
{
	switch (type) {
	case I18N_TYPE_Ok:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_SK2_OK");
	case I18N_TYPE_Find:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_COM_BODY_FIND");
	case I18N_TYPE_Cancel:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_SK_CANCEL");
	case I18N_TYPE_Wi_Fi:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_WI_FI");
	case I18N_TYPE_Name:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_NAME");
	case I18N_TYPE_Unknown:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_UNKNOWN");
	case I18N_TYPE_Scan:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BUTTON_SCAN");
	case I18N_TYPE_Next:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_NEXT");
	case I18N_TYPE_Prev:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_SK_PREVIOUS");
	case I18N_TYPE_On:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_OPT_MOBILEACCESSSERVICEATCIVATION_ON");
	case I18N_TYPE_Off:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_OPT_MOBILEACCESSSERVICEATCIVATION_OFF");
	case I18N_TYPE_Maximum_Number_Of_Characters_PD_Reached:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_TPOP_MAXIMUM_NUMBER_OF_CHARACTERS_PD_REACHED");
	case I18N_TYPE_Connected_To_Wi_Fi_Network:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_TPOP_CONNECTED_TO_WI_FI_NETWORK_PS");
	case I18N_TYPE_Wrong_Password:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_POP_WRONG_PASSWORD");
	case I18N_TYPE_No_Wi_Fi_AP_Found:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_POP_NO_WI_FI_AP_FOUND");
	case I18N_TYPE_Available_networks:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_HEADER_AVAILABLE_NETWORKS");
	case I18N_TYPE_Open:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_POP_OPEN");
	case I18N_TYPE_Secured:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_SECURED_ABB_M_WIFI_AP_SUMMARY");
	case I18N_TYPE_WPS_Available:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_WPS_AVAILABLE");
	case I18N_TYPE_EAP:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_EAP");
	case I18N_TYPE_Saved:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_CONFIGURATION_SAVED_M_STATUS");
	case I18N_TYPE_Connecting:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_CONNECTING_ING");
	case I18N_TYPE_Obtaining_IP_addr:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_OBTAINING_IP_ADDRESS_ING");
	case I18N_TYPE_Connected:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_SBODY_CONNECTED_M_STATUS");
	case I18N_TYPE_Find_Hidden_Network:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BUTTON_FIND_HIDDEN_NETWORK");
	case I18N_TYPE_Password:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_HEADER_PASSWORD");
	case I18N_TYPE_Show_password:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_OPT_SHOW_PASSWORD");
	case I18N_TYPE_Wi_Fi_network_info:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_HEADER_WI_FI_NETWORK_INFO_ABB");
	case I18N_TYPE_Connect:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_CONNECT");
	case I18N_TYPE_WPS:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_OPT_WPS");
	case I18N_TYPE_WPS_Button:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BUTTON_WPS_BUTTON");
	case I18N_TYPE_WPS_PIN:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_WPS_PIN");
	case I18N_TYPE_Press_WPS_On_Your_Wi_Fi_Access_Point:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_POP_PRESS_WPS_ON_YOUR_WI_FI_ACCESS_POINT_WITHIN_PD_MINUTES");
	case I18N_TYPE_Enter_PIN_number_on_your_WIFI_access_point:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_ENTER_THE_P1SS_PIN_ON_YOUR_WI_FI_ROUTER_THE_SETUP_CAN_TAKE_UP_TO_P2SD_MINUTES_TO_COMPLETE");
	case I18N_TYPE_Static_IP:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_STATIC_IP");
	case I18N_TYPE_IP_address:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_IP_ADDRESS");
	case I18N_TYPE_MAC_addr:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_MAC_ADDRESS");
	case I18N_TYPE_Proxy_address:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_HEADER_PROXY_ADDRESS");
	case I18N_TYPE_Proxy_port:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_PROXY_PORT");
	case I18N_TYPE_Subnet_mask:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_SUBNET_MASK");
	case I18N_TYPE_Gateway_address:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_GATEWAY_ADDRESS");
	case I18N_TYPE_DNS_1:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_DNS_1");
	case I18N_TYPE_DNS_2:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_DNS_2");
	case I18N_TYPE_Forget:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_SK_FORGET");
	case I18N_TYPE_Forget_Network:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_OPT_FORGET_NETWORK");
	case I18N_TYPE_EAP_method:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_EAP_METHOD");
	case I18N_TYPE_Phase_2_authentication:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_COM_BODY_PHASE_2_AUTHENTICATION");
	case I18N_TYPE_User_Certificate:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_USER_CERTIFICATE_ABB");
	case I18N_TYPE_Unspecified:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_UNSPECIFIED");
	case I18N_TYPE_Identity:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_IDENTITY");
	case I18N_TYPE_Enter_Identity:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_ENTER_IDENTITY");
	case I18N_TYPE_Enter_password:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_HEADER_ENTER_PASSWORD");
	case I18N_TYPE_Ssid:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_SSID");
	case I18N_TYPE_A_Wi_Fi_Network_Has_Been_Detected:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_A_WI_FI_NETWORK_HAS_BEEN_DETECTED_YOU_WILL_BE_CONNECTED");
	case I18N_TYPE_Advanced_setting:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_ADVANCED_SETTINGS");
	case I18N_TYPE_Network_notification:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_NETWORK_NOTIFICATION");
	case I18N_TYPE_Network_notify_me_later:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_RECEIVE_NOTIFICATIONS_WHEN_NETWORKS_ARE_AVAILABLE");
	case I18N_TYPE_Keep_WIFI_on_during_sleep:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_KEEP_WI_FI_ON_DURING_SLEEP");
	case I18N_TYPE_Always:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_CLD_OPT_ALWAYS_ABB");
	case I18N_TYPE_Plugged:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_WHILE_CHARGING_ONLY_ABB");
	case I18N_TYPE_Donot_Use:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_TMBODY_NEVER_M_ALWAYS_OFF");
	case I18N_TYPE_Increases_Data_Usage:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_SBODY_INCREASES_DATA_USAGE_ABB");
	case I18N_TYPE_Sort_by:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_SORT_BY");
	case I18N_TYPE_Alphabetical:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_EMAIL_POP_ALPHABETICAL");
	case I18N_TYPE_Signal_Strength:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_MBODY_SIGNAL_STRENGTH_KOR_SKT");
	case I18N_TYPE_Advanced:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_ADVANCED");
	case I18N_TYPE_Current_Network_Will_Be_Disconnected:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_POP_CURRENT_NETWORK_WILL_BE_DISCONNECTED");
	case I18N_TYPE_Select_WPS_Method:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_HEADER_SELECT_WPS_METHOD_ABB");

	case I18N_TYPE_Invalid_pin:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_GC_POP_INVALID_PIN");
	case I18N_TYPE_Invalid_certificate:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_BR_POP_INVALID_CERTIFICATE");
	case I18N_TYPE_Skip:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_CAST_SK4_SKIP");
	case I18N_TYPE_WiFi_network_detected_connect:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_WI_FI_NETWORK_DETECTED_CONNECT_Q");

		/* TTS */
	case I18N_TYPE_Button:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_BR_BODY_BUTTON_T_TTS");
	case I18N_TYPE_Double_tap:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_HEADER_DOUBLE_TAP");
	case I18N_TYPE_Connect_to_device:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_BT_BODY_CONNECT_TO_DEVICE");
	case I18N_TYPE_Excellent:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_EXCELLENT");
	case I18N_TYPE_Good:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_GOOD_M_BATTERY");
	case I18N_TYPE_Weak:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_WEAK_M_STRENGTH");
	case I18N_TYPE_Activating:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_BODY_TURNING_ON_ING");
	case I18N_TYPE_WiFi_network_will_disable_tethering:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_ST_POP_TURNING_ON_WI_FI_WILL_DISABLE_WI_FI_TETHERING");
	case I18N_TYPE_Scanning:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_WIFI_BODY_SCANNING_ING");


	case I18N_TYPE_Help_tap_the_network_to_connect:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_HELP_BODY_TAP_THE_NETWORK_YOU_WANT_TO_CONNECT_TO");
	case I18N_TYPE_Help_automatically_connected:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_HELP_BODY_YOU_HAVE_BEEN_AUTOMATICALLY_CONNECTED_TO_THE_REMEMBERED_NETWORK_TO_SEE_THE_NETWORK_DETAILS_TAP_THE_PS_ICON");
	case I18N_TYPE_Help_successfully_connected:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_HELP_BODY_YOU_HAVE_BEEN_CONNECTED_TO_THE_NETWORK_TO_SEE_THE_NETWORK_DETAILS_TAP_THE_PS_ICON");
	case I18N_TYPE_Help_connect_to_secured_network:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_HELP_POP_TO_CONNECT_TO_A_SECURED_NETWORK_YOU_MAY_NEED_TO_ENTER_A_PASSWORD_OR_OTHER_CREDENTIALS");
	case I18N_TYPE_Help_tap_to_scan:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_HELP_POP_TAP_TO_SCAN_FOR_WI_FI_NETWORKS");
	case I18N_TYPE_Help_no_wifi_networks:
		return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_HELP_POP_NO_WI_FI_NETWORKS_FOUND_CHANGE_YOUR_LOCATION_OR_TRY_LATER");
	case I18N_TYPE_Help_invalid_action:
			return apply_i18n(pkg_name, I18N_MODE_SELF_MADE, "IDS_HELP_POP_INVALID_ACTION_TRY_AGAIN");

	default:
		return apply_i18n(pkg_name, I18N_MODE_NO_NEEDED, "(debugapplying_i18n_failed");
	}
}
