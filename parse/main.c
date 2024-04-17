#include <stdio.h>

#include "obj.h"
#include "stack.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Wrong usage\n");
		return 1;
	}

	cfg_t *cfg = parse_obj(argv[1]);

	vec3_t *data = (vec3_t *)cfg->v->storage;
	for (int i = 0; i <= cfg->v->index; i++) {
		vec3_t val = data[i];
		printf("v %s\n", vec3_to_str(val));
	}
	printf("\n");

	vec2_t *data_vt = (vec2_t *)cfg->vt->storage;
	for (int i = 0; i <= cfg->vt->index; i++) {
		vec2_t val = data_vt[i];
		printf("vt %s\n", vec2_to_str(val));
	}
	printf("\n");

	vec3_t *data_vn = (vec3_t *)cfg->v->storage;
	for (int i = 0; i <= cfg->v->index; i++) {
		vec3_t val = data_vn[i];
		printf("vn %s\n", vec3_to_str(val));
	}
	printf("\n");

	/*face_t *data_f = (face_t *)cfg->f->storage;
	for (int i = 0; i <= cfg->f->index; i++) {
		face_t val = data_f[i];
		printf("%s\n", face_to_str(&val));
	}
	printf("\n");*/

	stack_t *data_f = (stack_t *)cfg->f->storage;
	for (int i = 0; i <= cfg->f->index; i++) {
		stack_t *v_list = data_f + i;
		printf("f ");
		for (int j = 0; j <= v_list->index; j++) {
			int *v_cfg = ((int **)v_list->storage)[j];
			printf("%s ", v_cfg_to_str(v_cfg));
		}
		printf("\n");
	}
	printf("\n");

	printf("DONE\n");

	return 0;
}