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




#include <tet_api.h>
#include <wav_player.h>
#include <pthread.h>

#define TESTFILE "./test.wav"


#define API_NAME_WAVPL_START "wav_player_start_playing"
#define API_NAME_WAVPL_STOP "wav_player_stop_playing"
#define API_NAME_WAVPL_CALLBACK "wav_player_stop_callback"


#define MY_ASSERT( fun , test , msg ) \
{\
	if( !test ) \
		dts_fail(fun , msg ); \
}		

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};



struct
{
	int proc_result;
	int proc_id;
} glob;



static void startup(void);
static void cleanup(void);

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

// basic tests for start and stop of play
static void utc_wav_player_start_p(void);
static void utc_wav_player_start_n(void);
static void utc_wav_player_stop_p(void);
static void utc_wav_player_stop_n(void);

static void utc_wav_player_callback_p(void);
static void utc_wav_player_callback2_p(void);


// more sophisticated tests for input parameters
static void utc_wav_player_wrong_param_n(void);
static void utc_wav_player_wrong_param2_n(void);
static void utc_wav_player_wrong_param3_n(void);


struct tet_testlist tet_testlist[] = {
	{ utc_wav_player_start_p , 1 },
	{ utc_wav_player_start_n , 2 },
	{ utc_wav_player_stop_p, 3 },
	{ utc_wav_player_stop_n, 4 },
	{ utc_wav_player_callback_p, 5 },
	{ utc_wav_player_callback2_p, 6 },	
	{ utc_wav_player_wrong_param_n, 7 },
	{ utc_wav_player_wrong_param2_n, 8 },
	{ utc_wav_player_wrong_param3_n, 9 },
	{ NULL, 0 },
};


static void startup(void)
{
	/* start of TC */
}

static void cleanup(void)
{
	/* end of TC */
}



//**********************************************************************/


/**
 * @brief all parameters fine, play music
 */
static void utc_wav_player_start_p(void)
{
	int ret;
	ret = wav_player_start(TESTFILE, SOUND_TYPE_MEDIA, NULL, NULL, NULL);
	dts_check_eq( API_NAME_WAVPL_START, ret, WAV_PLAYER_ERROR_NONE );
}



//**********************************************************************/



/**
 * @brief lack of sound file path
 */
static void utc_wav_player_start_n(void)
{
	int ret;
	ret = wav_player_start(NULL, SOUND_TYPE_MEDIA, NULL, NULL, NULL);
	dts_check_ne( API_NAME_WAVPL_START, ret, WAV_PLAYER_ERROR_NONE );
}



//**********************************************************************/



/**
 * @brief stop playing, all parameters fine
 */
static void utc_wav_player_stop_p(void)
{
	int ret;
	int id;
	ret = wav_player_start(TESTFILE, SOUND_TYPE_MEDIA, NULL, NULL, &id);
	ret = wav_player_stop(id);
	dts_check_eq( API_NAME_WAVPL_STOP, ret, WAV_PLAYER_ERROR_NONE );
}



//**********************************************************************/



/**
 * @brief stop playing, wrong id of playing process
 */
static void utc_wav_player_stop_n(void)
{
	int ret;
	ret = wav_player_stop(-1);
	dts_check_ne( API_NAME_WAVPL_STOP, ret, WAV_PLAYER_ERROR_NONE );
}



//**********************************************************************/


//**********************************************************************/



/**
 * @brief wrong parameter: wrong file name
 */
static void utc_wav_player_wrong_param_n(void)
{
	int ret;
	char file_name[100] = { '\0' };
	snprintf( file_name, 100 ,  "%s", TESTFILE );
	snprintf( file_name, 100 ,  "%s", "something" );
	
	ret = wav_player_start( file_name, SOUND_TYPE_MEDIA, NULL, NULL, NULL );
	dts_check_ne( API_NAME_WAVPL_START, ret, WAV_PLAYER_ERROR_NONE );
}




//**********************************************************************/



/**
 * @brief wrong parameter: wrong media file type
 */
static void utc_wav_player_wrong_param2_n(void)
{
	int ret;

	ret = wav_player_start( TESTFILE, SOUND_TYPE_CALL + 1, NULL, NULL, NULL );
	dts_check_ne( API_NAME_WAVPL_START, ret, WAV_PLAYER_ERROR_NONE );
}



//**********************************************************************/



/**
 * @brief wrong parameter: wrong media file type
 */
static void utc_wav_player_wrong_param3_n(void)
{
	int ret;

	ret = wav_player_start( TESTFILE, -1, NULL, NULL, NULL );
	dts_check_ne( API_NAME_WAVPL_START, ret, WAV_PLAYER_ERROR_NONE );
}


//************************************************************************/

void _playback_completed_cb(int id, void *user_data){
	int *result = (int*)user_data;
	*result = 1;
}

static void utc_wav_player_callback_p(void)
{
	int ret ;
	int result = 0;
	
	ret = wav_player_start( TESTFILE, SOUND_TYPE_MEDIA, _playback_completed_cb, &result, NULL );	
	sleep(6);
	dts_check_eq( API_NAME_WAVPL_CALLBACK, result , 1 );
	
}

//************************************************************************/

void _playback_completed_cb2(int id, void *user_data){
	int *result = (int*)user_data;
	*result = id;
}


static void utc_wav_player_callback2_p(void){

	int ret ;
	int callback_id = 0;
	int id;
	
	ret = wav_player_start( TESTFILE, SOUND_TYPE_MEDIA, _playback_completed_cb2, &callback_id, &id );	

	sleep(6);
	dts_check_eq( API_NAME_WAVPL_CALLBACK, callback_id , id );

	
}
