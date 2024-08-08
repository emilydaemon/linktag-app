#include <grrlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <sys/stat.h>

#include <jansson.h>

#include <winyl/winyl.h>
#include <winyl/request.h>
#include <winyl/header.h>
#include <winyl/version.h>

#include "gui.h"
#include "util.h"

#include "default-config_json.h"

extern int is_widescreen;

extern char winagent[32];

extern GRRLIB_texImg *background, *prompt, *prompt_sm, *pointer, *button, *button_hover; // UI elements
extern GRRLIB_texImg *fade_buffer;
extern GRRLIB_ttfFont *header_font, *body_font;
extern GRRLIB_texImg *text_layer, *button_layer;

extern int loading;

extern json_t *config_root;
extern json_error_t error;

ir_t ir;

winyl_response get_http(char *url, int port, char *path) {
	winyl host = winyl_open(url, port);
	winyl_change_http(&host, WINYL_HTTP_1_0);

	winyl_add_header(&host, "User-Agent", winagent);
	if (host.error != 0) {
		winyl_close(&host);
		while (1) {
			WPAD_ScanPads();
			draw_body("Failed to create winyl host.");
			draw_error_prompt();
			render_buttons();
			render_text();
			draw_cursor();
			render_finish();
			home_quit();
		}
	}

	winyl_response res = winyl_request(&host, path, 0);
	if (res.error != 0) {
		char wtf[64] = "";
		sprintf(wtf, "Unknown libwinyl error %d. CONTACT DEV!", res.error);

		winyl_response_close(&res);
		winyl_close(&host);
		while (1) {
			WPAD_ScanPads();
			switch (res.error) {
				case WINYL_ERROR_PORT:
					draw_body("Invalid port (not 0-65535)");
					break;
				case WINYL_ERROR_DNS:
					draw_body("Error calling net_gethostbyname()");
					break;
				case WINYL_ERROR_MALLOC:
					draw_body("Failed to allocate memory");
					break;
				default:
					draw_body(wtf);
					break;
			}
			draw_error_prompt();
			render_buttons();
			render_text();
			draw_cursor();
			render_finish();
			home_quit();
		}
	}

	if (res.status != 200) {
		char err_text[256] = "";
		sprintf(err_text, "HTTP %d on %s:%d%s", res.status, url, port, path);
		winyl_response_close(&res);
		winyl_close(&host);
		while (1) {
			WPAD_ScanPads();
			draw_body(err_text);
			draw_error_prompt();
			render_buttons();
			render_text();
			draw_cursor();
			render_finish();
			home_quit();
		}
	}

	winyl_close(&host);

	return res;
}

int main(int argc, char **argv) {
	s32 ret;
	char localip[16] = {0};
	char gateway[16] = {0};
	char netmask[16] = {0};

	char url[128] = "";

	GRRLIB_texImg *tag_tex;

	winyl_response res_img, res_api;

	init();

	fade_in();

	json_t *user_id_object = json_object_get(config_root, "user_id");
	const char *user_id;

	if (! json_is_string(user_id_object)) {
		while (1) {
			WPAD_ScanPads();
			draw_body("\"user_id\" in config is not a string.");
			draw_error_prompt();
			render_buttons();
			render_text();
			draw_cursor();
			render_finish();
			home_quit();
		}
	}

	user_id = json_string_value(user_id_object);

	if (strcmp(user_id, "0") == 0) {
		while (1) {
			WPAD_ScanPads();
			draw_body("Please edit /apps/linktag-app/config.json.");
			draw_error_prompt();
			render_buttons();
			render_text();
			draw_cursor();
			render_finish();
			home_quit();
		}
	}

	loading = 0;
	draw_prog_prompt();
	
	ret = if_config(localip, netmask, gateway, TRUE, 20);
	if (ret < 0) {
		while (1) {
			WPAD_ScanPads();
			draw_body("Failed to configure network.");
			draw_error_prompt();
			render_buttons();
			render_text();
			draw_cursor();
			render_finish();
			home_quit();
		}
	}

	loading++; draw_prog_prompt();

	sprintf(url, "/api/user/%s", user_id);
	res_api = get_http("tag.rc24.xyz", 80, url);
	loading++; draw_prog_prompt();

	json_t *api_root, *api_username_obj, *api_user;
	const char *username;

	api_root = json_loads(res_api.body, 0, &error);

	if (! api_root) {
		char err_text[256] = "";
		sprintf(err_text, "JSON error on line %d: %s", error.line, error.text);
		while (1) {
			WPAD_ScanPads();
			draw_body(err_text);
			draw_error_prompt();
			render_buttons();
			render_text();
			draw_cursor();
			render_finish();
			home_quit();
		}
	}

	api_user = json_object_get(api_root, "user");
	api_username_obj = json_object_get(api_user, "name");
	username = json_string_value(api_username_obj);

	sprintf(url, "/tag-resize-hack.php?id=%s", user_id);
	res_img = get_http("donut.eu.org", 80, url);
	loading++; draw_prog_prompt();

	u8 *tag_img = (unsigned char *) res_img.body;

	tag_tex = GRRLIB_LoadTexture(tag_img);

	char title[128];
	sprintf(title, "%s's tag", username);
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
	winyl_response_close(&res_api);

	quit();
	// we should never reach this point. WTF?
	exit(1);
}
