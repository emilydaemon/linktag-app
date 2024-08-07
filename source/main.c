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

#include "Rubik-Bold_ttf.h"
#include "Inter-Medium_ttf.h"

#include "background_png.h"
#include "prompt_png.h"
#include "prompt_sm_png.h"
#include "button_png.h"
#include "button_hover_png.h"
#include "pointer_png.h"

#include "default-config_json.h"

#define VERSION "1.0"

#define LOADING_MAX 4

GRRLIB_texImg *background, *prompt, *prompt_sm, *pointer, *button, *button_hover; // UI elements
GRRLIB_texImg *fade_buffer;

GRRLIB_ttfFont *header_font, *body_font;

GRRLIB_texImg *text_layer, *button_layer;

int loading = 0;
int is_widescreen = 0;
char winagent[32];

json_t *config_root;
json_error_t error;

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

void early_die(char *message) {
	while (1) {
		WPAD_ScanPads();
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) exit(1);

		GRRLIB_PrintfTTF(40, 40, header_font, "Early crash handler", 36, 0xFFFFFFFF);
		GRRLIB_PrintfTTF(40, 100, body_font, message, 18, 0xFFFFFFFF);
		GRRLIB_PrintfTTF(40, 427, body_font, "Press HOME to quit.", 12, 0xCCCCCCFF);

		render_finish();
	}
}

void init() {
	GRRLIB_Init();

	WPAD_Init();
	// IR initialization
	WPAD_SetVRes(0, 640, 480);
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
		is_widescreen = 1;
	}

	header_font = GRRLIB_LoadTTF(Rubik_Bold_ttf, Rubik_Bold_ttf_size);
	body_font = GRRLIB_LoadTTF(Inter_Medium_ttf, Inter_Medium_ttf_size);

	text_layer = GRRLIB_CreateEmptyTexture(640, 480);
	button_layer = GRRLIB_CreateEmptyTexture(640, 480);
	fade_buffer = GRRLIB_CreateEmptyTexture(640, 480);

	if (! fatInitDefault()) {
		early_die("Could not initialize FAT device.");
	}

	mkdir("/apps", 0600);
	mkdir("/apps/linktag-app", 0600);
	mkdir("/apps/linktag-app/theme", 0600);

	// does config exist?
	FILE *f;
	if (access("/apps/linktag-app/config.json", F_OK) == 0) {
		// There be config!
		f = fopen("/apps/linktag-app/config.json", "rb");
		config_root = json_loadf(f, 0, &error);
		fclose(f);
	} else {
		// No config found :(
		f = fopen("/apps/linktag-app/config.json", "wb");
		fwrite(default_config_json, 1, default_config_json_size, f);
		fclose(f);
		config_root = json_loads((char*) default_config_json, 0, &error);
	}

	if (! config_root) {
		char message[256] = "";
		sprintf(message, "JSON error at line %d: %s", error.line, error.text);
		early_die(message);
	}

	// custom theming
	background = GRRLIB_LoadTextureFromFile("/apps/linktag-app/theme/background.png");
	prompt = GRRLIB_LoadTextureFromFile("/apps/linktag-app/theme/prompt.png");
	prompt_sm = GRRLIB_LoadTextureFromFile("/apps/linktag-app/theme/prompt_sm.png");
	pointer = GRRLIB_LoadTextureFromFile("/apps/linktag-app/theme/pointer.png");
	button = GRRLIB_LoadTextureFromFile("/apps/linktag-app/theme/button.png");
	button_hover = GRRLIB_LoadTextureFromFile("/apps/linktag-app/theme/button_hover.png");

	if (background == NULL) { background = GRRLIB_LoadTexture(background_png); };
	if (prompt == NULL) { prompt = GRRLIB_LoadTexture(prompt_png); };
	if (prompt_sm == NULL) { prompt_sm = GRRLIB_LoadTexture(prompt_sm_png); };
	if (pointer == NULL) { pointer = GRRLIB_LoadTexture(pointer_png); };
	if (button == NULL) { button = GRRLIB_LoadTexture(button_png); };
	if (button_hover == NULL) { button_hover = GRRLIB_LoadTexture(button_hover_png); };

	GRRLIB_SetHandle(pointer, 48, 48);

	char *winyl_ver = malloc(winyl_version_len() + 1);
	winyl_version(winyl_ver);
	sprintf(winagent, "libwinyl/%s RiiTag App/" VERSION, winyl_ver);
	free(winyl_ver);
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

void quit() {
	fade_out();
	GRRLIB_FreeTexture(background);
	GRRLIB_FreeTexture(prompt);
	GRRLIB_FreeTexture(prompt_sm);
	GRRLIB_FreeTexture(button);
	GRRLIB_FreeTexture(button_hover);
	GRRLIB_FreeTexture(button_layer);
	GRRLIB_FreeTexture(text_layer);
	GRRLIB_FreeTTF(header_font);
	GRRLIB_FreeTTF(body_font);
	GRRLIB_Exit();
	fatUnmount(0);
	exit(0);
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

void home_quit() {
	if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) quit();
}

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
