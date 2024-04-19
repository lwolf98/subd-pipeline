#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <fstream>

#include "elements.h"
#include "parse/obj.h"
#include "parse/stack.h"

using namespace std;
using namespace glm;

int add_edge(edge_list &edges, int a, int b, int f_id, vector<vertex> &vertices);
void update_vertex(vector<vertex> &vertices, int v_id, int f_id, int e_id);
void subdivide(vector<vertex> &vertices, vector<vector<int>> &faces);
void write_obj(const vector<vertex> &vertices, const vector<vector<int>> &faces, const std::string name);

int main(int argc, char **argv) {
	if (argc < 2 || argc > 3) {
		printf("Wrong usage\n");
		return 1;
	}
	int subd_level = 1;
	if (argc == 3) {
		subd_level = atoi(argv[2]);
	}
	cout << "SUBDIVISION LEVEL: " << subd_level << endl;

	cfg_t *cfg = parse_obj(argv[1]);
	cout << "cfg address: " << cfg << endl;

	vector<vertex> vertices;
	vec3_t *data = (vec3_t *)cfg->v->storage;
	for (int i = 0; i <= cfg->v->index; i++) {
		vec3_t val = data[i];
		printf("v %s\n", vec3_to_str(val));
		vertices.push_back(vertex(val));
	}
	printf("\n");

	vector<vector<int>> faces;
	stack_t *data_f = (stack_t *)cfg->f->storage;
	for (int i = 0; i <= cfg->f->index; i++) {
		stack_t *v_list = data_f + i;
		printf("f ");
		vector<int> verts;
		for (int j = 0; j <= v_list->index; j++) {
			int *v_cfg = ((int **)v_list->storage)[j];
			verts.push_back(v_cfg[0]-1);
			printf("%s ", v_cfg_to_str(v_cfg));
		}
		printf("\n");
		faces.push_back(verts);
	}
	printf("\n");
	printf("PARSING DONE\n");

	for (int i = 0; i < subd_level; i++)
		subdivide(vertices, faces);

	std::string name = "out_" + std::string(cfg->name) + "_" + std::to_string(subd_level) + ".obj";
	write_obj(vertices, faces, name);

	cout << "Name: " << name << endl;

	return 0;
}

void subdivide(vector<vertex> &vertices, vector<vector<int>> &faces) {
	// Gather face vertices and edges and update vertex information
	edge_list edges;
	vector<vec3> face_vertices;
	for (uint i = 0; i < faces.size(); i++) {
		vector<int>& f = faces[i];
		vec3 f_new(0);
		for (uint j = 0; j < f.size(); j++)
			f_new += vertices[f[j]].v;

		f_new = 1.f/f.size() * f_new;
		face_vertices.push_back(f_new);
		cout << "f_new: (" << f_new.x << ", " << f_new.y << ", " << f_new.z << ")" << endl;

		for (uint j = 0; j < f.size(); j++)
			add_edge(edges, f[j], f[(j+1)%f.size()], i, vertices);

	}

	// Calculate current edge vertices
	vector<vec3> edge_vertices;
	for (int i = 0; i < edges.size(); i++) {
		edge &e = edges.get(i);
		edge_vertices.push_back(1.f/2 * (vertices[e.v1].v+vertices[e.v2].v));

		cout << "edge: (" << e.v1 << "," << e.v2 << ") f:";
		for (int j : e.face_ids) {
			cout << " " << j;
		}
		cout << endl;
	}

	// Calculate new edge vertices
	vector<vec3> e_news;
	for (int i = 0; i < edges.size(); i++) {
		edge &e = edges.get(i);
		vec3 e_new;
		int n_faces = e.face_ids.size();
		if (n_faces == 2)
			e_new = 1.f/4 * (vertices[e.v1].v + vertices[e.v2].v + face_vertices[e.face_ids[0]] + face_vertices[e.face_ids[1]]);
		else if (n_faces == 1)
			e_new = edge_vertices[i];
		else if (n_faces > 2) {
			// TODO: verify if this case is equivalent to literature!
			// Mine: implicit handling of an edge with n_faces > 2
			e_new = vertices[e.v1].v + vertices[e.v2].v;
			for (int j = 0; j < n_faces; j++)
				e_new += face_vertices[e.face_ids[j]];

			e_new *= 1.f/(2+n_faces);
		}
		else
			cout << "Error: unhandled face number for edge!!!" << endl;

		e_news.push_back(e_new);
		cout << "e_new: (" << e_new.x << ", " << e_new.y << ", " << e_new.z << ")" << endl;
	}

	// Calculate new vertex points
	vector<vec3> v_news;
	for (uint i = 0; i < vertices.size(); i++) {
		vertex &v = vertices[i];
		uint n = v.edge_ids.size();

		vec3 v_new;
		if (n == v.face_ids.size()) {
			vec3 Q(0), R(0);
			for (uint j = 0; j < n; j++)
				Q += face_vertices[v.face_ids[j]];

			Q /= n;

			for (uint j = 0; j < n; j++)
				R += edge_vertices[v.edge_ids[j]];

			R /= n;

			v_new = 1.f/n * (Q + 2.f*R + (n-3.f)*v.v);
		}
		else {
			int relevant_edges = 0;
			vec3 R(0);
			for (uint j = 0; j < v.edge_ids.size(); j++) {
				// check if edge is at hole
				edge &e = edges.get(v.edge_ids[j]);
				if (e.face_ids.size() != 1)
					continue;

				relevant_edges++;
				R += edge_vertices[v.edge_ids[j]];
			}

			// TODO: verify if this case is equivalent to literature!
			// Mine: double weight the vertex position here
			v_new = 1.f/(relevant_edges+2) * (R + v.v+v.v);
		}
		v_news.push_back(v_new);
		cout << "v_new: (" << v_new.x << ", " << v_new.y << ", " << v_new.z << ")" << endl;
	}

	uint old_vertices = vertices.size();
	int off_vert = 0;
	int off_edge = off_vert + vertices.size();
	int off_face = off_edge + edges.size();

	vector<vector<int>> new_faces;
	vertices.clear();

	// Assign vertices
	for (uint i = 0; i < old_vertices; i++) {
		vertices.push_back(v_news[i]);
	}
	for (vec3 &e : e_news) {
		vertices.push_back(e);
	}
	for (vec3 &f : face_vertices) {
		vertices.push_back(f);
	}

	// Assign faces
	for (uint i = 0; i < faces.size(); i++) {
		vector<int> &f = faces[i];
		
		int n = f.size();
		for (int j = 0; j < n; j++) {
			vector<int> new_f;
			new_f.push_back(off_vert+f[j]);
			new_f.push_back(off_edge+edges.get_id(f[j], f[(j+1)%n]));
			new_f.push_back(off_face+i);
			new_f.push_back(off_edge+edges.get_id(f[j], f[((j-1)%n+n)%n]));
			new_faces.push_back(new_f);
		}
	}

	faces.clear();
	for (vector<int> &f : new_faces) {
		faces.push_back(f);
	}
}

int add_edge(edge_list &edges, int a, int b, int f_id, vector<vertex> &vertices) {
	int e_id = edges.get_id(a, b);
	if (e_id == -1) {
		edges.add(a, b);
		e_id = edges.size()-1;
	}
	edge &e = edges.get(e_id);

	update_vertex(vertices, a, f_id, e_id);
	update_vertex(vertices, b, f_id, e_id);

	vector<int> &face_ids = e.face_ids;
	if (!e.face_exists(f_id)) {
		face_ids.push_back(f_id);
	}

	return e_id;
}

void update_vertex(vector<vertex> &vertices, int v_id, int f_id, int e_id) {
	vertex &v = vertices[v_id];
	if (!v.edge_exists(e_id))
		v.edge_ids.push_back(e_id);

	if (!v.face_exists(f_id))
		v.face_ids.push_back(f_id);
}

void write_obj(const vector<vertex> &vertices, const vector<vector<int>> &faces, const std::string name) {
	// Construct obj format

	// Write vertices
	ofstream outfile;
	//outfile.open("out.obj");
	outfile.open("out/" + name);

	outfile << "o MyCube" << endl;
	for (uint i = 0; i < vertices.size(); i++) {
		const vec3 &v = vertices[i].v;
		outfile << "v " << v.x << " " << v.y << " " << v.z << endl;
	}

	// Write faces
	outfile << endl;
	for (uint i = 0; i < faces.size(); i++) {
		const vector<int> &f = faces[i];
		outfile << "f";
		for (uint j = 0; j < f.size(); j++)
			outfile << " " << f[j]+1  << "/0/0";

		outfile << endl;
	}

	outfile.close();
}