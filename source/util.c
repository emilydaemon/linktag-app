#include <grrlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fat.h>
#include <sys/stat.h>
#include <jansson.h>

#include <winyl/winyl.h>
#include <winyl/request.h>
#include <winyl/header.h>
#include <winyl/version.h>

#include "gui.h"
#include "util.h"
#include "version.h"

// assets
#include "Rubik-Bold_ttf.h"
#include "Inter-Medium_ttf.h"

#include "background_png.h"
#include "prompt_png.h"
#include "prompt_sm_png.h"
#include "button_png.h"
#include "button_hover_png.h"
#include "pointer_png.h"

#include "default-config_json.h"

extern GRRLIB_texImg *background, *prompt, *prompt_sm, *pointer, *button, *button_hover; // UI elements
extern GRRLIB_texImg *fade_buffer;
extern GRRLIB_ttfFont *header_font, *body_font;
extern GRRLIB_texImg *text_layer, *button_layer;

int loading;
extern int is_widescreen;

json_t *config_root;
json_error_t error;

extern char winagent[];

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

void home_quit() {
	if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) quit();
}

