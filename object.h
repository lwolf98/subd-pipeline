#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "elements.h"

struct mesh {
	bool has_normals;
	bool has_texture;
	std::vector<vertex> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> tex_coords;
	std::vector<face> faces;
	edge_list creases;
	void subdivide();
	void triangulate();

private:
	int add_edge(edge_list &edges, int a, int b, int f_id);
	void update_vertex(vertex &v, int f_id, int e_id);

	glm::vec3 calc_smooth_edge_vertex(const edge &e, const std::vector<glm::vec3> &face_vertices);
	glm::vec3 calc_sharp_edge_vertex(const edge &e);
	glm::vec3 calc_vertex_vertex(const vertex &v, edge_list &edges, const std::vector<glm::vec3> &edge_vertices, const std::vector<glm::vec3> &face_vertices);
};

struct object {
	std::string name;
	std::string mtllib_path;
	std::string material;
	std::string source_path;

	struct mesh mesh;

	object() : name(""), mtllib_path(""), material("") {}
	object(cfg_t *cfg) : object() {
		init_object(cfg);
	}
	bool has_mtllib() { return mtllib_path != ""; }
	bool has_material() { return material != ""; }
	void write_obj(std::string outfile_name);
	void write_obj() {
		write_obj("out_" + name + ".obj");
	}

private:
	void init_object(cfg_t *);

};