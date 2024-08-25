#ifndef API_H
#define API_H

#include <jansson.h>

typedef struct user_gamedata_lastplayed_api {
	char game_id[6+1 /* NULL */];
	char *console;
	// "region" object intentionally left undefined
	char *cover_url;
	long int time;
} user_gamedata_lastplayed_api;

typedef struct user_gamedata_api {
	user_gamedata_lastplayed_api *last_played;
	char games[10][12+1];
} user_gamedata_api;

// GET /api/user/USER_ID
typedef struct user_api {
	char *username;
	char *user_id; // this will not be read from the API, there's no need to
	char *tag_url; // we're actually going to be using our donut.eu.org hackjob URL here.
	user_gamedata_api *game_data;
} user_api;

const char *voorhees_string_value(json_t *a, char *b);
json_t *voorhees_object_get(json_t *a, char *b);
json_t *voorhees_array_get(json_t *a, char *b);

user_api *get_user_api(const char *user_id);

void destroy_user_api(user_api *p);

#endif
