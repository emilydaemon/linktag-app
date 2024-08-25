#include <unistd.h>
#include <jansson.h>
#include <string.h>

#include "config.h"
#include "voorhees.h"
#include "util.h"

#include "default-config_json.h"

#define CONFIG_FILE "/apps/linktag-app/config.json"

typedef struct config config;

// Initialize FAT before calling this. You scrambling moron.
config *load_config() {
	config *p = malloc(sizeof(struct config));

	json_t *config_root;
	json_error_t error;

	// does config exist?
	FILE *f;
	if (access(CONFIG_FILE, F_OK) == 0) {
		// There be config!
		f = fopen(CONFIG_FILE, "rb");
		config_root = json_loadf(f, 0, &error);
		fclose(f);
	} else {
		// No config found :(
		f = fopen(CONFIG_FILE, "wb");
		fwrite(default_config_json, 1, default_config_json_size, f);
		fclose(f);
		config_root = json_loads((char*) default_config_json, 0, &error);
	}

	if (! config_root) {
		char message[256] = "";
		sprintf(message, "config.json: JSON error at line %d: %s", error.line, error.text);
		json_decref(config_root);
		early_die(message);
	}

	const char *config_user_id = voorhees_string_value(config_root, "user_id");

	p->user_id = malloc(strlen(config_user_id)+1);
	strcpy(p->user_id, config_user_id);

	json_decref(config_root);
	return p;
}

void destroy_config(config *p) {
	free(p->user_id);
	free(p);
}
