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

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#include <vconf-keys.h>
//#include <setting-cfg.h>
#include "ug_wifi.h"
#include "view_detail.h"
#include "i18nmanager.h"
#include "wlan_manager.h"
#include "winset_popup.h"
#include "common_utils.h"
#include "viewer_manager.h"
#include "view_ime_hidden.h"
#include "view_advanced.h"
#include "wifi-engine-callback.h"

static int wifi_exit_end = FALSE;
static bool is_scan_reqd = false;
wifi_appdata *ug_app_state = NULL;

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops);
UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops);
UG_MODULE_API int setting_plugin_reset(bundle *data, void *priv);
UG_MODULE_API int setting_plugin_search_init(app_control_h app_control, void *priv, char **domainname);

static gboolean __wifi_efl_ug_del_found_ap_noti(void *data)
{
	common_utils_send_message_to_net_popup(NULL, NULL,
			"del_found_ap_noti", NULL);

	return FALSE;
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

	int header_mode = viewer_manager_header_mode_get();
	viewer_manager_create_scan_btn();
	if (header_mode == HEADER_MODE_DEACTIVATING ||
			header_mode == HEADER_MODE_OFF) {
		__COMMON_FUNC_EXIT__;
		return;
	}

	g_variant_get(param, "(a{sv})", &iter);
	while (g_variant_iter_loop(iter, "{sv}", &key, &var)) {
		if (g_strcmp0(key, "Scanning") == 0) {
			value = g_variant_get_boolean(var);
			if (value) {
				if (header_mode != HEADER_MODE_CONNECTING) {
					viewer_manager_show(VIEWER_WINSET_SEARCHING_GRP_TITLE);
					viewer_manager_header_mode_set(HEADER_MODE_SEARCHING);
				}
			} else
				viewer_manager_header_mode_set(HEADER_MODE_ON);

			g_variant_unref(var);
			g_free(key);
			break;
		}
	}

	g_variant_iter_free(iter);

	__COMMON_FUNC_EXIT__;
}

static void *on_create(ui_gadget_h ug, enum ug_mode mode,
		app_control_h app_control, void *priv)
{
	__COMMON_FUNC_ENTER__;

	const char *uri = NULL;
	int state;
	char *viewtype = NULL;

	if (!ug || !priv) {
		INFO_LOG(UG_NAME_ERR, "UG and PRIV should not be NULL");

		__COMMON_FUNC_EXIT__;
		return NULL;
	}

	ug_app_state = g_new0(wifi_appdata, 1);
	retvm_if(NULL == ug_app_state, NULL);

	struct ug_data *ugd;
	ugd = (struct ug_data*)priv;
	ugd->ug = ug;

	if (NULL != app_control) {
		INFO_LOG(UG_NAME_NORMAL, "message load from caller");

		char *caller = NULL;
		app_control_get_extra_data(app_control, UG_CALLER, &caller);
		app_control_get_extra_data(app_control, UG_VIEWTYPE, &viewtype);

		if (app_control_get_uri(app_control, (char **)&uri) < 0)
			ERROR_LOG(UG_NAME_NORMAL, "Failed to get app_control URI");

		if (uri)
			free((void *)uri);

		if (caller != NULL) {
			SECURE_INFO_LOG(UG_NAME_NORMAL, "caller: %s", caller);

			if (strcmp(caller, "pwlock") == 0) {
				ug_app_state->ug_type = UG_VIEW_SETUP_WIZARD;

				ug_app_state->rbutton_setup_wizard_next = g_strdup(sc(PACKAGE,
					I18N_TYPE_Next));
				ug_app_state->rbutton_setup_wizard_skip = g_strdup(sc(PACKAGE,
					I18N_TYPE_Skip));
				ug_app_state->lbutton_setup_wizard_prev = g_strdup(sc(PACKAGE,
					I18N_TYPE_Prev));
			} else if (strcmp(caller, "notification") == 0){
				/* Remove the "WiFi networks found" from the notification tray.*/
				common_util_managed_idle_add(__wifi_efl_ug_del_found_ap_noti, NULL);
				ug_app_state->ug_type = UG_VIEW_DEFAULT;
			} else {
				ug_app_state->ug_type = UG_VIEW_DEFAULT;
			}

			free(caller);
		} else {
			INFO_LOG(UG_NAME_NORMAL, "caller is not defined");
			ug_app_state->ug_type = UG_VIEW_DEFAULT;
		}
	} else {
		INFO_LOG(UG_NAME_NORMAL, "caller is not defined");
		ug_app_state->ug_type = UG_VIEW_DEFAULT;
	}

	Evas_Object *parent_layout = ug_get_parent_layout(ug);
	if (parent_layout == NULL) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed to get parent layout");

		__COMMON_FUNC_EXIT__;
		return NULL;
	}

	ugd->win_main = ug_get_window();
	ug_app_state->gadget= ugd;
	ug_app_state->ug = ug;

	common_util_set_system_registry(VCONFKEY_WIFI_UG_RUN_STATE,
			VCONFKEY_WIFI_UG_RUN_STATE_ON_FOREGROUND);

	Evas_Object *layout_main = viewer_manager_create(parent_layout, ugd->win_main);
	if (layout_main == NULL) {
		INFO_LOG(UG_NAME_ERR, "Failed to create viewer_manager");

		__COMMON_FUNC_EXIT__;
		return NULL;
	}

	/* Enablee Changeable UI feature */
	ea_theme_object_changeable_ui_enabled_set(layout_main, EINA_TRUE);

	ug_app_state->color_table = common_utils_color_table_set();
	ea_theme_object_colors_set(layout_main, ug_app_state->color_table,
			EA_THEME_STYLE_DEFAULT);

	ug_app_state->font_table = common_utils_font_table_set();
	ea_theme_object_fonts_set(layout_main, ug_app_state->font_table);

#if defined TIZEN_TETHERING_ENABLE
	ug_app_state->popup_manager = winset_popup_manager_create(layout_main, PACKAGE);
#endif

	ugd->base = layout_main;
	ug_app_state->layout_main = layout_main;
	ug_app_state->bAlive = EINA_TRUE;

	wlan_manager_create();
	wlan_manager_set_message_callback(wlan_engine_callback);
	wlan_manager_set_refresh_callback(wlan_engine_refresh_callback);
	common_util_subscribe_scanning_signal(_bg_scan_status_callback);

	switch (wlan_manager_start()) {
	case WLAN_MANAGER_ERR_NONE:
		break;

	case WLAN_MANAGER_ERR_ALREADY_REGISTERED:
		break;

	default:
		ERROR_LOG(UG_NAME_NORMAL, "Cannot start wlan_manager");

		__COMMON_FUNC_EXIT__;
		return ugd->base;
	}

	state = wlan_manager_state_get();
	switch (state) {
	case WLAN_MANAGER_OFF:
		if (ug_app_state->ug_type == UG_VIEW_SETUP_WIZARD) {
			viewer_manager_header_mode_set(HEADER_MODE_ACTIVATING);
			power_control();
		} else {
				viewer_manager_hide(VIEWER_WINSET_SUB_CONTENTS);
				viewer_manager_header_mode_set(HEADER_MODE_OFF);
			}
		break;

	case WLAN_MANAGER_CONNECTING:
	case WLAN_MANAGER_UNCONNECTED:
	case WLAN_MANAGER_CONNECTED:
		viewer_manager_hide(VIEWER_WINSET_SEARCHING);
		viewer_manager_show(VIEWER_WINSET_SUB_CONTENTS);
		viewer_manager_header_mode_set(HEADER_MODE_SEARCHING);
		break;

	case WLAN_MANAGER_ERROR:
	default:
		return ugd->base;
	}

	evas_object_show(layout_main);

	if (viewtype != NULL) {
		if (strcmp(viewtype, "advancedsetting") == 0)
			view_advanced();
		g_free(viewtype);
	}

	__COMMON_FUNC_EXIT__;
	return ugd->base;
}

static gboolean load_initial_ap_list(gpointer data)
{
	__COMMON_FUNC_ENTER__;

	/* Because of transition effect performance,
	 * Wi-Fi lists might be better to be updated at maximum delayed
	 */
	wlan_manager_scanned_profile_refresh();

	__COMMON_FUNC_EXIT__;
	return FALSE;
}

static void on_start(ui_gadget_h ug, app_control_h app_control, void *priv)
{
	__COMMON_FUNC_ENTER__;

	common_util_managed_idle_add(load_initial_ap_list, NULL);

	connman_request_scan_mode_set(WIFI_BGSCAN_MODE_PERIODIC);

	__COMMON_FUNC_EXIT__;
}

static void on_pause(ui_gadget_h ug, app_control_h app_control, void *priv)
{
	__COMMON_FUNC_ENTER__;

	INFO_LOG(UG_NAME_NORMAL, "Wi-Fi UG paused");

	connman_request_scan_mode_set(WIFI_BGSCAN_MODE_EXPONENTIAL);

	__COMMON_FUNC_EXIT__;
}

static void on_resume(ui_gadget_h ug, app_control_h app_control, void *priv)
{
	__COMMON_FUNC_ENTER__;

	INFO_LOG(UG_NAME_NORMAL, "Wi-Fi UG resumed");

	connman_request_scan_mode_set(WIFI_BGSCAN_MODE_PERIODIC);

	common_util_set_system_registry(VCONFKEY_WIFI_UG_RUN_STATE,
			VCONFKEY_WIFI_UG_RUN_STATE_ON_FOREGROUND);

	view_manager_view_type_t top_viewID = viewer_manager_view_type_get();
	if (top_viewID == VIEW_MANAGER_VIEW_TYPE_MAIN) {
		viewer_manager_request_scan();
		is_scan_reqd = false;
	} else {
		is_scan_reqd = true;
	}

	__COMMON_FUNC_EXIT__;
}

static void on_destroy(ui_gadget_h ug, app_control_h app_control, void *priv)
{
	__COMMON_FUNC_ENTER__;

	int ret;
	common_util_set_system_registry(VCONFKEY_WIFI_UG_RUN_STATE,
			VCONFKEY_WIFI_UG_RUN_STATE_OFF);

	if (!ug || !priv){
		__COMMON_FUNC_EXIT__;
		return;
	}

	/*Added to handle incase of force closure*/
	passwd_popup_free(ug_app_state->passpopup);
	ug_app_state->passpopup = NULL;

#if defined TIZEN_TETHERING_ENABLE
	winset_popup_manager_destroy(ug_app_state->popup_manager);
	ug_app_state->popup_manager = NULL;
	DEBUG_LOG(UG_NAME_NORMAL, "* popup manager destroy complete");
#endif

	if (wifi_exit_end == FALSE) {
		connman_request_scan_mode_set(WIFI_BGSCAN_MODE_EXPONENTIAL);

		common_util_managed_idle_cleanup();
		common_util_managed_ecore_scan_update_timer_del();

		ret = wlan_manager_destroy();
		if (ret != WLAN_MANAGER_ERR_NONE) {
			ERROR_LOG(UG_NAME_NORMAL, "Failed to destroy wlan manager: %d",ret);
		} else {
			DEBUG_LOG(UG_NAME_NORMAL, "* wlan manager destroy complete");
		}
	}

	DEBUG_LOG(UG_NAME_NORMAL, "* viewer manager destroy complete");

	struct ug_data* ugd = priv;
	if (ugd->base){
		evas_object_del(ugd->base);
		ugd->base = NULL;
	}

	if (NULL != ug_app_state->rbutton_setup_wizard_next) {
		g_free(ug_app_state->rbutton_setup_wizard_next);
	}

	if (NULL != ug_app_state->rbutton_setup_wizard_skip) {
		g_free(ug_app_state->rbutton_setup_wizard_skip);
	}

	if (NULL != ug_app_state->lbutton_setup_wizard_prev) {
		g_free(ug_app_state->lbutton_setup_wizard_prev);
	}

	__COMMON_FUNC_EXIT__;
}

static void on_message(ui_gadget_h ug, app_control_h msg, app_control_h app_control, void *priv)
{
}

static void _language_changed(void)
{

	__COMMON_FUNC_ENTER__;

	if (NULL != ug_app_state->rbutton_setup_wizard_next) {
		g_free(ug_app_state->rbutton_setup_wizard_next);
		ug_app_state->rbutton_setup_wizard_next = NULL;
	}

	if (NULL != ug_app_state->rbutton_setup_wizard_skip) {
		g_free(ug_app_state->rbutton_setup_wizard_skip);
		ug_app_state->rbutton_setup_wizard_skip = NULL;
	}

	if (NULL != ug_app_state->lbutton_setup_wizard_prev) {
		g_free(ug_app_state->lbutton_setup_wizard_prev);
		ug_app_state->lbutton_setup_wizard_prev = NULL;
	}

	ug_app_state->rbutton_setup_wizard_next = g_strdup(sc(PACKAGE, I18N_TYPE_Next));
	ug_app_state->rbutton_setup_wizard_skip = g_strdup(sc(PACKAGE, I18N_TYPE_Skip));
	ug_app_state->lbutton_setup_wizard_prev = g_strdup(sc(PACKAGE, I18N_TYPE_Prev));

	viewer_manager_setup_wizard_button_controller();

	language_changed_refresh();

	__COMMON_FUNC_EXIT__;
}

static void on_event(ui_gadget_h ug, enum ug_event event, app_control_h app_control, void *priv)
{
	__COMMON_FUNC_ENTER__;

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		break;
	case UG_EVENT_LOW_BATTERY:
		break;
	case UG_EVENT_LANG_CHANGE:
		INFO_LOG(UG_NAME_NORMAL, "LANGUAGE");
		if (UG_VIEW_SETUP_WIZARD == ug_app_state->ug_type)
			_language_changed();
		break;
	case UG_EVENT_ROTATE_PORTRAIT:
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		INFO_LOG(UG_NAME_NORMAL, "PORTRAIT");
		viewer_manager_rotate_top_setupwizard_layout();
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		INFO_LOG(UG_NAME_NORMAL, "LANSCAPE");
		viewer_manager_rotate_top_setupwizard_layout();
		break;
	default:
		break;
	}

	__COMMON_FUNC_EXIT__;
}

static void on_key_event(ui_gadget_h ug, enum ug_key_event event, app_control_h app_control, void *priv)
{
	__COMMON_FUNC_ENTER__;

	if (!ug) {
		__COMMON_FUNC_EXIT__;
		return;
	}

	switch (event) {
	case UG_KEY_EVENT_END:
		INFO_LOG(UG_NAME_NORMAL, "UG_KEY_EVENT_END");

#if defined TIZEN_TETHERING_ENABLE
		/* popup key event determine */
		winset_popup_hide_popup(ug_app_state->popup_manager);
#endif
		view_manager_view_type_t top_view_id = viewer_manager_view_type_get();
		if (top_view_id == VIEW_MANAGER_VIEW_TYPE_MAIN) {
			INFO_LOG(UG_NAME_NORMAL, "same");
		} else {
			INFO_LOG(UG_NAME_NORMAL, "differ");
			elm_naviframe_item_pop(viewer_manager_get_naviframe());

			__COMMON_FUNC_EXIT__;
			return;
		}

		wifi_exit();
		break;
	default:
		INFO_LOG(UG_NAME_NORMAL, "UG_KEY_EVENT [%d]", event);
		break;
	}

	__COMMON_FUNC_EXIT__;
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	__COMMON_FUNC_ENTER__;

	wifi_exit_end = FALSE;

	retvm_if(NULL == ops, 0);

	struct ug_data *ugd;
	ugd = calloc(1, sizeof(struct ug_data));

	assertm_if(NULL == ugd, "Err!! calloc fail");

	ops->create = on_create;
	ops->start = on_start;
	ops->pause = on_pause;
	ops->resume = on_resume;
	ops->destroy = on_destroy;
	ops->message = on_message;
	ops->event = on_event;
	ops->key_event = on_key_event;
	ops->priv = ugd;
	ops->opt = UG_OPT_INDICATOR_ENABLE;

	__COMMON_FUNC_EXIT__;
	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	__COMMON_FUNC_ENTER__;

	retm_if(NULL == ops);

	struct ug_data *ugd;

	ugd = ops->priv;

	if (ugd) {
		free(ugd);
	}

	__COMMON_FUNC_EXIT__;
}

static bool setting_plugin_wifi_found_ap_cb(wifi_ap_h ap, void* user_data)
{
	bool favorite = false;

	wifi_ap_is_favorite(ap, &favorite);

	if (true == favorite) {
		wlan_manager_forget(ap);
	}

	return true;
}

UG_MODULE_API int setting_plugin_reset(bundle *data, void *priv)
{
	__COMMON_FUNC_ENTER__;

	int return_value = 0;

	return_value = wlan_manager_start();
	if (return_value != WLAN_MANAGER_ERR_NONE) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed to register : %d",return_value);
		return_value = -1;
		goto error;
	}

	wifi_foreach_found_aps(setting_plugin_wifi_found_ap_cb, NULL);
	return_value = wlan_manager_power_off();
	if (return_value != WLAN_MANAGER_ERR_NONE) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed to power_off: %d",return_value);
		return_value = -1;
		goto error;
	}

	common_util_set_system_registry(VCONFKEY_WIFI_ENABLE_QS,
									VCONFKEY_WIFI_QS_ENABLE);

error:
	wlan_manager_destroy();

	__COMMON_FUNC_EXIT__;
	return return_value;
}

int wifi_exit(void)
{
	__COMMON_FUNC_ENTER__;

	if (wifi_exit_end == TRUE) {
		__COMMON_FUNC_EXIT__;
		return FALSE;
	}
	wifi_exit_end = TRUE;

	int ret = WLAN_MANAGER_ERR_NONE;
	struct ug_data *ugd;
	ugd = ug_app_state->gadget;
	ug_app_state->bAlive = EINA_FALSE;

	connman_request_scan_mode_set(WIFI_BGSCAN_MODE_EXPONENTIAL);

	common_util_managed_idle_cleanup();
	common_util_managed_ecore_scan_update_timer_del();

	ret = wlan_manager_destroy();
	if (ret != WLAN_MANAGER_ERR_NONE) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed to destroy wlan manager: %d",ret);
	} else {
		DEBUG_LOG(UG_NAME_NORMAL, "* wlan manager destroy complete");
	}

	DEBUG_LOG(UG_NAME_NORMAL, "* ug_destroying...");
	ug_destroy_me(ugd->ug);

	__COMMON_FUNC_EXIT__;
	return TRUE;
}

bool wifi_is_scan_required(void)
{
	return is_scan_reqd;
}
#if 0
UG_MODULE_API int setting_plugin_search_init(app_control_h app_control, void *priv, char **domainname)
{
	void *node = NULL;
	*domainname = strdup(PACKAGE);
	Eina_List **pplist = (Eina_List **)priv;

	node = setting_plugin_search_item_add("IDS_ST_BODY_NETWORK_NOTIFICATION",
		"viewtype:advancedsetting", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);
	node = setting_plugin_search_item_add("IDS_WIFI_TMBODY_SMART_NETWORK_SWITCH",
		"viewtype:mainview", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);
	node = setting_plugin_search_item_add("IDS_WIFI_HEADER_PASSPOINT",
		"viewtype:advancedsetting", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);
	node = setting_plugin_search_item_add("IDS_ST_BODY_KEEP_WI_FI_ON_DURING_SLEEP",
		"viewtype:advancedsetting", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);
	node = setting_plugin_search_item_add("IDS_ST_MBODY_ALWAYS_ALLOW_SCANNING",
		"viewtype:advancedsetting", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);
	node = setting_plugin_search_item_add("IDS_WIFI_BODY_ADVANCED_SETTINGS",
		"viewtype:advancedsetting", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);

	return 0;
}
#endif
