/*
 * Easy Jansson, basically. Type checking & shit done automatically
 * The name is a pun.
 */

#include <jansson.h>

#include "voorhees.h"
#include "gui.h"

const char *voorhees_string_value(json_t *a, char *b) {
	json_t *obj = json_object_get(a, b);
	if (! json_is_string(obj)) {
		char err_text[256] = "";
		sprintf(err_text, "voorhees: %s is not a string.", b);
		json_decref(a);
		easy_error(err_text);
	}
	const char *string = json_string_value(obj);
	return string;
}

json_t *voorhees_object_get(json_t *a, char *b) {
	json_t *obj = json_object_get(a, b);
	if (! json_is_object(obj)) {
		char err_text[256] = "";
		sprintf(err_text, "voorhees: %s is not an object.", b);
		json_decref(a);
		easy_error(err_text);
	}
	return obj;
}

json_t *voorhees_array_get(json_t *a, char *b) {
	json_t *obj = json_object_get(a, b);
	if (! json_is_array(obj)) {
		char err_text[256] = "";
		sprintf(err_text, "voorhees: %s is not an array.", b);
		json_decref(a);
		easy_error(err_text);
	}
	return obj;
}

json_int_t voorhees_integer_value(json_t *a, char *b) {
	json_t *obj = json_object_get(a, b);
	if (! json_is_integer(obj)) {
		char err_text[256] = "";
		sprintf(err_text, "voorhees: %s is not an integer.", b);
		json_decref(a);
		easy_error(err_text);
	}
	json_int_t number = json_integer_value(obj);
	return number;
}

bool voorhees_boolean_value(json_t *a, char *b) {
	json_t *obj = json_object_get(a, b);
	if (! json_is_boolean(obj)) {
		char err_text[256] = "";
		sprintf(err_text, "voorhees: %s is not a boolean.", b);
		json_decref(a);
		easy_error(err_text);
	}
	bool value = json_boolean_value(obj);
	return value;
}
