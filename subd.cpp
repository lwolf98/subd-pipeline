#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <fstream>

#include "elements.h"
#include "object.h"
#include "parse/obj.h"
#include "parse/stack.h"

using namespace std;
using namespace glm;

int main(int argc, char **argv) {
	if (argc < 2 || argc > 6) {
		printf("Wrong usage\n");
		return 1;
	}
	int subd_level = 1;
	bool triangulate = false;

	for (int i = 2; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "-l") {
			if (i+1 >= argc) {
				printf("Wrong usage\n");
				return 1;
			}
			subd_level = atoi(argv[++i]);
		}
		else if (arg == "-tri") {
			triangulate = true;
		}
		else {
			printf("Wrong usage\n");
			return 1;
		}
	}

	cout << "SUBDIVISION LEVEL: " << subd_level << endl;

	// Parse obj file)
	cfg_t *cfg = parse_obj(argv[1]);

	// load object from parsed obj config
	object o(cfg);
	printf("PARSING DONE\n");

	for (int i = 0; i < subd_level; i++)
		o.mesh.subdivide();

	if (triangulate)
		o.mesh.triangulate();

	o.name = std::string(o.name) + "_" + std::to_string(subd_level);
	std::string outfile_path = "out/out_" + o.name + ".obj";
	o.write_obj(outfile_path);

	cout << "Name: " << o.name << endl;
	cout << "Out path: " << outfile_path << endl;

	return 0;
}
