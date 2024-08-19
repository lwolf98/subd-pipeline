#pragma once

#include "strop.h"

typedef struct {
	float x[2];
} vec2_t;

typedef struct {
	float x[3];
} vec3_t;

typedef struct {
	int x[4];
} vec4i_t;

typedef struct {
	int m[3][4];
} face_t;

char *vec2_to_str(vec2_t v);
char *vec3_to_str(vec3_t v);
char *vec4i_to_str(vec4i_t v);
char *face_to_str(const face_t *f);
char *face_v_to_str(const face_t *f);
char *face_vt_to_str(const face_t *f);
char *face_vn_to_str(const face_t *f);
char *v_cfg_to_str(const int *v_cfg);

typedef struct stack_t {
	void *storage;
	int index;
	int member_size;

	void (*push)(struct stack_t *, void *val);
	void (*pop)(struct stack_t *, void *out);
	void (*peek)(struct stack_t *, void *out);
	int (*isempty)(struct stack_t *);
	//val_t *(*lookup)(struct stack_t *, char *id);
	void (*pop_to_index)(struct stack_t *, int index);
} stack_t;

stack_t *new_stack(int member_size);

typedef struct {
	int a;
	int b;
	float s;
} crease_t;

typedef struct {
	char *name;
	char *mtllib_path;
	char *material;
	char *source_path;
	stack_t *v;
	stack_t *vt;
	stack_t *vn;
	stack_t *f;
	stack_t *creases;
} cfg_t;

cfg_t *new_cfg(char *source);