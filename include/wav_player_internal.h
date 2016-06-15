/*
* Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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




#ifndef __TIZEN_MEDIA_WAV_PLAYER_INTERNAL_H__
#define __TIZEN_MEDIA_WAV_PLAYER_INTERNAL_H__

#include <tizen.h>
#include <sound_manager.h>
#include <wav_player.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file wav_player_internal.h
 * @brief This file contains the Wav Player Internal API
 */

/**
 * @addtogroup CAPI_MEDIA_WAV_PLAYER_MODULE
 * @{
 */

/**
 * @internal
 * @brief Plays a WAV file with loop.
 * @since_tizen 3.0
 *
 * @param[in] path	The file path to play
 * @param[in] stream_info	The sound stream information handle
 * @param[in] loop_count	The number of loop (@c 0 = infinite)
 * @param[in] callback	The callback function to be invoked when a WAV file is no longer being played
 * @param[in] user_data	The user data to be passed to the callback function
 * @param[out] id	The WAV player ID (can be set to @c NULL)
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #WAV_PLAYER_ERROR_NONE Successful
 * @retval #WAV_PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #WAV_PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #WAV_PLAYER_ERROR_FORMAT_NOT_SUPPORTED Not supported format
 * @retval #WAV_PLAYER_ERROR_NOT_SUPPORTED_TYPE Not supported stream type
 *
 * @post	It invokes wav_player_playback_completed_cb() when a WAV file is no longer being played.
 * @see wav_player_stop()
 * @see wav_player_playback_completed_cb()
 * @see sound_manager_create_stream_information()
 * @see sound_manager_destroy_stream_information()
 */
int wav_player_start_loop(const char *path, sound_stream_info_h stream_info, unsigned int loop_count, wav_player_playback_completed_cb callback, void *user_data, int *id);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_WAV_PLAYER_INTERNAL_H__ */
