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

#include <wifi.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <appcore-efl.h>
#include <ui-gadget-module.h>
#include <utilX.h>

#include "common.h"
#include "view-main.h"
#include "i18nmanager.h"
#include "view-alerts.h"
#include "common_utils.h"
#include "wlan_manager.h"
#include "wifi-syspopup.h"
#include "appcoreWrapper.h"
#include "wifi-syspopup-engine-callback.h"

typedef enum {
	SIZE_INDEX_SMALL = 0,
	SIZE_INDEX_NORMAL,
	SIZE_INDEX_LARGE,
	SIZE_INDEX_HUGE,
	SIZE_INDEX_GIANT
} font_size_index;

#define FONT_SIZE_SMALL_GENLIST_H 96
#define FONT_SIZE_NORMAL_GENLIST_H 96
#define FONT_SIZE_LARGE_GENLIST_H 163
#define FONT_SIZE_HUGE_GENLIST_H 182
#define FONT_SIZE_GIANT_GENLIST_H 216
#define FONT_SIZE_SMALL_GRP_TITLE_GENLIST_H 51
#define FONT_SIZE_NORMAL_GRP_TITLE_GENLIST_H 51
#define FONT_SIZE_LARGE_GRP_TITLE_GENLIST_H 95
#define FONT_SIZE_HUGE_GRP_TITLE_GENLIST_H 108
#define FONT_SIZE_GIANT_GRP_TITLE_GENLIST_H 132

wifi_object* devpkr_app_state = NULL;

static void __idle_lock_state_change_cb(keynode_t *node, void *user_data);

gboolean wifi_devpkr_get_scan_status(void)
{
	Evas_Object *btn_scan = NULL;
	Eina_Bool status = EINA_FALSE;
	gboolean ret = FALSE;

	btn_scan = elm_object_part_content_get(devpkr_app_state->popup,
			"button2");
	status = elm_object_disabled_get(btn_scan);

	if (status == EINA_TRUE) {
		ret = TRUE;
	}

	return ret;
}

void wifi_devpkr_enable_scan_btn(void)
{
	Evas_Object *btn_scan = NULL;

	btn_scan = elm_object_part_content_get(devpkr_app_state->popup,
			"button2");
	elm_object_disabled_set(btn_scan, EINA_FALSE);
}

void wifi_devpkr_disable_scan_btn(void)
{
	Evas_Object *btn_scan = NULL;

	btn_scan = elm_object_part_content_get(devpkr_app_state->popup,
			"button2");
	elm_object_disabled_set(btn_scan, EINA_TRUE);
}

static void _scan_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	int scan_result;

	wifi_devpkr_disable_scan_btn();
	view_main_update_group_title(FALSE);

	scan_result = wlan_manager_scan();
	if (scan_result != WLAN_MANAGER_ERR_NONE) {
		wifi_devpkr_enable_scan_btn();
		ERROR_LOG(SP_NAME_ERR, "Scan failed");
	}

	__COMMON_FUNC_EXIT__;
}

static int __wifi_devpkr_calculate_height(int rotate_angle,
		int profile_count)
{
	int height = 0;
	int item_h = 0;
	int grp_title_h = 0;
	int x = 0;
	font_size_index font_index;

	if (profile_count == 0) {
		height = (int)(ELM_SCALE_SIZE(DEVICE_PICKER_EMPTY_POPUP_H));
	}

	vconf_get_int(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &x);
	font_index = x;

	switch (font_index) {
	case SIZE_INDEX_SMALL :
		item_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_SMALL_GENLIST_H));
		grp_title_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_SMALL_GRP_TITLE_GENLIST_H));
		break;
	case SIZE_INDEX_NORMAL :
		item_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_NORMAL_GENLIST_H));
		grp_title_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_NORMAL_GRP_TITLE_GENLIST_H));
		break;
	case SIZE_INDEX_LARGE :
		item_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_LARGE_GENLIST_H));
		grp_title_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_LARGE_GRP_TITLE_GENLIST_H));
		break;
	case SIZE_INDEX_HUGE :
		item_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_HUGE_GENLIST_H));
		grp_title_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_HUGE_GRP_TITLE_GENLIST_H));
		break;
	case SIZE_INDEX_GIANT :
		item_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_GIANT_GENLIST_H));
		grp_title_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_GIANT_GRP_TITLE_GENLIST_H));
		break;
	default:
		item_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_NORMAL_GENLIST_H));
		grp_title_h = (int)(ELM_SCALE_SIZE(FONT_SIZE_NORMAL_GRP_TITLE_GENLIST_H));
	}

	if (profile_count) {
		height = profile_count * item_h;
		height += grp_title_h;
	} else if (wifi_devpkr_get_scan_status() == TRUE) {
		height += grp_title_h;
	}

	if (0 == rotate_angle || 180 == rotate_angle) {
		if (height > DEVICE_PICKER_POPUP_H) {
			height = ELM_SCALE_SIZE(DEVICE_PICKER_POPUP_H);
		}
	} else {
		if (height > DEVICE_PICKER_POPUP_LN_H) {
			height = ELM_SCALE_SIZE(DEVICE_PICKER_POPUP_LN_H);
		}
	}

	return height;
}

static void wifi_devpkr_rotate_cb(void *data, Evas_Object *obj, void *event)
{
	int rotate_angle;
	Evas_Object *box = NULL;
	int profile_count = 0;
	int height = 0;

	if (obj == NULL) {
		return;
	}

	rotate_angle = elm_win_rotation_get(obj);
	box = elm_object_content_get(devpkr_app_state->popup);

	profile_count = view_main_get_profile_count();
	INFO_LOG(SP_NAME_NORMAL, "Profiles count: %d", profile_count);

	height = __wifi_devpkr_calculate_height(rotate_angle, profile_count);

	evas_object_size_hint_min_set(box, -1, height);

	if (0 == rotate_angle || 180 == rotate_angle) {
		common_utils_contents_rotation_adjust(UG_EVENT_ROTATE_PORTRAIT);
	} else {
		common_utils_contents_rotation_adjust(UG_EVENT_ROTATE_LANDSCAPE);
	}

	if (devpkr_app_state->eap_popup) {
		eap_view_rotate_popup(devpkr_app_state->eap_popup, rotate_angle);
	}

	INFO_LOG(SP_NAME_NORMAL, "rotate_angle: %d", rotate_angle);
}

static void wifi_devpkr_set_rotation(Evas_Object *win)
{
	/* TODO: Temporarily disable rotation for device-picker as default width
	 * is quite less for the popup
	 */
	/* int rots[4] = { 0, 90, 180, 270 }; */
	int rots[1] = { 0 };

	if (!elm_win_wm_rotation_supported_get(win)) {
		return;
	}

	/* elm_win_wm_rotation_available_rotations_set(win, rots, 4); */
	elm_win_wm_rotation_available_rotations_set(win, rots, 1);

	/* evas_object_smart_callback_add(win, "wm,rotation,changed",
			wifi_devpkr_rotate_cb, NULL); */
}

static void _exit_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	wifi_devpkr_destroy();

	__COMMON_FUNC_EXIT__;
}

static void __idle_lock_state_change_cb(keynode_t *node, void *user_data)
{
	__COMMON_FUNC_ENTER__;

	wifi_devpkr_destroy();

	__COMMON_FUNC_EXIT__;
}

void wifi_devpkr_redraw(void)
{
	__COMMON_FUNC_ENTER__;

	wifi_devpkr_rotate_cb(NULL, devpkr_app_state->win_main, NULL);

	__COMMON_FUNC_EXIT__;
}

int wifi_devpkr_destroy(void)
{
	if (VCONFKEY_WIFI_QS_WIFI_CONNECTED == devpkr_app_state->connection_result) {
		INFO_LOG(SP_NAME_NORMAL, "Wi-Fi connected");
	} else if (VCONFKEY_WIFI_QS_3G == devpkr_app_state->connection_result) {
		INFO_LOG(SP_NAME_NORMAL, "Cellular connected");
	} else {
		WARN_LOG(SP_NAME_NORMAL, "Result: [%d]",
				devpkr_app_state->connection_result);

		devpkr_app_state->connection_result = VCONFKEY_WIFI_QS_3G;
	}

	common_util_set_system_registry("memory/wifi/wifi_qs_exit",
			devpkr_app_state->connection_result);

	vconf_ignore_key_changed(VCONFKEY_IDLE_LOCK_STATE, __idle_lock_state_change_cb);

	connman_request_scan_mode_set(WIFI_BGSCAN_MODE_EXPONENTIAL);

	elm_exit();

	return 1;
}

static void __keydown_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	Evas_Event_Key_Down *event = event_info;

	if (g_strcmp0(event->keyname, KEY_BACK) == 0) {
		wifi_devpkr_destroy();
	}

	__COMMON_FUNC_EXIT__;
}

static int wifi_devpkr_create(void)
{
	__COMMON_FUNC_ENTER__;

	Evas_Object *btn_scan = NULL;
	Evas_Object *btn_cancel = NULL;

	if (NULL == devpkr_app_state->popup) {
		devpkr_app_state->popup = elm_popup_add(devpkr_app_state->layout_main);
		elm_object_content_set(devpkr_app_state->layout_main, devpkr_app_state->popup);
		assertm_if(NULL == devpkr_app_state->popup, "syspopup is NULL!!");
	}

	evas_object_event_callback_add(devpkr_app_state->popup, EVAS_CALLBACK_KEY_DOWN, __keydown_cb, NULL);

	elm_object_style_set(devpkr_app_state->popup, "default");
	elm_object_part_text_set(devpkr_app_state->popup, "title,text", sc(PACKAGE, I18N_TYPE_Wi_Fi));
	evas_object_size_hint_weight_set(devpkr_app_state->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	btn_cancel = elm_button_add(devpkr_app_state->popup);
	elm_object_style_set(btn_cancel, "popup");
	elm_object_text_set(btn_cancel, sc(PACKAGE, I18N_TYPE_Cancel));
	elm_object_part_content_set(devpkr_app_state->popup, "button1", btn_cancel);
	evas_object_smart_callback_add(btn_cancel, "clicked", _exit_cb, NULL);

	btn_scan = elm_button_add(devpkr_app_state->popup);
	elm_object_style_set(btn_scan, "popup");
	elm_object_text_set(btn_scan, sc(PACKAGE, I18N_TYPE_Scan));
	elm_object_part_content_set(devpkr_app_state->popup, "button2", btn_scan);
	evas_object_smart_callback_add(btn_scan, "clicked", _scan_cb, NULL);

	view_main_create_main_list();

	wifi_devpkr_set_rotation(devpkr_app_state->win_main);
	wifi_devpkr_redraw();

	elm_win_indicator_mode_set(devpkr_app_state->win_main, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(devpkr_app_state->win_main, ELM_WIN_INDICATOR_TRANSPARENT);

	vconf_notify_key_changed(VCONFKEY_IDLE_LOCK_STATE, __idle_lock_state_change_cb, NULL);

	__COMMON_FUNC_EXIT__;
	return 1;
}

static void _bg_scan_status_callback(GDBusConnection *conn,
		const gchar *name, const gchar *path, const gchar *interface,
		const gchar *sig, GVariant *param, gpointer user_data)
{
	__COMMON_FUNC_ENTER__;

	GVariantIter *iter = NULL;
	GVariant *var = NULL;
	gchar *key = NULL;
	gboolean value = FALSE;

	if (NULL == devpkr_app_state) {
		INFO_LOG(SP_NAME_ERR, "devpkr_app_state is NULL!! Is it test mode?");

		__COMMON_FUNC_EXIT__;
		return;
	}

	g_variant_get(param, "(a{sv})", &iter);
	while (g_variant_iter_loop(iter, "{sv}", &key, &var)) {
		if (g_strcmp0(key, "Scanning") == 0) {
			value = g_variant_get_boolean(var);
			if (value) {
				wifi_devpkr_disable_scan_btn();
				view_main_update_group_title(TRUE);
			} else
				wifi_devpkr_enable_scan_btn();

			g_variant_unref(var);
			g_free(key);
			break;
		}
	}

	g_variant_iter_free(iter);

	__COMMON_FUNC_EXIT__;
}

static int wifi_devpkr_init()
{
	__COMMON_FUNC_ENTER__;

	int wlan_ret;
	bool activated = FALSE;

	wlan_manager_create();
	wlan_manager_set_message_callback(wlan_engine_callback);
	wlan_manager_set_refresh_callback(wlan_engine_refresh_callback);
	common_util_subscribe_scanning_signal(_bg_scan_status_callback);

	wlan_ret = wlan_manager_start();
	switch (wlan_ret) {
	case WLAN_MANAGER_ERR_ALREADY_REGISTERED:
		ERROR_LOG(SP_NAME_ERR, "Already registered.");
		/* fall through */
	case WLAN_MANAGER_ERR_NONE:
		wlan_ret = wifi_is_activated(&activated);
		if (WIFI_ERROR_NONE == wlan_ret) {
			INFO_LOG(SP_NAME_NORMAL, "Wi-Fi activated: %d", activated);
		}

		INFO_LOG(SP_NAME_NORMAL, "wlan_manager start complete" );
		break;

	default:
		ERROR_LOG(SP_NAME_ERR, "Failed to start wlan_manager (%d)", wlan_ret);
		break;
	}

	connman_request_scan_mode_set(WIFI_BGSCAN_MODE_PERIODIC);

	__COMMON_FUNC_EXIT__;
	return wlan_ret;
}

#if 0
/* TODO: Check if bundle paramter check is required later:
 * "[Wi-Fi_syspopup wifi_syspopup_supports:support]"
 */
static int devpkr_support_set(const char* support)
{
	__COMMON_FUNC_ENTER__;

	if (NULL == support) {
		__COMMON_FUNC_EXIT__;
		return 0;
	}

	if (g_strcmp0("WIFI_SYSPOPUP_SUPPORT_QUICKPANEL",support) == 0) {
		devpkr_app_state->wifi_devpkr_support =
			WIFI_DEVPKR_SUPPORT_QUICKPANEL;
	} else {
		__COMMON_FUNC_EXIT__;
		return 0;
	}

	__COMMON_FUNC_EXIT__;
	return 1;
}
#endif

static gboolean _power_on_check(void)
{
	int connection_state;

	connection_state = wlan_manager_state_get();
	switch (connection_state) {
	case WLAN_MANAGER_OFF:
		INFO_LOG(SP_NAME_NORMAL, "current state is wifi-off");

		int wlan_ret = wlan_manager_power_on();
		if (wlan_ret == WLAN_MANAGER_ERR_NONE) {
			view_alerts_powering_on_show();

			__COMMON_FUNC_EXIT__;
			return TRUE;
#if defined TIZEN_TETHERING_ENABLE
		} else if (wlan_ret == WLAN_MANAGER_ERR_WIFI_TETHERING_OCCUPIED) {
			__COMMON_FUNC_EXIT__;
			return TRUE;
		} else if (wlan_ret == WLAN_MANAGER_ERR_WIFI_AP_TETHERING_OCCUPIED) {
			__COMMON_FUNC_EXIT__;
			return TRUE;
#endif
		} else {
			__COMMON_FUNC_EXIT__;
			return FALSE;
		}
		break;

	case WLAN_MANAGER_UNCONNECTED:
	case WLAN_MANAGER_CONNECTING:
		__COMMON_FUNC_EXIT__;
		return TRUE;

	case WLAN_MANAGER_CONNECTED:
		ERROR_LOG(SP_NAME_NORMAL, "current state is wifi-connected");

		__COMMON_FUNC_EXIT__;
		return FALSE;

	case WLAN_MANAGER_ERROR:
		ERROR_LOG(SP_NAME_NORMAL, "current state is wifi error");

		__COMMON_FUNC_EXIT__;
		return FALSE;

	default:
		ERROR_LOG(SP_NAME_NORMAL, "current state is wifi etc");

		__COMMON_FUNC_EXIT__;
		return FALSE;
	}

	__COMMON_FUNC_EXIT__;
	return TRUE;
}

static gboolean load_initial_ap_list(void)
{
	__COMMON_FUNC_ENTER__;

	wlan_manager_scanned_profile_refresh();

	__COMMON_FUNC_EXIT__;
	return FALSE;
}

static void app_control(app_control_h request, void *data)
{
	__COMMON_FUNC_ENTER__;

	Evas_Object *win_main = NULL;
	Evas *evas = NULL;
	int pw_lock_state = 0;

	assertm_if(NULL == data, "data param is NULL!!");

	INFO_LOG(SP_NAME_NORMAL, "Creating device-picker popup");

	win_main = appcore_create_win(PACKAGE);
	assertm_if(NULL == win_main, "win_main is NULL!!");
	evas = evas_object_evas_get(win_main);
	assertm_if(NULL == evas, "evas is NULL!!");

	devpkr_app_state = data;
	devpkr_app_state->win_main = win_main;
	devpkr_app_state->evas = evas;

	elm_win_alpha_set(devpkr_app_state->win_main, EINA_TRUE); /* invisible window */
	elm_win_borderless_set(devpkr_app_state->win_main, EINA_TRUE); /* No borders */
	elm_win_conformant_set(devpkr_app_state->win_main, TRUE); /* Popup autoscroll */

	Evas_Object *conformant = elm_conformant_add(devpkr_app_state->win_main);
	elm_win_conformant_set(devpkr_app_state->win_main, EINA_TRUE);
	elm_win_resize_object_add(devpkr_app_state->win_main, conformant);
	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);
	devpkr_app_state->conformant = conformant;

	Evas_Object *layout = elm_layout_add(conformant);
	elm_object_content_set(conformant, layout);
	devpkr_app_state->layout_main = layout;

	/* TODO: Check if the parameter check '-t' is required later */

	devpkr_app_state->devpkr_type = WIFI_DEVPKR_WITH_AP_LIST;

	int wlan_ret = wifi_devpkr_init();
	if (WLAN_MANAGER_ERR_NONE != wlan_ret || _power_on_check() == FALSE) {
		INFO_LOG(SP_NAME_ERR, "failed to wifi_devpkr_init()");
		goto exit;
	}

	devpkr_app_state->popup = elm_popup_add(devpkr_app_state->win_main);

	/* Enablee Changeable UI feature */
	ea_theme_object_changeable_ui_enabled_set(layout, EINA_TRUE);

	devpkr_app_state->color_table = common_utils_color_table_set();
	ea_theme_object_colors_set(layout, devpkr_app_state->color_table,
			EA_THEME_STYLE_DEFAULT);

	devpkr_app_state->font_table = common_utils_font_table_set();
	ea_theme_object_fonts_set(layout, devpkr_app_state->font_table);

	/* TODO: Check if below bundle parameter check is required later:
	 * "[Wi-Fi_syspopup wifi_syspopup_supports:support]" */

	if (vconf_get_int(VCONFKEY_PWLOCK_STATE, &pw_lock_state) != 0) {
		INFO_LOG(SP_NAME_ERR, "failed to get vconfkey : VCONFKEY_PWLOCK_STATE");
		goto exit;
	}

	if (pw_lock_state == VCONFKEY_PWLOCK_BOOTING_LOCK) {
		INFO_LOG(SP_NAME_ERR, "pw_lock state is BOOTING_LOCK");
		goto exit;
	}

	wifi_devpkr_create();

	load_initial_ap_list();

	__COMMON_FUNC_EXIT__;
	return;

exit:
	wifi_devpkr_destroy();
	__COMMON_FUNC_EXIT__;

	return;
}

static bool app_create(void *data)
{
	__COMMON_FUNC_ENTER__;

	elm_app_base_scale_set(1.8);
	bindtextdomain(PACKAGE, LOCALEDIR);

	__COMMON_FUNC_EXIT__;
	return true;
}

static void app_terminate(void *data)
{
	__COMMON_FUNC_ENTER__;

	if (devpkr_app_state->passpopup) {
		passwd_popup_free(devpkr_app_state->passpopup);
		devpkr_app_state->passpopup = NULL;
	}

	if (devpkr_app_state->win_main) {
		evas_object_del(devpkr_app_state->win_main);
		devpkr_app_state->win_main = NULL;
	}

	common_util_managed_ecore_scan_update_timer_del();
	wlan_manager_destroy();

	__COMMON_FUNC_EXIT__;
	return;
}

static void app_pause(void *data)
{
	__COMMON_FUNC_ENTER__;

	connman_request_scan_mode_set(WIFI_BGSCAN_MODE_EXPONENTIAL);

	__COMMON_FUNC_EXIT__;
	return;
}

static void app_resume(void *data)
{
	__COMMON_FUNC_ENTER__;

	connman_request_scan_mode_set(WIFI_BGSCAN_MODE_PERIODIC);

	__COMMON_FUNC_EXIT__;
	return;
}

EXPORT_API int main(int argc, char *argv[])
{
	__COMMON_FUNC_ENTER__;

	INFO_LOG(SP_NAME_NORMAL, "argc [%d]", argc);

	wifi_object ad;
	memset(&ad, 0x0, sizeof(wifi_object));

	ad.connection_result = VCONFKEY_WIFI_QS_3G;
	ad.win_main = NULL;
	ad.evas = NULL;
	ad.popup = NULL;
	ad.passpopup = NULL;
	ad.alertpopup = NULL;

	app_event_callback_s app_callback = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.app_control = app_control,
	};

	return app_efl_main(&argc, &argv, &app_callback, &ad);
}
