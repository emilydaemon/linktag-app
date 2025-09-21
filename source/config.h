#ifndef CONFIG_H
#define CONFIG_H

typedef struct config {
	char *user_id;
	char *instance_host;
} config;

config *load_config();
void destroy_config(config *cfg);

#endif
