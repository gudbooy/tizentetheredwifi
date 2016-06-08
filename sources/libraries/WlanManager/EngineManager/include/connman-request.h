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

#ifndef __CONNMAN_REQUEST_H__
#define __CONNMAN_REQUEST_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <network-wifi-intf.h>

int connman_request_specific_scan(const char *ssid);
int connman_request_scan_mode_set(net_wifi_background_scan_mode_t scan_mode);
int connman_request_register(void);
int connman_request_deregister(void);

#ifdef __cplusplus
}
#endif

#endif
