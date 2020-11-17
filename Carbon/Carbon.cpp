//Carbon 1.1 Update
//Copyright Joseph Buskmiller

#include "stdafx.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include "structs.h"
#include <carbon_calc.h>
#include <carbon_ren.h>
#include <carbon_phys.h>
#include <carbon_sou.h>
#include <carbon_load.h>
#include <carbon_init.h>

int main(int argc, char* args[]) {
	SketchInit();
	SDL_Window* carbonwind = 0;
	SDL_Renderer* carbonrend = 0;
	SDL_Event carbonevent;
	srand(time(NULL));
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
	SDL_Surface *cursorbmp = SDL_LoadBMP("textures/cursor.bmp");
	SDL_Cursor *cursor = SDL_CreateColorCursor(cursorbmp, 0, 0);
	SDL_SetCursor(cursor);
	SDL_Surface * image = SDL_LoadBMP("textures/backgroundHD.bmp");
	SDL_Texture * texture = SDL_CreateTextureFromSurface(carbonrend, image);
	SDL_Rect rect;
	while (run) {
		clock_t bframe = clock();
		delta = FetchDelta(bframe);

		//
		//INPUT
		
		SDL_PollEvent(&carbonevent);
		GetCursorPos(&m);

		switch (carbonevent.type) {
		case SDL_KEYDOWN:
			keyboard[carbonevent.key.keysym.sym] = 1;
			break;
		case SDL_KEYUP:
			keyboard[carbonevent.key.keysym.sym] = 0;
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
		

		//Sorting Algorithms
		//gnome
		
		if (value[cart] > value[cart + 1]) {
			tempi = value[cart];
			value[cart] = value[cart + 1];
			value[cart + 1] = tempi;
		}
		cart++;
		if (cart > limit) {
			cart = 0;
			limit--;
		}

		//Calculate percent sorted

		sorted = 0;
		for (int i = 0; i < MX; i++) {
			if (value[i] == i + 1) {
				sorted++;
			}
		}

		sorted /= MX;
		sorted *= 100;

		

		//
		//CLEAR FRAME
		SDL_RenderCopy(carbonrend, texture, NULL, NULL);

		//
		//RENDER
		
		//Draw set
		for (int i = 0; i < MX; i++) {
			DWCOLO5.g = (value[i] * 255 ) / MX;
			DWCOLO5.r = 255 - DWCOLO5.g;
			DWCOLO5.b = 0;
			
			pery = (value[i] * r_refh * 0.5) / MX;
			perx = ((i * r_refw) / MX) + padding;
			gap = floor(r_refw / MX) + 1;
			

			lstart.u = perx;
			lstart.v = pery + padding;
			lend.u = perx + gap;
			lend.v = height - pery - padding;

			SketchRenderRectangle(carbonrend, lstart, lend, DWCOLO5);
		}

		//Draw Cartridge
		t1.u = ((cart * r_refw / MX) ) + padding + (gap / 2);
		t1.v = height - (padding / 2);
		t2.u = t1.u - (width / 120);
	    t2.v = t1.v + (width / 120);
		t3.u = t1.u + (width / 120);
		t3.v = t1.v + (width / 120);
		SketchRenderTriangle(carbonrend, t1, t2, t3, DWCOLO3, true);

		//
		//WINDOWS
		
		info = "FPS: " + to_string(int(fps_accepted));
		info += "&*";
		temp = to_string(trunc((1 / fps_accepted) * 10000) / 10);
		temp.erase(temp.find_first_of(".") + 2, 12);
		info += "RT: " + temp + " MS";
		info += "&*";
		info += to_string(r_refh);
		info += "&*";
		SketchWindow(carbonrend, fpssite, "STATS", info);
		

		temp = to_string(sorted);
		temp.erase(temp.find_first_of(".") + 2, 12);

		if(sorted != 100) {
			info = temp;
			info += "&*";
			SketchWindow(carbonrend, ssorted, "% SORTED", info);
		}
		
		if (sorted == 100) {
			SketchBlockText(carbonrend, ssorted, "SORTED");
		}


		SDL_RenderPresent(carbonrend);
		carbonfps = FetchFPS(bframe);
		if (carbonfps != 9874555) {
			fps_accepted = carbonfps;
		}

	}

	SDL_DestroyRenderer(carbonrend);
	SDL_DestroyWindow(carbonwind);
	TTF_Quit();
	SDL_Quit();
	return 0;
}