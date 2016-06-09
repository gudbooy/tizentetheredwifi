#ifndef __BT_TETHERED_SERVICE_H__
#define __BT_TETHERED_SERVICE_H__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlog.h>
#include <glib.h>
#include <bluetooth.h>
#include "common_bt.h"
#include <glib.h>
static GMainLoop* gMainLoop = NULL;
static bt_adapter_visibility_mode_e gVisibilityMode = BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE;
static int gSocketFd = -1;
static bt_adapter_state_e gBtState = BT_ADAPTER_DISABLED;
static const char service_uuid[] = "c6bd7060-295d-11e6-bdf4-0800200c9a66";
static char* bt_server_address = NULL;
static const char *remote_server_name = "server device";



int rhtf_initialize_bluetooth(const char* device_name);
int rhtf_send_data_bluetooth(void* data);
int rhtf_finalize_bluetooth_socket(void);
int rhtf_finalize_bluetooth(void);

gboolean start_bt_service(void *data);




#endif /*__BT_TETHERED_SERVICE_H__*/
