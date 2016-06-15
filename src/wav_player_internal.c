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


#define LOG_TAG "CAPI_MEDIA_WAV_PLAYER"

#include <sound_manager.h>
#include <sound_manager_internal.h>
#include <mm_sound.h>
#include <mm_sound_private.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <dlog.h>
#include "wav_player.h"
#include "wav_player_private.h"
#include <stdlib.h>

int wav_player_start_loop(const char *path, sound_stream_info_h stream_info, unsigned int loop_count, wav_player_playback_completed_cb callback, void *user_data, int *id)
{
	int ret = MM_ERROR_NONE;
	int player = -1;
	char m_path[PATH_MAX];
	void (*_completed_cb)(void *, int);
	_completed_cb = NULL;
	_cb_data *cb_data = NULL;
	char *stream_type = NULL;
	int stream_id;
	bool result = false;

	if (path == NULL || stream_info == NULL)
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_PARAMETER);

	ret = sound_manager_is_available_stream_information(stream_info, NATIVE_API_WAV_PLAYER, &result);
	if (!result)
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_NOT_SUPPORTED_TYPE);

	ret = sound_manager_get_type_from_stream_information(stream_info, &stream_type);
	if (ret)
		return __convert_wav_player_error_code(__func__, ret);
	ret = sound_manager_get_index_from_stream_information(stream_info, &stream_id);
	if (ret)
		return __convert_wav_player_error_code(__func__, ret);

	m_path[0] = '\0';
	if (path[0] != '/') {

		if (getcwd(m_path, PATH_MAX) != NULL)
			strncat(m_path, "/", PATH_MAX-strlen(m_path));
	}
	strncat(m_path, path, PATH_MAX-strlen(m_path));

	if (callback) {
		_completed_cb = __internal_complete_cb;
		cb_data = (_cb_data *)malloc(sizeof(_cb_data));
		if (cb_data == NULL)
			return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_OPERATION);
		cb_data->cb = callback;
		cb_data->user_data = user_data;
	}

	ret = mm_sound_play_sound_with_stream_info(m_path, stream_type, stream_id, loop_count, _completed_cb , cb_data, &player);

	if (ret == 0 && id != NULL)
		*id = player;

	if (ret != 0 && cb_data != NULL)
		free(cb_data);

	return __convert_wav_player_error_code(__func__, ret);
}
