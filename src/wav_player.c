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
#include <wav_player.h>
#include <stdlib.h>


typedef struct _cb_data_{
	wav_player_playback_completed_cb cb;
	void * uesr_data;
	int id;
} _cb_data;



static int __convert_wav_player_error_code(const char *func, int code){
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
			
	}	
	LOGE( "[%s] %s(0x%08x)",func, errorstr, ret);
	return ret;
}


static void __internal_complete_cb(void *user_data){
	_cb_data * cb_data = (_cb_data*)user_data;
	if(!cb_data)
		return;
	
	if( cb_data->cb )
		cb_data->cb(cb_data->id, cb_data->uesr_data);
	free(cb_data);	
}


int wav_player_start(const char *path,  sound_type_e type , wav_player_playback_completed_cb cb, void *user_data,  int * id){
	int ret ; 
	int player;
	char m_path[PATH_MAX];
	void (*_completed_cb)(void *);
	_completed_cb = NULL;
	_cb_data *cb_data = NULL;
	
	
	if( path == NULL)
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_PARAMETER);

	if( type < SOUND_TYPE_SYSTEM || type >  SOUND_TYPE_CALL )
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_PARAMETER);

	m_path[0] = '\0';
	if( path[0] != '/' ){

		if( getcwd(m_path, PATH_MAX) != NULL){
			strncat(m_path, "/",PATH_MAX-strlen(m_path) );
		}
	}
	strncat(m_path, path, PATH_MAX-strlen(m_path));

	if( cb ){
		_completed_cb = __internal_complete_cb;
		cb_data = (_cb_data *)malloc(sizeof(_cb_data));
		if(cb_data == NULL )
			return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_OPERATION);
		cb_data->cb = cb;
		cb_data->uesr_data = user_data;		
	}
	
	ret = mm_sound_play_sound(m_path, type, _completed_cb , cb_data, &player);
	if( ret == 0 && id != NULL)
		*id = player;		
	if( ret == 0 && cb_data )
		cb_data->id = player;
	
	if( ret != 0 && cb_data != NULL)
		free(cb_data);

			
	return __convert_wav_player_error_code(__func__, ret);
}

int wav_player_stop(int id){
	return __convert_wav_player_error_code(__func__, mm_sound_stop_sound(id));
}

