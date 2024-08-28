#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "parse/stack.h"

struct edge {
	int v1, v2;
	float sharpness;
	std::vector<int> face_ids;

	edge(int v1, int v2, float sharpness) : v1(v1), v2(v2), sharpness(sharpness) {}
	edge(int v1, int v2) : edge(v1, v2, 0.f) {}

	bool face_exists(int id) const;
};

struct vertex {
	glm::vec3 v;
	std::vector<int> edge_ids;
	std::vector<int> face_ids;

	vertex(glm::vec3 v) : v(v) {}
	vertex(vec3_t v) : vertex(glm::vec3(v.x[0], v.x[1], v.x[2])) {}

	bool edge_exists(int id) const;
	bool face_exists(int id) const;
};

class edge_list {
	std::vector<edge> edges;

public:
	int add(int a, int b);
	int add(int a, int b, float sharpness);
	int get_id(int a, int b) const;
	edge& get(int id);
	int size() const;
	bool exists(int a, int b) const;
	void clear();
};

struct vertex_config {
	uint pos;
	uint tc;
};

struct face {
	glm::vec3 normal;
	std::vector<vertex_config> verts;
	uint size() { return verts.size(); }
};