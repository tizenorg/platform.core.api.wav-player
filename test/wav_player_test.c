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



#include <stdio.h>
#include <wav_player_internal.h>
#include <sound_manager.h>
#include <glib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#define FILE_PATH_MAX   128
#define DEFAULT_FILE    "/usr/share/sounds/alsa/Front_Center.wav"

static GMainLoop *g_mainloop = NULL;

void help()
{
	printf("Usage : ");
	printf("wav_player_test [OPTION]\n\n"
		   "  -f, --file                file path to play\n"
		   "  -i, --iterate             how many times to play\n"
		   "  -h, --help                help\n");
}

void _player_stop_cb(int id, void *user_data)
{
	printf("complete id = %d,%p\n", id, user_data);
	sound_manager_destroy_stream_information((sound_stream_info_h)user_data);
	g_main_loop_quit(g_mainloop);
}

void stream_focus_cb(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason, const char *extra_info, void *user_data)
{
	return;
}

void wav_play_test(const char* file_path, int iterate)
{
	int ret = 0;
	int id;
	int i;
	sound_stream_info_h stream_info;

	if (iterate <= 0 || file_path == NULL) {
		printf("invalid param, iterate(%d), file_path(%s)\n", iterate, file_path);
		return;
	}

	if (sound_manager_create_stream_information(SOUND_STREAM_TYPE_MEDIA, stream_focus_cb, NULL, &stream_info)) {
		printf("failed to create stream info\n");
		return;
	}

	printf("Play Wav, File Path : %s, Iterate : %d\n", file_path, iterate);
	ret = wav_player_start_loop(file_path, stream_info, iterate, _player_stop_cb, (void*)stream_info, &id);
	printf("wav_player_start(%d)(id=%d) ret = %d\n", i, id, ret);
	if (ret) {
		sound_manager_destroy_stream_information(stream_info);
		return;
	}

	g_mainloop = g_main_loop_new(NULL, 0);
	g_main_loop_run(g_mainloop);
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

	wav_play_test(file_path, iterate);

	return 0;
}
