#ifndef __MAIN__
#define __MAIN__

#define SPR_MAIN		0
#define FLOOR 			86
#define JUMP_HEIGHT 		40

#define ST_STAND		1
#define ST_WALK			2
#define ST_AIR			4
#define ST_CROUCH		8
#define ST_ATTACK		16

/*
#define GOB_ANIM_STAND		0
#define GOB_ANIM_WALK		1
#define GOB_ANIM_JUMP		2
#define GOB_ANIM_CROUCH		3
*/

// gob->anim_index
#define MAIN_ATK_SLASH		4
#define MAIN_ATK_DKSLASH	5
#define MAIN_ATK_PUNCH		6

void load_images();

void fnc_cmd_walk( gobject *gob );
void fnc_cmd_jump( gobject *gob );
void fnc_cmd_crouch( gobject *gob );
void fnc_cmd_attack( gobject *gob );

void fnc_st_stand( gobject *gob );
void fnc_st_walk( gobject *gob );
void fnc_st_air( gobject *gob );
void fnc_st_crouch( gobject *gob );
void fnc_st_stand_atk( gobject *gob );

// ANIMATIONS
static int stand_frames[] = {0};
static animation anim_stand = { stand_frames, 1, 0 };

static int walk_frames[] = {0, 1, 2};			// offset by 1 so tapping shows some movement
static animation anim_walk = { walk_frames, 3, 10 };

static int jump_frames[] = {3};
static animation anim_jump = { jump_frames, 1, 0 };

static int crouch_frames[] = {8};
static animation anim_crouch = { crouch_frames, 1, 0 };

static int slash_frames[] = {4, 5, 6, 7};
static animation anim_slash = { slash_frames, 4, 5 };

static int dkslash_frames[] = {4, 5, 9, 10};
static animation anim_dkslash = { dkslash_frames, 4, 7 };

static int punch_frames[] = {11, 12, 13, 13, 11, 12, 13};
static animation anim_punch = { punch_frames, 7, 5 };

static animation *guy_main_anims[] = {
	&anim_stand,
	&anim_walk,
	&anim_jump,
	&anim_crouch,
	&anim_slash,
	&anim_dkslash,
	&anim_punch
};

// the indices in this list correlate to the indices of
// the above list.  get_gob_anim( gobject *gob ) will iterate through
// gob->anim_list until it finds the bits matching gob->st.
// this list MUST terminate with ST_NONE
static int guy_main_anim_list[] = {
	ST_STAND,
	ST_WALK,
	ST_AIR,
	ST_CROUCH,
	ST_NONE
};

// ATTACKS
// all of them must end with BUFF_DONE
// movements should be listed in reverse order
static int dkslash_keys[] = { BUFF_F, BUFF_DF, BUFF_D, BUFF_DONE };
static int punch_keys[] = { BUFF_B, BUFF_DB, BUFF_D, BUFF_DONE };

// COMMANDS
static u16 cmd_walk_OK = ST_STAND;
static command cmd_walk = {fnc_cmd_walk, &cmd_walk_OK};

static u16 cmd_jump_OK = ST_STAND | ST_WALK | ST_CROUCH;
static command cmd_jump = {fnc_cmd_jump, &cmd_jump_OK};

static u16 cmd_crouch_OK = ST_STAND | ST_WALK;
static command cmd_crouch = {fnc_cmd_crouch, &cmd_crouch_OK}; 

static u16 cmd_attack_OK = ST_STAND | ST_CROUCH | ST_AIR | ST_WALK;
static command cmd_attack = {fnc_cmd_attack, &cmd_attack_OK};

// STATES
/*
static state st_stand = {ST_STAND, GOB_ANIM_STAND, fnc_st_stand};
static state st_walk = {ST_WALK, GOB_ANIM_WALK, fnc_st_walk};
static state st_air = {ST_AIR, GOB_ANIM_JUMP, fnc_st_air};
static state st_crouch = {ST_CROUCH, GOB_ANIM_CROUCH, fnc_st_crouch};
static state st_stand_atk = {ST_STAND_ATK, GOB_ANIM_STAND, fnc_st_stand_atk};
*/

// GUY DATA
static gobject *guy_main;
static gobject *guy_second;

#endif
