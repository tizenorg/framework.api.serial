/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __TIZEN_NETWORK_SERIAL_H__
#define __TIZEN_NETWORK_SERIAL_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
* @addtogroup CAPI_NETWORK_SERIAL_MODULE
* @{
*/


/**
* @brief The handle of serial
*/
typedef void* serial_h;

/**
* @brief Enumerations for the state of serial
*/
typedef enum
{
    SERIAL_STATE_OPENED = 0,  /**< Opened */
    SERIAL_STATE_CLOSED = 1,  /**< Closed */
} serial_state_e;

/**
* @brief Enumerations for error code
*/
typedef enum
{
    SERIAL_ERROR_NONE = TIZEN_ERROR_NONE,  /**< Successful */
    SERIAL_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY,  /**< Out of memory */
    SERIAL_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,  /**< Invalid parameter */
    SERIAL_ERROR_INVALID_OPERATION = TIZEN_ERROR_INVALID_OPERATION,  /**< Invalid operation */
    SERIAL_ERROR_OPERATION_FAILED = TIZEN_ERROR_NETWORK_CLASS|0x0601,  /**< Operation failed */
} serial_error_e;

/**
* @brief  Called when you receive data.
* @remarks  @a data is valid only in this function.
* @param[in]  data  The received data
* @param[in]  user_data  The user data passed from serial_set_data_received_cb()
* @pre  If you register callback function using serial_set_data_received_cb(), this will be invoked when you receive data.
* @see  serial_set_data_received_cb()
* @see  serial_unset_data_received_cb()
*/
typedef bool (*serial_data_received_cb)(const char *data, int data_length, void *user_data);

/**
* @brief  Called when the state of serial is changed.
* @param[in]  result  The result of opening the serial
* @param[in]  state  The state of serial
* @param[in]  user_data  The user data passed from serial_set_state_changed_cb()
* @pre  If you register callback function using serial_set_state_changed_cb(), this will be invoked when the state of serial is changed.
* @see	serial_set_state_changed_cb()
* @see	serial_unset_state_changed_cb()
*/
typedef void (*serial_state_changed_cb)(serial_error_e result, serial_state_e state, void *user_data);


/**
* @brief  Creates the handle of serial.
* @param[out]  serial  The serial handle
* @return  0 on success, otherwise a negative error value.
* @retval  #SERIAL_ERROR_NONE  Successful
* @retval  #SERIAL_ERROR_OUT_OF_MEMORY  Out of memory
* @retval  #SERIAL_ERROR_INVALID_PARAMETER  Invalid parameter
* @retval  #SERIAL_ERROR_OPERATION_FAILED  Operation failed
* @see  serial_destroy()
*/
int serial_create(serial_h *serial);

/**
* @brief  Destroys the handle of serial.
* @param[in]  serial  The serial handle
* @return  0 on success, otherwise a negative error value.
* @retval  #SERIAL_ERROR_NONE  Successful
* @retval  #SERIAL_ERROR_INVALID_PARAMETER  Invalid parameter
* @see  serial_create()
*/
int serial_destroy(serial_h serial);

/**
* @brief  Opens the serial.
* @param[in]  serial  The serial handle
* @return  0 on success, otherwise a negative error value.
* @retval  #SERIAL_ERROR_NONE  Successful
* @retval  #SERIAL_ERROR_INVALID_PARAMETER  Invalid parameter
* @retval  #SERIAL_ERROR_OPERATION_FAILED  Operation failed
* @retval  #SERIAL_ERROR_INVALID_OPERATION  Invalid operation
* @post  When a serial is opened, serial_state_changed_cb() will be called with #SERIAL_STATE_OPENED state
* @see  serial_close()
*/
int serial_open(serial_h serial);

/**
* @brief  Closes the serial.
* @param[in]  serial  The serial handle
* @return  0 on success, otherwise a negative error value.
* @retval  #SERIAL_ERROR_NONE  Successful
* @retval  #SERIAL_ERROR_INVALID_PARAMETER  Invalid parameter
* @retval  #SERIAL_ERROR_OPERATION_FAILED  Operation failed
* @retval  #SERIAL_ERROR_INVALID_OPERATION  Invalid operation
* @pre  The serial must be opened.
* @post  When a serial is closed, serial_state_changed_cb() will be called with #SERIAL_STATE_CLOSED state
* @see  serial_open()
*/
int serial_close(serial_h serial);

/**
* @brief  Writes data to serial server.
* @param[in]  serial  The serial handle
* @param[in]  data The data to send
* @param[in]  data_length The length of data to send
* @return  The number of characters sent on success, otherwise a negative error value.
* @retval  #SERIAL_ERROR_INVALID_PARAMETER  Invalid parameter
* @retval  #SERIAL_ERROR_OPERATION_FAILED  Operation failed
* @pre  The serial must be opened.
* @see  serial_open()
*/
int serial_write(serial_h serial, const char *data, int data_length);

/**
* @brief  Register a callback function to be invoked when you receive data.
* @param[in]  serial  The serial handle
* @param[in]  callback  The callback function to be invoked
* @param[in]  user_data  The user data to be passed to the callback function
* @return  0 on success, otherwise a negative error value.
* @retval  #SERIAL_ERROR_NONE  Successful
* @retval  #SERIAL_ERROR_INVALID_PARAMETER  Invalid parameter
* @post  serial_data_received_cb() will be invoked when you receive data.
* @see  serial_unset_data_received_cb()
* @see  serial_data_received_cb()
*/
int serial_set_data_received_cb(serial_h serial, serial_data_received_cb callback, void *user_data);

/**
* @brief  Unregister a callback function to be invoked when you receive data.
* @param[in]  serial  The serial handle
* @return   0 on success, otherwise a negative error value.
* @retval  #SERIAL_ERROR_NONE  Successful
* @retval  #SERIAL_ERROR_INVALID_PARAMETER  Invalid parameter
* @see  serial_set_data_received_cb()
*/
int serial_unset_data_received_cb(serial_h serial);

/**
* @brief  Register a callback function to be invoked when the state of serial is changed.
* @param[in]  serial  The serial handle
* @param[in]  callback  The callback function to be invoked
* @param[in]  user_data  The user data to be passed to the callback function
* @return   0 on success, otherwise a negative error value.
* @retval  #SERIAL_ERROR_NONE  Successful
* @retval  #SERIAL_ERROR_INVALID_PARAMETER  Invalid parameter
* @post  serial_state_changed_cb() will be invoked when the state of serial is changed.
* @see  serial_unset_state_changed_cb()
* @see  serial_state_changed_cb()
*/
int serial_set_state_changed_cb(serial_h serial, serial_state_changed_cb callback, void *user_data);

/**
* @brief  Unregister a callback function to be invoked when the state of serial is changed.
* @param[in]  serial  The serial handle
* @return   0 on success, otherwise a negative error value.
* @retval  #SERIAL_ERROR_NONE  Successful
* @retval  #SERIAL_ERROR_INVALID_PARAMETER  Invalid parameter
* @see  serial_set_state_changed_cb()
*/
int serial_unset_state_changed_cb(serial_h serial);

/**
* @}
*/


#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_NETWORK_SERIAL_H__ */
