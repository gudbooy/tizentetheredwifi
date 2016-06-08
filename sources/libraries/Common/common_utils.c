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
#include <aul.h>
#include <ui-gadget-module.h>

#include "common.h"
#include "common_utils.h"
#include "i18nmanager.h"

#define SUPPLICANT_SERVICE				"fi.w1.wpa_supplicant1"
#define SUPPLICANT_SERVICE_INTERFACE	SUPPLICANT_SERVICE ".Interface"
#define COLOR_TABLE "/usr/apps/wifi-efl-ug/shared/res/tables/ug-wifi-efl_ChangeableColorTable.xml"
#define FONT_TABLE "/usr/apps/wifi-efl-ug/shared/res/tables/ug-wifi-efl_FontInfoTable.xml"

typedef struct {
	char *title_str;
	char *info_str;
} two_line_disp_data_t;

struct managed_idle_data {
	GSourceFunc func;
	gpointer user_data;
	guint id;
};

struct gdbus_connection_data {
	GDBusConnection *connection;
	guint subscribe_id_supplicant;
};

static __thread struct gdbus_connection_data gdbus_conn = { NULL, 0 };
static GSList *managed_idler_list = NULL;
static int (*rotate_cb)(enum appcore_rm, void*, Eina_Bool, Eina_Bool) = NULL;
static void *rotate_cb_data = NULL;
static Eina_Bool is_wps = EINA_FALSE;
static Eina_Bool is_setting = EINA_FALSE;
static Eina_Bool is_portrait_mode = EINA_TRUE;
static Ecore_Timer *scan_update_timer = NULL;

Ea_Theme_Color_Table *common_utils_color_table_set(void)
{
	Ea_Theme_Color_Table *table;

	table = ea_theme_color_table_new(COLOR_TABLE);
	INFO_LOG(UG_NAME_NORMAL, "Wi-Fi color table : %p", table);
	ea_theme_colors_set(table, EA_THEME_STYLE_DEFAULT);

	return table;
}

Ea_Theme_Font_Table *common_utils_font_table_set(void)
{
	Ea_Theme_Font_Table *table;

	table = ea_theme_color_table_new(FONT_TABLE);
	INFO_LOG(UG_NAME_NORMAL, "Wi-Fi font table : %p", table);

	return table;
}

static char *__common_utils_2line_text_get(void *data, Evas_Object *obj, const char *part)
{
	two_line_disp_data_t *item_data = (two_line_disp_data_t *)data;
	if (!strcmp(part, "elm.text.sub.left.bottom")) {
		return g_strdup(item_data->info_str);
	} else if (!strcmp(part, "elm.text.main.left.top")) {
		return g_strdup(item_data->title_str);
	}
	return NULL;
}

static void __common_utils_2line_text_del(void *data, Evas_Object *obj)
{
	two_line_disp_data_t *item_data = (two_line_disp_data_t *)data;
	if (item_data) {
		g_free(item_data->info_str);
		g_free(item_data->title_str);
		g_free(item_data);
	}
}

static void __common_utils_separator_del(void *data, Evas_Object *obj)
{
	elm_genlist_item_class_free(data);
	return;
}

Eina_Bool common_utils_is_portrait_mode(void)
{
	return is_portrait_mode;
}

static void __common_utils_set_portrait_mode(Eina_Bool on)
{
	is_portrait_mode = on;
}

void common_utils_set_rotate_cb(int (*func)(enum appcore_rm, void*, Eina_Bool, Eina_Bool),
		void *data, Eina_Bool wps_value, Eina_Bool setting_value)
{
	rotate_cb = func;
	rotate_cb_data = data;
	is_wps = wps_value;
	is_setting = setting_value;
}

static void __common_utils_rotate_popup(enum appcore_rm rotmode)
{
	if (rotate_cb) {
		rotate_cb(rotmode, rotate_cb_data, is_wps, is_setting);
	}
}

void common_utils_contents_rotation_adjust(int event)
{
	if (event == UG_EVENT_ROTATE_PORTRAIT ||
			event == UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN) {
		__common_utils_rotate_popup(APPCORE_RM_PORTRAIT_NORMAL);
		__common_utils_set_portrait_mode(EINA_TRUE);
	} else {
		__common_utils_rotate_popup(APPCORE_RM_LANDSCAPE_NORMAL);
		__common_utils_set_portrait_mode(EINA_FALSE);
	}
}

Elm_Object_Item* common_utils_add_dialogue_separator(Evas_Object* genlist, const char *separator_style)
{
	assertm_if(NULL == genlist, "NULL!!");

	static Elm_Genlist_Item_Class *separator_itc;
	separator_itc = elm_genlist_item_class_new();
	separator_itc->item_style = separator_style;
	separator_itc->func.text_get = NULL;
	separator_itc->func.content_get = NULL;
	separator_itc->func.state_get = NULL;
	separator_itc->func.del = __common_utils_separator_del;

	Elm_Object_Item* sep = elm_genlist_item_append(
					genlist,
					separator_itc,
					NULL,
					NULL,
					ELM_GENLIST_ITEM_GROUP,
					NULL,
					NULL);

	assertm_if(NULL == sep, "NULL!!");

	elm_genlist_item_select_mode_set(sep, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	return sep;
}

char *common_utils_get_ap_security_type_info_txt(const char *pkg_name,
	wifi_device_info_t *device_info, bool check_fav)
{
	bool favorite = false;
	char *status_txt = NULL;
	switch (device_info->security_mode)
	{
		case WLAN_SEC_MODE_NONE:		/** Security disabled */
			status_txt = g_strdup(sc(pkg_name, I18N_TYPE_Open));
			break;
		case WLAN_SEC_MODE_IEEE8021X:	/** EAP */
			status_txt = g_strdup_printf("%s (%s)", sc(pkg_name, I18N_TYPE_Secured),
				sc(pkg_name, I18N_TYPE_EAP));
			break;
		case WLAN_SEC_MODE_WEP:			/** WEP */
		case WLAN_SEC_MODE_WPA_PSK:		/** WPA-PSK */
		case WLAN_SEC_MODE_WPA2_PSK:	/** WPA2-PSK */
			if (TRUE == device_info->wps_mode) {
				status_txt = g_strdup_printf("%s (%s)", sc(pkg_name,
					I18N_TYPE_Secured), sc(pkg_name, I18N_TYPE_WPS_Available));
			} else {
				status_txt = g_strdup(sc(pkg_name, I18N_TYPE_Secured));
			}
			break;
		default:						/** Unknown */
			status_txt = g_strdup(sc(pkg_name, I18N_TYPE_Unknown));
			break;
	}

	if (true == check_fav) {
		wifi_ap_is_favorite(device_info->ap, &favorite);
		if (favorite == true) {
			if(status_txt != NULL) {
				g_free(status_txt);
				status_txt = NULL;
			}
			status_txt = g_strdup_printf("%s, %s", sc(pkg_name, I18N_TYPE_Saved),
				status_txt);
		}
	}
	return status_txt;
}

void common_utils_get_wps_icon(char **icon_path, gboolean pressed)
{
	char buf[MAX_DEVICE_ICON_PATH_STR_LEN] = {'\0', };

	g_strlcat(buf, "A01-3_icon_WPS_support_AP.png", sizeof(buf));

	if (icon_path) {
		*icon_path = g_strdup_printf("%s", buf);
	}
}

void common_utils_get_device_icon(wifi_device_info_t *device_info, char **icon_path)
{
	char buf[MAX_DEVICE_ICON_PATH_STR_LEN] = {'\0', };

	g_strlcat(buf, "A01-3_icon", sizeof(buf));

	if (device_info->security_mode != WLAN_SEC_MODE_NONE) {
		g_strlcat(buf, "_lock", sizeof(buf));
	}

	switch (wlan_manager_get_signal_strength(device_info->rssi)) {
	case SIGNAL_STRENGTH_TYPE_EXCELLENT:
		g_strlcat(buf, "_03", sizeof(buf));
		break;
	case SIGNAL_STRENGTH_TYPE_GOOD:
		g_strlcat(buf, "_02", sizeof(buf));
		break;
	case SIGNAL_STRENGTH_TYPE_WEAK:
		g_strlcat(buf, "_01", sizeof(buf));
		break;
	case SIGNAL_STRENGTH_TYPE_VERY_WEAK:
	case SIGNAL_STRENGTH_TYPE_NULL:
	default:
		g_strlcat(buf, "_00", sizeof(buf));
		break;
	}

	if (icon_path) {
		*icon_path = g_strdup_printf("%s", buf);
	}
}

char *common_utils_get_rssi_text(const char *str_pkg_name, int rssi)
{
	switch (wlan_manager_get_signal_strength(rssi)) {
	case SIGNAL_STRENGTH_TYPE_EXCELLENT:
		return g_strdup(sc(str_pkg_name, I18N_TYPE_Excellent));
	case SIGNAL_STRENGTH_TYPE_GOOD:
		return g_strdup(sc(str_pkg_name, I18N_TYPE_Good));
	default:
		return g_strdup(sc(str_pkg_name, I18N_TYPE_Weak));
	}
}

void common_utils_set_edit_box_imf_panel_evnt_cb(Elm_Object_Item *item,
						imf_ctxt_panel_cb_t input_panel_cb, void *user_data)
{
	__COMMON_FUNC_ENTER__;
	common_utils_entry_info_t *entry_info;
	entry_info = elm_object_item_data_get(item);
	if (!entry_info) {
		return;
	}

	entry_info->input_panel_cb = input_panel_cb;
	entry_info->input_panel_cb_data = user_data;

	Evas_Object *entry = elm_object_item_part_content_get(item, "elm.icon.entry");
	Ecore_IMF_Context *imf_ctxt = elm_entry_imf_context_get(entry);
	if (imf_ctxt && entry_info->input_panel_cb) {
		/* Deleting the previously attached callback */
		ecore_imf_context_input_panel_event_callback_del(imf_ctxt,
				ECORE_IMF_INPUT_PANEL_STATE_EVENT,
				entry_info->input_panel_cb);
		ecore_imf_context_input_panel_event_callback_add(imf_ctxt,
				ECORE_IMF_INPUT_PANEL_STATE_EVENT,
				entry_info->input_panel_cb,
				entry_info->input_panel_cb_data);
		DEBUG_LOG(UG_NAME_NORMAL, "set the imf ctxt cbs");
	}

	__COMMON_FUNC_EXIT__;
	return;
}

void common_utils_edit_box_focus_set(Elm_Object_Item *item, Eina_Bool focus_set)
{
	__COMMON_FUNC_ENTER__;
	if (!item) {
		return;
	}

	Evas_Object *entry = elm_object_item_part_content_get(item, "elm.icon.entry");
	elm_object_focus_set(entry, focus_set);
	elm_object_focus_allow_set(entry, focus_set);

	__COMMON_FUNC_EXIT__;
	return;
}

void common_utils_edit_box_allow_focus_set(Elm_Object_Item *item,
		Eina_Bool focus_set)
{
	__COMMON_FUNC_ENTER__;
	if (!item) {
		return;
	}

	Evas_Object *entry = elm_object_item_part_content_get(item, "elm.icon.entry");
	elm_object_focus_allow_set(entry, focus_set);

	__COMMON_FUNC_EXIT__;
	return;
}

Elm_Object_Item *common_utils_add_2_line_txt_disabled_item(
		Evas_Object* view_list, const char *style_name,
		const char *line1_txt, const char *line2_txt)
{
	static Elm_Genlist_Item_Class two_line_display_itc;
	two_line_disp_data_t *two_line_data = NULL;
	Elm_Object_Item *item = NULL;

	two_line_display_itc.item_style = style_name;
	two_line_display_itc.func.text_get = __common_utils_2line_text_get;
	two_line_display_itc.func.content_get = NULL;
	two_line_display_itc.func.state_get = NULL;
	two_line_display_itc.func.del = __common_utils_2line_text_del;

	two_line_data = g_new0(two_line_disp_data_t, 1);
	two_line_data->title_str = g_strdup(line1_txt);
	two_line_data->info_str = g_strdup(line2_txt);
	SECURE_INFO_LOG(UG_NAME_NORMAL, "title_str = %s info_str = %s",
			two_line_data->title_str, two_line_data->info_str);

	item = elm_genlist_item_append(view_list, &two_line_display_itc,
			two_line_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_object_item_disabled_set(item, TRUE);

	return item;
}

char *common_utils_get_list_item_entry_txt(Elm_Object_Item *entry_item)
{
	common_utils_entry_info_t *entry_info =
			(common_utils_entry_info_t *)elm_object_item_data_get(entry_item);
	if (entry_info == NULL) {
		return NULL;
	}

	DEBUG_LOG(UG_NAME_NORMAL, "entry_info: 0x%x", entry_info);

	return g_strdup(entry_info->entry_txt);
}

Evas_Object *common_utils_create_layout(Evas_Object *navi_frame)
{
	Evas_Object *layout;
	layout = elm_layout_add(navi_frame);
	elm_layout_theme_set(layout, "layout", "application", "noindicator");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object* bg = elm_bg_add(layout);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_style_set(bg, "group_list");
	elm_object_part_content_set(layout, "elm.swallow.bg", bg);

	evas_object_show(layout);

	return layout;
}

static void __common_utils_del_popup(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = (Evas_Object *)data;
	evas_object_del(popup);
}

Evas_Object *common_utils_show_info_popup(Evas_Object *parent,
		popup_btn_info_t *popup_data)
{
	__COMMON_FUNC_ENTER__;

	Evas_Object *popup = elm_popup_add(parent);

	if (!popup) {
		ERROR_LOG(UG_NAME_ERR, "Could not add popup");
		return NULL;
	}

	if (popup_data->popup_with_entry == true) {
		elm_object_style_set(popup, "no_effect");
	}
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (popup_data->title_txt) {
		elm_object_part_text_set(popup, "title,text", popup_data->title_txt);
	}
	if (popup_data->info_txt) {
		elm_object_text_set(popup, popup_data->info_txt);
	}
	if (popup_data->btn1_txt) {
		Evas_Object *btn_1 = elm_button_add(popup);
		elm_object_style_set(btn_1, "popup");
		elm_object_text_set(btn_1, popup_data->btn1_txt);
		elm_object_part_content_set(popup, "button1", btn_1);
		if (popup_data->btn1_cb) {
			evas_object_smart_callback_add(btn_1, "clicked",
					popup_data->btn1_cb, popup_data->btn1_data);
			ea_object_event_callback_add(popup, EA_CALLBACK_BACK,
					popup_data->btn1_cb, popup_data->btn1_data);
		} else {
			evas_object_smart_callback_add(btn_1, "clicked",
					__common_utils_del_popup, popup);
			ea_object_event_callback_add(popup, EA_CALLBACK_BACK,
					__common_utils_del_popup, popup);
		}
	}
	if (popup_data->btn2_txt) {
		Evas_Object *btn_2 = elm_button_add(popup);
		elm_object_style_set(btn_2, "popup");
		/* This button reference used in case of hidden AP case */
		popup_data->btn = btn_2;
		elm_object_text_set(btn_2, popup_data->btn2_txt);
		elm_object_part_content_set(popup, "button2", btn_2);
		//evas_object_smart_callback_add(btn_2, "clicked", popup_data->btn2_cb, NULL);
		evas_object_show(popup);
		if (popup_data->btn2_cb) {
			evas_object_smart_callback_add(btn_2, "clicked",
					popup_data->btn2_cb, popup_data->btn2_data);
		} else {
			evas_object_smart_callback_add(btn_2, "clicked",
					__common_utils_del_popup, popup);
		}
	}

	evas_object_show(popup);
	elm_object_focus_set(popup, EINA_TRUE);

	return popup;
}

Evas_Object *common_utils_show_info_ok_popup(Evas_Object *win,
		const char *str_pkg_name, const char *info_txt,
		Evas_Smart_Cb ok_cb, void *cb_data)
{
	popup_btn_info_t popup_data;

	memset(&popup_data, 0, sizeof(popup_data));
	popup_data.info_txt = (char *)info_txt;
	popup_data.btn1_txt = sc(str_pkg_name, I18N_TYPE_Ok);
	popup_data.btn1_cb = ok_cb;
	popup_data.btn1_data = cb_data;

	return common_utils_show_info_popup(win, &popup_data);
}

Evas_Object *common_utils_show_info_timeout_popup(Evas_Object *win,
		const char* info_text, const double timeout)
{
	Evas_Object *popup = elm_popup_add(win);

	elm_object_text_set(popup, info_text);
	elm_popup_timeout_set(popup, timeout);
	evas_object_smart_callback_add(popup, "timeout",
			__common_utils_del_popup, popup);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_CENTER);
	evas_object_show(popup);

	return popup;
}

int common_utils_get_rotate_angle(enum appcore_rm rotate_mode)
{
	int rotate_angle;
	if (APPCORE_RM_UNKNOWN == rotate_mode) {
		appcore_get_rotation_state(&rotate_mode);
	}

	DEBUG_LOG(SP_NAME_NORMAL, "rotate_mode = %d", rotate_mode);

	switch (rotate_mode) {
	case APPCORE_RM_PORTRAIT_NORMAL:	 /**< Portrait mode */
		DEBUG_LOG(SP_NAME_NORMAL, "rotate mode is APPCORE_RM_PORTRAIT_NORMAL");
		rotate_angle = 0;
		break;

	case APPCORE_RM_PORTRAIT_REVERSE:	  /**< Portrait upside down mode */
		DEBUG_LOG(SP_NAME_NORMAL, "rotate mode is APPCORE_RM_PORTRAIT_REVERSE");
		rotate_angle = 180;
		break;

	case APPCORE_RM_LANDSCAPE_NORMAL:	  /**< Left handed landscape mode */
		DEBUG_LOG(SP_NAME_NORMAL, "rotate mode is APPCORE_RM_LANDSCAPE_NORMAL");
		rotate_angle = 270;
		break;

	case APPCORE_RM_LANDSCAPE_REVERSE:	    /**< Right handed landscape mode */
		DEBUG_LOG(SP_NAME_NORMAL, "rotate mode is APPCORE_RM_LANDSCAPE_REVERSE");
		rotate_angle = 90;
		break;

	default:
		ERROR_LOG(SP_NAME_ERR, "Invalid rotate mode. The default value (0) is set to 'rotate_angle'.");
		rotate_angle = 0;
		break;
	}

	return rotate_angle;
}

wlan_security_mode_type_t common_utils_get_sec_mode(wifi_security_type_e sec_type)
{
	switch (sec_type) {
	case WIFI_SECURITY_TYPE_NONE:
		return WLAN_SEC_MODE_NONE;
	case WIFI_SECURITY_TYPE_WEP:
		return WLAN_SEC_MODE_WEP;
	case WIFI_SECURITY_TYPE_WPA_PSK:
		return WLAN_SEC_MODE_WPA_PSK;
	case WIFI_SECURITY_TYPE_WPA2_PSK:
		return WLAN_SEC_MODE_WPA_PSK;
	case WIFI_SECURITY_TYPE_EAP:
		return WLAN_SEC_MODE_IEEE8021X;
	default:
		return WLAN_SEC_MODE_NONE;
	}

	return WLAN_SEC_MODE_NONE;
}

int common_utils_send_message_to_net_popup(const char *title,
		const char *content, const char *type, const char *ssid)
{
	int ret = 0;
	bundle *b = bundle_create();

	bundle_add(b, "_SYSPOPUP_TITLE_", title);
	bundle_add(b, "_SYSPOPUP_CONTENT_", content);
	bundle_add(b, "_SYSPOPUP_TYPE_", type);
	bundle_add(b, "_AP_NAME_", ssid);

	ret = aul_launch_app("net.netpopup", b);

	bundle_free(b);

	return ret;
}

int common_util_set_system_registry(const char *key, int value)
{
	__COMMON_FUNC_ENTER__;

	if (vconf_set_int(key, value) < 0) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed to set vconf");

		__COMMON_FUNC_EXIT__;
		return -1;
	}

	__COMMON_FUNC_EXIT__;
	return 0;
}

int common_util_get_system_registry(const char *key)
{
	__COMMON_FUNC_ENTER__;

	int value = 0;

	if (vconf_get_int(key, &value) < 0) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed to get vconf");

		__COMMON_FUNC_EXIT__;
		return -1;
	}

	__COMMON_FUNC_EXIT__;
	return value;
}

static void __common_util_managed_idle_destroy_cb(gpointer data)
{
	if (!data) {
		return;
	}

	managed_idler_list = g_slist_remove(managed_idler_list, data);
	g_free(data);
}

static gboolean __common_util_managed_idle_cb(gpointer user_data)
{
	struct managed_idle_data *data = (struct managed_idle_data *)user_data;

	if (!data) {
		return FALSE;
	}

	return data->func(data->user_data);
}

guint common_util_managed_idle_add(GSourceFunc func, gpointer user_data)
{
	guint id;
	struct managed_idle_data *data;

	if (!func) {
		return 0;
	}

	data = g_try_new0(struct managed_idle_data, 1);
	if (!data) {
		return 0;
	}

	data->func = func;
	data->user_data = user_data;

	id = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, __common_util_managed_idle_cb,
			data, __common_util_managed_idle_destroy_cb);
	if (!id) {
		g_free(data);
		return id;
	}

	data->id = id;

	managed_idler_list = g_slist_append(managed_idler_list, data);

	return id;
}

void common_util_managed_idle_cleanup(void)
{
	if (managed_idler_list == NULL) {
		return;
	}

	GSList *cur = managed_idler_list;
	GSource *src;
	struct managed_idle_data *data;

	while (cur) {
		GSList *next = cur->next;
		data = (struct managed_idle_data *)cur->data;

		src = g_main_context_find_source_by_id(g_main_context_default(), data->id);
		if (src) {
			g_source_destroy(src);
			cur = managed_idler_list;
		} else {
			cur = next;
		}
	}

	g_slist_free(managed_idler_list);
	managed_idler_list = NULL;
}

void common_util_managed_ecore_scan_update_timer_add(double interval,
		common_util_scan_update_cb callback, void *data)
{
	if (callback == NULL)
		return;

	common_util_managed_ecore_scan_update_timer_del();

	scan_update_timer = ecore_timer_add(interval, callback, data);
}

void common_util_managed_ecore_scan_update_timer_del(void)
{
	if (scan_update_timer != NULL) {
		ecore_timer_del(scan_update_timer);
		scan_update_timer = NULL;
	}
}

void common_util_manager_ecore_scan_update_timer_reset(void)
{
	scan_update_timer = NULL;
}

static GDBusConnection *common_util_get_gdbus_conn(void)
{
	GError *error = NULL;

	if (gdbus_conn.connection != NULL)
		return gdbus_conn.connection;

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif

	gdbus_conn.connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (gdbus_conn.connection == NULL) {
		ERROR_LOG(UG_NAME_NORMAL,
				"Failed to connect to the D-BUS daemon: [%s]\n", error->message);
		g_error_free(error);
		return NULL;
	}

	return gdbus_conn.connection;
}

gboolean common_util_subscribe_scanning_signal(GDBusSignalCallback callback)
{
	GDBusConnection *connection;

	connection = common_util_get_gdbus_conn();
	if (connection == NULL) {
		ERROR_LOG(UG_NAME_NORMAL, "failed to get gdbus_conn");
		return FALSE;
	}

	/* Create supplicant service connection */
	gdbus_conn.subscribe_id_supplicant = g_dbus_connection_signal_subscribe(
			connection,
			SUPPLICANT_SERVICE,
			SUPPLICANT_SERVICE_INTERFACE,
			"PropertiesChanged",
			NULL,
			NULL,
			G_DBUS_SIGNAL_FLAGS_NONE,
			callback,
			NULL,
			NULL);

	if (gdbus_conn.subscribe_id_supplicant == 0) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed register signals "
				"supplicant(%d)\n",
				gdbus_conn.subscribe_id_supplicant);
		return FALSE;
	}

	return TRUE;
}

gboolean common_util_unsubscribe_scanning_signal(void)
{
	GDBusConnection *connection;

	connection = common_util_get_gdbus_conn();
	if (connection == NULL) {
		ERROR_LOG(UG_NAME_NORMAL, "failed to get gdbus_conn");
		return FALSE;
	}

	g_dbus_connection_signal_unsubscribe(connection,
			gdbus_conn.subscribe_id_supplicant);

	g_object_unref(gdbus_conn.connection);
	gdbus_conn.connection = NULL;

	return TRUE;
}
