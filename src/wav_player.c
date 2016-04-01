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

#include <glib.h>

#include <muse_wav_player.h>
#include <muse_wav_player_msg.h>
#include <muse_core_ipc.h>
#include <muse_core_module.h>

#define USE_MUSE

typedef struct _ret_msg_s{
	gint api;
	gchar *msg;
	struct _ret_msg_s *next;
} wav_player_ret_msg_s;


typedef struct {
	gint bufLen;
	gchar *recvMsg;
	gint recved;
	wav_player_ret_msg_s *retMsgHead;
} wav_player_msg_buff_s;

typedef struct _callback_cb_info {
	GThread *thread;
	gint running;
	gint fd;
	gint handle;
	gint data_fd;
	gpointer user_cb[MUSE_WAV_PLAYER_EVENT_TYPE_NUM];
	gpointer user_data[MUSE_WAV_PLAYER_EVENT_TYPE_NUM];
	GMutex mutex;
	GCond cond[MUSE_WAV_PLAYER_API_MAX];
	wav_player_msg_buff_s buff;
} wav_player_callback_cb_info_s;

typedef struct _wav_player_cli_s {
    uint32_t handle;
    int sock_fd;
	wav_player_callback_cb_info_s *cb_info;
} wav_player_cli_s;

#define MM_SOUND_HANDLE_MAX                 32
static struct {
    uint32_t handle_count; /* use amotic operations */
    GList* handles;
    pthread_mutex_t lock;
} handle_mgr;

#define CHECK_HANDLE_RANGE(x) \
    do { \
        if (x == 0) { \
            LOGE("invalid handle(%d)", x); \
            return MM_ERROR_INVALID_ARGUMENT; \
        } \
    } while(0);

#define ATOMIC_INC(l, x) \
    do { \
        pthread_mutex_lock(l); \
        x = x + 1; \
        if (x == 0) \
            x = x + 1; \
        pthread_mutex_unlock(l); \
    } while(0);

// phandle(ret), GList, userdata, coimpare func
#define GET_HANDLE_DATA(p, l, u) \
    do { \
        GList* list = 0; \
        list = g_list_find_custom(l, u, __handle_comparefunc); \
        if (list != 0) \
            p = (wav_player_cli_s*)list->data; \
        else \
            p = NULL; \
    } while(0);

__attribute__ ((constructor)) void __wav_player_init(void)
{
    memset(&handle_mgr, 0, sizeof(handle_mgr));
    handle_mgr.handles = g_list_alloc();
    handle_mgr.handle_count = 1;
    pthread_mutex_init(&handle_mgr.lock, NULL);
}

__attribute__ ((destructor)) void __wav_player_deinit(void)
{
     g_list_free(handle_mgr.handles);
     pthread_mutex_destroy(&handle_mgr.lock);
     handle_mgr.handle_count = 0;
}

static gint __handle_comparefunc(gconstpointer a, gconstpointer b)
{
	wav_player_cli_s* phandle = (wav_player_cli_s*)a;
    int* handle = (int*)b;

    if (phandle == NULL)
        return -1;

    return (phandle->handle == *handle)? 0 : -1;
}

/* ----------- End ---------------*/


static void _add_ret_msg(muse_wav_player_api_e api, wav_player_callback_cb_info_s *cb_info, int offset, int parse_len)
{
	wav_player_ret_msg_s *msg = NULL;
	wav_player_ret_msg_s *last = cb_info->buff.retMsgHead;

	msg = g_new(wav_player_ret_msg_s, 1);
	if (msg) {
		msg->api = api;
		msg->msg = strndup(cb_info->buff.recvMsg + offset, parse_len);
		msg->next = NULL;
		if (last == NULL) {
			cb_info->buff.retMsgHead = msg;
		} else {
			while (last->next)
				last = last->next;
			last->next = msg;
		}
	} else
		LOGE("g_new failure");
}

static wav_player_ret_msg_s *_get_ret_msg(muse_wav_player_api_e api, wav_player_callback_cb_info_s *cb_info)
{
	wav_player_ret_msg_s *msg = cb_info->buff.retMsgHead;
	wav_player_ret_msg_s *prev = NULL;

	while (msg) {
		if (msg->api == api) {
			if (!prev)
				cb_info->buff.retMsgHead = msg->next;
			else
				prev->next = msg->next;
			return msg;
		}
		prev = msg;
		msg = msg->next;
	}
	return NULL;
}


int client_wait_for_cb_return(muse_wav_player_api_e api, wav_player_callback_cb_info_s *cb_info, char **ret_buf, int time_out)
{
	int ret = WAV_PLAYER_ERROR_NONE;
	gint64 end_time = g_get_monotonic_time() + time_out * G_TIME_SPAN_SECOND;
	wav_player_msg_buff_s *buff = &cb_info->buff;
	wav_player_ret_msg_s *msg = NULL;

	g_mutex_lock(&cb_info->mutex);

	msg = _get_ret_msg(api, cb_info);
	if (!buff->recved || !msg) {
		if (!g_cond_wait_until(&cb_info->cond[api], &cb_info->mutex, end_time)) {
			LOGW("api %d return msg does not received %ds", api, time_out);
			g_mutex_unlock(&cb_info->mutex);
			return WAV_PLAYER_ERROR_INVALID_OPERATION;
		}
	}
	if (!msg)
		msg = _get_ret_msg(api, cb_info);
	if (msg) {
		*ret_buf = msg->msg;
		g_free(msg);
	} else {
		LOGE("api %d return msg is not exist", api);
		ret = WAV_PLAYER_ERROR_INVALID_OPERATION;
	}
	buff->recved--;

	g_mutex_unlock(&cb_info->mutex);

	return ret;
}

static int _wav_player_recv_msg(wav_player_callback_cb_info_s *cb_info, int len)
{
	int recvLen;
	wav_player_msg_buff_s *buff = &cb_info->buff;
	char *new;

	if (len && buff->bufLen - MUSE_MSG_MAX_LENGTH <= len) {
		LOGD("realloc Buffer %d -> %d, Msg Length %d", buff->bufLen, buff->bufLen + MUSE_MSG_MAX_LENGTH, len);
		buff->bufLen += MUSE_MSG_MAX_LENGTH;
		new = g_renew(char, buff->recvMsg, buff->bufLen);
		if (new && new != buff->recvMsg)
			buff->recvMsg = new;
	}

	recvLen = muse_core_ipc_recv_msg(cb_info->fd, buff->recvMsg + len);
	len += recvLen;

	return len;
}

static void _process_event_receive(wav_player_callback_cb_info_s *cb_info, muse_wav_player_event_e event)
{
	switch (event) {
	case MUSE_WAV_PLAYER_EVENT_TYPE_EOS:
		if (cb_info->user_cb[event]) {
			wav_player_playback_completed_cb cb = cb_info->user_cb[event];
			if (cb)
				cb(cb_info->handle, cb_info->user_data[event]);
		}
		muse_core_connection_close(cb_info->fd);
		g_atomic_int_set(&cb_info->running, 0);
		break;

	default:
		break;
	}
}

static const char* api_str[MUSE_WAV_PLAYER_CB_EVENT+1] = {
	"MUSE_WAV_PLAYER_API_START",
	"MUSE_WAV_PLAYER_API_DUMMY",
	"MUSE_WAV_PLAYER_API_STOP",
	"MUSE_WAV_PLAYER_API_MAX",
	"MUSE_WAV_PLAYER_CB_EVENT"
};

static const char* event_str[MUSE_WAV_PLAYER_EVENT_TYPE_NUM] = {
	"MUSE_WAV_PLAYER_EVENT_TYPE_EOS"
};

static gpointer _client_cb_handler(gpointer data)
{
	muse_wav_player_api_e api;
	int len = 0;
	int parse_len = 0;
	int offset = 0;
	wav_player_cli_s * wpc = (wav_player_cli_s *)data;
	wav_player_callback_cb_info_s *cb_info = wpc->cb_info;
	char *recvMsg = cb_info->buff.recvMsg;
	muse_core_msg_parse_err_e err;

	while (g_atomic_int_get(&cb_info->running)) {
		len = 0;
		err = MUSE_MSG_PARSE_ERROR_NONE;
		do {
			LOGE("Trying to RECV ------------------ len(%d)", len);
			len = _wav_player_recv_msg(cb_info, len);
			if (len <= 0)
				break;
			recvMsg[len] = '\0';
			parse_len = len;
			offset = 0;

			LOGE("Received ------------------ return len(%d)", len);

			while (offset < len) {
				api = MUSE_WAV_PLAYER_API_MAX;
				if (muse_core_msg_json_deserialize("api", recvMsg + offset, &len, &api, &err, MUSE_TYPE_INT)) {
					LOGE("######### API [%s][%d] ##########", api_str[api], api);
					if (api < MUSE_WAV_PLAYER_API_MAX) {
						g_mutex_lock(&cb_info->mutex);

						cb_info->buff.recved++;
						_add_ret_msg(api, cb_info, offset, parse_len);
						g_cond_signal(&cb_info->cond[api]);

						g_mutex_unlock(&cb_info->mutex);

					} else if (api == MUSE_WAV_PLAYER_CB_EVENT) {
						muse_wav_player_event_e event;
						char *buffer = strndup(recvMsg + offset, parse_len);
						g_mutex_lock(&cb_info->mutex);

						muse_core_msg_json_deserialize(PARAM_EVENT, buffer, NULL, &event, NULL, MUSE_TYPE_INT);
						LOGE(" - Event : [%s][%d] : Buffer[%s][%d]", event_str[event], event, buffer, parse_len);
						_process_event_receive(cb_info, event);

						g_mutex_unlock(&cb_info->mutex);
						free(buffer);
					}
				}
				if (parse_len == 0)
					break;
				offset += parse_len;
				parse_len = len - parse_len;
			}
		} while (err == MUSE_MSG_PARSE_ERROR_CONTINUE);
		if (len <= 0)
			break;
	}

	LOGE("FREE wpc!");
	if (wpc) {
		if (wpc->cb_info)
			free(wpc->cb_info);
		free(wpc);
	}

	LOGD("client cb exit");
	return NULL;
}

static void _callback_new(wav_player_cli_s * wpc, wav_player_playback_completed_cb cb, void *user_data)
{
	wav_player_callback_cb_info_s *cb_info;
	wav_player_msg_buff_s *buff;
	int i;

	g_return_val_if_fail(wpc, NULL);

	cb_info = g_new(wav_player_callback_cb_info_s, 1);
	memset(cb_info, 0, sizeof(wav_player_callback_cb_info_s));

	g_mutex_init(&cb_info->mutex);
	for (i = 0; i < MUSE_WAV_PLAYER_API_MAX; i++)
		g_cond_init(&cb_info->cond[i]);

	buff = &cb_info->buff;
	buff->recvMsg = g_new(char, MUSE_MSG_MAX_LENGTH + 1);
	buff->bufLen = MUSE_MSG_MAX_LENGTH + 1;
	buff->recved = 0;
	buff->retMsgHead = NULL;

	g_atomic_int_set(&cb_info->running, 1);
	cb_info->fd = wpc->sock_fd;;
	cb_info->handle = wpc->handle;
	if (cb) {
		cb_info->user_cb[MUSE_WAV_PLAYER_EVENT_TYPE_EOS] = cb;
		cb_info->user_data[MUSE_WAV_PLAYER_EVENT_TYPE_EOS] = user_data;
	}
	cb_info->thread = g_thread_new("wav_player_callback_thread", _client_cb_handler, (gpointer)wpc);

	wpc->cb_info = cb_info;
}

#ifdef USE_MUSE /* muse-wav-player */
int wav_player_start(const char *path, sound_type_e type, wav_player_playback_completed_cb cb, void *user_data,  int * id)
{
	muse_wav_player_api_e api = MUSE_WAV_PLAYER_API_START;
	wav_player_cli_s * wpc = NULL;
	int ret = MM_ERROR_NONE;
	int sock_fd = -1;
	char *send_msg = NULL;
	char *ret_buf = NULL;

	if (path == NULL)
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_PARAMETER);
	if (type < SOUND_TYPE_SYSTEM || type >= SOUND_TYPE_NUM)
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_PARAMETER);

	/* create handle  */
	wpc = g_new0(wav_player_cli_s, 1);
	if (!wpc)
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_OPERATION);

	/* client new */
	sock_fd = muse_core_client_new();
	if (sock_fd < 0) {
		LOGE("muse_core_client_new failed - returned fd %d", sock_fd);
		goto error;
	}

	/* Send msg */
	send_msg = muse_core_msg_json_factory_new(api, MUSE_TYPE_INT, "module", MUSE_WAV_PLAYER,
												MUSE_TYPE_STRING, "path", path,
												0);
	if (send_msg) {
		LOGD("sock_fd : %d, msg : %s", sock_fd, send_msg);
		ret = muse_core_ipc_send_msg(sock_fd, send_msg);
		muse_core_msg_json_factory_free(send_msg);
		if (ret == -1) {
			LOGE("failed to send msg!!! errno=%d", errno);
			goto error;
		}
	} else {
		LOGE("failed to create msg!!!");
		goto error;
	}

	/* callback new */
	wpc->sock_fd = sock_fd;
	wpc->handle = handle_mgr.handle_count;
	_callback_new(wpc, cb, user_data);

	/* wait for api return */
	ret = client_wait_for_cb_return(api, wpc->cb_info, &ret_buf, WAV_PLAYER_CALLBACK_TIME_OUT);
	if (ret == WAV_PLAYER_ERROR_NONE) {
		LOGD("wav_player_start done!!, buffer[%s], handle = %d", ret_buf, sock_fd);

		/* store in global handle manager */
		ATOMIC_INC(&handle_mgr.lock, handle_mgr.handle_count); // 0 is not used
		handle_mgr.handles = g_list_append(handle_mgr.handles, wpc);

		/* set id */
		if (id)
			*id = wpc->handle;
	} else {
		LOGE("wav_player_start failed...%d", ret);
	}

	/* clean-up */
	if (ret_buf)
		g_free(ret_buf);

	return __convert_wav_player_error_code(__func__, ret);

error:
	if (wpc) {
		if (wpc->cb_info) {
			free (wpc->cb_info);
		}
		free(wpc);
	}
	return __convert_wav_player_error_code(__func__, MM_ERROR_SOUND_INTERNAL);
}
#else
int wav_player_start(const char *path, sound_type_e type, wav_player_playback_completed_cb cb, void *user_data,  int * id)
{
	int ret = MM_ERROR_NONE;
	int player = -1;
	char m_path[PATH_MAX];
	void (*_completed_cb)(void *, int);
	_completed_cb = NULL;
	_cb_data *cb_data = NULL;


	if (path == NULL)
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_PARAMETER);

	if (type < SOUND_TYPE_SYSTEM || type >= SOUND_TYPE_NUM)
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_PARAMETER);

	m_path[0] = '\0';
	if (path[0] != '/') {

		if (getcwd(m_path, PATH_MAX) != NULL)
			strncat(m_path, "/", PATH_MAX-strlen(m_path));
	}
	strncat(m_path, path, PATH_MAX-strlen(m_path));

	if (cb) {
		_completed_cb = __internal_complete_cb;
		cb_data = (_cb_data *)malloc(sizeof(_cb_data));
		if (cb_data == NULL)
			return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_OPERATION);
		cb_data->cb = cb;
		cb_data->user_data = user_data;
	}


	ret = mm_sound_play_sound(m_path, type, _completed_cb , cb_data, &player);

	if (ret == 0 && id != NULL)
		*id = player;

	if (ret != 0 && cb_data != NULL)
		free(cb_data);

	return __convert_wav_player_error_code(__func__, ret);
}
#endif

int wav_player_start_with_stream_info(const char *path, sound_stream_info_h stream_info, wav_player_playback_completed_cb cb, void *user_data, int *id)
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

	if (cb) {
		_completed_cb = __internal_complete_cb;
		cb_data = (_cb_data *)malloc(sizeof(_cb_data));
		if (cb_data == NULL)
			return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_OPERATION);
		cb_data->cb = cb;
		cb_data->user_data = user_data;
	}


	ret = mm_sound_play_sound_with_stream_info(m_path, stream_type, stream_id, _completed_cb , cb_data, &player);

	if (ret == 0 && id != NULL)
		*id = player;

	if (ret != 0 && cb_data != NULL)
		free(cb_data);

	return __convert_wav_player_error_code(__func__, ret);

}

#ifdef USE_MUSE /* muse-wav-player */
int wav_player_stop(int id)
{
	muse_wav_player_api_e api = MUSE_WAV_PLAYER_API_STOP;
	wav_player_cli_s * wpc = NULL;
	int ret = WAV_PLAYER_ERROR_NONE;
	char *send_msg = NULL;
	char *ret_buf = NULL;

	if (id < 0)
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_PARAMETER);

	/* Get Handle */
	GET_HANDLE_DATA(wpc, handle_mgr.handles, &id);
	if (wpc == NULL) {
		LOGE("failed to get wpc!!!! list=%p[%d], id=%d", handle_mgr.handles, handle_mgr.handle_count, id);
		return __convert_wav_player_error_code(__func__, WAV_PLAYER_ERROR_INVALID_OPERATION);
	}

	/* Send msg */
	send_msg = muse_core_msg_json_factory_new(api, MUSE_TYPE_INT, "module", MUSE_WAV_PLAYER, 0);
	LOGD("sock_fd : %d, msg : %s", wpc->sock_fd, send_msg);
	muse_core_ipc_send_msg(wpc->sock_fd, send_msg);
	muse_core_msg_json_factory_free(send_msg);

	/* wait for api return */
	ret = client_wait_for_cb_return(api, wpc->cb_info, &ret_buf, WAV_PLAYER_CALLBACK_TIME_OUT);
	if (ret == WAV_PLAYER_ERROR_NONE) {
		LOGD("wav_player_stop done!!, buffer[%s], sock_fd = %d", ret_buf, wpc->sock_fd);
	} else {
		LOGE("wav_player_start failed...%d", ret);
	}
	if (ret_buf)
		g_free(ret_buf);

	/* remove handle from handle manager */
	handle_mgr.handles = g_list_remove(handle_mgr.handles, wpc);

	/* close muse connection, wpc will be deleted when client callback ends */
	LOGE("Now close connection!!!");
	muse_core_connection_close(wpc->cb_info->fd);

	return __convert_wav_player_error_code(__func__, ret);
}
#else

int wav_player_stop(int id)
{
	return __convert_wav_player_error_code(__func__, mm_sound_stop_sound(id));
}
#endif

