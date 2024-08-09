#ifndef GUI_H
#define GUI_H

#include <grrlib.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>

float ar_correct(int w);
int center_img(int w);
void draw_text(int x, int y, const char *string, GRRLIB_ttfFont *font, int size, u32 color);
void render_text();
void render_buttons();
void render_finish();
void draw_center_text(int y, const char *string, GRRLIB_ttfFont *font, int size, u32 color);
void draw_title(const char *string);
void draw_body(const char *string);
void draw_prompt(bool small);
void draw_progbar(int x, int y, int w, int h, int prog, int prog_max);
void draw_prog_prompt();
void draw_button(int x, int y, char *label, void (*func)(void));
void fade_in();
void fade_out();
void draw_error_prompt();
void draw_cursor();
void easy_error(char *text);

#endif
