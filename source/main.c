#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <citro3d.h>
#include <citro2d.h>
#include <3ds.h>
#include <time.h>
#include "sprites.h"

int x = 0, y = 0;
int xtemp, ytemp;

int projectileX[5];
int projectileY[5];
int projectileA[5];

float enemyX[3] = {400,400,400};
int enemyY[3] = {50,100,150};
int enemyA[3] = {200,250,300};

float speed = 1;
int speedprecent = 0;

int score = 0;

int i = 0;
int bulletsremaining = 5;

int backgroundscroll = 0;
int backgroundscrollaj = 0;

int temporary = 0;

bool dead = false;

int randomnumb(int cap){
	int temp = rand();
	int temp2 = temp % cap;
	return temp2;
}

int isinsiderange(int pos, int offset, int compare){
	int result = 0;
	if (compare > pos && compare < pos + offset) result = 1;
	return result;
}

int checkifhit(int enemy){
	int result = 0;
	int i2 = 0;
	while(i2 != 5){
		if (projectileA[i2] == 1 && isinsiderange(projectileX[i2], 20, enemyX[enemy]) && isinsiderange(projectileY[i2] - 20, 30, enemyY[enemy])) {
			result = 1;
			projectileA[i2] = 0;
		}
		i2++;
	}
	return result;
}

void killenemy(int enemy, bool hit){
	enemyA[enemy] = 100;
	if (hit) score++;
	else dead = true;
	enemyX[enemy] = 400;
	enemyY[enemy] = randomnumb(220);
}

int main(int argc, char* argv[])
{
	gfxInitDefault(); // init code
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	u32 backgroundColor = C2D_Color32f(0,0,0,1);
	romfsInit();
	C2D_SpriteSheet spritesheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	consoleInit(GFX_BOTTOM, NULL);

	start:
	printf("Hello, world!\n");

	// Main loop
	while (aptMainLoop())
	{
		srand ( time(NULL) );

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW); //begin drawing frame
    	C2D_SceneBegin(top);
    	C2D_TargetClear(top, backgroundColor);

    	// render background
    	C2D_DrawImageAt(C2D_SpriteSheetGetImage(spritesheet, sprites_space_idx), 0 - backgroundscrollaj, 0, 0.5f, NULL, 1, 1);
    	C2D_DrawImageAt(C2D_SpriteSheetGetImage(spritesheet, sprites_space_idx), 400 - backgroundscrollaj, 0, 0.5f, NULL, 1, 1);

    	// render player
    	C2D_DrawImageAt(C2D_SpriteSheetGetImage(spritesheet, sprites_ship_idx), x, y, 0.5f, NULL, 1, 1);

    	i = 0; //bullet render code and physics
    	while(i != 5){
    		if (projectileA[i] == 1){
				C2D_DrawImageAt(C2D_SpriteSheetGetImage(spritesheet, sprites_bullet_idx), projectileX[i], projectileY[i], 0.5f, NULL, 1, 1);
    			if (projectileX[i] > 400) projectileA[i] = 0;
    			projectileX[i] = projectileX[i] + 3;
    		}
    		i++;
    	}

    	i = 0;
    	while(i != 3){ //enemy render code and physics
    		if (enemyA[i] == 1){
    			C2D_DrawImageAt(C2D_SpriteSheetGetImage(spritesheet, sprites_enemy_idx), enemyX[i], enemyY[i], 0.5f, NULL, 1, 1);
    			if (checkifhit(i) == 1) killenemy(i, true);
    			enemyX[i] = enemyX[i] - (1 * speed);
    			if (enemyX[i] < 0) killenemy(i, false); 
    		}
    		else enemyA[i] = enemyA[i] - 1;
    		i++;
    	}

    	C3D_FrameEnd(0); //end frame
		

		hidScanInput(); //check for inputs
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;

		circlePosition pos; // get joystick
		hidCircleRead(&pos);


		backgroundscroll++; // background scroll code
		backgroundscrollaj = backgroundscroll * speed / 3;
		if (backgroundscrollaj >= 400) backgroundscroll = 0;

		
		if (kDown & KEY_A){ // fire a projectile
			i = 0;
			while (i != 5){
				if (projectileA[i] == 0) break;
				i++;
			}

			if (i != 5) projectileX[i] = x + 20, projectileY[i] = y + 15, projectileA[i] = 1;
		}

		if (pos.dx > 25 || pos.dx < -25 || pos.dy > 25 || pos.dy < -25){ //movement code
			xtemp = pos.dx / 75;
			ytemp = pos.dy / 75;

			x = x + xtemp;
			y = y - ytemp;
		} 

		if (x > 368) x = 368; //make sure the player can't leave the screen
		if (x < 0) x = 0;
		if (y > 208) y = 208;
		if (y < 0) y = 0; 

		i = 0; //calculate bulletsremaining
		bulletsremaining = 5;
		while (i != 5){
			if (projectileA[i] == 1) bulletsremaining = bulletsremaining - 1;
			i++;
		}

		speed = speed + 0.0005f; //increase speed
		speedprecent = speed * 100;

		printf("\x1b[2;1Hdx: %d   \ndy: %d    \n\nX: %d   \nY: %d    \n\nBullets: %d   \nScore: %d\nSpeed: %d%%", pos.dx, pos.dy, x, y, bulletsremaining, score, speedprecent);
		
		if (dead) break;
	}

	consoleInit(GFX_BOTTOM, NULL);	

	while(dead){
		printf("\x1b[1;1HYou died!\nPress A to restart, start to exit\n\nFinal score: %d\nSpeed: %d%%", score, speedprecent);
		hidScanInput(); //check for inputs
		u32 kDown = hidKeysDown();
		if (kDown & KEY_A){
			dead = false;
			i = 0;
			temporary = 0;
			projectileA[3] = 0;
			projectileA[4] = 0;
			while(i != 3){
				temporary = temporary + 50;
				projectileA[i] = 0;
				enemyA[i] = 200 + temporary;
				enemyX[i] = 400;
				enemyY[i] = temporary;
				i++;
			}
			speed = 1;
			score = 0;
			bulletsremaining = 5;
			backgroundscroll = 0;
			consoleInit(GFX_BOTTOM, NULL);	
			goto start;
		}
		if (kDown & KEY_START) break;
	}


	romfsExit();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}