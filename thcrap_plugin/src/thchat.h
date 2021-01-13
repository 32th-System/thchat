#include <thcrap.h>

#define YT_CID_LEN 24
#define MSG_MAX_CHARS 512

union vec2f {
	struct {
		float x;
		float y;
	};
	float a[2];
};

struct enm_command_t {
	DWORD action_flags;
	int id;
	int bullet_manager;
	vec2f move;
	DWORD btype[2];
	float bspd[2];
	WORD bcnt[2];
	float bangle[2];
};

void thchat_send_cmd_to_game(enm_command_t *cmd);
