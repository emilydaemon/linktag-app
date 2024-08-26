#include <grrlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#include "gui.h"
#include "util.h"
#include "hwbutton.h"

// assets
#include "Rubik-Bold_ttf.h"
#include "Inter-Medium_ttf.h"

#include "background_png.h"
#include "prompt_png.h"
#include "prompt_sm_png.h"
#include "button_png.h"
#include "button_hover_png.h"
#include "pointer_png.h"

#define LOADING_MAX 4

GRRLIB_texImg *background, *prompt, *prompt_sm, *pointer, *button, *button_hover; // UI elements
GRRLIB_texImg *fade_buffer;

GRRLIB_ttfFont *header_font, *body_font;

GRRLIB_texImg *text_layer, *button_layer;

int is_widescreen = 0;

extern int loading;
ir_t ir;

float ar_correct(int w) {
	if (is_widescreen) {
		return w * 0.75;
	} else {
		return w;
	}
}

int center_img(int w) {
	float wf = w;
	if (is_widescreen) {
		wf = w * 0.75;
	}
	return (640/2)-(wf/2);
}

void draw_text(int x, int y, const char *string, GRRLIB_ttfFont *font, int size, u32 color) {
	GRRLIB_CompoStart();
	GRRLIB_DrawImg(0, 0, text_layer, 0, 1, 1, 0xFFFFFFFF);
	GRRLIB_PrintfTTF(x, y, font, string, size, color);
	GRRLIB_CompoEnd(0, 0, text_layer);
}

void render_text() {
	GRRLIB_DrawImg(center_img(640), 0, text_layer, 0, ar_correct(1), 1, 0xFFFFFFFF);
}

void render_buttons() {
	GRRLIB_DrawImg(center_img(640), 0, button_layer, 0, ar_correct(1), 1, 0xFFFFFFFF);
}

void render_finish() {
	GRRLIB_Screen2Texture(0, 0, fade_buffer, false);
	GRRLIB_Render();
	GRRLIB_FillScreen(0x000000FF); // avoid graphical issues/flickering
}

void draw_center_text(int y, const char *string, GRRLIB_ttfFont *font, int size, u32 color) {
	int x;
	int w;
	w = GRRLIB_WidthTTF(font, string, size);
	x = (640/2)-(w/2);
	draw_text(x, y, string, font, size, color);
}

void draw_title(const char *string) {
	draw_center_text(89, string, header_font, 36, 0xFFFFFFFF);
}

void draw_body(const char *string) {
	draw_center_text(228, string, body_font, 18, 0xFFFFFFFF);
}

void draw_prompt(bool small) {
	GRRLIB_DrawImg(0, 0, background, 0, 1, 1, 0xFFFFFFFF);
	if (small) {
		GRRLIB_DrawImg(center_img(640), 0, prompt_sm, 0, ar_correct(1), 1, 0xFFFFFFFF);
	} else {
		GRRLIB_DrawImg(center_img(640), 0, prompt, 0, ar_correct(1), 1, 0xFFFFFFFF);
	}
}

void draw_progbar(int x, int y, int w, int h, int prog, int prog_max) {
	w = ar_correct(w);
	prog = prog * 10;
	prog_max = prog_max;
	int prog_filled = (prog / prog_max) * (w/10);
	GRRLIB_Rectangle(x, y, w, h, 0x333333FF, true);
	GRRLIB_Rectangle(x, y, prog_filled, h, 0xFFFFFFFF, true);
}

void draw_prog_prompt() {
	draw_center_text(211, "Please wait...", body_font, 18, 0xFFFFFFFF);
	draw_prompt(1);
	draw_progbar(center_img(320), 241, 320, 24, loading, LOADING_MAX);
	render_text();
	render_finish();
}

void draw_button(int x, int y, char *label, void (*func)(void)) {
	int text_width = GRRLIB_WidthTTF(body_font, label, 18);

	GRRLIB_CompoStart();

	GRRLIB_DrawImg(0, 0, button_layer, 0, 1, 1, 0xFFFFFFFF);

	if (GRRLIB_PtInRect(x, y, ar_correct(200), 60, ir.x, ir.y)) {
		GRRLIB_DrawImg(x, y, button_hover, 0, 1, 1, 0xFFFFFFFF);
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) (*func)();
	} else {
		GRRLIB_DrawImg(x, y, button, 0, 1, 1, 0xFFFFFFFF);
	}

	GRRLIB_CompoEnd(0, 0, button_layer);
	draw_text(((200/2)-(text_width/2))+x, (y + 60/2)-(20/2), label, body_font, 18, 0xFFFFFFFF);
}

void fade_in() {
	int i;
	for (i = 0; i < 256; i = i + 16) {
		GRRLIB_DrawImg(0, 0, background, 0, 1, 1, 0xFFFFFFFF);
		GRRLIB_Rectangle(0, 0, 640, 480, 0x000000FF - i, true);
		GRRLIB_Render();
		usleep(1000);
	}
}

void fade_out() {
	int i;
	for (i = 0; i < 256; i = i + 16) {
		GRRLIB_DrawImg(0, 0, fade_buffer, 0, 1, 1, 0xFFFFFFFF);
		GRRLIB_Rectangle(0, 0, 640, 480, 0x00000000 + i, true);
		GRRLIB_Render();
		usleep(1000);
	}
	GRRLIB_FreeTexture(fade_buffer);
}

void draw_error_prompt() {
	draw_button((640/2)-(200/2), 352, "Return to Loader", quit);
	draw_title("Error");
	//draw_center_text(352, "Press HOME to exit.", body_font, 18, 0xFFFFFFFF);
	draw_prompt(0);
}

void draw_cursor() {
	WPAD_IR(0, &ir);
	if (ir.valid) {
		GRRLIB_DrawImg(ir.x-48, ir.y-48, pointer, ir.angle, ar_correct(1), 1, 0xFFFFFFFF);
	}
}

void easy_error(char *text) {
	while (1) {
		WPAD_ScanPads();
		hwbutton_check();
		draw_body(text);
		draw_error_prompt();
		render_buttons();
		render_text();
		draw_cursor();
		render_finish();
		home_quit();
	}
}
