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
#include "wav_player.h"
#include "wav_player_private.h"

int wav_player_start_loop(const char *path, sound_stream_info_h stream_info, unsigned int loop_count, wav_player_playback_completed_cb callback, void *user_data, int *id)
{
	return _start_with_stream_info(path, stream_info, loop_count, callback, user_data, id);
}
