#include <jansson.h>
#include <string.h>

#include "api.h"
#include "gui.h"
#include "http.h"
#include "voorhees.h"

typedef struct user_gamedata_lastplayed_api user_gamedata_lastplayed_api;
typedef struct user_gamedata_api user_gamedata_api;
typedef struct user_api user_api;

// Only call this after configuring the network. You bumbling idiot.
user_api *get_user_api(const char *user_id) {
	char url[128];

	// allocate memory for every struct
	user_gamedata_lastplayed_api *lpp = malloc(sizeof(struct user_gamedata_lastplayed_api));
	user_gamedata_api *gdp = malloc(sizeof(struct user_gamedata_api));
	user_api *p = malloc(sizeof(struct user_api));

	json_error_t error;

	// download API data
	sprintf(url, "/api/user/%s", user_id);
	winyl_response res_api = get_http("tag.rc24.xyz", 80, url);

	json_t *api_root = json_loads(res_api.body, 0, &error);

	if (! api_root) {
		char err_text[256] = "";
		sprintf(err_text, "JSON error on line %d: %s", error.line, error.text);
		easy_error(err_text);
	}

	// get username, we don't need the user ID
	json_t *api_user = voorhees_object_get(api_root, "user");
	const char *api_username = voorhees_string_value(api_user, "name");
	sprintf(url, "/tag-resize-hack.php?id=%s", user_id); // YAWN.
	const char *api_tag_url = url;

	// Now it's time for gamedata.
	json_t *api_game_data = voorhees_object_get(api_root, "game_data");
	// Oh wait, no, we should do last_played.
	json_t *api_last_played = voorhees_object_get(api_game_data, "last_played");
	const char *api_lp_game_id = voorhees_string_value(api_last_played, "game_id");
	const char *api_lp_console = voorhees_string_value(api_last_played, "console");
	const char *api_lp_cover_url = voorhees_string_value(api_last_played, "cover_url");
	const long int api_lp_time = voorhees_integer_value(api_last_played, "time");
	// Alright, back to gamedata. Here's the games array.
	json_t *api_gd_games = voorhees_array_get(api_game_data, "games");

	size_t i;
	json_t *v;
	// we initialize games as 0x00 to clear out any potential garbage data
	char games[10][12+1] = { 0x00 };
	json_array_foreach(api_gd_games, i, v) {
		strcpy(games[i], json_string_value(v));
	}

	// now we map everything to each struct
	// user_api struct
	p->username = malloc(strlen(api_username)+1);
	strcpy(p->username, api_username);

	// this is not read from the API
	p->user_id = malloc(strlen(user_id)+1);
	strcpy(p->user_id, user_id);

	// also not read from the API
	p->tag_url = malloc(strlen(api_tag_url)+1);
	strcpy(p->tag_url, api_tag_url);

	p->game_data = gdp;

	// user_gamedata_api struct
	gdp->last_played = lpp;
	memcpy(gdp->games, games, 10*(12+1)*sizeof(char));

	// user_gamedata_lastplayed_api struct
	//lpp->game_id = "";
	strcpy(lpp->game_id, api_lp_game_id);

	lpp->console = malloc(strlen(api_lp_console)+1);
	strcpy(lpp->console, api_lp_console);

	lpp->cover_url = malloc(strlen(api_lp_cover_url)+1);
	strcpy(lpp->cover_url, api_lp_cover_url);

	lpp->time = api_lp_time;

	return p;
}

void destroy_user_api(user_api *p) {
	if (p == NULL) return;

	// we'll be freeing the structs last
	free(p->username);
	free(p->user_id);
	free(p->tag_url);

	free(p->game_data->last_played->console);
	free(p->game_data->last_played->cover_url);

	// structs
	free(p->game_data->last_played);
	free(p->game_data);
	free(p);
}
