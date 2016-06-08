
#include "bt_tethered_service.h"
#include "common_bt.h"

#undef LOG_TAG
#define LOG_TAG "REMOTE_HOTSPOT_TRIGGER_FW"

//static GMainLoop* gMainLoop = NULL;
//static bt_adapter_visibility_mode_e gVisibilityMode = BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE;
//static int gSocketFd = -1;
//tatic bt_adapter_state_e gBtState = BT_ADAPTER_DISABLED;
//static const char service_uuid[] = "00001101-0000-1000-8000-00805F9B34FB";
//static char* bt_server_address = NULL;
//static const char *remote_server_name = "server device";

// Lifecycle of Remote Hotspot Frameworki
//int rhtf_initialize_bluetooth(void);
//int rhtf_send_data_bluetooth(void* data);
//int rhtf_finalize_bluetooth_socket(void);
//int rhtf_finalize_bluetooth(void);

// Callbacks
void rhtf_received_data_cb(bt_socket_received_data_s *, void *);
void rhtf_socket_connection_state_changed_cb(int, bt_socket_connection_state_e, bt_socket_connection_s *, void *);
void rhtf_state_changed_cb(int, bt_adapter_state_e, void *);
gboolean timeout_func_cb(gpointer);

int rhtf_initialize_bluetooth(const char *device_name) {
	// Initialize bluetooth and get adapter state
	int ret;
	ret = bt_initialize();
	if(ret != BT_ERROR_NONE) {
		ALOGD("Unknown exception is occured in bt_initialize(): %x", ret);
		return -1;
	}

	ret = bt_adapter_get_state(&gBtState);
	if(ret != BT_ERROR_NONE) {
		ALOGD("Unknown exception is occured in bt_adapter_get_state(): %x", ret);
		return -2;
	}

	// Enable bluetooth device manually
	if(gBtState == BT_ADAPTER_DISABLED)
	{
		ALOGE("[%s] bluetooth is not enabled.", __FUNCTION__);
		return -3;
	}
	else
	{
		ALOGI("[%s] BT was already enabled.", __FUNCTION__);
	}

	// Set adapter's name
	if(gBtState == BT_ADAPTER_ENABLED) {
		char *name = NULL;
		ret = bt_adapter_get_name(&name);
		if(name == NULL) {
			ALOGD("NULL name exception is occured in bt_adapter_get_name(): %x", ret);
			return -5;
		}

		if(strncmp(name, device_name, strlen(name)) != 0) {
			if(bt_adapter_set_name(device_name) != BT_ERROR_NONE)
			{   
				if (NULL != name)
					free(name);
				ALOGD("Unknown exception is occured in bt_adapter_set_name : %x", ret);
				return -6;
			}   
		}
		free(name);
	} else {
		ALOGD("Bluetooth is not enabled");
		return -7;
	}

	/* No need to visualize the device when connecting as a client
	//  Set visibility as BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE
	if(bt_adapter_get_visibility(&gVisibilityMode, NULL) != BT_ERROR_NONE)
	{
		LOGE("[%s] bt_adapter_get_visibility() failed.", __FUNCTION__);
		return -11; 
	}

	if(gVisibilityMode != BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE)
	{
		if(bt_adapter_set_visibility(BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE, 0) != BT_ERROR_NONE)
		{   
			LOGE("[%s] bt_adapter_set_visibility() failed.", __FUNCTION__);
			return -12; 
		}   
		gVisibilityMode = BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE;
	}
	else
	{
		LOGI("[%s] Visibility mode was already set as BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE.", __FUNCTION__);
	}
	*/

	// Connecting socket as a client

	ret = bt_socket_set_connection_state_changed_cb(rhtf_socket_connection_state_changed_cb, NULL);
	if(ret != BT_ERROR_NONE) {
		ALOGD("Unknown exception is occured in bt_socket_set_connection_state_changed_cb(): %x", ret);
		return -9;
	}
	
	ret = bt_socket_set_data_received_cb(rhtf_received_data_cb, NULL);
	if(ret != BT_ERROR_NONE) {
		ALOGD("Unknown exception is occured in bt_socket_set_data_received_cb(): %x", ret);
		return -10;
	}

	
	return 0;
}

int rhtf_send_data_bluetooth(void* data){
	int ret, len;
	const char* buf = (char*) data;

	len = strlen(buf);
	// Send data to connected socket
	ret = bt_socket_send_data(gSocketFd, buf, len);
	if(ret != BT_ERROR_NONE)
	{
		ALOGD("[%s] bt_socket_send_data failed.", __FUNCTION__);
	}
	return ret;
}


int rhtf_finalize_bluetooth_socket(void) {
	int ret;
	sleep(5); // Wait for completing delivery
	ret = bt_socket_disconnect_rfcomm(gSocketFd);
	if(ret != BT_ERROR_NONE)
	{
		ALOGD("Unknown exception is occured in bt_socket_disconnect_rfcomm(): %x", ret);
		return -1;
	}


	return 0;
}

int rhtf_finalize_bluetooth(void) {
	bt_deinitialize();
	return 0;
}



int gReceiveCount = 0;

// bt_socket_data_received_cb
void rhtf_received_data_cb(bt_socket_received_data_s *data, void *user_data) {
	static char buffer[1024];
	char menu_string[]="menu";
	char home_string[]="home";
	char back_string[]="back";

	strncpy(buffer, data->data, 1024);
	buffer[data->data_size] = '\0';
	ALOGD("RemoteKeyFW: received a data!(%d) %s", ++gReceiveCount, buffer);

	//____________________________HYEMIN'S working part______________________
	// ACTION!
	if(strncmp(buffer, menu_string, strlen(menu_string)) == 0) {
		system("/bin/echo 1 > /sys/bus/platform/devices/homekey/coordinates");
	} else if(strncmp(buffer, home_string, strlen(home_string)) == 0) {
		system("/bin/echo 11 > /sys/bus/platform/devices/homekey/coordinates");
	} else if(strncmp(buffer, back_string, strlen(back_string)) == 0) {
		system("/bin/echo 111 > /sys/bus/platform/devices/homekey/coordinates");
	}

}

// bt_socket_connection_state_changed_cb
void rhtf_socket_connection_state_changed_cb(int result, bt_socket_connection_state_e connection_state_event, bt_socket_connection_s *connection, void *user_data) {		
	char *msg = "TEST STRING";
	
	if(result == BT_ERROR_NONE) {
		ALOGD("RemoteKeyFW: connection state changed (BT_ERROR_NONE)");
	} else {
		ALOGD("RemoteKeyFW: connection state changed (not BT_ERROR_NONE)");
	}

	if(connection_state_event == BT_SOCKET_CONNECTED) {
		ALOGD("RemoteKeyFW: connected");
		gSocketFd = connection->socket_fd;

		//Send Data to connected socket
		rhtf_send_data_bluetooth((void*)msg);
	} else if(connection_state_event == BT_SOCKET_DISCONNECTED) {
		ALOGD("RemoteKeyFW: disconnected");
		g_main_loop_quit(gMainLoop);
	}
}

void rhtf_state_changed_cb(int result, bt_adapter_state_e adapter_state, void *user_data) {
	if(adapter_state == BT_ADAPTER_ENABLED) {
		if(result == BT_ERROR_NONE) {
			ALOGD("RemoteKeyFW: bluetooth was enabled successfully.");
			gBtState = BT_ADAPTER_ENABLED;
		} else {
			ALOGD("RemoteKeyFW: failed to enable BT.: %x", result);
			gBtState = BT_ADAPTER_DISABLED;
		}
	}
	if(gMainLoop) {
		ALOGD("It will terminate gMainLoop.", result);
		g_main_loop_quit(gMainLoop);
	}
}

gboolean timeout_func_cb(gpointer data)
{
	ALOGE("timeout_func_cb");
	if(gMainLoop)
	{
		g_main_loop_quit((GMainLoop*)data);
	}
	return FALSE;
}

int start_bt_service(char* start_address)
{
	int error, ret = 0;
	const char default_device_name[] = "Tizen-RHT";
	const char *device_name = NULL;
	gMainLoop = g_main_loop_new(NULL, FALSE);


	ALOGD("Remote Hotspot Trigger started\n");

/*	if(argc != 2){
		printf("usage: %s <device_MAC_address_to_connect>");
		exit(1);
	}
	device_name = default_device_name;
	bt_server_address = argv[1];
*/
	/*
	if(argc < 2) {
		char errMsg[] = "No bluetooth device name, so its name is set as default.";
		printf("%s\n", errMsg);
		ALOGW("%s\n", errMsg);
		device_name = default_device_name;
	} else {
		device_name = argv[1];
	}
	*/

	device_name = default_device_name;
	bt_server_address = start_address;

	// Initialize bluetooth
	error = rhtf_initialize_bluetooth(device_name);
	if(error != 0) {
		ret = -2;
		goto error_end_without_socket;
	}
	ALOGD("succeed in rhtf_initialize_bluetooth()\n");

	ret = bt_socket_connect_rfcomm(bt_server_address, service_uuid);
	if(ret != BT_ERROR_NONE)
	{
		LOGE("[%s] bt_socket_connect_rfcomm failed.", __FUNCTION__);
		return -50;
	}
	else
	{
		ALOGI("[%s] bt_socket_connect_rfcomm succeeded. bt_socket_connection_state_changed_cb will be called.", __FUNCTION__);
	
	}
	

	// If succeed to accept a connection, start a main loop.
	g_main_loop_run(gMainLoop);

	ALOGI("Server is terminated successfully\n");

error_end_with_socket:
	// Finalized bluetooth
	rhtf_finalize_bluetooth_socket();

error_end_without_socket:
	rhtf_finalize_bluetooth();
	return ret;
}

//! End of a file
