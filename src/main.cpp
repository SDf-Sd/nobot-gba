#include <gba_console.h>
#include <gba_sprites.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>

#include "pico8.h"
#include "ecs.h"
#include "sine.h"
#include "fixed.h"
#include "flags.h"
#include "fnt.h"
#include "gfx.h"
#include "map.h"
#include "circ.h"

static u16 kheld = 0;
static u16 kdown = 0;


#define btn(x) ((kheld & (x))? 1: 0)
#define btnp(x) ((kdown & (x))? 1: 0)

#define TYPE_PLAYER 1
#define TYPE_DOOR 2
#define TYPE_SHADOW 3

const int draw_query[] = {C_SPR, C_POS};
const int player_query[] = {C_CTRL, C_PHYS};
const int player_anim_query[] = {C_PHYS, C_CTRL, C_SPR};
const int phys_query[] = {C_POS, C_PHYS};
const int mcol_query[] = {C_MCOL};
const int door_query[] = {C_DOOR};
const int camera_query[] = {C_CAM, C_POS};
const int shadow_query[] = {C_POS, C_FLW};
const int boom_query[] = {C_BOOM};

void query_entities(std::vector<Entity*> entities, const int query[], const int query_length, std::vector<Entity*> &captured) {
	for (u16 i = 0; i < entities.size(); i++) {
		for (int j = 0; j < query_length; j++)
			if (entities[i]->cmpt[query[j]] == nullptr) goto skip;
		captured.push_back(entities[i]);
		skip: ;
	}
}

static u32 tick = 0;

static bool game_over = false;
static u8 shadow_count = 0;
static u8 cur_level = 0;

void load_level(u8 n, std::vector<Entity*> &entities) {
	std::vector<Entity*> captured;
	query_entities(entities, player_query, 2, captured);
	query_entities(entities, door_query, 1, captured);
	query_entities(entities, camera_query, 2, captured);
	PosComponent* plr_pos = (PosComponent*) captured[0]->cmpt[C_POS];
	CtrlComponent* plr_ctrl = (CtrlComponent*) captured[0]->cmpt[C_CTRL];
	PhysComponent* plr_phys = (PhysComponent*) captured[0]->cmpt[C_PHYS];
	SprComponent* plr_spr = (SprComponent*) captured[0]->cmpt[C_SPR];
	SprComponent* door_spr = (SprComponent*) captured[1]->cmpt[C_SPR];
	PosComponent* door_pos = (PosComponent*) captured[1]->cmpt[C_POS]; // door should always have position
	PosComponent* cam_pos = (PosComponent*) captured[2]->cmpt[C_POS];
	CameraComponent* cam_settings = (CameraComponent*) captured[2]->cmpt[C_CAM];

	// door_spr->visible = true;
	plr_phys->disabled = false;
	plr_phys->colbit = 2;
	plr_spr->visible = true;
	plr_ctrl->moved = false;
	plr_ctrl->prev_positions.clear();
	for (int i = entities.size() - 1; i >= 0; i--) {
		if (entities.at(i)->typ == TYPE_SHADOW) {
			entities.erase(entities.begin()+i);		
		}
	}
	shadow_count = 0;

	game_over = false;
	tick = 0;

	switch (n) {
		case 0:
			map(0, 0);
			plr_pos->x = int2fx(26+56);
			plr_pos->y = int2fx(112+16);
			door_pos->x = int2fx(24+56);
			door_pos->y = int2fx(112+16);
			cam_pos->x = int2fx(0);
			cam_pos->y = int2fx(0);
			cam_settings->x_min = 0;
			cam_settings->x_max = 128;
			cam_settings->y_lim = 0;
			break;
		case 1:
			map(0, 16);
			plr_pos->x = int2fx(18+56);
			plr_pos->y = int2fx(112+16);
			door_pos->x = int2fx(16+56);
			door_pos->y = int2fx(112+16);
			cam_pos->x = int2fx(0);
			cam_pos->y = int2fx(0);
			cam_settings->x_min = 0;
			cam_settings->x_max = 128;
			cam_settings->y_lim = 0;			
			break;
		case 2:
			map(0, 32); // 3rd level
			plr_pos->x = int2fx(26+56);
			plr_pos->y = int2fx(24+16);
			door_pos->x = int2fx(24+56);
			door_pos->y = int2fx(24+16);
			cam_pos->x = int2fx(0);
			cam_pos->y = int2fx(0);
			cam_settings->x_min = 0;
			cam_settings->x_max = 128;
			cam_settings->y_lim = 0;
			break;
		case 3:
			map(0, 48, 20, 16);
			plr_pos->x = int2fx(16+56);
			plr_pos->y = int2fx(64+16);
			door_pos->x = int2fx(16+56);
			door_pos->y = int2fx(72+16);			
			cam_pos->x = int2fx(0);
			cam_pos->y = int2fx(0);
			cam_settings->x_min = 0;
			cam_settings->x_max = 160;
			cam_settings->y_lim = 0;
			break;
		case 4:
			map(16, 0);
			plr_pos->x = int2fx(98+56);
			plr_pos->y = int2fx(16+16);
			door_pos->x = int2fx(96+56);
			door_pos->y = int2fx(24+16);
			cam_pos->x = int2fx(0);
			cam_pos->y = int2fx(0);
			cam_settings->x_min = 0;
			cam_settings->x_max = 128;
			cam_settings->y_lim = 0;
			break;
		case 5:
			map(16, 16);
			plr_pos->x = int2fx(24+56);
			plr_pos->y = int2fx(104+16);
			door_pos->x = int2fx(24+56);
			door_pos->y = int2fx(112+16);
			cam_pos->x = int2fx(0);
			cam_pos->y = int2fx(0);
			cam_settings->x_min = 0;
			cam_settings->x_max = 128;
			cam_settings->y_lim = 0;
			break;
		case 6:
			map(16, 32);
			plr_pos->x = int2fx(24+56);
			plr_pos->y = int2fx(48+16);
			door_pos->x = int2fx(24+56);
			door_pos->y = int2fx(48+16);
			cam_pos->x = int2fx(0);
			cam_pos->y = int2fx(0);
			cam_settings->x_min = 0;
			cam_settings->x_max = 128;
			cam_settings->y_lim = 0;
			break;
		case 7:
			map(20, 48);
			plr_pos->x = int2fx(24+56);
			plr_pos->y = int2fx(112+16);
			door_pos->x = int2fx(24+56);
			door_pos->y = int2fx(112+16);
			cam_pos->x = int2fx(0);
			cam_pos->y = int2fx(0);
			cam_settings->x_min = 0;
			cam_settings->x_max = 128;
			cam_settings->y_lim = 0;
			break;
		case 8: // final level (end screen)
			map(32, 0);
			plr_spr->visible = false;
			door_spr->visible = false;
			break;

	}
}

void system_draw(std::vector<Entity*> entities) {
	std::vector<Entity*> captured;

	query_entities(entities, camera_query, 2, captured);

	if (captured.size() > 0) {
		PosComponent* cam_pos = (PosComponent*) captured[0]->cmpt[C_POS];
		camera(fx2int(cam_pos->x), fx2int(cam_pos->y));
	}

	captured.clear();
	query_entities(entities, draw_query, 2, captured);

	for (Entity* ent : captured) {
		SprComponent* ent_spr = (SprComponent*) ent->cmpt[C_SPR];
		PosComponent* ent_pos = (PosComponent*) ent->cmpt[C_POS];

		Animation &anim = ent_spr->anims[ent_spr->cur_anim];
		u8 spr_id = anim.frames[(tick / anim.speed) % anim.length];

		if (anim.oneshot) {
			if (spr_id == anim.frames[anim.length - 1]) {
				anim.set_paused(true);
				ent_spr->visible = false;
			}
			if (anim.paused) spr_id = anim.frames[anim.length - 1];
		}

		u8 layer = 1;
		u8 palette = 0;
		if (ent->typ == TYPE_PLAYER) {
			layer = 0;
			PhysComponent* ent_phys = (PhysComponent*) ent->cmpt[C_PHYS];
			pal(11, 8, PAL_SPR);
			if (ent_phys->colbit == 1) {
				pal(8, 8, PAL_BG);
				pal(12, 13, PAL_BG);
			}
			else {
				pal(8, 2, PAL_BG);
				pal(12, 12, PAL_BG);
			}
		}
		else if (ent->typ == TYPE_SHADOW) {
			layer = 0;
			palette = 1;
		}

		if (ent_spr->visible) spr(spr_id, fx2int(ent_pos->x) + camx, fx2int(ent_pos->y) - camy, layer, palette, ent_spr->flip_h, ent_spr->flip_v);
	}
}

void system_boom_draw(std::vector<Entity*> entities) {
	std::vector<Entity*> captured;
	query_entities(entities, boom_query, 1, captured);

	for (Entity* ent : entities) {
		for (Spark spark : ((ExplosionComponent*)ent->cmpt[C_BOOM])->sparks) {
			u8 r_offset = 0;

			if (spark.r >= int2fx(4))
				r_offset = 0;
			else if (spark.r < int2fx(4) && spark.r >= 896) // 3.5
				r_offset = 1;
			else if (spark.r < 896 && spark.r >= int2fx(3))
				r_offset = 2;
			else if (spark.r < int2fx(3) && spark.r >= 640)
				r_offset = 3;
			else if (spark.r < 640 && spark.r >= int2fx(2))
				r_offset = 4;
			else if (spark.r < int2fx(2) && spark.r >= 384)
				r_offset = 5;
			else if (spark.r < 384 && spark.r >= int2fx(1))
				r_offset = 6;
			else if (spark.r < int2fx(1) && spark.r >= 64)
				r_offset = 7;
			else
				r_offset = 8;

			spr(512 + r_offset, fx2int(spark.x), fx2int(spark.y), 0, 2 + intfxround(rnd(int2fx(5))), false, false);
		}
	}
}

void system_walk(std::vector<Entity*> entities) {
	std::vector<Entity*> captured;
	query_entities(entities, player_query, 2, captured);

	PhysComponent* phys = (PhysComponent*) captured[0]->cmpt[C_PHYS];
	CtrlComponent* ctrl = (CtrlComponent*) captured[0]->cmpt[C_CTRL];

	if (ctrl->disabled) return;

	if btn(KEY_LEFT) phys->vx = int2fx(-1); 
	else if btn(KEY_RIGHT) phys->vx = int2fx(1);
	else phys->vx = 0;//fxmul(phys->vx, 0x40); // 64 / 256 = 0.25

	if (phys->vx != 0) ctrl->moved = true;
	phys->vx = fxmul(phys->vx, phys->spd);
}

void system_jump(std::vector<Entity*> entities) {
	std::vector<Entity*> captured;
	query_entities(entities, player_query, 2, captured);

	PhysComponent* phys = (PhysComponent*) captured[0]->cmpt[C_PHYS];
	CtrlComponent* ctrl = (CtrlComponent*) captured[0]->cmpt[C_CTRL];

	if (ctrl->disabled) return;

	if (btnp(KEY_A) and phys->onground) {
		phys->vy = phys->jh;
		ctrl->jumping = true;
		//sfx(2);
		phys->colbit = (phys->colbit == 1) ? 2 : 1;
	}
	else if (!btn(KEY_A) and ctrl->jumping) {
		phys->vy = fxdiv(phys->vy, 0x200);
		ctrl->jumping = false;
	}
}

void system_player_anim(std::vector<Entity*> entities) {
	std::vector<Entity*> captured;
	query_entities(entities, player_anim_query, 3, captured);

	PhysComponent* phys = (PhysComponent*) captured[0]->cmpt[C_PHYS];
	// CtrlComponent* ctrl = (CtrlComponent*) captured[0]->cmpt[C_CTRL];
	SprComponent* spr = (SprComponent*) captured[0]->cmpt[C_SPR];

	if (phys->vx != 0 && (btn(KEY_LEFT) || btn(KEY_RIGHT))) {
		spr->cur_anim = 1; // walk animation
		spr->flip_h = (phys->vx < 0);
	}
	else
		spr->cur_anim = 0;
}

void system_player_cam(std::vector<Entity*> entities) {
	std::vector<Entity*> captured;
	query_entities(entities, player_query, 2, captured);
	query_entities(entities, camera_query, 2, captured);
	
	PosComponent* plr_pos = (PosComponent*) captured[0]->cmpt[C_POS];
	
	CameraComponent* cam_set = (CameraComponent*) captured[1]->cmpt[C_CAM];
	PosComponent* cam_pos = (PosComponent*) captured[1]->cmpt[C_POS];

	cam_pos->x = int2fx(clamp(cam_set->x_min, fx2int(plr_pos->x) - 64 - 56, cam_set->x_max - 128));
	cam_pos->x = fxadd(cam_pos->x, cam_set->shake_x);
	cam_pos->y = int2fx(cam_set->y_lim);
	cam_pos->y = fxadd(cam_pos->y, cam_set->shake_y);

	cam_set->shake_x = fxmul(cam_set->shake_x, 156); // ~0.6
	cam_set->shake_y = fxmul(cam_set->shake_y, 156); // ~0.6
}

void system_move(std::vector<Entity*> entities) {
	std::vector<Entity*> captured;
	query_entities(entities, phys_query, 2, captured);

	for (Entity* ent : captured) {
		PhysComponent* phys = (PhysComponent*) ent->cmpt[C_PHYS];
		PosComponent* pos = (PosComponent*) ent->cmpt[C_POS];

		if (ent->typ == TYPE_PLAYER) {
			SprComponent* spr = (SprComponent*) ent->cmpt[C_SPR];
			((CtrlComponent*) ent->cmpt[C_CTRL])->prev_positions.emplace(tick, (FollowPoint) {pos->x, pos->y, spr->flip_h, spr->cur_anim} );
		}

		if (phys->disabled) continue;

		pos->x = fxadd(pos->x, phys->vx);
		pos->y = fxadd(pos->y, phys->vy);
	}
}

void system_boom(std::vector<Entity*> &entities) {
	std::vector<Entity*> captured;
	query_entities(entities, boom_query, 1, captured);

	for (Entity* ent : captured) {
		ExplosionComponent* exp = (ExplosionComponent*) ent->cmpt[C_BOOM];
		for (int i = exp->sparks.size() - 1; i >= 0; i--){ // todo delete sparks when small radius
			Spark *spark = &exp->sparks.at(i);
			spark->x = fxadd(spark->x, spark->vx);
			spark->y = fxadd(spark->y, spark->vy);
			spark->r = fxsub(spark->r, 26); // 0.1
			if (spark->r < 26)
				exp->sparks.erase(exp->sparks.begin() + i);
		}
	}

	for (int i = entities.size() - 1; i >= 0; i--) {
		for (int j = captured.size() - 1; j >= 0; j--) {
			if (entities.at(i) == captured.at(j)) {
				if (((ExplosionComponent*)captured.at(j)->cmpt[C_BOOM])->sparks.size() == 0) {
					entities.erase(entities.begin() + i);
					captured.erase(captured.begin() + j);
				}
			}
		}
	}
}

void system_grav(std::vector<Entity*> entities) {
	std::vector<Entity*> captured;
	query_entities(entities, phys_query, 2, captured);
	
	for (Entity* ent : captured) {
		PhysComponent* phys = (PhysComponent*) ent->cmpt[C_PHYS];

		if (phys->vy < int2fx(5)) phys->vy = fxadd(phys->vy, 0x80); // 128 / 256 = 0.5
	}
}

void make_shadow(Entity &entity, FIXED x, FIXED y, u32 tick);

void system_spawn_shadows(std::vector<Entity*> &entities) { // adds entities 
	std::vector<Entity*> captured;
	query_entities(entities, player_query, 2, captured);
	query_entities(entities, door_query, 1, captured);

	CtrlComponent* ctrl = (CtrlComponent*) captured[0]->cmpt[C_CTRL];
	SprComponent* door_spr = (SprComponent*) captured[1]->cmpt[C_SPR];

	if (ctrl->moved && shadow_count < 5 && tick % 20 == 0) {
		Entity* shadow = new Entity;
		make_shadow(*shadow, ctrl->prev_positions[0].x, ctrl->prev_positions[0].y, (shadow_count+1)*20);
		entities.push_back(shadow);
		door_spr->anims[door_spr->cur_anim].set_paused(false);
		door_spr->visible = true;
		shadow_count++;
	} 
}

void system_follow(std::vector<Entity*> entities) {
	std::vector<Entity*> captured;
	query_entities(entities, player_query, 2, captured);
	CtrlComponent* ctrl = (CtrlComponent*) entities[0]->cmpt[C_CTRL];
	captured.clear();

	query_entities(entities, shadow_query, 2, captured);

	for (Entity* ent : captured) {
		SprComponent* spr = (SprComponent*) ent->cmpt[C_SPR];
		PosComponent* pos = (PosComponent*) ent->cmpt[C_POS];
		FollowComponent* flw = (FollowComponent*) ent->cmpt[C_FLW];

		FollowPoint fp = ctrl->prev_positions[tick - flw->offset];
		
		pos->x = fp.x;
		pos->y = fp.y;
		spr->flip_h = fp.flph;
		spr->cur_anim = fp.canm;
	}
}

bool mcol(s16 x, u8 y, u8 w, u8 h, u8 cb) {
	return fget(mget(x/8, y/8), cb) || fget(mget((x+w)/8, (y+h)/8), cb);
}

void system_mcol(std::vector<Entity*> &entities) { // changes entities
	std::vector<Entity*> captured;
	query_entities(entities, mcol_query, 1, captured);

	for (Entity* ent : captured) {
		PosComponent* pos = (PosComponent*) ent->cmpt[C_POS];
		PhysComponent* phys = (PhysComponent*) ent->cmpt[C_PHYS];

		int x = fx2int(pos->x);
		int y = fx2int(pos->y);
		int dx = fx2int(fxadd(pos->x, phys->vx));
		int dy = fx2int(fxadd(pos->y, phys->vy));

		u8 cb = phys->colbit;

		phys->onground = mcol(x, dy+8, 7, 0, 0) or mcol(x, dy+8, 7, 0, cb);

		if (mcol(dx, y, 0, 7, 0) or mcol(dx, y, 0, 7, cb)) {
			phys->vx = 0;
			pos->x = int2fx(dx/8*8 + 8);
		}
		if (mcol(dx+8, y, 0, 7, 0) or mcol(dx+8, y, 0, 7, cb)) {
			phys->vx = 0;
			pos->x = int2fx(dx/8*8);
		}
		if (mcol(x, dy, 7, 0, 0) or mcol(x, dy, 7, 0, cb)) {
			phys->vy = 0;
			pos->y = int2fx(dy/8*8+8);
		}
		if (mcol(x, dy+7, 7, 0, 0) or mcol(x, dy+7, 7, 0, cb)) {
			phys->vy = 0;
			pos->y = int2fx(dy/8*8);
		}

		if (mcol(x, dy+7, 7, 1, 3)) phys->vx = fxsub(phys->vx, int2fx(1));
		if (mcol(x, y, 8, 8, 6)) {
			// sfx(5)
			cur_level++;
			load_level(cur_level, entities);
			print("wow", 0, 0, 7);
			// map(0, 32);
		}
	}
}

bool e_col(FIXED dx, FIXED dy, FIXED x, FIXED y, FIXED w, FIXED h) {
	return dx < x + w && dx > x - w && dy < y + h && y - h < dy;
}

void make_boom(Entity &entity, FIXED x, FIXED y, FIXED r, u32 particles);

void system_p_col(std::vector<Entity*> &entities) {
	std::vector<Entity*> captured;
	query_entities(entities, player_query, 2, captured);
	query_entities(entities, camera_query, 2, captured);

	PhysComponent* phys = (PhysComponent*) captured[0]->cmpt[C_PHYS];
	PosComponent* pos = (PosComponent*) captured[0]->cmpt[C_POS];
	SprComponent* spr = (SprComponent*) captured[0]->cmpt[C_SPR];

	CameraComponent* cmra = (CameraComponent*) captured[1]->cmpt[C_CAM];

	captured.clear();
	query_entities(entities, shadow_query, 2, captured);

	FIXED dx = pos->x + phys->vx;
	FIXED dy = pos->y + phys->vy;

	std::vector<Entity*> queue_for_destruction;
	for (Entity* ent : captured) {
		PosComponent* e_pos = (PosComponent*) ent->cmpt[C_POS];
		if (e_col(dx, dy, e_pos->x, e_pos->y, int2fx(8), int2fx(8))) {
			Entity* boom = new Entity;
			make_boom(*boom, fxadd(dx, int2fx(4)), fxadd(dy, int2fx(4)), int2fx(2), 50);
			entities.push_back(boom);
			queue_for_destruction.push_back(ent);
			game_over = true;
			//sfx(2)
			phys->vx = 0;
			phys->disabled = true;
			spr->visible = false;
			cmra->shake_x = fxsub(int2fx(24), rnd(32));
			cmra->shake_y = fxsub(int2fx(24), rnd(32));
		}
	}
	for (int i = entities.size() - 1; i >= 0; i--) {
		for (int j = queue_for_destruction.size() - 1; j >= 0; j--) {
			if (entities[i] == queue_for_destruction[j]) {
				entities.erase(entities.begin() + i);
				queue_for_destruction.erase(queue_for_destruction.begin() + j);
			}
		}
	}
}

Animation plr_def(1, {16}, 1, false);
Animation plr_wlk(2, {16, 17, 18, 19}, 4, false);

void make_player(Entity &entity, FIXED x, FIXED y) {
	PosComponent* pos = new PosComponent(x, y);
	SprComponent* spr = new SprComponent(0, {plr_def, plr_wlk});
	CtrlComponent* ctrl = new CtrlComponent();
	PhysComponent* phys = new PhysComponent(int2fx(2), int2fx(-6));
	MColComponent* mcol = new MColComponent();

	entity.typ = TYPE_PLAYER;
	entity.Add(pos, C_POS);
	entity.Add(spr, C_SPR);
	entity.Add(phys, C_PHYS);
	entity.Add(ctrl, C_CTRL);
	entity.Add(mcol, C_MCOL);
}

void make_shadow(Entity &entity, FIXED x, FIXED y, u32 tick) {
	entity.typ = TYPE_SHADOW;
	entity.Add(new PosComponent(x, y), C_POS);
	entity.Add(new SprComponent(0, {plr_def, plr_wlk}), C_SPR);
	entity.Add(new FollowComponent(tick), C_FLW);
}

Animation door_def(1, {32}, 1, false);
Animation door_open(6, {32, 33, 34}, 3, true);
Animation door_close(6, {34, 33, 32}, 3, true);

void make_door(Entity &entity, FIXED x, FIXED y) {
	PosComponent* pos = new PosComponent(x, y);
	SprComponent* spr = new SprComponent(2, {door_def, door_open, door_close});
	DoorComponent* door = new DoorComponent();

	entity.typ = TYPE_DOOR;
	entity.Add(pos, C_POS);
	entity.Add(spr, C_SPR);
	entity.Add(door, C_DOOR);
}

void make_cam(Entity &entity, FIXED x, FIXED y, u32 xmin, u32 xmax, u32 ylim) {
	PosComponent* pos = new PosComponent(x, y);
	CameraComponent* cam = new CameraComponent(xmin, xmax, ylim);

	entity.Add(pos, C_POS);
	entity.Add(cam, C_CAM);
}

void make_boom(Entity &entity, FIXED x, FIXED y, FIXED r, u32 particles) {
	entity.Add(new ExplosionComponent(x, y, r, particles), C_BOOM);
}



#define MENU_STATE 1
#define LEVEL_STATE 2

static int game_state = MENU_STATE;

void level_init(std::vector<Entity*> &entities) {
	tick = 0;

	SetMode(MODE_0 | BG0_ON | BG1_ON | BG2_ON | OBJ_ENABLE);

	game_state = LEVEL_STATE;
	load_level(cur_level, entities); // cur_level = 0
}

const char* msg = "press ^ to start";

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);

	pal(0, 0, 0); // basically same as CpuFastSet(palette, SPRITE_PALETTE, (16/4)| COPY32);
	
	SPRITE_PALETTE[32+7] = RGB8(0x5F,0x57,0x4F);
	SPRITE_PALETTE[48+7] = RGB8(0xC2,0xC3,0xC7);
	SPRITE_PALETTE[64+7] = RGB8(0xFF,0xF1,0xE8);
	SPRITE_PALETTE[80+7] = RGB8(0xFF,0x00,0x4D);
	SPRITE_PALETTE[96+7] = RGB8(0xFF,0xA3,0x00);
	SPRITE_PALETTE[112+7] = RGB8(0xFF,0xEC,0x27);
	
	CpuFastSet(GFX_DATA, TILE_BASE_ADR(0), (GFX_SIZE/4) | COPY32);
	CpuFastSet(GFX_DATA, SPRITE_GFX, (GFX_SIZE/4) | COPY32);
	CpuFastSet(FNT_DATA, SPRITE_GFX + 0x1000, (FNT_SIZE/4) | COPY32);
	CpuFastSet(CIRC_DATA, SPRITE_GFX + 0x2000, (CIRC_SIZE/4) | COPY32);

	SetMode(MODE_0 | BG0_ON | BG1_ON | BG2_ON | OBJ_ENABLE); // BG2_ON enabled during levels

	REG_BG0CNT = TILE_BASE(0) | MAP_BASE(3) | BG_16_COLOR | BG_SIZE_0 | BG_PRIORITY(1);
	REG_BG1CNT = TILE_BASE(0) | MAP_BASE(4) | BG_16_COLOR | BG_SIZE_0 | BG_PRIORITY(2);
	REG_BG2CNT = TILE_BASE(0) | MAP_BASE(5) | BG_16_COLOR | BG_SIZE_0 | BG_PRIORITY(3);

	// background
	u16* map_adr = (u16*) MAP_BASE_ADR(5);
	for (int i = 0; i < 16; i++) {
		for(int j = 0; j < 16; j++) {
			*map_adr = 0x0001;
			map_adr++;
		}
		map_adr += 16;
	}

	*((u32 *)MAP_BASE_ADR(3)) = 0x10011001;
	CpuFastSet(MAP_BASE_ADR(3), MAP_BASE_ADR(3), (0x800/4) | FILL);

	map_adr = (u16*) MAP_BASE_ADR(3);
	for (int i = 0; i < 16; i++) {
		for(int j = 0; j < 16; j++) {
			*map_adr = 0x0000;
			map_adr++;
		}
		map_adr += 16;
	}

	map(0, 0);

	Entity plr;
	make_player(plr, int2fx(32+56), int2fx(16+16));

	Entity door;
	make_door(door, int2fx(16), int2fx(0));

	Entity cam;
	make_cam(cam, 0, 0, 0, 128, 0);

	std::vector<Entity*> entities;
	entities.push_back(&plr);
	entities.push_back(&door);
	entities.push_back(&cam);

	while (1) {
		VBlankIntrWait();
		
		scanKeys();
		kheld = keysHeld();
		kdown = keysDown();

		switch (game_state) {
			case MENU_STATE:
				tick += 1;
				if (btnp(KEY_A)) {
					level_init(entities);
					// sfx (5)

				}
			break;

			case LEVEL_STATE:
				tick += 1;


				if (tick % 3)
					CpuFastSet(GFX_DATA + 0x140, TILE_BASE_ADR(0) + 0x140, (0x18) | COPY32);
				else
					CpuFastSet(GFX_DATA + 0x340, TILE_BASE_ADR(0) + 0x140, (0x18) | COPY32);
				
				system_boom(entities);

				if (cur_level < 8) {
					system_grav(entities);
					system_follow(entities);
					system_player_anim(entities);
					system_player_cam(entities);
					system_p_col(entities);
				}
				else {
					if (tick % 30 == 0) {
						Entity* boom = new Entity;
						make_boom(*boom, fxadd(int2fx(56),rnd(int2fx(128))), fxadd(rnd(int2fx(128)),int2fx(16)), int2fx(2), 25);
						entities.push_back(boom);
					}
				}

				if (!game_over) {
					system_spawn_shadows(entities);
					system_walk(entities);
					system_jump(entities);
				}

				system_mcol(entities);
				system_move(entities);

				if (btnp(5) && game_over) {
					load_level(cur_level, entities);
				}



			break;
		}

		VBlankIntrWait();

		switch (game_state) {
			case MENU_STATE:
				camera(0, 0);
				sspr(76, 30+56, 24+16, OBJ_SHAPE(1), OBJ_SIZE(1), 0, 1);
				// sspr(92, 30+56, 36+16, OBJ_SHAPE(1), OBJ_SIZE(1), 0, 1);

				{
					u8 n2 = 0;
					u32 t = (tick << 10);
					for (int i = 0; i < 16; i++) {
						char c = msg[i];
						u8 n = print_char(c, 0, 0, false);

						u32 t1 = t + (i<<8);
						u8 x = 30+56 + n2;
						u8 y = 59+16 + fx2int(fxmul(lu_cos(t1), int2fx(8)));
						print_char(c, x, y, true);
						n2 += n;
					}
				}
			break;

			case LEVEL_STATE:
				camera(0, 0);
				if (cur_level < 8)
					system_draw(entities);
				else {
					sspr(64, 32+56, 32+16, OBJ_SHAPE(1), OBJ_SIZE(1), 0, 1, false);
					sspr(68, 64+56, 32+16, OBJ_SHAPE(1), OBJ_SIZE(1), 0, 1, false);
					sspr(82, 48+56, 40+16, OBJ_SHAPE(1), OBJ_SIZE(1), 0, 1, false);
					sspr(96, 32+56, 48+16, OBJ_SHAPE(1), OBJ_SIZE(1), 0, 1, false);
					sspr(100, 64+56, 48+16, OBJ_SHAPE(1), OBJ_SIZE(1), 0, 1, false);
				}
				system_boom_draw(entities);
				if (game_over)
					print("press ^ to restart", 24+56, 63+16, 0);
			break;
		}

		update_screen();
	}
}
