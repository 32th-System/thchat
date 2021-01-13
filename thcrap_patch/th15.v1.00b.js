{
	"codecaves": {
		"show_enemyid": "50 FF B3 40 57 00 00 68 18 B7 4C 00 81 C3 50 12 00 00 53 FF 35 58 9A 4E 00 F3 0F 10 3D 20 00 4D 00 F3 0F 10 33 F3 0F 58 F7 F3 0F 11 33 B8 70 C5 40 00 FF D0 83 C4 10 F3 0F 5C F7 F3 0F 11 33 58 5B 89 EC 5D C3"
	},
	"binhacks": {
		"show_enemyid_jmp": {
			"code": "e9 [codecave:show_enemyid]",
			"addr": "Rx2892a"
		}
	},
	"breakpoints": {
		"thchat_init": {
			"addr": "Rx26a45",
			"cavesize": 6,
		
			"shoot_pattern_func": "Rx1c5c0",
			"find_enemy_by_id_func": "Rx255e0",
			
			"bmgr_size": "0x380",
			
			"bmgr_off": "0x17a4",
			"enm_pos_off": "0x1250",
			
			"btype_off":   "0x0",
			"bcolor_off":  "0x4",
			"boff_off":    "0x8",
			"bspeed1_off": "0x1c",
			"bspeed2_off": "0x20",
			"bcount1_off": "0x364",
			"bcount2_off": "0x366",
			"bangle1_off": "0x14",
			"bangle2_off": "0x18"
		},
		"thchat_uninit": {
			"addr": "Rx2697e",
			"cavesize": 6
		}
	}
}
