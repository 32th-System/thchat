#include "thchat.h"

#include <mutex>
#include <string>
#include <unordered_map>

#define PI 3.14159265359

struct timed_out_channels_t {
	std::unordered_map<std::string, bool> list;
	std::mutex m;
};

struct c_untimeout_param_t {
	timed_out_channels_t *channels;
	const char *cid;
};

DWORD WINAPI channel_untimeout(LPVOID lpParam) {
	c_untimeout_param_t *param = (c_untimeout_param_t*)lpParam;

	Sleep(6000);
	std::scoped_lock lock(param->channels->m);
	param->channels->list[param->cid] = false;
	free((void*)param->cid);
	thcrap_free(lpParam);
	return 0;
}

void channel_timeout(timed_out_channels_t *channels, const char *cid) {
	std::scoped_lock lock(channels->m);
	channels->list[cid] = true;
	c_untimeout_param_t *untimeout_params = (c_untimeout_param_t*)thcrap_alloc(sizeof(c_untimeout_param_t));
	untimeout_params->channels = channels;
	untimeout_params->cid = strdup(cid);

	// Issue: This will create a new thread for every single user
	// that sends a valid command in chat. If you are a successful
	// streamer, it will bring the program to it's knees.
	CreateThread(NULL, 0, &channel_untimeout, untimeout_params, 0, NULL);
}

bool channel_check(timed_out_channels_t *channels, const char *cid) {
	std::scoped_lock lock(channels->m);
	return channels->list[cid];
}

enum class paramtype : int {
	none,
	eid,
	bmgr,
	epos,
	btype,
	bspeed,
	bcount,
	bangle
};

DWORD WINAPI msg_read_thread(LPVOID lpParam) {
	char buffer[MSG_MAX_CHARS];
	wchar_t args[MSG_MAX_CHARS];
	char channel_id[YT_CID_LEN + 1];
	channel_id[YT_CID_LEN] = 0;
	timed_out_channels_t timedout;
	DWORD byteRet;
	paramtype pt = paramtype::none;
	int remaining_args = 0;

	HANDLE hMail = CreateMailslotW(L"\\\\.\\mailslot\\ThMail", MSG_MAX_CHARS, MAILSLOT_WAIT_FOREVER, NULL);
	if (hMail == INVALID_HANDLE_VALUE) {
		log_printf("Failed to create mailslot, error %d\n", GetLastError());
		return 1;
	}

	for (;;) {
		ReadFile(hMail, buffer, MSG_MAX_CHARS, &byteRet, NULL);
		if (buffer[YT_CID_LEN] == '\\') continue;
		buffer[byteRet] = 0;
		strncpy(channel_id, buffer, YT_CID_LEN);
		if (channel_check(&timedout, channel_id)) { log_printf("Channel %s timedout\n", channel_id); continue; };
		log_printf("[thchat] Processing message %s from channel %s\n", buffer + YT_CID_LEN, channel_id);

		enm_command_t *cmd = (enm_command_t *)thcrap_alloc(sizeof(enm_command_t));
		*cmd = {};
		MultiByteToWideChar(CP_UTF8, 0, buffer + YT_CID_LEN, -1, args, MSG_MAX_CHARS);
		int argc;
		wchar_t **argv = CommandLineToArgvW(args, &argc);
#define ARG(a) wcsncmp(a, argv[i], wcslen(a)) == 0
		for (int i = 0; i < argc; i++) {
			if (ARG(L"ENEMYID")) {
				pt = paramtype::eid;
				remaining_args = 1;
				continue;
			}

			if (ARG(L"BULLETMANAGER")) {
				pt = paramtype::bmgr;
				remaining_args = 1;
				continue;
			}

			if (ARG(L"ENEMYPOSITION")) {
				pt = paramtype::epos;
				remaining_args = 2;
				cmd->action_flags |= 1;
				continue;
			}

			if (ARG(L"SHOOTPATTERN")) {
				cmd->action_flags |= (1 << 1);
				continue;
			}

			if (ARG(L"BULLETTYPE")) {
				pt = paramtype::btype;
				remaining_args = 2;
				cmd->action_flags |= (1 << 2);
				continue;
			}

			if (ARG(L"BULLETSPEED")) {
				pt = paramtype::bspeed;
				remaining_args = 2;
				cmd->action_flags |= (1 << 3);
				continue;
			}

			if (ARG(L"BULLETCOUNT")) {
				pt = paramtype::bcount;
				remaining_args = 2;
				cmd->action_flags |= (1 << 4);
				continue;
			}

			if (ARG(L"BULLETANGLE")) {
				pt = paramtype::bangle;
				remaining_args = 2;
				cmd->action_flags |= (1 << 5);
				continue;
			}
#undef ARG
			if (remaining_args == 0) continue;

			switch (pt) {
			case paramtype::eid:
				cmd->id = wcstol(argv[i], NULL, 10);
				break;
			case paramtype::bmgr:
				cmd->bullet_manager = wcstol(argv[i], NULL, 10);
				break;
			case paramtype::epos:
				cmd->move.a[2 - remaining_args] = wcstof(argv[i], NULL);
				break;
			case paramtype::btype:
				cmd->btype[2 - remaining_args] = wcstol(argv[i], NULL, 10);
				break;
			case paramtype::bspeed:
				cmd->bspd[2 - remaining_args] = wcstof(argv[i], NULL);
				break;
			case paramtype::bcount:
				cmd->bcnt[2 - remaining_args] = wcstol(argv[i], NULL, 10);
				break;
			case paramtype::bangle:
				cmd->bangle[2 - remaining_args] = PI / 180 * wcstof(argv[i], NULL);
				break;
			}
			remaining_args--;
		}
		LocalFree(argv);
		if (cmd->action_flags == 0) {
			thcrap_free(cmd);
			continue;
		} else {
			channel_timeout(&timedout, channel_id);
			thchat_send_cmd_to_game(cmd);
		}
	}
}

int __stdcall thcrap_plugin_init() {
	DWORD threadId;
	HANDLE hThread = CreateThread(NULL, 0, msg_read_thread, NULL, 0, &threadId);
	log_printf("[thchat] Message reading thread running with thread ID %d and handle %x\n", threadId, hThread);
	return 0;
}
