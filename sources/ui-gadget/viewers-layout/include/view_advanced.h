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

#ifndef __VIEW_ADVANCED_H__
#define __VIEW_ADVANCED_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define VCONF_SORT_BY "file/private/wifi/sort_by"

void view_advanced(void);
int _convert_sort_by_value_to_vconf(int i18n_key);
int _convert_vconf_to_sort_by_value(int vconf_value);

#ifdef __cplusplus
}
#endif

#endif
