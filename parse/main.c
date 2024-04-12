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

	face_t *data_f = (face_t *)cfg->f->storage;
	for (int i = 0; i <= cfg->f->index; i++) {
		face_t val = data_f[i];
		printf("%s\n", face_to_str(&val));
	}
	printf("\n");

	printf("DONE\n");

	return 0;
}