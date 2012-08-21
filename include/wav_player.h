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




#ifndef __TIZEN_MEDIA_WAV_PLAYER_H__
#define __TIZEN_MEDIA_WAV_PLAYER_H__

#include <tizen.h>
#include <sound_manager.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define WAV_PLAYER_ERROR_CLASS          TIZEN_ERROR_MULTIMEDIA_CLASS | 0x50

/**
 * @file wav_player.h
 * @brief This file contains the WAV player API
 */

/**
 * @addtogroup CAPI_MEDIA_WAV_PLAYER_MODULE
 * @{
 */

/**
 * @brief Enumerations of error code for WAV player.
 */
typedef enum
{
    WAV_PLAYER_ERROR_NONE        = TIZEN_ERROR_NONE,                      /**< Successful */
    WAV_PLAYER_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,       /**< Invalid parameter */
    WAV_PLAYER_ERROR_INVALID_OPERATION = TIZEN_ERROR_INVALID_OPERATION,       /**< Invalid operation */
    WAV_PLAYER_ERROR_FORMAT_NOT_SUPPORTED = WAV_PLAYER_ERROR_CLASS | 0x01 			  /**< Not supported format */
} wav_player_error_e;

/**
 * @}
 */


/**
 * @addtogroup CAPI_MEDIA_WAV_PLAYER_MODULE
 * @{
 */

/**
 * @brief Called when a WAV file has finished playing.
 * @details This callback is not invoked by calling wav_player_stop().
 *
 * @param[in] id	The completed wav player ID
 * @param[in] user_data	The user data passed from the callback registration function
 * @see wav_player_start()
 */
typedef void (*wav_player_playback_completed_cb)(int id, void *user_data);


/**
 * @brief Plays a WAV file.
 * @param[in] path	The WAV file path to play
 * @param[in] type	The sound type
 * @param[in] callback	The callback function to invoke when a WAV file is finished playing
 * @param[in] user_data	The user data to be passed to the callback function
 * @param[out] id	The WAV player ID ( can be set to NULL )
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #WAV_PLAYER_ERROR_NONE Successful
 * @retval #WAV_PLAYER_ERROR_INVALID_PARAMETER Invalid parameter 
 * @retval #WAV_PLAYER_ERROR_INVALID_OPERATION Invalid operation
 *
 * @post	It invokes wav_player_playback_completed_cb() when a WAV file has finished playing.
 * @see wav_player_stop()
 * @see wav_player_playback_completed_cb()
 */
int wav_player_start(const char *path, sound_type_e type, wav_player_playback_completed_cb callback, void *user_data, int *id);

/**
 * @brief Stops playing the WAV file.
 *
 * @param[in] id	The WAV player ID to stop
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #WAV_PLAYER_ERROR_NONE Successful
 * @retval #WAV_PLAYER_ERROR_INVALID_PARAMETER Invalid parameter 
 * @retval #WAV_PLAYER_ERROR_INVALID_OPERATION Invalid operation  
 *
 * @see	wav_player_start()
 */
int wav_player_stop(int id);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_WAV_PLAYER_H__ */
