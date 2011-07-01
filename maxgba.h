#ifndef __MAXGBA__
#define __MAXGBA__

typedef struct animation_s{
	int *frames;
	int amt_frames;
	int pause;
} animation;

/*
typedef struct state_s{
	u16 bits;
	int anim_index;		// GOB_ANIM_STAND, GOB_ANIM etc...
	void (*pfunc)( struct gobject_s * );
} state;
*/

typedef struct gobject_s{
	int sprite_number;
	int y_vel;		// actual frames per cycle * 10
	int x_vel;
	int dir;
	int fcounter;
	int pcounter;
	int anim_is_finished;
	u16 state;
	animation **anims;
	int *anim_list;
	int anim_index;
} gobject;

typedef struct command_s{
	void (*pfunc)( gobject * );
	u16 *ok_states;
} command;

typedef struct keys_s {
	u32 key_curr;
	struct keys_s *next;
} keys;

// buffer key macros
#define BUFF_F		0x0010
#define BUFF_DF		0x0090
#define BUFF_D		0x0080
#define BUFF_B		0x0020
#define BUFF_DB		0x00A0
#define BUFF_U		0x0040
#define BUFF_UF		0x0050
#define BUFF_UB		0x0060
#define BUFF_DONE	0x1000		// this should never be a key state

#define KEY_HORZ     0x0030

// these "raw" bitfield macros take shift and mask values directly,
// as opposed to generating them based on Tonc's _name##SHIFT and 
// _name##MASK convention.  they are mainly useful for use with the 
// algorithmically consistent single-bit-per-field structure of sprite states

#define BF_RAW_MSK(_x, _shift, _mask)		( ( (_x) << _shift ) & _mask )
#define BF_RAW_UNMSK(_x, _shift, _mask)		( ( (_x) & _mask ) >> _shift )
#define BF_RAW_INS(_dst, _x, _shift, _mask)	( _dst = ((_dst) & ~_mask) | BF_RAW_MSK(_x, _shift, _mask) )

// These are specifically "single-bit-per-field" macros.
// 1 << _bit is equivalent to (short)pow(2,_bit)

#define BF_SINGLE_MSK(_x, _bit)			( ( (_x) << _bit ) & (1 << _bit) )
#define BF_SINGLE_UNMSK(_x, _bit)		( ( (_x) & (1 << _bit) ) >> _bit )
#define BF_SINGLE_INS(_dst, _x, _bit)		( _dst = ((_dst) & ~(1 << _bit)) | BF_SINGLE_MSK(_x, _bit) )

// these work with pre-shifted values
#define BF_MSK_LAZY(_x, _name)      ( (_x) & _name##_MASK )
#define BF_INS_LAZY(_y, _x, _name)  (_y = ((_y)&~_name##_MASK) | BF_MSK_LAZY(_x,_name) )

#define FALSE			0
#define TRUE			1

#define IMAGE_8x8		0
#define IMAGE_16x16		1
#define IMAGE_32x32		2
#define IMAGE_64x64		3
#define IMAGE_16x8		4
#define IMAGE_32x8		5
#define IMAGE_32x16		6
#define IMAGE_64x32		7
#define IMAGE_8x16		8
#define IMAGE_8x32		9
#define IMAGE_16x32		10
#define IMAGE_32x64		11

#define IMAGE_4BPP		0
#define IMAGE_8BPP		1
#define IMAGE_16BPP		2

INLINE u32 key_tri_shoulder_hit(){
	return bit_tribool( key_hit(-1), KI_R, KI_L );
}

INLINE void set_sprite_pos( sprite_number, x, y ){
	OAM_ENTRY *sprite = &oe_buffer[ sprite_number ];
	oe_set_pos( sprite, x, y );
}

// initialization
void activate_sprites();
void load_sprite_palette( int *pal, int len );
void load_image( int image_number, const unsigned int *tiles, int size, int depth );
int num_tiles( int size, int depth );
void video_mode( int mode );

// key buffer
void get_keys();
void keys_front_insert( u16 key_add );
void init_key_buffer();
int buff_got_keys( int *keylist, int sprite_is_flipped );
int buff_key_down( u16 keys, u16 buffkeys );

// sprites
void set_sprite_image( int sprite_number, int image_number, int palette );
int get_sprite_x( int spr_num );
int get_sprite_y( int spr_num );
void flip_sprite( int spr_num );

// commands
void do_command( gobject *gob, command cmd );

// determines whether set_gob_state runs an AND or an OR
#define ST_SET			0
#define ST_APPEND		1

#define	ST_NONE			-1	// to terminate gob_get_anim() search

// game objects
gobject *make_gobject( int sprite_number, u16 state, animation **anims, int *anim_list );
void set_gob_pos( gobject *gob, int x, int y );
void set_gob_dir( gobject *gob, int dir );
// void handle_gob_state( gobject *gob );
void draw_gob( gobject *gob );
void set_gob_state( gobject *gob, u16 state, int behavior );
void del_gob_state( gobject *gob, u16 state );
void animate_gob( gobject *gob );
animation *get_gob_anim( gobject *gob );
void reset_gob_anim( gobject *gob );
int gob_state_is( gobject *gob, u16 state );

int rand( int min, int max );
static void srand( void );
#endif
