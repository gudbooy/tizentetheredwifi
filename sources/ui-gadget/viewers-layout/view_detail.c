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

#include "ug_wifi.h"
#include "view_detail.h"
#include "i18nmanager.h"
#include "viewer_manager.h"
#include "winset_popup.h"
#include "common_utils.h"
#include "common_ip_info.h"
#include "common_eap_connect.h"

typedef struct _view_detail_data {
	Evas_Object *win;
	Evas_Object *nav;
	char *ap_image_path;
	wifi_ap_h ap;
	eap_info_list_t *eap_info_list;
	ip_info_list_t *ip_info_list;
	Evas_Object *forget_confirm_popup;
	Evas_Object *view_detail_list;

	Evas_Object *ctxpopup;
} view_detail_data;

static int view_detail_end = TRUE;
extern wifi_appdata *ug_app_state;
static view_detail_data *_detail_data = NULL;
static Eina_Bool rotate_flag = EINA_FALSE;
static int pos_changed = 0;

static Eina_Bool detailview_sk_cb(void *data, Elm_Object_Item *it);
static void forget_sk_cb(void *data, Evas_Object *obj, void *event_info);
static void _transition_finished_sub_cb(void *data, Evas_Object *obj, void *event_info);
static void _create_ctxpopup_forget_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void _ctxpopup_del_cb(void *data, Evas_Object *obj, void *event_info);
static void _ctxpopup_move(Evas_Object *parent);
static void _ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
static void _ctxpopup_delete_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _ctxpopup_rotate_cb(void *data, Evas_Object *obj, void *event_info);
static void _ctxpopup_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _create_ctxpopup_forget_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *parent = data;

	if (!parent || !_detail_data) {
		return;
	}

	if (_detail_data->ctxpopup) {
		evas_object_del(_detail_data->ctxpopup);
	}

	_detail_data->ctxpopup = elm_ctxpopup_add(parent);

	elm_ctxpopup_auto_hide_disabled_set(_detail_data->ctxpopup, EINA_TRUE);
	elm_object_style_set(_detail_data->ctxpopup, "more/default");
	ea_object_event_callback_add(_detail_data->ctxpopup, EA_CALLBACK_BACK,
			_ctxpopup_del_cb, NULL);
	ea_object_event_callback_add(_detail_data->ctxpopup, EA_CALLBACK_MORE,
			_ctxpopup_del_cb, NULL);
	evas_object_smart_callback_add(_detail_data->ctxpopup, "dismissed",
			_ctxpopup_dismissed_cb, NULL);
	evas_object_event_callback_add(_detail_data->ctxpopup, EVAS_CALLBACK_DEL,
			_ctxpopup_delete_cb, parent);
	evas_object_event_callback_add(parent, EVAS_CALLBACK_RESIZE,
			_ctxpopup_resize_cb, _detail_data->ctxpopup);

	evas_object_smart_callback_add(elm_object_top_widget_get(_detail_data->ctxpopup),
			"rotation,changed", _ctxpopup_rotate_cb, _detail_data->ctxpopup);

	elm_ctxpopup_item_append(_detail_data->ctxpopup,
			sc(PACKAGE, I18N_TYPE_Forget), NULL, forget_sk_cb, _detail_data);

	elm_ctxpopup_direction_priority_set(_detail_data->ctxpopup,
			ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	_ctxpopup_move(_detail_data->ctxpopup);
	evas_object_show(_detail_data->ctxpopup);
}

static void _ctxpopup_del_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	if (!_detail_data)
		return;

	evas_object_del(_detail_data->ctxpopup);
	_detail_data->ctxpopup = NULL;

	__COMMON_FUNC_EXIT__;
}

static void _ctxpopup_move(Evas_Object *parent)
{
	__COMMON_FUNC_ENTER__;

	if (!_detail_data)
		return;

	Evas_Object *win;
	Evas_Coord w, h;
	int pos = -1;

	win = elm_object_top_widget_get(_detail_data->ctxpopup);
	elm_win_screen_size_get(win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(win);
	if (pos == 0 || pos == 180) {
		pos_changed = 0;
	} else if (pos == 90 || pos == 270) {
		pos_changed = 1;
	}

	switch (pos) {
		case 0:
		case 180:
			evas_object_move(_detail_data->ctxpopup, (w/2), h);
			break;
		case 90:
			evas_object_move(_detail_data->ctxpopup, (h/2), w);
			break;
		case 270:
			evas_object_move(_detail_data->ctxpopup, (h/2), w);
			break;
	}
	__COMMON_FUNC_EXIT__;
}

static void _ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	if (!_detail_data)
		return;

	Evas_Object *_win_main = data;

	if (!rotate_flag) {
		evas_object_del(_detail_data->ctxpopup);
		_detail_data->ctxpopup = NULL;
	} else {
		_ctxpopup_move(_win_main);
		evas_object_show(_detail_data->ctxpopup);
		rotate_flag = EINA_FALSE;
	}

	__COMMON_FUNC_EXIT__;
}

static void _ctxpopup_delete_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	Evas_Object *navi = (Evas_Object *)data;
	Evas_Object *ctx = obj;

	if (navi == NULL)
		return;

	if (ctx == NULL)
		return;

	evas_object_smart_callback_del(ctx, "dismissed",
			_ctxpopup_dismissed_cb);
	evas_object_event_callback_del(navi, EVAS_CALLBACK_RESIZE,
			_ctxpopup_resize_cb);
	evas_object_smart_callback_del(elm_object_top_widget_get(ctx),
			"rotation,changed", _ctxpopup_rotate_cb);
	evas_object_event_callback_del_full(ctx, EVAS_CALLBACK_DEL,
			_ctxpopup_delete_cb, navi);

	__COMMON_FUNC_EXIT__;

}

static void _ctxpopup_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{

	__COMMON_FUNC_ENTER__;

	if (!_detail_data)
		return;

	Evas_Object *_win_main = data;

	_ctxpopup_move(_win_main);
	evas_object_show(_detail_data->ctxpopup);

	__COMMON_FUNC_EXIT__;
}

static void _ctxpopup_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	if (_detail_data->ctxpopup) {
		Evas_Object *win;
		Evas_Coord w, h;
		int pos = -1;

		win = elm_object_top_widget_get(_detail_data->ctxpopup);
		elm_win_screen_size_get(win, NULL, NULL, &w, &h);
		pos = elm_win_rotation_get(win);
		if (pos == 0 || pos == 180) {
			pos = 0;
		} else if (pos == 90 || pos == 270) {
			pos = 1;
		}
		if (pos_changed != pos) {
			rotate_flag = EINA_TRUE;
		} else
			rotate_flag = EINA_FALSE;
	}
	else
		rotate_flag = EINA_FALSE;

	__COMMON_FUNC_EXIT__;
}

static void _gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	retm_if(NULL == data);

//	view_detail_data *_detail_data = (view_detail_data *)data;
	_detail_data = (view_detail_data *)data;

	ip_info_items_realize(_detail_data->ip_info_list);
	eap_info_items_realize(_detail_data->eap_info_list);
}

static char *_view_detail_grouptitle_text_get(void *data,
		Evas_Object *obj, const char *part)
{
	__COMMON_FUNC_ENTER__;

	retvm_if(NULL == part, NULL);

	char *ret = NULL;

	if (!strncmp(part, "elm.text.sub.left.top", strlen(part))) {
		ret = (char*) g_strdup(sc(PACKAGE, I18N_TYPE_Name));
	} else if (!strncmp(part, "elm.text.main.left.bottom", strlen(part))) {
//		view_detail_data *_detail_data = (view_detail_data *)data;
		_detail_data = (view_detail_data *)data;
		retvm_if(NULL == _detail_data, NULL);

		if (wifi_ap_get_essid(_detail_data->ap, &ret) != WIFI_ERROR_NONE) {
			ret = NULL;
		}
	}

	__COMMON_FUNC_EXIT__;
	return ret;
}

static Evas_Object *_view_detail_grouptitle_content_get(void *data, Evas_Object *obj, const char *part)
{
	retvm_if(NULL == data || NULL == part, NULL);

	view_detail_data *_detail_data = (view_detail_data *)data;
	_detail_data = (view_detail_data *)data;
	Evas_Object* icon = NULL;
	Evas_Object* ic = NULL;

	ic = elm_layout_add(obj);


	if (_detail_data->ap_image_path == NULL) {
		/* if there is no ap_image_path (NO AP Found situation) */
		DEBUG_LOG(UG_NAME_ERR, "Fatal: Image path is NULL");
	} else if (!strncmp(part, "elm.icon.2", strlen(part))) {
		elm_layout_theme_set(ic, "layout", "list/C/type.1", "default");
		/* for strength */
		icon = elm_image_add(ic);
		retvm_if(NULL == icon, NULL);

		char *temp_str = NULL;
		temp_str = g_strdup_printf("%s.png", _detail_data->ap_image_path);
		ea_theme_object_color_set(icon,"AO001");

		elm_image_file_set(icon, CUSTOM_EDITFIELD_PATH, temp_str);
		g_free(temp_str);

		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_content_set(ic, "elm.swallow.content", icon);
	}
	return ic;
}

static void _remove_all(view_detail_data *_detail_data)
{
	__COMMON_FUNC_ENTER__;

	if (_detail_data) {

		if (_detail_data->forget_confirm_popup != NULL) {
			evas_object_del(_detail_data->forget_confirm_popup);
			_detail_data->forget_confirm_popup = NULL;
		}

		if (_detail_data->ctxpopup) {
			evas_object_del(_detail_data->ctxpopup);
			_detail_data->ctxpopup = NULL;
		}

		if (_detail_data->eap_info_list) {
			eap_info_remove(_detail_data->eap_info_list);
		}

		ip_info_remove(_detail_data->ip_info_list);

		_detail_data->ip_info_list = NULL;
		_detail_data->eap_info_list = NULL;

		g_free(_detail_data->ap_image_path);
		g_free(_detail_data);

		_detail_data = NULL;
	}

	__COMMON_FUNC_EXIT__;
}

static gboolean __forget_wifi_ap(gpointer data)
{
	__COMMON_FUNC_ENTER__;

	wifi_ap_h ap = (wifi_ap_h)data;

	wlan_manager_forget(ap);

	viewer_manager_update_item_favorite_status(ap);
	wifi_ap_destroy(ap);

	__COMMON_FUNC_EXIT__;
	return FALSE;
}

static void ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	guint id;
	wifi_ap_h ap = NULL;
	//view_detail_data *_detail_data;

	if (view_detail_end == TRUE) {
		return;
	}

	view_detail_end = TRUE;
	_detail_data = (view_detail_data *)data;
	retm_if(NULL == _detail_data);

	wifi_ap_clone(&ap, _detail_data->ap);

	if (NULL != _detail_data->ctxpopup) {
		evas_object_del(_detail_data->ctxpopup);
		_detail_data->ctxpopup = NULL;
	}

	if (_detail_data->forget_confirm_popup != NULL) {
		evas_object_del(_detail_data->forget_confirm_popup);
		_detail_data->forget_confirm_popup = NULL;
	}

	_remove_all(_detail_data);

	elm_naviframe_item_pop(viewer_manager_get_naviframe());

	id = common_util_managed_idle_add(__forget_wifi_ap, (gpointer)ap);
	if (!id) {
		wifi_ap_destroy(ap);
	}

	__COMMON_FUNC_EXIT__;
}

static void cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

//	view_detail_data *_detail_data = (view_detail_data *)data;
	_detail_data = (view_detail_data *)data;
	retm_if(NULL == _detail_data);

	if (_detail_data->forget_confirm_popup != NULL) {
		evas_object_del(_detail_data->forget_confirm_popup);
		_detail_data->forget_confirm_popup = NULL;
	}

	if (_detail_data->ctxpopup) {
		evas_object_del(_detail_data->ctxpopup);
		_detail_data->ctxpopup = NULL;
	}

	__COMMON_FUNC_EXIT__;
}

static void forget_sk_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

//	view_detail_data *_detail_data = (view_detail_data *)data;
	_detail_data = (view_detail_data *)data;
	retm_if(NULL == _detail_data);

	if (!_detail_data->forget_confirm_popup) {
		popup_btn_info_t popup_data;
		memset(&popup_data, 0, sizeof(popup_data));

		popup_data.title_txt = sc(PACKAGE, I18N_TYPE_Forget_Network);
		popup_data.info_txt = g_strdup(sc(PACKAGE, I18N_TYPE_Current_Network_Will_Be_Disconnected));
		popup_data.btn1_cb = cancel_cb;
		popup_data.btn1_txt = sc(PACKAGE, I18N_TYPE_Cancel);
		popup_data.btn1_data = _detail_data;
		popup_data.btn2_cb = ok_cb;
		popup_data.btn2_txt = sc(PACKAGE, I18N_TYPE_Forget);
		popup_data.btn2_data = _detail_data;

		_detail_data->forget_confirm_popup = common_utils_show_info_popup(_detail_data->win, &popup_data);
	}

	__COMMON_FUNC_EXIT__;
}

static Eina_Bool detailview_sk_cb(void *data, Elm_Object_Item *it)
{
	__COMMON_FUNC_ENTER__;

	Evas_Object *navi_frame = NULL;

	if (view_detail_end == TRUE) {
		return EINA_TRUE;
	}

	view_detail_end = TRUE;

//	view_detail_data *_detail_data = (view_detail_data *)data;
	_detail_data = (view_detail_data *)data;
	retvm_if(NULL == _detail_data, EINA_TRUE);

	passwd_popup_show(ug_app_state->passpopup);

	evas_object_smart_callback_del(_detail_data->view_detail_list, "realized",
			_gl_realized);

	navi_frame = viewer_manager_get_naviframe();
	retvm_if(NULL == navi_frame, EINA_TRUE);

	evas_object_smart_callback_add(navi_frame, "transition,finished", _transition_finished_sub_cb, _detail_data);

	__COMMON_FUNC_EXIT__;

	return EINA_TRUE;
}

static void __view_detail_imf_ctxt_evnt_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	if (!data) {
		return;
	}
	if (value == ECORE_IMF_INPUT_PANEL_STATE_SHOW) {
		DEBUG_LOG(UG_NAME_NORMAL, "Key pad is now open");
		elm_object_item_signal_emit(data, "elm,state,sip,shown", "");
	} else if (value == ECORE_IMF_INPUT_PANEL_STATE_HIDE) {
		DEBUG_LOG(UG_NAME_NORMAL, "Key pad is now closed");
		elm_object_item_signal_emit(data, "elm,state,sip,hidden", "");
	}
	return;
}

static gboolean __view_detail_load_ip_info_list_cb(void *data)
{
	Elm_Object_Item *navi_it = NULL;
	Evas_Object *list = NULL;
	Evas_Object *layout;
	_detail_data = (view_detail_data *)data;

	if (!_detail_data) {
		return FALSE;
	}

	navi_it = elm_naviframe_top_item_get(viewer_manager_get_naviframe());
	layout = elm_object_item_part_content_get(navi_it, "elm.swallow.content");

	/* Create an EAP connect view list */
	list = elm_object_part_content_get(layout, "elm.swallow.content");

	/* Append ip info list */
	_detail_data->ip_info_list = ip_info_append_items(_detail_data->ap,
			PACKAGE, list, __view_detail_imf_ctxt_evnt_cb, navi_it);

	return FALSE;
}

static void _transition_finished_sub_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	Evas_Object *navi = NULL;

	_detail_data = (view_detail_data *)data;
	if (!_detail_data) {
		__COMMON_FUNC_EXIT__;
		return;
	}

	if (_detail_data->eap_info_list) {
		eap_info_save_data(_detail_data->eap_info_list);
	}

	navi = (Evas_Object *)viewer_manager_get_naviframe();
	if (navi == NULL) {
		__COMMON_FUNC_EXIT__;
		return;
	}
	else {
		ip_info_save_data(_detail_data->ip_info_list);
		_remove_all(_detail_data);
		evas_object_smart_callback_del(navi, "transition,finished", _transition_finished_sub_cb);
	}

	__COMMON_FUNC_EXIT__;
}

void view_detail(wifi_device_info_t *device_info, Evas_Object *win_main)
{
	__COMMON_FUNC_ENTER__;

	bool favorite = 0;
	guint id;
	wifi_ap_h ap;
	static Elm_Genlist_Item_Class grouptitle_itc;
	Evas_Object *layout = NULL;
	Evas_Object *navi_frame = NULL;
	Evas_Object *detailview_list = NULL;
	Elm_Object_Item *title = NULL;
	Elm_Object_Item *navi_it = NULL;
	Evas_Object *more_btn = NULL;

	if (device_info == NULL) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed: device_info is NULL");
		return;
	}

	navi_frame = viewer_manager_get_naviframe();
	if (navi_frame == NULL) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed to get naviframe");
		return;
	}

	view_detail_end = FALSE;

	_detail_data = g_try_new0(view_detail_data, 1);
	retm_if(NULL == _detail_data);

	_detail_data->win = win_main;
	_detail_data->nav = navi_frame;
	_detail_data->ap = ap = device_info->ap;
	wifi_ap_is_favorite(ap, &favorite);
	_detail_data->ap_image_path = g_strdup(device_info->ap_image_path);

	if (ug_app_state->ug_type == UG_VIEW_SETUP_WIZARD) {
		layout = elm_layout_add(navi_frame);

		elm_layout_file_set(layout, SETUP_WIZARD_EDJ_PATH, "detail_pwlock");
		elm_object_domain_translatable_part_text_set(layout,
			"text.title", PACKAGE, sc(PACKAGE, I18N_TYPE_Wi_Fi_network_info));

		navi_it = elm_naviframe_item_push(navi_frame,
					NULL, NULL, NULL, layout, NULL);
		elm_naviframe_item_title_enabled_set(navi_it, EINA_FALSE, EINA_FALSE);

		struct ug_data *ugd = (struct ug_data *)ug_app_state->gadget;
		int change_ang = elm_win_rotation_get(ugd->win_main);
		if (change_ang == 0 || change_ang == 180) {
			common_utils_contents_rotation_adjust(UG_EVENT_ROTATE_PORTRAIT);
			edje_object_signal_emit((Evas_Object *)elm_layout_edje_get(layout)
				,"location,vertical", "elm");
		} else {
			common_utils_contents_rotation_adjust(UG_EVENT_ROTATE_LANDSCAPE);
			edje_object_signal_emit((Evas_Object *)elm_layout_edje_get(layout),
				"location,horizontal", "elm");
		}
	} else {
		layout = common_utils_create_layout(navi_frame);

		navi_it = elm_naviframe_item_push(navi_frame,
					sc(PACKAGE, I18N_TYPE_Wi_Fi_network_info), NULL, NULL, layout, NULL);
	}

	evas_object_show(layout);

	detailview_list = elm_genlist_add(layout);
	assertm_if(NULL == detailview_list, "NULL!!");
	elm_genlist_fx_mode_set(detailview_list, EINA_FALSE);
	elm_genlist_realization_mode_set(detailview_list, TRUE);

	elm_object_style_set(detailview_list, "dialogue");
	_detail_data->view_detail_list = detailview_list;

	evas_object_smart_callback_add(detailview_list, "realized",
			_gl_realized, _detail_data);

	grouptitle_itc.item_style = "2line.bottom";
	grouptitle_itc.func.text_get = _view_detail_grouptitle_text_get;
	grouptitle_itc.func.content_get = _view_detail_grouptitle_content_get;
	grouptitle_itc.func.state_get = NULL;
	grouptitle_itc.func.del = NULL;

	/* AP name and signal strength icon */
	title = elm_genlist_item_append(detailview_list,
				&grouptitle_itc, _detail_data, NULL, ELM_GENLIST_ITEM_GROUP,
				NULL, NULL);
	elm_genlist_item_select_mode_set(title, ELM_OBJECT_SELECT_MODE_NONE);

	elm_object_part_content_set(layout, "elm.swallow.content", detailview_list);

	evas_object_data_set(navi_frame, SCREEN_TYPE_ID_KEY,
				(void *)VIEW_MANAGER_VIEW_TYPE_DETAIL);

	/* Set pop callback */
	elm_naviframe_item_pop_cb_set(navi_it, detailview_sk_cb, _detail_data);

	if (favorite) {
		/* Toolbar Forget button */
#if 0
		Evas_Object *toolbar = NULL;

		toolbar = elm_toolbar_add(navi_frame);
		elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
		elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
		elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);

		elm_toolbar_item_append(toolbar, NULL,
				sc(PACKAGE, I18N_TYPE_Forget),
				forget_sk_cb, _detail_data);

		elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
#endif

		more_btn = elm_button_add(_detail_data->nav);
		elm_object_style_set(more_btn, "naviframe/more/default");
		evas_object_smart_callback_add(more_btn, "clicked",
				_create_ctxpopup_forget_btn_cb, win_main);
		elm_object_item_part_content_set(navi_it, "toolbar_more_btn", more_btn);
	}

	wifi_security_type_e type = WIFI_SECURITY_TYPE_NONE;
	wifi_ap_get_security_type(ap, &type);
	if (WIFI_SECURITY_TYPE_EAP == type) {
		wifi_connection_state_e connection_state;
		wifi_ap_get_connection_state(ap, &connection_state);
		if (favorite || WIFI_CONNECTION_STATE_CONNECTED == connection_state) {
			_detail_data->eap_info_list = eap_info_append_items(ap,
					detailview_list, PACKAGE, __view_detail_imf_ctxt_evnt_cb,
					navi_it);
		}
	}

	/* Append the ip info details */
	id = common_util_managed_idle_add(__view_detail_load_ip_info_list_cb, _detail_data);
	if (!id) {
		g_free(_detail_data);
	}

	__COMMON_FUNC_EXIT__;
}
