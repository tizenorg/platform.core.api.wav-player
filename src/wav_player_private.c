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


#define LOG_TAG "CAPI_MEDIA_WAV_PLAYER"

#include <mm_sound.h>
#include <mm_sound_private.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <dlog.h>
#include <stdlib.h>
#include "wav_player_private.h"

int __convert_wav_player_error_code(const char *func, int code)
{
	int ret = WAV_PLAYER_ERROR_INVALID_OPERATION;
	char *errorstr = NULL;
	switch(code)
	{
		case MM_ERROR_NONE:
			ret = WAV_PLAYER_ERROR_NONE;
			errorstr = "ERROR_NONE";
			break;
		case MM_ERROR_INVALID_ARGUMENT:
		case MM_ERROR_SOUND_INVALID_POINTER:
		case WAV_PLAYER_ERROR_INVALID_PARAMETER:
			ret = WAV_PLAYER_ERROR_INVALID_PARAMETER;
			errorstr = "INVALID_PARAMETER";
			break;
		case MM_ERROR_SOUND_INTERNAL:
			ret = WAV_PLAYER_ERROR_INVALID_OPERATION;
			errorstr = "INVALID_OPERATION";
			break;
		case MM_ERROR_SOUND_UNSUPPORTED_MEDIA_TYPE:
			ret = WAV_PLAYER_ERROR_FORMAT_NOT_SUPPORTED;
			errorstr = "FORMAT_NOT_SUPPORTED";
			break;
		default:
			ret = WAV_PLAYER_ERROR_INVALID_OPERATION;
			errorstr = "INVALID_OPERATION";
			break;
	}
	LOGE( "[%s] %s(0x%08x)",func, errorstr, ret);
	return ret;
}


void __internal_complete_cb(void *user_data, int id)
{
	_cb_data * cb_data = (_cb_data*)user_data;
	if(!cb_data)
		return;

	if( cb_data->cb ){
		LOGD( "user callback for handle %d call", id);
		cb_data->cb(id, cb_data->user_data);
	}
	free(cb_data);
}

