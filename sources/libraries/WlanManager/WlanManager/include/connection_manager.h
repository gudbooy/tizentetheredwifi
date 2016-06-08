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

#ifndef __WIFI_CONNECTION_MANAGER_H__
#define __WIFI_CONNECTION_MANAGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>


gboolean connection_manager_create(void);
gboolean connection_manager_destroy(void);
gboolean connection_manager_is_wifi_connection_used(void);


#ifdef __cplusplus
}
#endif

#endif /* __WIFI_CONNECTION_MANAGER_H__ */
