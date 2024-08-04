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
#include "default-config_json.h"

#define VERSION "1.0"

#define LOADING_MAX 3

GRRLIB_texImg *background, *prompt, *prompt_sm;
GRRLIB_ttfFont *header_font, *body_font;

int loading = 0;
int is_widescreen = 0;
char winagent[32];

json_t *config_root;
json_error_t error;

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

void draw_center_text(int y, const char *string, GRRLIB_ttfFont *font, int size, u32 color) {
	int x;
	int w;
	w = GRRLIB_WidthTTF(font, string, size);
	if (is_widescreen) {
		// what goes here?
	}
	x = (640/2)-(w/2);
	GRRLIB_PrintfTTF(x, y, font, string, size, color);
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

		GRRLIB_Render();
	}
}

void init() {
	GRRLIB_Init();

	WPAD_Init();

	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
		is_widescreen = 1;
	}

	header_font = GRRLIB_LoadTTF(Rubik_Bold_ttf, Rubik_Bold_ttf_size);
	body_font = GRRLIB_LoadTTF(Inter_Medium_ttf, Inter_Medium_ttf_size);

	if (! fatInitDefault()) {
		early_die("Could not initialize FAT device.");
	}

	mkdir("/apps", 0600);
	mkdir("/apps/linktag", 0600);
	mkdir("/apps/linktag/theme", 0600);

	// does config exist?
	FILE *f;
	if (access("/apps/linktag/config.json", F_OK) == 0) {
		// There be config!
		f = fopen("/apps/linktag/config.json", "rb");
		config_root = json_loadf(f, 0, &error);
		fclose(f);
	} else {
		// No config found :(
		f = fopen("/apps/linktag/config.json", "wb");
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
	background = GRRLIB_LoadTextureFromFile("/apps/linktag/theme/background.png");
	prompt = GRRLIB_LoadTextureFromFile("/apps/linktag/theme/prompt.png");
	prompt_sm = GRRLIB_LoadTextureFromFile("/apps/linktag/theme/prompt_sm.png");

	if (background == NULL) { background = GRRLIB_LoadTexture(background_png); };
	if (prompt == NULL) { prompt = GRRLIB_LoadTexture(prompt_png); };
	if (prompt_sm == NULL) { prompt_sm = GRRLIB_LoadTexture(prompt_sm_png); };

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
	draw_prompt(1);
	draw_center_text(211, "Please wait...", body_font, 18, 0xFFFFFFFF);
	draw_progbar(center_img(320), 241, 320, 24, loading, LOADING_MAX);
	GRRLIB_Render();
}

void draw_error_prompt() {
	draw_prompt(0);
	draw_title("Error");
	draw_body("Press HOME to exit.");
}

void quit() {
	GRRLIB_FreeTexture(background);
	GRRLIB_FreeTexture(prompt);
	GRRLIB_FreeTexture(prompt_sm);
	GRRLIB_FreeTTF(header_font);
	GRRLIB_FreeTTF(body_font);
	GRRLIB_Exit();
	fatUnmount(0);
	exit(0);
}

void home_quit() {
	WPAD_ScanPads();
	if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) quit();
}

int main(int argc, char **argv) {
	s32 ret;
	char localip[16] = {0};
	char gateway[16] = {0};
	char netmask[16] = {0};

	char url[128] = "";

	//char *user_id = argv[0];
	//char *user_id = "1098651906768908292";
	GRRLIB_texImg *tag_tex;

	init();

	json_t *user_id_object = json_object_get(config_root, "user_id");
	const char *user_id;

	if (! json_is_string(user_id_object)) {
		while (1) {
			draw_error_prompt();
			draw_body("\"user_id\" in config is not a string.");
			GRRLIB_Render();
			home_quit();
		}
	}

	user_id = json_string_value(user_id_object);

	if (strcmp(user_id, "0") == 0) {
		while (1) {
			draw_error_prompt();
			draw_body("Please edit /apps/linktag/config.json.");
			GRRLIB_Render();
			home_quit();
		}
	}

	loading = 0;
	draw_prog_prompt();
	
	ret = if_config(localip, netmask, gateway, TRUE, 20);
	if (ret < 0) {
		while (1) {
			draw_error_prompt();
			draw_body("Failed to configure network.");
			GRRLIB_Render();
			home_quit();
		}
	}
	loading++; draw_prog_prompt();

	winyl host = winyl_open("donut.eu.org", 80);
	winyl_change_http(&host, WINYL_HTTP_1_0);

	winyl_add_header(&host, "User-Agent", winagent);
	if (host.error != 0) {
		winyl_close(&host);
		while (1) {
			draw_error_prompt();
			draw_body("Failed to create winyl host.");
			GRRLIB_Render();
			home_quit();
		}
	}
	loading++; draw_prog_prompt();

	sprintf(url, "/tag-resize-hack.php?id=%s", user_id);

	winyl_response res = winyl_request(&host, url, 0);
	if (res.status == 404) {
		winyl_response_close(&res);
		winyl_close(&host);
		while (1) {
			draw_error_prompt();
			draw_body("HTTP 404; RiiTag does not exist.");
			GRRLIB_Render();
			home_quit();
		}
	}
	if (res.error != 0) {
		char wtf[64] = "";
		sprintf(wtf, "Unknown libwinyl error %d. CONTACT DEV!", res.error);

		winyl_response_close(&res);
		winyl_close(&host);
		while (1) {
			draw_error_prompt();
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
			GRRLIB_Render();
			home_quit();
		}
	}

	u8 *tag_img = (unsigned char *) res.body;

	tag_tex = GRRLIB_LoadTexture(tag_img);

	while (1) {
		WPAD_ScanPads();

		// If [HOME] was pressed on the first Wiimote, break out of the loop
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)  break;

		draw_prompt(0);
		draw_center_text(352, "Press HOME to exit.", body_font, 18, 0xFFFFFFFF);
		GRRLIB_DrawImg(center_img(514), 143, tag_tex, 0, ar_correct(1), 1, 0xFFFFFFFF);

		GRRLIB_Render();
	}

	winyl_response_close(&res);
	winyl_close(&host);

	quit();
	// we should never reach this point. WTF?
	exit(1);
}
