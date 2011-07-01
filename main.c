#include <string.h>
#include <tonc_gba.h>
#include "maxgba.h"
#include "pics.h"
#include "pal.h"
#include "main.h"

int main(){
	int ccount = 0;
	command ai_cmd = cmd_crouch;
	int rnd; 

	// video_mode( 3 );
	activate_sprites();
	load_sprite_palette( palPal, palPalLen );

	load_images();

	init_key_buffer();
	
	guy_main = make_gobject( SPR_MAIN, ST_STAND, guy_main_anims, guy_main_anim_list );
	/*
	guy_second = make_gobject( 1, st_stand, guy_main_anims );
	*/

	set_gob_pos( guy_main, 20, FLOOR );
	/*
	set_gob_pos( guy_second, 90, FLOOR );
	*/

	while( 1 ){
		get_keys();

		/* ***********************
		 * ** MAIN GUY CONTROLS **
		 * *********************** */

		if( key_is_down( KEY_HORZ ) ){
			if( !key_is_down( KEY_R ) ) 
				set_gob_dir( guy_main, key_tri_horz() );
			if( gob_state_is( guy_main, ST_WALK ) )
				guy_main->x_vel = 2 * key_tri_horz();

			do_command( guy_main, cmd_walk );
		}

		if( key_is_down( KEY_DOWN ) ){
			do_command( guy_main, cmd_crouch );
		}

		if( key_hit( KEY_B ) ){
			do_command( guy_main, cmd_jump );
		}

		if( key_hit( KEY_A ) ){
			do_command( guy_main, cmd_attack );
		}
		
		// walls
		if( get_sprite_x( 1 ) <= 1 ||
		    get_sprite_x( 1 ) >= 180 ){
			set_gob_dir( guy_second, guy_second->dir * -1 );
		}

		guy_second->x_vel = 2 * guy_second->dir;

		// shitty AI
		if( ccount++ == 50 ){
			rnd = rand( 0, 15 );
			
			if( rnd >= 0 && rnd < 9 ){
				ai_cmd = cmd_walk;
			} else if( rnd >= 9 && rnd < 12 ){
				ai_cmd = cmd_crouch;
			} else if( rnd >= 12 && rnd <= 15 ){
				ai_cmd = cmd_jump;
			}

			ccount = 0;

			do_command( guy_second, ai_cmd );
		}

		vid_vsync();
		
		handle_gob_state( guy_main );
	//	handle_gob_state( guy_second );

		draw_gob( guy_main );
		
		// draw_gob( guy_second );
		
		oe_update( 0, 2 );
	}

	return 0;
}

void load_images(){
        load_image( 0, walk1Tiles, IMAGE_64x64, IMAGE_4BPP );
        load_image( 1, walk2Tiles, IMAGE_64x64, IMAGE_4BPP );
        load_image( 2, walk3Tiles, IMAGE_64x64, IMAGE_4BPP );
        load_image( 3, jumpTiles, IMAGE_64x64, IMAGE_4BPP );
        load_image( 4, slash1Tiles, IMAGE_64x64, IMAGE_4BPP );
        load_image( 5, slash2Tiles, IMAGE_64x64, IMAGE_4BPP );
        load_image( 6, slash3Tiles, IMAGE_64x64, IMAGE_4BPP );
        load_image( 7, slash4Tiles, IMAGE_64x64, IMAGE_4BPP );
        load_image( 8, crouchTiles, IMAGE_64x64, IMAGE_4BPP );
	load_image( 9, dkslash3Tiles, IMAGE_64x64, IMAGE_4BPP );
	load_image( 10, dkslash4Tiles, IMAGE_64x64, IMAGE_4BPP );
        load_image( 11, punch1Tiles, IMAGE_64x64, IMAGE_4BPP );
	load_image( 12, punch2Tiles, IMAGE_64x64, IMAGE_4BPP );
	load_image( 13, punch3Tiles, IMAGE_64x64, IMAGE_4BPP );
}

// COMMAND FUNCTIONS

void fnc_cmd_walk( gobject *gob ){
	set_gob_state( gob, ST_WALK, ST_SET );
}

void fnc_cmd_jump( gobject *gob ){
	set_gob_state( gob, ST_AIR, ST_SET );
	gob->y_vel = -60;

	// if left/right is down at the beginning of the jump, but the gobject
	// isn't already moving, set the x velocity
	if( key_is_down( KEY_HORZ ) && gob->x_vel == 0 ){
		gob->x_vel = key_tri_horz();
	}
}

void fnc_cmd_crouch( gobject *gob ){
	set_gob_state( gob, ST_CROUCH, ST_SET );
}

void fnc_cmd_attack( gobject *gob ){
	int sprite_is_flipped = gob->dir != 1;

	set_gob_state( gob, ST_ATTACK, ST_APPEND );

	if( buff_got_keys( dkslash_keys, sprite_is_flipped ) ){
		gob->anim_index = MAIN_ATK_DKSLASH;
	} else if( buff_got_keys( punch_keys, sprite_is_flipped ) ){
		gob->anim_index = MAIN_ATK_PUNCH;
	} else {
		gob->anim_index = MAIN_ATK_SLASH;
	}
}

void handle_gob_state( gobject *gob ){
		if( gob->state & ST_STAND ){
			gob->x_vel = 0;
			gob->y_vel = 0;
		}

		if( gob->state & ST_WALK ){
			if( key_released( KEY_HORZ ) ){
				set_gob_state( gob, ST_STAND, ST_SET );
			} else {
				// gob->x_vel = 2 * key_tri_horz();
			}
		}

		if( gob->state & ST_AIR ){
			// < is actually > because 0, 0 is top left
			gob->y_vel += 3;

			if( get_sprite_y( gob->sprite_number ) > FLOOR ){
				gob->y_vel = 0;
				set_gob_pos( gob, get_sprite_x( gob->sprite_number ), FLOOR );
				set_gob_state( gob, ST_STAND, ST_SET );
			}
		}

		if( gob->state & ST_CROUCH ){
			gob->x_vel = 0;

			if( gob->sprite_number == 0 ){
				if( key_released( KEY_DOWN ) ){
					set_gob_state( gob, ST_STAND, ST_SET );
				}
			}
		}

		if( gob->state & ST_ATTACK ){
			if( gob->anim_is_finished ){
				del_gob_state( gob, ST_ATTACK );
			}
		}
}

