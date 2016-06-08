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

#ifndef __COMMON_UTILS_H__
#define __COMMON_UTILS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>
#include <gio/gio.h>
#include <Elementary.h>

/* Fix build warning (redefine '_()' in appcore-common.h) */
#ifdef _
#undef _
#endif
#include <appcore-common.h>
#include <efl_assist.h>

#include "wlan_manager.h"

typedef void (*imf_ctxt_panel_cb_t)(void *data, Ecore_IMF_Context *ctx, int value);

typedef enum {
	ENTRY_TYPE_USER_ID,
	ENTRY_TYPE_PASSWORD,
	ENTRY_TYPE_IP_ADDR,
	ENTRY_TYPE_SUBNET_MASK,
	ENTRY_TYPE_GATEWAY,
	ENTRY_TYPE_DNS_1,
	ENTRY_TYPE_DNS_2,
	ENTRY_TYPE_PROXY_ADDR,
	ENTRY_TYPE_PROXY_PORT,
} entry_id_type_t;

typedef struct {
	entry_id_type_t entry_id;
	char *title_txt;
	char *guide_txt;
	char *entry_txt;
	Elm_Object_Item *item;
	imf_ctxt_panel_cb_t input_panel_cb;
	void *input_panel_cb_data;
	const char *str_pkg_name;
} common_utils_entry_info_t;

typedef struct {
	char *title_txt;
	char *info_txt;
	Evas_Object *btn;
	char *btn1_txt;
	char *btn2_txt;
	Evas_Smart_Cb btn1_cb;
	Evas_Smart_Cb btn2_cb;
	void *btn1_data;
	void *btn2_data;
	bool popup_with_entry;
} popup_btn_info_t;

typedef Eina_Bool (*common_util_scan_update_cb)(void *data);

Eina_Bool common_utils_is_portrait_mode(void);
void common_utils_set_rotate_cb(int (*func)(enum appcore_rm, void*, Eina_Bool, Eina_Bool),
		void* data, Eina_Bool wps_value, Eina_Bool setting_value);
void common_utils_contents_rotation_adjust(int event);
Elm_Object_Item *common_utils_add_dialogue_separator(
		Evas_Object* genlist, const char *separator_style);
char *common_utils_get_ap_security_type_info_txt(
		const char *pkg_name, wifi_device_info_t *device_info, bool check_fav);
void common_utils_get_device_icon(wifi_device_info_t *device_info,
		char **icon_path);
char *common_utils_get_rssi_text(
		const char *str_pkg_name, int rssi);

Evas_Object *common_utils_add_edit_box(Evas_Object *parent,
		common_utils_entry_info_t *entry_info);
void common_utils_set_edit_box_imf_panel_evnt_cb(Elm_Object_Item *item,
		imf_ctxt_panel_cb_t input_panel_cb,	void *user_data);
void common_utils_edit_box_focus_set(Elm_Object_Item *item, Eina_Bool focus_set);
void common_utils_edit_box_allow_focus_set(Elm_Object_Item *item,
		Eina_Bool focus_set);
Elm_Object_Item *common_utils_add_2_line_txt_disabled_item(
		Evas_Object* view_list, const char *style_name,
		const char *line1_txt, const char *line2_txt);
char *common_utils_get_list_item_entry_txt(Elm_Object_Item *entry_item);
Evas_Object *common_utils_create_layout(Evas_Object *navi_frame);
Evas_Object *common_utils_show_info_popup(Evas_Object *win,
		popup_btn_info_t *popup_data);
Evas_Object *common_utils_show_info_ok_popup(Evas_Object *win,
		const char *str_pkg_name, const char *info_txt,
		Evas_Smart_Cb ok_cb, void *cb_data);
Evas_Object *common_utils_show_info_timeout_popup(Evas_Object *win,
		const char* info_text, const double timeout);
int common_utils_get_rotate_angle(enum appcore_rm rotate_mode);
wlan_security_mode_type_t common_utils_get_sec_mode(
		wifi_security_type_e sec_type);
int common_utils_send_message_to_net_popup(const char *title,
		const char *content, const char *type, const char *ssid);

int common_util_set_system_registry(const char *key, int value);
int common_util_get_system_registry(const char *key);

guint common_util_managed_idle_add(GSourceFunc func, gpointer user_data);
void common_util_managed_idle_cleanup(void);
void common_utils_get_wps_icon(char **icon_path, gboolean pressed);
void common_util_managed_ecore_scan_update_timer_add(double interval,
		common_util_scan_update_cb callback, void *data);
void common_util_managed_ecore_scan_update_timer_del(void);
void common_util_manager_ecore_scan_update_timer_reset(void);

gboolean common_util_subscribe_scanning_signal(GDBusSignalCallback callback);
gboolean common_util_unsubscribe_scanning_signal(void);
Ea_Theme_Color_Table *common_utils_color_table_set(void);
Ea_Theme_Font_Table *common_utils_font_table_set(void);

#ifdef __cplusplus
}
#endif

#endif
