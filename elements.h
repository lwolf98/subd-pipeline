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
	//glm::vec3 n;
	glm::vec2 tc;
	std::vector<int> edge_ids;
	std::vector<int> face_ids;

	vertex(glm::vec3 v, glm::vec2 tc) : v(v), tc(tc) {}
	vertex(vec3_t v, vec2_t tc) : vertex(
					glm::vec3(v.x[0], v.x[1], v.x[2]),
					glm::vec2(tc.x[0], tc.x[1])) {}
	vertex(glm::vec3 v) : v(v), tc(glm::vec2(0)) {}
	vertex(vec3_t v) : vertex(glm::vec3(v.x[0], v.x[1], v.x[2])) {}

	bool edge_exists(int id) const;
	bool face_exists(int id) const;
};

class edge_list {
	std::vector<edge> edges;

public:
	int add(int a, int b);
	int add(int a, int b, float sharpness);
	void clear();
	int get_id(int a, int b) const;
	edge& get(int id);
	int size() const;
	bool exists(int a, int b) const;
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