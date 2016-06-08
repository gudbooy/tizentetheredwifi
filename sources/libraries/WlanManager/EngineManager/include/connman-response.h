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

#ifndef __CONNMAN_RESPONSE_H__
#define __CONNMAN_RESPONSE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "connman-request.h"

/**
* @brief Called when the specific scan is finished.
* @param[in] error_code  The error code
* @param[in] user_data The user data passed from the callback registration function
* @see wifi_set_specific_scan_cb()
*/
typedef void(*wifi_specific_scan_finished_cb)(wifi_error_e error_code, GSList *bss_info_list, void* user_data);


int wifi_set_specific_scan_cb(wifi_specific_scan_finished_cb cb, void *data);
int wifi_unset_specific_scan_cb(void);
void network_evt_cb(const net_event_info_t* net_event, void* user_data);

#ifdef __cplusplus
}
#endif

#endif
