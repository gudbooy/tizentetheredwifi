#include <vconf.h>
#include <vconf-keys.h>
#include <net_connection.h>

#include "common.h"
#include "wlan_manager.h"
#include "connection_manager.h"

static connection_h connection = NULL;

static void _connection_type_changed_cb(connection_type_e type, void* user_data)
{
	__COMMON_FUNC_ENTER__;

	if (wlan_manager_state_get() != WLAN_MANAGER_CONNECTED)
		return;

	if (type == CONNECTION_TYPE_CELLULAR ||
			type == CONNECTION_TYPE_WIFI)
		wlan_manager_scanned_profile_refresh();

	__COMMON_FUNC_EXIT__;
}

gboolean connection_manager_create(void)
{
	__COMMON_FUNC_ENTER__;

	int ret;

	ret = connection_create(&connection);
	if (ret != CONNECTION_ERROR_NONE)
		return FALSE;

	ret = connection_set_type_changed_cb(connection, _connection_type_changed_cb, NULL);
	if (ret != CONNECTION_ERROR_NONE)
		return FALSE;

	__COMMON_FUNC_EXIT__;
	return TRUE;
}

gboolean connection_manager_destroy(void)
{
	__COMMON_FUNC_ENTER__;

	int ret;

	if (connection == NULL)
		return FALSE;

	ret = connection_destroy(connection);
	if (ret != CONNECTION_ERROR_NONE)
		return FALSE;

	connection = NULL;

	__COMMON_FUNC_EXIT__;
	return TRUE;
}

gboolean connection_manager_is_wifi_connection_used(void)
{
	__COMMON_FUNC_ENTER__;

	int ret;
	connection_type_e type;

	if (connection == NULL)
		return FALSE;

	ret = connection_get_type(connection, &type);
	if (ret != CONNECTION_ERROR_NONE)
		return FALSE;

	if (type != CONNECTION_TYPE_WIFI)
		return FALSE;

	__COMMON_FUNC_EXIT__;
	return TRUE;
}
