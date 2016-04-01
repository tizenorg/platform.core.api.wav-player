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
#include <getopt.h>

#define FILE_PATH_MAX   256
#define DEFAULT_FILE    "test.wav"

static GMainLoop *g_mainloop = NULL;
int id;

void help()
{
	printf("Usage : ");
	printf("multimedia_wav_player_test [OPTION]\n\n"
		   "  -i, --iterate                 how many time to play\n"
		   "  -f, --file                    file path to play\n"
		   "  -h, --help                help\n");
}

void _player_stop_cb(int id, void *user_data)
{
	printf("complete id = %d,%d\n", id, (int)user_data);

	g_main_loop_quit(g_mainloop);
}

void _sound_stream_focus_state_changed_cb(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data)
{
	printf("stream_info = %p, reason = %d, additional_info = %s, user_data = %p\n", stream_info, (int)reason_for_change, additional_info, user_data);
}


void wav_play_test(const char* file_path, int iterate)
{
	int ret = 0;

	sound_stream_info_h stream_info;

	int i;
	if (iterate <= 0 || file_path == NULL) {
		printf("invalid param : file_path [%s], iterate [%d]\n", file_path, iterate);
		return;
	}

	sound_manager_create_stream_information(SOUND_STREAM_TYPE_SYSTEM, _sound_stream_focus_state_changed_cb, NULL, &stream_info);

	printf("Play Wav, File Path : %s, Iterate : %d\n", file_path, iterate);
	for (i = 0 ; i < iterate; i++) {
	//	ret = wav_player_start(file_path, SOUND_TYPE_MEDIA, _player_stop_cb, (void*)i, &id);
		ret = wav_player_start_with_stream_info(file_path, stream_info, _player_stop_cb, (void*)i, &id);
		printf("wav_player_start(%d)(id=%d) ret = 0x%x\n", i, id, ret);

	}
}

static gboolean _timeout_cb(gpointer user_data)
{
	int ret = wav_player_stop(id);
	printf("wav_player_stop(id=%d) ret = 0x%x\n", id, ret);
	g_main_loop_quit(g_mainloop);
	return FALSE;
}


int main(int argc, char**argv)
{
	int iterate = 1;
	char file_path[FILE_PATH_MAX] = DEFAULT_FILE;

	while (1) {
		int opt;
		int opt_idx = 0;

		static struct option long_options[] = {
			{"iterate"    , required_argument, 0, 'i'},
			{"file"       , required_argument, 0, 'f'},
			{ 0, 0, 0, 0 }
		};

		if ((opt = getopt_long(argc, argv, "i:f:", long_options, &opt_idx)) == -1)
			break;

		switch (opt) {
		case 'f':
			strcpy(file_path, optarg);
			break;
		case 'i':
			iterate = atoi(optarg);
			break;
		case 'h':
		default:
			help();
			return 0;
		}
	}

	g_mainloop = g_main_loop_new(NULL, 0);

	wav_play_test(file_path, iterate);

	g_timeout_add(5000, _timeout_cb, NULL);

	g_main_loop_run(g_mainloop);

	return 0;
}
