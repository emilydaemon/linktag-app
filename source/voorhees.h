#ifndef VOORHEES_H
#define VOORHEES_H

const char *voorhees_string_value(json_t *a, char *b);
json_t *voorhees_object_get(json_t *a, char *b);
json_t *voorhees_array_get(json_t *a, char *b);
json_int_t voorhees_integer_value(json_t *a, char *b);

#endif
