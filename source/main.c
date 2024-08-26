#include <grrlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <wiiuse/wpad.h>

#include <jansson.h>

#include "gui.h"
#include "util.h"
#include "http.h"
#include "api.h"
#include "config.h"

extern int is_widescreen;

extern char winagent[];

extern GRRLIB_texImg *background, *prompt, *prompt_sm, *pointer, *button, *button_hover; // UI elements
extern GRRLIB_texImg *fade_buffer;
extern GRRLIB_ttfFont *header_font, *body_font;
extern GRRLIB_texImg *text_layer, *button_layer;

extern int loading;

extern json_t *config_root;
extern json_error_t error;

extern user_api *api_res;
extern config *cfg;

int main(int argc, char **argv) {
	s32 ret;
	char localip[16] = {0};
	char gateway[16] = {0};
	char netmask[16] = {0};

	char url[128] = "";

	GRRLIB_texImg *tag_tex;

	winyl_response res_img;

	init();

	fade_in();

	if (strcmp(cfg->user_id, "0") == 0) {
		easy_error("Please edit /apps/linktag-app/config.json.");
	}

	loading = 0;
	draw_prog_prompt();

	// configure network
	ret = if_config(localip, netmask, gateway, TRUE, 20);
	if (ret < 0) {
		easy_error("Failed to configure network.");
	}

	loading++; draw_prog_prompt();

	// get API data
	api_res = get_user_api(cfg->user_id);
	loading++; draw_prog_prompt();

	// get tag image
	sprintf(url, "/tag-resize-hack.php?id=%s", cfg->user_id);
	res_img = get_http("donut.eu.org", 80, url);
	loading++; draw_prog_prompt();

	u8 *tag_img = (unsigned char *) res_img.body;

	tag_tex = GRRLIB_LoadTexture(tag_img);

	char title[128];
	sprintf(title, "%s's tag", api_res->username);
	while (1) {
		WPAD_ScanPads();

		// If [HOME] was pressed on the first Wiimote, break out of the loop
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)  break;

		draw_title(title);
		draw_button((640/2)-(200/2), 352, "Return to Loader", quit);
		draw_prompt(0);
		GRRLIB_DrawImg(center_img(514), 143, tag_tex, 0, ar_correct(1), 1, 0xFFFFFFFF);
		render_buttons();
		render_text();

		draw_cursor();
		render_finish();
	}

	winyl_response_close(&res_img);

	quit();
	// we should never reach this point. WTF?
	exit(1);
}
