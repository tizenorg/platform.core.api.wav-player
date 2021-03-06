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



#include <stdio.h>
#include <wav_player.h>
#include <glib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

static GMainLoop *g_mainloop = NULL;
static GThread *event_thread;

gpointer GmainThread(gpointer data){
	g_mainloop = g_main_loop_new (NULL, 0);
	g_main_loop_run (g_mainloop);

	return NULL;
}

void _player_stop_cb(int id, void *user_data){
	printf("complete id = %d,%d\n", id, (int)user_data);
}



void wav_play_test(){
	int ret=0;
	int id;
	int i;
	for(i =0 ; i < 100; i++){	
		ret = wav_player_start("test.wav", SOUND_TYPE_MEDIA, _player_stop_cb,(void*)i, &id);
		printf("wav_player_start(%d)(id=%d) ret = %d\n",i,id, ret);

	}
}

void audio_io_test(){
	
}

int main(int argc, char**argv)
{
	if( !g_thread_supported() )
	{
		g_thread_init(NULL);
	}

	GError *gerr = NULL;
	event_thread = g_thread_create(GmainThread, NULL, 1, &gerr);

	wav_play_test();
	return 0;
}
