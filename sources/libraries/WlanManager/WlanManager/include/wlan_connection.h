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

#ifndef __WLAN_ALTERNATIVE_CONNECTION_H__
#define __WLAN_ALTERNATIVE_CONNECTION_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>
#include <wifi.h>

int wlan_connect(wifi_ap_h ap, wifi_connected_cb callback, void *user_data);
void wlan_validate_alt_connection(void);

#ifdef __cplusplus
}
#endif

#endif /* __WLAN_ALTERNATIVE_CONNECTION_H__ */
