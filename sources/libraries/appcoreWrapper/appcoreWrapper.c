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

#include <Ecore_X.h>
#include <Elementary.h>

#include "common.h"
#include "appcoreWrapper.h"

static void __appcore_win_del(void *data, Evas_Object *obj, void *event)
{
	INFO_LOG(UG_NAME_NORMAL, "win_del");
	elm_exit();
}

Evas_Object* appcore_create_win(const char *name)
{
	Evas_Object *eo;
	int w, h;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		elm_win_borderless_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request",
				__appcore_win_del, NULL);
		ecore_x_window_size_get(ecore_x_window_root_first_get(),
				&w, &h);
		evas_object_resize(eo, w, h);
	}

	return eo;
}

Evas_Object* appcore_load_edj(Evas_Object *parent, const char *file, const char *group)
{
	Evas_Object *eo;
	int r;

	eo = elm_layout_add(parent);
	if (eo) {
		r = elm_layout_file_set(eo, file, group);
		if (!r) {
			evas_object_del(eo);
			return NULL;
		}

		evas_object_size_hint_weight_set(eo,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	}

	return eo;
}
