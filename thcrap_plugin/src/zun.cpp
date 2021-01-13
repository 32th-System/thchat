#include "thchat.h"

typedef size_t __stdcall find_enemy_by_id_t(int id);
typedef void  __stdcall shoot_pattern_t(size_t bmgr_addr);

shoot_pattern_t *shoot_pattern = NULL;
find_enemy_by_id_t *find_enemy_by_id = NULL;

size_t bmgr_off = 0;
size_t bmgr_size = 0;
size_t enm_pos_off = 0;

size_t btype_off = 0;
size_t bcolor_off = 0;
size_t boff_off = 0;
size_t bspeed1_off = 0;
size_t bspeed2_off = 0;
size_t bcount1_off = 0;
size_t bcount2_off = 0;
size_t bangle1_off = 0;
size_t bangle2_off = 0;

int btype_color_max[] = { 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 3, 8, 8, 8, 8, 8, 8, 8, 1, 1, 1, 1, 8, 8, 8, 4, 8, 16, 16, 16, 16, 16, 1, 1, 1, 1, 8 };

template<typename T>
void __forceinline poke(size_t addr, T val) {
	*(T*)addr = val;
}

bool initialized = false;

int BP_thchat_init(x86_reg_t *regs, json_t *bp_info) {
	shoot_pattern = (shoot_pattern_t*)json_object_get_hex(bp_info, "shoot_pattern_func");
	find_enemy_by_id = (find_enemy_by_id_t*)json_object_get_hex(bp_info, "find_enemy_by_id_func");

#define SET_OFF(a) a = json_object_get_hex(bp_info, #a)
	SET_OFF(bmgr_off);
	SET_OFF(bmgr_size);
	SET_OFF(enm_pos_off);
	SET_OFF(btype_off);
	SET_OFF(bcolor_off);
	SET_OFF(boff_off);
	SET_OFF(bspeed1_off);
	SET_OFF(bspeed2_off);
	SET_OFF(bcount1_off);
	SET_OFF(bcount2_off);
	SET_OFF(bangle1_off);
	SET_OFF(bangle2_off);
#undef SET_OFF

	initialized = true;

	log_print("[thchat] Initialized game addresses and offsets\n");

	return breakpoint_cave_exec_flag(bp_info);
}

int BP_thchat_uninit(x86_reg_t *regs, json_t *bp_info) {
	initialized = false;
	return breakpoint_cave_exec_flag(bp_info);
}

void thchat_send_cmd_to_game(enm_command_t *cmd) {
	if (!initialized || !find_enemy_by_id) goto end;

	size_t enm_addr = find_enemy_by_id(cmd->id);
	vec2f *enm_pos = (vec2f*)(enm_addr + enm_pos_off);

	if (!enm_addr) goto end;

	if (cmd->action_flags & 1) {
		enm_pos->x += cmd->move.x;
		enm_pos->y += cmd->move.y;
	}

	size_t bmgr_addr = enm_addr + bmgr_off + bmgr_size * cmd->bullet_manager;

	if ((cmd->action_flags >> 2) & 1) {
		cmd->btype[0] %= 44;
		poke(bmgr_addr + btype_off, cmd->btype[0]);
		poke(bmgr_addr + bcolor_off, cmd->btype[1] % btype_color_max[cmd->btype[0]]);
	}

	if ((cmd->action_flags >> 3) & 1) {
		poke(bmgr_addr + bspeed1_off, cmd->bspd[0]);
		poke(bmgr_addr + bspeed2_off, cmd->bspd[1]);
	}

	if ((cmd->action_flags >> 4) & 1) {
		poke(bmgr_addr + bcount1_off, cmd->bcnt[0]);
		poke(bmgr_addr + bcount2_off, cmd->bcnt[1]);
	}

	if ((cmd->action_flags >> 5) & 1) {
		poke(bmgr_addr + bangle1_off, cmd->bangle[0]);
		poke(bmgr_addr + bangle2_off, cmd->bangle[1]);
	}

	if ((cmd->action_flags >> 1) & 1) {
		if (shoot_pattern) {
			poke(bmgr_addr + boff_off, *enm_pos);
			shoot_pattern(bmgr_addr);
		}
	}
end:
	thcrap_free(cmd);
	return;
}
