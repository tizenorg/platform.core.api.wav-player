/*
* Copyright (c) 2011-2016 Samsung Electronics Co., Ltd All Rights Reserved
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

#ifndef __TIZEN_MEDIA_WAV_PLAYER_PRIVATE_H__
#define __TIZEN_MEDIA_WAV_PLAYER_PRIVATE_H__

#include <tizen.h>
#include <sound_manager.h>
#include <sound_manager_internal.h>
#include <wav_player.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define WAV_PLAYER_ERROR_CLASS          TIZEN_ERROR_MULTIMEDIA_CLASS | 0x50

/**
 * @file wav_player_private.h
 * @brief This file contains the WAV player API
 */

/**
 * @addtogroup CAPI_MEDIA_WAV_PLAYER_MODULE
 * @{
 */


typedef struct _cb_data_ {
	wav_player_playback_completed_cb cb;
	void * user_data;
} _cb_data;

/**
 * @}
 */

int _convert_wav_player_error_code(const char *func, int code);
void _internal_complete_cb(void *user_data, int id);
int _start_with_stream_info(const char *path, sound_stream_info_h stream_info, int loop_count, wav_player_playback_completed_cb cb, void *user_data, int *id);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_WAV_PLAYER_PRIVATE_H__ */
