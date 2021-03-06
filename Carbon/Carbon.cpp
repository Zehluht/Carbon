//Boid Unsupervised Simulating Kilobyte

#include "stdafx.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include "structs.h""
#include <carbon_calc.h>
#include <carbon_ren.h>
#include <carbon_phys.h>

SDL_Window* carbonwind = 0;
SDL_Renderer* carbonrend = 0;

bool WithinRadius(site s1, site s2, float radius) {
	if ((pow(pow((s1.u - s2.u), 2) + pow((s1.v - s2.v), 2), 0.5)) < radius) {
		return true;
	}
	return false;
}

float GetDistance(site s1, site s2) {
	float ret = pow(pow((s1.u - s2.u), 2) + pow((s1.v - s2.v), 2), 0.5);
	return ret;
}

////////////////////////
//CONST
////////////////////////
int MAX_BOIDS = 50;
float MAX_SPEED = 3;

////////////////////////
//TEMP
////////////////////////
colo RED;
colo BLU;
colo GRE;
////

///////////////////////
//PERM
///////////////////////
map<int, bool> keyboard;
float carbonfps;
float fps_accepted = 30;
site fpssite;
site versionsite;
string info;
Uint32 * pixels = new Uint32[width * height];
bool run = true;
float fov = 103;
float camdist = 10;
Uint32 BGCOLOR;
Uint32 DWCOLOR;
colo DWCOLO;
colo DWCOLO2;
colo DWCOLO3;
POINT m;
POINT minit;
POINT m2init;
float temp;
bool m1 = 0;
bool m2 = 0;

site s1;
site s2;
site s3;
////

int main(int argc, char* args[]) {
	srand(time(NULL));

	//set up individual window parameters
	fpssite.u = 15; fpssite.v = 60;

	RED.r = 255;
	BLU.b = 255;
	GRE.g = 255;

	//initialize global structs
	cam.rx =  - 1.4 * pi / 2;
	cam.x = -15;
	cam.y = 0;
	cam.z = -2;
	cam.rz = pi / 2;

	sun.rad.x = 0.40290;
	sun.rad.y = -0.31159;
	sun.rad.z = -0.86057;
	sun.stren = 64;
	sun.size = 255;
	sun.ambstren = 0;

	BGCOLOR = RGB(31, 38, 42);
	DWCOLOR = RGB(85, 100, 100);
	DWCOLO.r = 230; DWCOLO.g = 225; DWCOLO.b = 240;
	DWCOLO2.r = 31; DWCOLO2.b = 38; DWCOLO2.g = 42;
	
	//Initialize and spawn boids
	boidset MyBoidSet;
	for (int i = 0; i < MAX_BOIDS; i++) {
		MyBoidSet.boidref[i].pos.u = rand() % 1490;
		MyBoidSet.boidref[i].pos.v = rand() % 950;

		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		MyBoidSet.boidref[i].ang = r * (2 * pi);
		

		MyBoidSet.boidref[i].color.r = 205 + rand() % 50;
		MyBoidSet.boidref[i].color.g = 0 + rand() % 90;
		MyBoidSet.boidref[i].color.b = 0 + rand() % 90;
	}

	SDL_Event carbonevent;
	if (SDL_Init(SDL_INIT_EVERYTHING) >= 0) {
		carbonwind = SDL_CreateWindow("Carbon Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_FULLSCREEN);
		TTF_Init();
		if (carbonwind != 0) {
			carbonrend = SDL_CreateRenderer(carbonwind, -1, SDL_RENDERER_ACCELERATED || SDL_RENDERER_PRESENTVSYNC);
		}
	}
	else {
		return 1;
	}
	
	//load textures
	SDL_Surface * image = SDL_LoadBMP("textures/background.bmp");
	SDL_Texture * texture = SDL_CreateTextureFromSurface(carbonrend, image);
	SDL_Rect rect;

	SDL_Surface *cursorbmp = SDL_LoadBMP("textures/cursor.bmp");
	SDL_Cursor *cursor = SDL_CreateColorCursor(cursorbmp, 0, 0);
	SDL_SetCursor(cursor);

	//load models
	vert torigin;
	mesh boid = FetchMesh("models/boid.busk", 1, torigin);

	while (run) {
		clock_t bframe = clock();
		
		//input
		SDL_PollEvent(&carbonevent);
		GetCursorPos(&m);

		switch (carbonevent.type) {
		case SDL_KEYDOWN:
			keyboard[carbonevent.key.keysym.sym] = true;
			break;
		case SDL_KEYUP:
			keyboard[carbonevent.key.keysym.sym] = false;
			break;
		}
		if (carbonevent.type == SDL_MOUSEBUTTONDOWN) {
			if (carbonevent.button.button == SDL_BUTTON_LEFT) {
				m1 = true;
				GetCursorPos(&minit);
			}
			if (carbonevent.button.button == SDL_BUTTON_RIGHT) {
				m2 = true;
				GetCursorPos(&m2init);
			}
		}
		if (carbonevent.type == SDL_MOUSEBUTTONUP) {
			if (carbonevent.button.button == SDL_BUTTON_LEFT) {
				m1 = false;
			}
			if (carbonevent.button.button == SDL_BUTTON_RIGHT) {
				m2 = false;
			}
		}
		

		if (keyboard[SDLK_ESCAPE]) {
			run = false;
		}

		if (keyboard[SDLK_w]) {
			MyBoidSet.boidref[0].pos.v--;;
		}
		if (keyboard[SDLK_s]) {
			MyBoidSet.boidref[0].pos.v++;
		}
		if (keyboard[SDLK_a]) {
			MyBoidSet.boidref[0].pos.u--;
		}
		if (keyboard[SDLK_d]) {
			MyBoidSet.boidref[0].pos.u++;
		}

		if (keyboard[SDLK_LEFT]) {
			s2.u--;
		}
		if (keyboard[SDLK_RIGHT]) {
			s2.u++;
		}
		if (keyboard[SDLK_UP]) {
			s2.v--;
		}
		if (keyboard[SDLK_DOWN]) {
			s2.v++;
		}
		site debug_center;
		//Transform Boids
		if(1){
			//unify angle
			for (int i = 0; i < MAX_BOIDS; i++) {
				if (MyBoidSet.boidref[i].ang > (2 * pi)) {
					MyBoidSet.boidref[i].ang = 0;
				}
				else {
					if (MyBoidSet.boidref[i].ang < 0) {
						MyBoidSet.boidref[i].ang = 2 * pi;
					}
				}
			}

			for (int i = 0; i < MAX_BOIDS; i++) {
				float center_angle = 0;

				float avoid_center_X = 0;
				float avoid_center_Y = 0;
				float avoid_angle;
				int avoid_count = 0;

				//Separation
				float s_fac = 0;
				//Adhesion
				float a_fac = 0;
				//drive
				float d_fac = 0.01;
				
				site mouse;
				mouse.u = m.x;
				mouse.v = m.y;

				float mouseangle;
				float mouseforce;

				if (WithinRadius(MyBoidSet.boidref[i].pos, mouse, 200) && m1) {
					mouseangle = atan2(MyBoidSet.boidref[i].pos.v - mouse.v, MyBoidSet.boidref[i].pos.u - mouse.u);
					mouseforce = GetDistance(MyBoidSet.boidref[i].pos, mouse);
				}

				for (int j = 0; j < MAX_BOIDS; j++) {		
					if (i != j) {
						if (WithinRadius(MyBoidSet.boidref[i].pos, MyBoidSet.boidref[j].pos, 200)) {
							avoid_center_X += MyBoidSet.boidref[j].pos.u;
							avoid_center_Y += MyBoidSet.boidref[j].pos.v;
							avoid_count++;
						}
					}
				}
				
				s_fac = 0;
				avoid_angle = 0;
				if (avoid_count > 0) {
					avoid_center_X /= avoid_count;
					avoid_center_Y /= avoid_count;
					site ac; ac.u = avoid_center_X; ac.v = avoid_center_Y;
					s_fac = ((GetDistance(ac, MyBoidSet.boidref[i].pos) / -200) + 0.2) * 0.1;
					avoid_angle = atan2(MyBoidSet.boidref[i].pos.v - avoid_center_Y, MyBoidSet.boidref[i].pos.u - avoid_center_X);
				}

				site s1; s1.u = cos(center_angle); s1.v = sin(center_angle);
				site s2; s2.u = cos(MyBoidSet.boidref[i].ang); s2.v = sin(MyBoidSet.boidref[i].ang);
				site s3; s3.u = (s1.u + s2.u) / 2; s3.v = (s1.v + s2.v) / 2;

				float align;
				align = atan2(s3.v, s3.u) - pi;
				MyBoidSet.boidref[i].force.u = (s_fac * cos(avoid_angle)) + (d_fac * cos(MyBoidSet.boidref[i].ang)) + (mouseforce * cos(mouseangle));
				MyBoidSet.boidref[i].force.v = (s_fac * sin(avoid_angle)) + (d_fac * sin(MyBoidSet.boidref[i].ang)) + (mouseforce * sin(mouseangle));
				
				if (MyBoidSet.boidref[i].pos.u > 1490) {
					MyBoidSet.boidref[i].pos.u = -50;
				}	
				if (MyBoidSet.boidref[i].pos.u < -50) {
					MyBoidSet.boidref[i].pos.u = 1490;
				}	
				if (MyBoidSet.boidref[i].pos.v > 950) {
					MyBoidSet.boidref[i].pos.v = -50;
				}
				if (MyBoidSet.boidref[i].pos.v < -50) {
					MyBoidSet.boidref[i].pos.v = 950;
				}
			}
		}
		
		//Apply Forces
		for (int i = 0; i < MAX_BOIDS; i++) {
			MyBoidSet.boidref[i].veloc.u += MyBoidSet.boidref[i].force.u;
			MyBoidSet.boidref[i].veloc.v += MyBoidSet.boidref[i].force.v;

			
			
			//Velocity Cap
			//Calculate speed
			float speed = pow(pow(MyBoidSet.boidref[i].veloc.u, 2) + pow(MyBoidSet.boidref[i].veloc.v, 2), 0.1);
			
			if (speed > MAX_SPEED) {
				speed = MAX_SPEED;
			}
			
			//normalize velocity vector with speed
			MyBoidSet.boidref[i].veloc.u /= speed;
			MyBoidSet.boidref[i].veloc.v /= speed;

			

			MyBoidSet.boidref[i].pos.u += MyBoidSet.boidref[i].veloc.u;
			MyBoidSet.boidref[i].pos.v += MyBoidSet.boidref[i].veloc.v;
			//Turn to face correct direction
			if (MyBoidSet.boidref[i].veloc.v != 0 || MyBoidSet.boidref[i].veloc.u != 0) {
				MyBoidSet.boidref[i].ang = atan2(MyBoidSet.boidref[i].veloc.v, MyBoidSet.boidref[i].veloc.u);
			}
		}

		//Clear Frame/Textures
		SDL_RenderCopy(carbonrend, texture, NULL, NULL);

		//Draw all boids
		for (int i = 0; i < MAX_BOIDS; i++) {
			s1.u = MyBoidSet.boidref[i].pos.u + (50 * cos(MyBoidSet.boidref[i].ang));
			s1.v = MyBoidSet.boidref[i].pos.v + (50 * sin(MyBoidSet.boidref[i].ang));
			s2.u = MyBoidSet.boidref[i].pos.u + (14 * cos(MyBoidSet.boidref[i].ang - (pi / 2)));
			s2.v = MyBoidSet.boidref[i].pos.v + (14 * sin(MyBoidSet.boidref[i].ang - (pi / 2)));
			s3.u = MyBoidSet.boidref[i].pos.u + (14 * cos(MyBoidSet.boidref[i].ang + (pi / 2)));
			s3.v = MyBoidSet.boidref[i].pos.v + (14 * sin(MyBoidSet.boidref[i].ang + (pi / 2)));
			SketchRenderTriangle(carbonrend, s1, s2, s3, MyBoidSet.boidref[i].color, true);
		}

		//Window Calls
		info = "FPS: " + to_string(int(fps_accepted));
		info += "&*";
		info += "RT: " + to_string((1 / fps_accepted) * 1000) + " ms";
		info += "&*";

		vect vcam; vcam.x = cam.rx;
		vcam.y = cam.ry; vcam.z = cam.rz;

		//SketchWindow(carbonrend, fpssite, "Stats", info);
		
		//Update Window States
		SDL_RenderPresent(carbonrend);

		//frame updates
		carbonfps = FetchFPS(bframe);
		if (carbonfps != 9874555) {
			fps_accepted = carbonfps;
		}
	}
	delete[] pixels;
	SDL_DestroyRenderer(carbonrend);
	SDL_DestroyWindow(carbonwind);
	TTF_Quit();
	SDL_Quit();
	return 0;
}