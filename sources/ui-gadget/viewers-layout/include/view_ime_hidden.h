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

#ifndef __VIEW_IME_HIDDEN_H__
#define __VIEW_IME_HIDDEN_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <Evas.h>

typedef struct hiddep_ap_popup_data hiddep_ap_popup_data_t;

void view_hidden_ap_popup_create(Evas_Object *win_main,
		const char *str_pkg_name);
void view_hidden_ap_popup_delete(void);
void view_hidden_ap_popup_destroy(void);
hiddep_ap_popup_data_t *view_hidden_ap_popup_data_get(void);

#ifdef __cplusplus
}
#endif

#endif
