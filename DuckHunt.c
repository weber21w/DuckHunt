/*
 *  Uzebox(tm) Whack-a-Mole
 *  Copyright (C) 2009  Alec Bourque
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Uzebox is a reserved trade mark
*/
#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>

#include "data/patches.inc"
#include "data/music-compressed.inc"
#include "data/title.inc"
#include "data/tiles.inc"
#include "data/sprites1.inc"
#include "data/sprites2.inc"

extern uint8_t ram_tiles[];
extern uint8_t free_tile_index;

#define DUCK_COLOR_BLACK_GREEN	0
#define DUCK_COLOR_BLUE_MAGENTA	1
#define DUCK_COLOR_RED_BLACK	2

uint8_t numTargets;
uint8_t shotsRemaining;
uint8_t level;
uint32_t score;

uint8_t deactivateTimer;

#define MAX_TARGETS	2
#define TARGET_TYPE_BIRD	1
#define TARGET_TYPE_DOG		3

#define TARGET_STATE_UNAVAILABLE	0
#define TARGET_STATE_READY		2
#define TARGET_STATE_ACTIVE		3
#define TARGET_STATE_HIT		5
#define TARGET_STATE_FALLING		7
#define TARGET_STATE_KILLED		8

uint8_t target_type[MAX_TARGETS];
uint8_t target_state[MAX_TARGETS];
uint8_t target_x[MAX_TARGETS];
uint8_t target_y[MAX_TARGETS];
uint8_t target_frame[MAX_TARGETS];

uint8_t game_type;

#define BTN_GUN_TRIGGER	(1U<<12U)
#define BTN_GUN_SENSE	(1U<<13U)

#define FIELD_TOP 5
#define BUTTONS_COUNT 1
#define BUTTON_UNPRESSED 0
#define BUTTON_PRESSED 1

uint8_t calibrateGun();
uint8_t lightgunScan();

#define MAX_GUN_LAG	10
uint8_t gunLag = 5;
uint8_t gunSenseBuf[MAX_GUN_LAG];
void DogIntro();

void TitleScreen(){
	DDRC = 0;
	FadeIn(4,0);
	SetTileTable(title_data);
	ClearVram();
	DrawMap(0,0,title_map);
	//StartSong(mus_got_duck);

DogIntro();
	TriggerFx(4,223,1);
WaitVsync(120);
	TriggerFx(5,223,1);
WaitVsync(120);
	TriggerFx(6,223,1);
WaitVsync(120);
	TriggerFx(7,223,1);
WaitVsync(120);
	TriggerFx(8,255,0);
	TriggerFx(8,255,0);
WaitVsync(120);
	StartSong(mus_title);
WaitVsync(240);
	StartSong(mus_intro);
WaitVsync(240);
WaitVsync(240);
	StartSong(mus_perfect);
WaitVsync(240);
	StartSong(mus_lose);
WaitVsync(240);
	StartSong(mus_got_duck);
WaitVsync(240);
	StartSong(mus_game_over);
WaitVsync(240);
WaitVsync(240);
	StartSong(mus_round_clear);
WaitVsync(240);
	while(1){
		WaitVsync(1);
	}
}

const uint8_t dog_intro_pattern[] PROGMEM = {
7,0, 7,1, 7,2, 7,3,//484 steps...
7,0, 7,1, 7,2, 7,3,
7,0, 7,1, 7,2, 7,3,
7,0, 7,1, 7,2, 7,3,

14,0, 9,4, 9,0, 9,4, 9,0, 10,4,//sniff event

7,0, 7,1, 7,2, 7,3,//4*4 steps
7,0, 7,1, 7,2, 7,3,
7,0, 7,1, 7,2, 7,3,
7,0, 7,1, 7,2, 7,3,

14,0, 9,4, 9,0, 9,4, 9,0, 10,4,//snif event

7,0, 1,1, 18,5, 17,6, 32,7,//step, alarm jump, glide...
255,
};

#define DOG_GROUND_Y 120

void DrawForeground(uint8_t x, uint8_t y, uint8_t w, uint8_t h){
	uint16_t voff = y*VRAM_TILES_H;
	for(uint8_t j=0; j<h; j++){
		for(uint8_t i=0; i<w; i++){
			uint8_t vt = vram[voff++];
			if(vt < RAM_TILES_COUNT){//have a ram_tile[]? check if it was blit over a foreground tile...
				uint8_t mt = pgm_read_byte(&map_main[voff+1]);
				if(mt){//not blue sky?
					uint16_t roff = vt*TILE_WIDTH*TILE_HEIGHT;
					uint16_t toff = mt*TILE_WIDTH*TILE_HEIGHT;
					for(uint8_t k=0; k<TILE_WIDTH*TILE_HEIGHT; k++){
						uint8_t p = pgm_read_byte(&tile_data[toff++]);
						if(p != 0xE2)//not blue sky?
							ram_tiles[roff+k] = p;
					}
				}
			}
		}
		voff += VRAM_TILES_H;
	}
}

void DogIntro(){//full dog walk at beginning of game
	SetTileTable(tile_data);
	uint8_t dx = 0;
	uint8_t dwait = 0;
	uint8_t dframe;
	uint8_t dpos = 0;
	while(1){
		DrawMap2(0, 0, map_main);//we save RAM by not using ram_tiles_restore[], so we must always redraw
		if(dwait)
			dwait--;
		else{
			dwait = pgm_read_byte(&dog_intro_pattern[dpos++]);
			if(dwait == 255)
				break;
			dframe = pgm_read_byte(&dog_intro_pattern[dpos++]);
			if(dframe < 4)
				dx++;
		}
		uint16_t map_pos = (uint16_t)(2+(dframe*7*6));
		uint8_t t;
		free_tile_index = 0;
		for(uint8_t j=0; j<6; j++){
			for(uint8_t i=0; i<7; i++){
				t = pgm_read_byte(&dog_map[map_pos++]);
				if(t)
					BlitSprite(0, t, dx+(i*8), DOG_GROUND_Y+(j*8));
			}
		}
		WaitVsync(1);
	}
}


void LaunchDuck(uint8_t d){/*
	//based on statistical analysis, this is an approximation that avoids a huge table
	uint8_t t;
	if(GetPrngNumber(0) < 154){//~60 chance to come from the right area(see graph under research dir)
		t = 144-(GetPrngNumber(0)%27);//min 116
		t += (GetPrngNumber(0)%34);//max 176
		if(GetPrngNumber(0) > 127)//~50% to move to right peak(second peak on right side roughly)
			t += 48;
	}else{//otherwise come from left area(~22-95), bell curve with 65(22+43) being peak
		t = 64-(GetPrngNumber(0)%43);//min 22
		t += (GetPrngNumber(0)%33);//max 96
	}
	duck_y[d] = DUCK_START_Y;
	duck_x[d] = t;
	duck_wait[d] = 16+(GetPrngNumber(0)%33);
	t = (GetPrngNumber(0)%3);//get duck type
	duck_state[d] = (t<<6);
	t = (level < 20)?level:t;//flyaway times averages max at level 1(~690 frames), and min at level 20(306 frames)
	duck_flyaway[d] = (690-(t*20));//~20 frames less time each round
	duck_flyaway[d] += (GetPrngNumber(0)%60);//this variation seems roughly true through all levels...
	duck_flyaway[d] -= (GetPrngNumber(0)%30);
*/
}


void UpdateDucks(){

}

void TallyDucks(){

}


void UpdateScore(){

}


void Shoot(){

}

void main(){
	SetSpritesTileBank(0, sprite_data1);
	SetSpritesTileBank(1, sprite_data2);
	InitMusicPlayer(patches);
	GetPrngNumber(GetTrueRandomSeed());
	if(GetPrngNumber(0) == 0)
		GetPrngNumber(0xACE);
	SetMasterVolume(224);
	TitleScreen();
	//SetTileTable();
	SetFontTilesIndex(0);
   } 



void SaveVRAM(){
/*
	for(uint16_t i=VRAM_SIZE;i<VRAM_SIZE+(MAX_SPRITES);i++){
		ram_tiles[i] = sprites[i-VRAM_SIZE].flags;
		sprites[i-VRAM_SIZE].flags = SPRITE_OFF;
	}

	for(uint16_t i=0;i<VRAM_SIZE;i++)
		ram_tiles[i] = vram[i];
*/
}


void RestoreVRAM(){
/*
	for(uint16_t i=0;i<VRAM_SIZE;i++)
		vram[i] = ram_tiles[i];

	for(uint16_t i=VRAM_SIZE;i<VRAM_SIZE+MAX_SPRITES;i++)
		sprites[i-VRAM_SIZE].flags = ram_tiles[i];
*/
}


uint8_t calibrateGun(){//returns gun latency frames
	SaveVRAM();

GUN_CALIBRATE_TOP:
	ClearVram();//DDRC=0;
	while(ReadJoypad(0) & BTN_GUN_TRIGGER)//wait for trigger to be released
		WaitVsync(1);

	Print((SCREEN_TILES_H/2)-12,SCREEN_TILES_V-2,PSTR("SHOOT CENTER TO CALIBRATE"));
	while(ReadJoypad(0) & BTN_GUN_SENSE)//wait until we don't sense light on the black screen
		WaitVsync(1);

	while(1){
		WaitVsync(1);
		if(ReadJoypad(0) & BTN_GUN_SENSE)//detected light when we shouldn't have?
			goto GUN_CALIBRATE_TOP;
		if(ReadJoypad(0) & BTN_GUN_TRIGGER)
			break;
	}

	for(uint16_t j=VRAM_SIZE;j<VRAM_SIZE+64;j++){ram_tiles[j] = 0xFF;}//create a white ram_tile if necessary
	for(uint16_t j=0;j<VRAM_SIZE;j++){vram[j] = VRAM_SIZE/64;}//SetTile(j%VRAM_TILES_H,j/VRAM_TILES_H,WHITE_TILE);

	uint8_t k;
	for(k=0;k<MAX_GUN_LAG;k++){//display the white frame until it's detected or timed out
		WaitVsync(1);
		if(ReadJoypad(0) & BTN_GUN_SENSE){
			goto GUN_CALIBRATE_END;		}
	}
	goto GUN_CALIBRATE_TOP;

GUN_CALIBRATE_END:
	RestoreVRAM();
	return k;
}


uint16_t drawTargetMask(uint16_t mask){//draws (active)moles as targets, according to the bitmask and returns the Joypad state
	ClearVram();
	for(uint8_t y=0;y<4;y++){
		for(uint8_t x=0;x<4;x++){
			if(mask & 0b1000000000000000){
				//if(moles[y][x].active)
				//	DrawMap((x*6)+4,(y*4)+FIELD_TOP+3,map_target);
			}
			mask <<= 1;
		}
	}
	WaitVsync(1);
	return ReadJoypad(0);
}


uint8_t lightgunScan(){//performs screen black/white checks to return ((hitColumn<<4)|(hitRow&0x0F)) or 255 for no hit
	uint8_t gunSensePos = 0;
	gunSenseBuf[gunSensePos++] = drawTargetMask(0);//black screen, we shouldn't detect light this frame...

	//filter column(0/1 if sensed, otherwise 2/3)
	gunSenseBuf[gunSensePos++] = drawTargetMask(0b\
1100\
1100\
1100\
1100\
);
	//determine column
	gunSenseBuf[gunSensePos++] = drawTargetMask(0b\
0110\
0110\
0110\
0110\
);


	//verify hit(verifies timing, covers row 3, column 3 case not covered elsewhere)
	gunSenseBuf[gunSensePos++] = drawTargetMask(0b\
1111\
1111\
1111\
1111\
);

	//filter row(0/1 if sensed, otherwise 2/3)
	gunSenseBuf[gunSensePos++] = drawTargetMask(0b\
1111\
1111\
0000\
0000\
);
	//determine row
	gunSenseBuf[gunSensePos++] = drawTargetMask(0b\
0000\
1111\
1111\
0000\
);
	for(uint8_t i=0;i<gunLag;i++){//make sure gun has had time to see entire sequence
		WaitVsync(1);
		gunSenseBuf[gunSensePos++] = ReadJoypad(0);
	}
	for(uint8_t i=0;i<gunLag;i++)//compensate the guns signal pattern for measured lag frames
		gunSenseBuf[i] = gunSenseBuf[i+gunLag];

	uint8_t hitColumn = 255;
	uint8_t hitRow = 255;

	if(!gunSenseBuf[0] && gunSenseBuf[3]){//saw nothing on blank frame and something on all light frame?
		if(gunSenseBuf[1])//saw something on first column filter? then column is 0 or 1
			hitColumn = (gunSenseBuf[2]?1:0);
		else//otherwise column is 2 or 3
			hitColumn = (gunSenseBuf[2]?2:3);

		if(gunSenseBuf[4])//saw something on first row filter? then row is 0 or 1
			hitRow = (gunSenseBuf[5]?1:0);
		else//otherwise row is 2 or 3
			hitRow = (gunSenseBuf[5]?2:3);
	}//otherwise they are pointed at a lightbulb :)

	return (uint8_t)((hitColumn<<4)|(hitRow&0x0F));
}