/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <gio/gio.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dlog.h>

#include "serial.h"
#include "serial_private.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CAPI_NETWORK_SERIAL"

#define DBG(fmt, args...) SLOGD(fmt, ##args)
#define ERR(fmt, args...) SLOGE(fmt, ##args)

#define SERIAL_SOCKET_PATH	"/tmp/.dr_common_stream"
#define SERIAL_BUF_SIZE		65536
#define SERIAL_INTERFACE		"User.Data.Router.Introspectable"
#define SERIAL_STATUS_SIGNAL	"serial_status"

GDBusConnection *dbus_connection = NULL;


/*
 *  Internal Functions
 */
static gboolean __g_io_client_handler(GIOChannel *io, GIOCondition cond, void *data)
{
	int fd;
	serial_s *pHandle = (serial_s *)data;
	if (pHandle == NULL)
		return FALSE;

	if (pHandle->data_handler.callback) {
		char buffer[SERIAL_BUF_SIZE] = { 0 };
		int len = 0;
		fd = g_io_channel_unix_get_fd(io);
		len = recv(fd, buffer, SERIAL_BUF_SIZE, 0);
		if(len <= 0) {
			ERR("Error occured or the peer is shutdownd. [%d]\n", len);
			((serial_state_changed_cb)pHandle->state_handler.callback)
					(SERIAL_ERROR_NONE,
					SERIAL_STATE_CLOSED,
					pHandle->state_handler.user_data);
			return FALSE;
		}

		((serial_data_received_cb)pHandle->data_handler.callback)
			(buffer, len, pHandle->data_handler.user_data);
	}
	return TRUE;
}

static void __init_client_giochannel(void *data)
{
	GIOChannel *io;
	serial_s *pHandle = (serial_s *)data;
	if (pHandle == NULL)
		return;

	io = g_io_channel_unix_new(pHandle->client_socket);
	g_io_channel_set_close_on_unref(io, TRUE);
	pHandle->g_watch_id = g_io_add_watch(io,
				G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
				__g_io_client_handler, pHandle);
	g_io_channel_unref(io);
	return;
}

static int __connect_to_serial_server(void *data)
{
	int client_socket = -1;
	struct sockaddr_un	server_addr;
	serial_s *pHandle = (serial_s *)data;
	if (pHandle == NULL) {
		ERR("Invalid parameter\n");
		return -1;
	}

	client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (client_socket < 0) {
		ERR("Create socket failed\n");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	g_strlcpy(server_addr.sun_path, SERIAL_SOCKET_PATH, sizeof(server_addr.sun_path));

	if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		ERR("Connect failed\n");
		close(client_socket);
		return -1;
	}
	pHandle->client_socket = client_socket;

	__init_client_giochannel(pHandle);

	return client_socket;
}

static void __serial_status_signal_cb(GDBusConnection *connection,
					const gchar *sender_name,
					const gchar *object_path,
					const gchar *interface_name,
					const gchar *signal_name,
					GVariant *parameters,
					gpointer user_data)
{
	static int socket = -1;
	int status = 0;

	if (strcasecmp(signal_name, SERIAL_STATUS_SIGNAL) == 0) {
		g_variant_get(parameters, "(i)", &status);

		DBG("serial_status : %d\n", status);

		serial_s *pHandle = (serial_s *)user_data;
		if (status == SERIAL_OPENED) {
			socket = __connect_to_serial_server(pHandle);
			if (socket < 0) {
				((serial_state_changed_cb)pHandle->state_handler.callback)
						(SERIAL_ERROR_OPERATION_FAILED,
						SERIAL_STATE_OPENED,
						pHandle->state_handler.user_data);
				return;
			}

			((serial_state_changed_cb)pHandle->state_handler.callback)
					(SERIAL_ERROR_NONE,
					SERIAL_STATE_OPENED,
					pHandle->state_handler.user_data);
		} else if (status == SERIAL_CLOSED) {
			if (socket < 0)
				return;

			((serial_state_changed_cb)pHandle->state_handler.callback)
					(SERIAL_ERROR_NONE,
					SERIAL_STATE_CLOSED,
					pHandle->state_handler.user_data);
		}
	}
}

int __send_serial_ready_done_signal(void)
{
	GError *error = NULL;
	gboolean ret;

	ret =  g_dbus_connection_emit_signal(dbus_connection, NULL,
				"/Network/Serial",
				"Capi.Network.Serial",
				"ready_for_serial",
				g_variant_new("(s)", "OK"),
				&error);
	if (!ret) {
		if (error != NULL) {
			ERR("D-Bus API failure: errCode[%x], message[%s]",
				error->code, error->message);
			g_clear_error(&error);
			return SERIAL_ERROR_OPERATION_FAILED;
		}
	}

	DBG("Serial is ready");

	return SERIAL_ERROR_NONE;
}

static int __serial_set_state_changed_cb(serial_h serial, void *callback, void *user_data)
{
	if (!serial) {
		ERR("Invalid parameter\n");
		return SERIAL_ERROR_INVALID_PARAMETER;
	}

	serial_s *pHandle = (serial_s *)serial;

	if (callback) {
		pHandle->state_handler.callback = callback;
		pHandle->state_handler.user_data = user_data;
	} else {
		pHandle->state_handler.callback = NULL;
		pHandle->state_handler.user_data = NULL;
	}

	return SERIAL_ERROR_NONE;
}

static int __serial_set_data_received_cb(serial_h serial, void *callback, void *user_data)
{
	if (!serial) {
		ERR("Invalid parameter\n");
		return SERIAL_ERROR_INVALID_PARAMETER;
	}

	serial_s *pHandle = (serial_s *)serial;

	if (callback) {
		pHandle->data_handler.callback = callback;
		pHandle->data_handler.user_data = user_data;
	} else {
		pHandle->data_handler.callback = NULL;
		pHandle->data_handler.user_data = NULL;
	}

	return SERIAL_ERROR_NONE;
}



/*
 *  Public Functions
 */
int serial_create(serial_h *serial)
{
	DBG("%s\n", __FUNCTION__);

	GError *err = NULL;
	serial_s *pHandle = NULL;

	if (serial == NULL)
		return SERIAL_ERROR_INVALID_PARAMETER;

	pHandle = (serial_s *)g_try_malloc0(sizeof(serial_s));
	if (pHandle == NULL)
		return SERIAL_ERROR_OUT_OF_MEMORY;

	pHandle->serial_sig_id = -1;
	dbus_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if(!dbus_connection) {
		ERR(" DBUS get failed");
		g_error_free(err);
		return SERIAL_ERROR_OPERATION_FAILED;
	}

	/* Add the filter for network client functions */
	pHandle->serial_sig_id  = g_dbus_connection_signal_subscribe(dbus_connection, NULL,
			SERIAL_INTERFACE,
			SERIAL_STATUS_SIGNAL,
			NULL, NULL, 0,
			__serial_status_signal_cb, pHandle, NULL);

	*serial = (serial_h)pHandle;

	return SERIAL_ERROR_NONE;
}


int serial_open(serial_h serial)
{
	DBG("%s\n", __FUNCTION__);

	if (!serial) {
		ERR("Invalid parameter\n");
		return SERIAL_ERROR_INVALID_PARAMETER;
	}

	return __send_serial_ready_done_signal();
}

int serial_close(serial_h serial)
{
	DBG("%s\n", __FUNCTION__);

	if (!serial) {
		ERR("Invalid parameter\n");
		return SERIAL_ERROR_INVALID_PARAMETER;
	}

	serial_s *pHandle = (serial_s *)serial;

	if (pHandle->client_socket >= 0) {
		if (close(pHandle->client_socket) < 0)
			return SERIAL_ERROR_OPERATION_FAILED;

		pHandle->client_socket = -1;

		return SERIAL_ERROR_NONE;
	} else {
		return SERIAL_ERROR_INVALID_OPERATION;
	}
}

int serial_destroy(serial_h serial)
{
	DBG("%s\n", __FUNCTION__);

	if (!serial) {
		ERR("Invalid parameter\n");
		return SERIAL_ERROR_INVALID_PARAMETER;
	}

	serial_s *pHandle = (serial_s *)serial;

	if (pHandle->serial_sig_id != -1) {
		g_dbus_connection_signal_unsubscribe(dbus_connection,
				pHandle->serial_sig_id);
		pHandle->serial_sig_id = -1;
	}

	if (pHandle->g_watch_id > 0) {
		g_source_remove(pHandle->g_watch_id);
		pHandle->g_watch_id = 0;
	}

	if (pHandle->client_socket >= 0) {
		close(pHandle->client_socket);
		pHandle->client_socket = -1;
	}

	g_free(pHandle);
	serial = NULL;

	return SERIAL_ERROR_NONE;
}

int serial_write(serial_h serial, const char *data, int data_length)
{
	if (!serial) {
		ERR("Invalid parameter\n");
		return SERIAL_ERROR_INVALID_PARAMETER;
	}
	int ret;
	serial_s *pHandle = (serial_s *)serial;

	ret = send(pHandle->client_socket, data, data_length, MSG_EOR);
	if (ret == -1) {
		ERR("Send failed. ");
		return SERIAL_ERROR_OPERATION_FAILED;
	}

	return ret;
}

int serial_set_state_changed_cb(serial_h serial, serial_state_changed_cb callback, void *user_data)
{
	DBG("%s\n", __FUNCTION__);

	if (!serial || !callback) {
		ERR("Invalid parameter\n");
		return SERIAL_ERROR_INVALID_PARAMETER;
	}

	return (__serial_set_state_changed_cb(serial, callback, user_data));
}

int serial_unset_state_changed_cb(serial_h serial)
{
	DBG("%s\n", __FUNCTION__);

	if (!serial) {
		ERR("Invalid parameter\n");
		return SERIAL_ERROR_INVALID_PARAMETER;
	}

	return (__serial_set_state_changed_cb(serial, NULL, NULL));
}

int serial_set_data_received_cb(serial_h serial, serial_data_received_cb callback, void *user_data)
{
	DBG("%s\n", __FUNCTION__);

	if (!serial || !callback) {
		ERR("Invalid parameter\n");
		return SERIAL_ERROR_INVALID_PARAMETER;
	}

	return (__serial_set_data_received_cb(serial, callback, user_data));
}

int serial_unset_data_received_cb(serial_h serial)
{
	DBG("%s\n", __FUNCTION__);

	if (!serial) {
		ERR("Invalid parameter\n");
		return SERIAL_ERROR_INVALID_PARAMETER;
	}

	return (__serial_set_data_received_cb(serial, NULL, NULL));
}
