#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "parse/stack.h"

struct edge {
	int v1, v2;
	std::vector<int> face_ids;

	edge(int v1, int v2) : v1(v1), v2(v2) {}

	bool face_exists(int id) const;
};

struct vertex {
	glm::vec3 v;
	//glm::vec3 n;
	//glm::vec2 tc;
	std::vector<int> edge_ids;
	std::vector<int> face_ids;

	vertex(glm::vec3 v) : v(v) {}
	vertex(vec3_t v) : v(glm::vec3(v.x[0], v.x[1], v.x[2])) {}

	bool edge_exists(int id) const;
	bool face_exists(int id) const;
};

class edge_list {
	std::vector<edge> edges;

public:
	int add(int a, int b);
	int get_id(int a, int b) const;
	edge& get(int id);
	int size() const;
	bool exists(int a, int b) const;
};