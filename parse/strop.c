#include <string.h>
#include <stdlib.h>
#include "strop.h"

string *str_append(string *s, char *text) {
	if (text == NULL) return s;
	if (*text == '\0') {
		return s;
	}

	int len = strlen(text);
	s->value = (char *)realloc(s->value, (s->size+len+1)*sizeof(char));

	s->size += len;
	strcat(s->value, text);

	return s;
}

string *str_append_char(string *s, char ch) {
	s->value = (char *)realloc(s->value, (s->size+2)*sizeof(char));
	s->value[s->size] = ch;
	s->size++;
	s->value[s->size] = '\0';

	return s;
}

static int dbg_created_strings;
static int dbg_reused_buffers;
char *str_get(string *s) {
	char *val = s->value;

	s->value = (char *)calloc(1, sizeof(char));
	s->size = 0;

	dbg_created_strings++;
	dbg_reused_buffers++;

	return val;
}

void init_string(string *s) {
	s->value = (char *)calloc(1, sizeof(char));
	dbg_created_strings++;
	s->size = 0;
	s->append = str_append;
	s->append_char = str_append_char;
	s->get = str_get;
}
