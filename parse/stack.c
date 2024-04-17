#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "stack.h"
#include "strop.h"

void s_push(stack_t *s, void *val);
void s_pop(stack_t *s, void *out);
void s_peek(stack_t *s, void *out);
int s_isempty(stack_t *s);
//val_t *s_lookup(struct stack_t *s, char *id);
void s_pop_to_index(stack_t *s, int index);

cfg_t *new_cfg() {
	cfg_t *cfg = (cfg_t *)malloc(sizeof *cfg);

	cfg->v = new_stack(sizeof(vec3_t));
	cfg->vt = new_stack(sizeof(vec2_t));
	cfg->vn = new_stack(sizeof(vec3_t));
	cfg->f = new_stack(sizeof(stack_t));

	return cfg;
}

stack_t *new_stack(int member_size) {
	stack_t *stack = (stack_t *)malloc(sizeof *stack);

	stack->index = -1;
	stack->member_size = member_size;
	stack->storage = NULL;
	stack->push = s_push;
	stack->pop = s_pop;
	stack->peek = s_peek;
	stack->isempty = s_isempty;
	//stack->lookup = s_lookup;
	stack->pop_to_index = s_pop_to_index;

	return stack;
}

void s_push(stack_t *s, void *val) {
	s->index++;
	if (s->storage == NULL)
		s->storage = malloc(1 * s->member_size);
	else
		s->storage = realloc(s->storage, (s->index+1) * s->member_size);

	memcpy(((char *)s->storage)+(s->index*s->member_size), val, s->member_size);
}

void s_pop(stack_t *s, void *out) {
	if (s->index < 0) {
		out = NULL;
		return;
	}

	void *value = (char*)s->storage + (s->index * s->member_size);
	s->index--;
	memcpy(out, value, s->member_size);
	s->storage = realloc(s->storage, (s->index+1) * s->member_size);
}

void s_peek(stack_t *s, void *out) {
	if (s->index < 0) {
		out = NULL;
		return;
	}

	void *source = (char*)s->storage + (s->index * s->member_size);
	memcpy(out, source, s->member_size);
}

int s_isempty(stack_t *s) {
	return s->index < 0;
}

/*val_t *s_lookup(stack_t *s, char *id, void *out) {
	val_t *var = NULL;
	for (int i = s->index; i >= 0; i--) {
		var = &s->storage[i];
		if (strcmp(var->id, id) == 0)
			return var;
	}

	return NULL;
}*/

void s_pop_to_index(stack_t *s, int index) {
	if (index < 0) index = 0;
	while (s->index > index) {
		char *val = NULL;
		s->pop(s, val);
	}
}

char *vec2_to_str(vec2_t v) {
	int len = snprintf(NULL, 0, "%f %f", v.x[0], v.x[1]);
	char *result = (char *)malloc(len + 1);
	snprintf(result, len+1, "%f %f", v.x[0], v.x[1]);
	return result;
}
char *vec3_to_str(vec3_t v) {
	int len = snprintf(NULL, 0, "%f %f %f", v.x[0], v.x[1], v.x[2]);
	char *result = (char *)malloc(len + 1);
	snprintf(result, len+1, "%f %f %f", v.x[0], v.x[1], v.x[2]);
	return result;
}
char *vec4i_to_str(vec4i_t v) {
	int len = snprintf(NULL, 0, "%d %d %d %d", v.x[0], v.x[1], v.x[2], v.x[3]);
	char *result = (char *)malloc(len + 1);
	snprintf(result, len+1, "%d %d %d %d", v.x[0], v.x[1], v.x[2], v.x[3]);
	return result;
}
char *face_to_str(const face_t *f) {
	int len = snprintf(NULL, 0,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				f->m[0][0], f->m[1][0], f->m[2][0],
				f->m[0][1], f->m[1][1], f->m[2][1],
				f->m[0][2], f->m[1][2], f->m[2][2],
				f->m[0][3], f->m[1][3], f->m[2][3]);
	char *result = (char *)malloc(len + 1);
	snprintf(result, len+1,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				f->m[0][0], f->m[1][0], f->m[2][0],
				f->m[0][1], f->m[1][1], f->m[2][1],
				f->m[0][2], f->m[1][2], f->m[2][2],
				f->m[0][3], f->m[1][3], f->m[2][3]);
	return result;
}
char *face_v_to_str(const face_t *f) {
	int len = snprintf(NULL, 0,
				"f %d %d %d %d",
				f->m[0][0],
				f->m[0][1],
				f->m[0][2],
				f->m[0][3]);
	char *result = (char *)malloc(len + 1);
	snprintf(result, len+1,
				"f %d %d %d %d",
				f->m[0][0],
				f->m[0][1],
				f->m[0][2],
				f->m[0][3]);
	return result;
}
char *face_vt_to_str(const face_t *f) {
	int len = snprintf(NULL, 0,
				"f %d/%d %d/%d %d/%d %d/%d",
				f->m[0][0], f->m[1][0],
				f->m[0][1], f->m[1][1],
				f->m[0][2], f->m[1][2],
				f->m[0][3], f->m[1][3]);
	char *result = (char *)malloc(len + 1);
	snprintf(result, len+1,
				"f %d/%d %d/%d %d/%d %d/%d",
				f->m[0][0], f->m[1][0],
				f->m[0][1], f->m[1][1],
				f->m[0][2], f->m[1][2],
				f->m[0][3], f->m[1][3]);
	return result;
}
char *face_vn_to_str(const face_t *f) {
	int len = snprintf(NULL, 0,
				"f %d//%d %d//%d %d//%d %d//%d",
				f->m[0][0], f->m[2][0],
				f->m[0][1], f->m[2][1],
				f->m[0][2], f->m[2][2],
				f->m[0][3], f->m[2][3]);
	char *result = (char *)malloc(len + 1);
	snprintf(result, len+1,
				"f %d//%d %d//%d %d//%d %d//%d",
				f->m[0][0], f->m[2][0],
				f->m[0][1], f->m[2][1],
				f->m[0][2], f->m[2][2],
				f->m[0][3], f->m[2][3]);
	return result;
}

char *v_cfg_to_str(const int *v_cfg) {
	int len = snprintf(NULL, 0,
				"%d/%d/%d",
				v_cfg[0], v_cfg[1], v_cfg[2]);
	char *result = (char *)malloc(len + 1);
	snprintf(result, len+1,
				"%d/%d/%d",
				v_cfg[0], v_cfg[1], v_cfg[2]);
	return result;
}
