#ifndef CONFIG_H
#define CONFIG_H

typedef struct config {
	char *user_id;
} config;

config *load_config();
void destroy_config(config *cfg);

#endif
