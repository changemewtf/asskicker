#include <string.h>
#include <tonc_gba.h>
#include "maxgba.h"

static int free_tile;

static unsigned int image_tile[512];
static unsigned char image_size[512];

const static unsigned char num_tiles_table_4bpp[] = {
	1,		// IMAGE_8x8
	4,		// IMAGE_16x16
	16,		// IMAGE_32x32
	64,		// IMAGE_64x64
	2,		// IMAGE_16x8
	4,		// IMAGE_32x8
	8,		// IMAGE_32x16
	32,		// IMAGE_64x32
	2,		// IMAGE_8x16
	4,		// IMAGE_8x32
	8,		// IMAGE_16x32
	32		// IMAGE_32x64
};

// const static unsigned int image_shape[] = {
const u16 image_shape[] = {
	_OE_A0_SQUARE,	// IMAGE_8x8
	_OE_A0_SQUARE,	// IMAGE_16x16
	_OE_A0_SQUARE,	// IMAGE_32x32
	_OE_A0_SQUARE,	// IMAGE_64x64
	OE_A0_WIDE,	// IMAGE_16x8
	OE_A0_WIDE,	// IMAGE_32x8
	OE_A0_WIDE,	// IMAGE_32x16
	OE_A0_WIDE,	// IMAGE_64x32
	OE_A0_TALL,	// IMAGE_8x16
	OE_A0_TALL,	// IMAGE_8x32
	OE_A0_TALL,	// IMAGE_16x32
	OE_A0_TALL	// IMAGE_32x64
};

// const static unsigned int image_fsize[] = {
const u16 image_fsize[] = {
	_OE_A1_SIZE_8,	// IMAGE_8x8
	OE_A1_SIZE_16,	// IMAGE_16x16
	OE_A1_SIZE_32,	// IMAGE_32x32
	OE_A1_SIZE_64,	// IMAGE_64x64
	_OE_A1_SIZE_8,	// IMAGE_16x8
	OE_A1_SIZE_16,	// IMAGE_32x8
	OE_A1_SIZE_32,	// IMAGE_32x16
	OE_A1_SIZE_64,	// IMAGE_64x32
	_OE_A1_SIZE_8,	// IMAGE_8x16
	OE_A1_SIZE_16,	// IMAGE_8x32
	OE_A1_SIZE_32,	// IMAGE_16x32
	OE_A1_SIZE_64	// IMAGE_32x64
};

// Initialization 

void activate_sprites(){
	REG_DISPCNT = DCNT_OBJ_ON | DCNT_OBJ_1D;
	oam_init();

	free_tile = 0;
}

void load_sprite_palette( int *pal, int len ){
	memcpy( pal_obj_mem, pal, len );
}
	
void load_image( int image_number, const unsigned int *tiles, int size, int depth ){
	int tile, amt_tiles, len;

	tile = free_tile;
	amt_tiles = num_tiles( size, depth );
	len = amt_tiles * 32;
	
	memcpy( &tile_mem[ CBB_OBJ ][ free_tile ], tiles, len );
	free_tile += amt_tiles;

	image_tile[ image_number ] = tile;
	image_size[ image_number ] = size;
}

int num_tiles( int size, int depth ){
	int num;

	num = num_tiles_table_4bpp[ size ];
	num = num * ( depth + 1 );	// IMAGE_4BPP means * 1, etc...

	return num;
}

void video_mode( int mode ){
	REG_DISPCNT = (REG_DISPCNT & 0xFFF8) | mode;
	
	if ( mode == 3 || mode == 4 ) {
		REG_DISPCNT |= DCNT_BG2_ON;
		free_tile = 512;
/*		video_base = (unsigned char *) 0x06000000;
		if (mode == 4)
			load_web_palette(BG_COLORS); */
	}
}

/* ***************************
 * ****** KEY BUFFER *********
 * *************************** */

static keys *key_buffer;
static amt_keystates;		// amt of key states in the key_buffer
// why does having a lower MAX_KEYS screw things up so much?
// something is throwing the pcounter off kilter
#define MAX_KEYS	10	// maximum amt of key states in memory

void get_keys(){
	key_poll();

	keys_front_insert( key_prev_state() );
}

void keys_front_insert( u16 key_add ){
	keys *temp = malloc( sizeof( keys ) );
	
	temp->key_curr = key_add;
	temp->next = key_buffer;
	key_buffer = temp;

	if( amt_keystates++ > MAX_KEYS ){
		keys *last = key_buffer;
		keys *prev = NULL;
		while( last->next != NULL ){
			prev = last;
			last = last->next;
		}

		prev->next = NULL;
		amt_keystates--;
	}
}

void init_key_buffer(){
	key_buffer = NULL;
}

int buff_got_keys( int *keylist, int sprite_is_flipped ){
	keys *trav = key_buffer;
	int keycounter = 0;	// start at the beginning of the key list (which is the last key of the move)
	
	while( trav != NULL ){
		u16 check_keys = *( keylist + keycounter );

		// bitwise method to do this?
		if( sprite_is_flipped ){
			switch( check_keys ){
				case BUFF_F:
					check_keys = BUFF_B;
				case BUFF_DF:
					check_keys = BUFF_DB;
				case BUFF_UF:
					check_keys = BUFF_UB;
				case BUFF_B:
					check_keys = BUFF_F;
				case BUFF_UB:
					check_keys = BUFF_UF;
				case BUFF_DB:
					check_keys = BUFF_DF;
			}
		}

		// if the current key_buffer contains the current key state in the keylist
		if( buff_key_down( check_keys, trav->key_curr ) ){
			if( *( keylist + ++keycounter ) == BUFF_DONE ){
				return TRUE;
			}
		}

		trav = trav->next;
	}

	// we made it to the end of the key buffer and got nothing
	return FALSE;
}

int buff_key_down( u16 keys, u16 buffkeys ){
	// it seems like you have to release the directional button before
	// hitting the attack button to get a special move off, but if this is
	// a bitwise &, which it is, why is that the case?
	return( buffkeys & keys );
}
	
/* ***************************
 * *** SPRITES ***************
 * *************************** */

void set_sprite_image( int sprite_number, int image_number, int palette ){
	int tile, shape, size, fsize;
	
	OAM_ENTRY *sprite = &oe_buffer[ sprite_number ];

	size = image_size[ image_number ];
	tile = image_tile[ image_number ];
	shape = image_shape[ size ];
	fsize = image_fsize[ size ];
	
	// use bitfield macros to avoid disturbing the rest of the attributes
	BF_INS_LAZY( sprite->attr0, shape, OE_A0_SHAPE );
	BF_INS_LAZY( sprite->attr1, fsize, OE_A1_SIZE );
	sprite->attr2 = OE_A2_BUILD( tile, palette, 0 );
}

int get_sprite_x( int spr_num ){
	OAM_ENTRY *sprite = &oe_buffer[ spr_num ];
	return( BF_UNMSK( sprite->attr1, OE_A1_X ) );
}

int get_sprite_y( int spr_num ){
	OAM_ENTRY *sprite = &oe_buffer[ spr_num ];
	return( BF_UNMSK( sprite->attr0, OE_A0_Y ) );
}

void flip_sprite( int spr_num ){
	OAM_ENTRY *sprite = &oe_buffer[ spr_num ];
	sprite->attr1 ^= OE_A1_HFLIP;
}

/* ***************************
 * ****** COMMANDS ***********
 * *************************** */

void do_command( gobject *gob, command cmd ){
	// do a bitwise AND on the command's OK states and the gobject's state itself
	if( (*cmd.ok_states) & (gob->state) ){
		(*cmd.pfunc)( gob );
	}
}


/* ***************************
 * ******* GAME OBJECTS ******
 * *************************** */

gobject *make_gobject( int sprite_number, u16 state, animation **anims, int *anim_list ){
	gobject *temp = malloc( sizeof( gobject ) );

	temp->sprite_number = sprite_number;
	temp->y_vel = 0;
	temp->x_vel = 0;
	temp->dir = 1;
	temp->fcounter = 0;
	temp->pcounter = 0;
	temp->anim_is_finished = FALSE;
	temp->state = state;
	temp->anims = anims;
	temp->anim_list = anim_list;
	/*
	temp->substates = substates;
	*/
	temp->anim_index = 0;

	return temp;
}

void del_gob_state( gobject *gob, u16 state ){
	state = ~state;
	gob->state &= state;

	reset_gob_anim( gob );
}

void set_gob_state( gobject *gob, u16 state, int behavior ){
	if( behavior == ST_SET ){
		gob->state = state;
	} else {
		gob->state = gob->state | state;
	}

	reset_gob_anim( gob );
}

void reset_gob_anim( gobject *gob ){
	u16 consider, test;
	int anim_index = 0;

	gob->fcounter = 0;
	gob->pcounter = 0;
	gob->anim_is_finished = FALSE;

	// set the anim_index
	
	consider = gob->state;
	test = 0;

	while( test != ST_NONE ){
		test = *(gob->anim_list + anim_index);

		if( consider & test ){
			gob->anim_index = anim_index;
			break;
		}

		anim_index++;
	}
}

void set_gob_pos( gobject *gob, int x, int y ){
	set_sprite_pos( gob->sprite_number, x, y );
}

void set_gob_dir( gobject *gob, int dir ){
	if( gob->dir != dir ){
		gob->dir = dir;
		flip_sprite( gob->sprite_number );
	}
}

void draw_gob( gobject *gob ){
	int sprite_number = gob->sprite_number;
	int x, y;
	
	x = get_sprite_x( sprite_number );
	y = get_sprite_y( sprite_number );

	x += gob->x_vel;
	y += gob->y_vel / 10;

	set_gob_pos( gob, x, y );
	animate_gob( gob );
}

void animate_gob( gobject *gob ){
	animation *anim = get_gob_anim( gob );

	if( anim->amt_frames != 0 ){
		if( gob->pcounter++ == anim->pause ){
			if( gob->fcounter++ == anim->amt_frames - 1 ){
				gob->fcounter = 0;
				gob->anim_is_finished = TRUE;
			}

			set_sprite_image( gob->sprite_number, *( anim->frames + gob->fcounter ), 0 );
			gob->pcounter = 0;
		}
	}
}

animation *get_gob_anim( gobject *gob ){
	return *(gob->anims + gob->anim_index);
}
	
int gob_state_is( gobject *gob, u16 state ){
	return ( gob->state == state );
}

// RANDOM NUMBER STUFF
static int nxti, nxtp;
static unsigned long ma[56];
static int seeded = 0;

#define INITIAL_SEED	161803398L


int rand(int min, int max) {

	unsigned long mj;

	if (!seeded)
		srand();

	if (++nxti == 56) nxti = 1;
	if (++nxtp == 56) nxtp = 1;
	mj = ma[nxti] - ma[nxtp];
	ma[nxti] = mj;

	mj = ((mj / 10000) % (max - min + 1)) + min;
	return (int) mj;

}


static void srand(void) {

	unsigned long mj, mk;
	int i, k;

	mj = INITIAL_SEED;

	ma[55] = mj;
	mk = 1;

	for (i = 1; i <= 54; ++i) {
		int ii = (21 * i) % 55;
		ma[ii] = mk;
		mk = mj - mk;
		mj = ma[ii];
	}

	for (k = 1; k <= 4; ++k)
		for (i = 1; i <= 55; ++i)
			ma[i] -= ma[1 + (i + 30) % 55];

	nxti = 0;
	nxtp = 31;

	seeded = 1;

}
