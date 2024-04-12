#include <stdio.h>

#include "stack.h"

int main() {
	printf("Size: %ld\n", sizeof(int));
	stack_t *s = new_stack(sizeof(int));
	//stack_t *s = s_new(4);
	int num = 3;
	s->push(s, &num);
	num = 6;
	s->push(s, &num);
	num = 1;
	s->push(s, &num);
	num = 3;
	s->push(s, &num);

	while (!s->isempty(s)) {
		int val;
		s->pop(s, &val);
		printf("num: %d\n", val);
	}

	cfg_t *cfg = new_cfg();
	stack_t *v_stack = cfg->v;
	//vec3_t v = { .x = 1, .y = 2, .z = 3 };
	vec3_t v = { .x = {1, 2, 3} };
	cfg->v->push(cfg->v, &v);
	v.x[0] = 4, v.x[1] = 5, v.x[2] = 6;
	cfg->v->push(cfg->v, &v);
	v.x[0] = 7, v.x[1] = 8, v.x[2] = 9;
	cfg->v->push(cfg->v, &v);

	vec3_t *data = cfg->v->storage;
	for (int i = 0; i <= cfg->v->index; i++) {
		vec3_t val = data[i];
		printf("v %s\n", vec3_to_str(val));
	}
	printf("\n");

	while (!cfg->v->isempty(cfg->v)) {
		vec3_t val;
		cfg->v->pop(cfg->v, &val);
		printf("v %s\n", vec3_to_str(val));
	}
	printf("\n");

	face_t face;
	face.m[0][0] = 1, face.m[0][1] = 9, face.m[0][2] = 21, face.m[0][3] = 12;
	cfg->f->push(cfg->f, &face);

	face.m[0][0] = 9, face.m[0][1] = 5,
	face.m[0][2] = 10, face.m[0][3] = 21;
	cfg->f->push(cfg->f, &face);

	face_t *data_f = cfg->f->storage;
	for (int i = 0; i <= cfg->f->index; i++) {
		face_t val = data_f[i];
		printf("%s\n", face_v_to_str(&val));
	}
	printf("\n");
	
	return 0;
}