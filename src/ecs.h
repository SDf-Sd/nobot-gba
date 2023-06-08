#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <gba_types.h>
#include "fixed.h"


#define C_NONE 0
#define C_SPR 1
#define C_POS 2
#define C_PHYS 3
#define C_CTRL 4
#define C_MCOL 5
#define C_DOOR 6
#define C_FLW 7
#define C_CAM 8
#define C_BOOM 9

#define TYPE_NONE 0

struct AbstractComponent {};

struct Animation {
	u8 speed;
	std::vector<u8> frames;
	u8 length;
	bool paused = false;
	bool oneshot;

	void set_paused(bool value) {
		paused = value;
	}

	Animation(u8 _speed, std::vector<u8> _frames, u8 _length, bool _oneshot) : speed(_speed), frames(_frames), length(_length), oneshot(_oneshot) {}
};

struct SprComponent : AbstractComponent {
	// u8 id;
	u8 cur_anim;
	std::vector<Animation> anims;
	bool flip_h = false;
	bool flip_v = false;
	bool visible = true;

	SprComponent(u8 _cur_anim, std::vector<Animation> _anims) : cur_anim(_cur_anim), anims(_anims)  {} 
};

struct PosComponent : AbstractComponent {
	FIXED x, y;
	PosComponent(FIXED _x, FIXED _y) : x(_x), y(_y) {} 
};

struct PhysComponent : AbstractComponent {
	FIXED vx, vy, jh, spd;
	bool onground;
	u8 colbit = 2;
	bool disabled = false;
	PhysComponent(FIXED _spd, FIXED _jh) : jh(_jh), spd(_spd) {}
};

struct FollowPoint {
	FIXED x, y;
	bool flph;
	u8 canm;
};

struct CtrlComponent : AbstractComponent {
	bool jumping, moved, disabled;
	std::map<u32, FollowPoint> prev_positions;
	// prev_pos (struct?)
};

struct MColComponent : AbstractComponent {

};

struct DoorComponent : AbstractComponent {

};

struct Spark {
	FIXED x, y, vx, vy, r, mass;
	bool alive;
};

struct ExplosionComponent : AbstractComponent {
	std::vector<Spark> sparks;

	ExplosionComponent(FIXED x, FIXED y, FIXED r, u32 particle_count) {
		for (u32 i = 0; i < particle_count; i++) {
			sparks.push_back({x, y, fxadd(-256, rnd(512)), fxadd(-256, rnd(512)), fxadd(r, rnd(512)), fxadd(128, rnd(2)), true});
		}
	}
};

struct FollowComponent : AbstractComponent {
	u32 offset;
	FollowComponent(u32 _offset) : offset(_offset) {}
};

struct CameraComponent : AbstractComponent {
	s32 x_min, x_max, y_lim;
	FIXED shake_x = 0, shake_y = 0;
	CameraComponent(u32 _x_min, u32 _x_max, u32 _y_lim) : x_min(_x_min), x_max(_x_max), y_lim(_y_lim) {}
};

struct Entity
{
	u8 typ = TYPE_NONE;
	std::map<u8, AbstractComponent*> cmpt;
	void Add(AbstractComponent* cmp, u8 type) {
		if (cmpt[type] != NULL) return;
		cmpt[type] = cmp;
	}

	~Entity()
	{
		for (std::pair<u8, AbstractComponent*> cmp : cmpt) {
			delete cmp.second;
		}
		cmpt.clear();
	}

};

#endif