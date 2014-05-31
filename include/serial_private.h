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


#ifndef __TIZEN_NETWORK_SERIAL_PRIVATE_H__
#define __TIZEN_NETWORK_SERIAL_PRIVATE_H__

#include <dbus/dbus-glib.h>

#include "serial.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @internal
 * @brief Serial callback.
 */
 typedef struct _serial_event_s
 {
	 const void *callback;
	 void *user_data;
 } serial_event_s;


 /**
  * @internal
  * @brief Serial handle
  */
 typedef struct _serial_s{
	DBusGConnection *client_bus;
	int client_socket;
	guint g_watch_id;
	serial_event_s state_handler;
	serial_event_s data_handler;
	void *handle;
}serial_s;

/**
 * @internal
 * @brief dbus signal from socket server
 */
typedef enum {
	SERIAL_CLOSED,		/* Serial session closed */
	SERIAL_OPENED,		/* Serial session opened */
}serial_event_e;


#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_NETWORK_SERIAL_PRIVATE_H__ */
