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
void subdivide(vector<vertex> &vertices, vector<vec4> &faces);
void write_obj(const vector<vertex> &vertices, const vector<vec4> &faces);

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
		//vertices[vertices.size()-1]
	}
	printf("\n");

	vector<vec4> faces;
	stack_t *data_f = (stack_t *)cfg->f->storage;
	for (int i = 0; i <= cfg->f->index; i++) {
		stack_t *v_list = data_f + i;
		printf("f ");
		vec4 verts;
		verts.x = ((int **)v_list->storage)[0][0] - 1;
		verts.y = ((int **)v_list->storage)[1][0] - 1;
		verts.z = ((int **)v_list->storage)[2][0] - 1;
		verts.w = ((int **)v_list->storage)[3][0] - 1;
		for (int j = 0; j <= v_list->index; j++) {
			int *v_cfg = ((int **)v_list->storage)[j];
			printf("%s ", v_cfg_to_str(v_cfg));
		}
		printf("\n");
		faces.push_back(verts);
	}
	printf("\n");
	printf("PARSING DONE\n");

	for (int i = 0; i < subd_level; i++)
		subdivide(vertices, faces);

	write_obj(vertices, faces);

	return 0;
}

void subdivide(vector<vertex> &vertices, vector<vec4> &faces) {
	// Gather face vertices and edges and update vertex information
	edge_list edges;
	vector<vec3> face_vertices;
	for (uint i = 0; i < faces.size(); i++) {
		vec4& f = faces[i];
		face_vertices.push_back(1.f/4 * (vertices[f.x].v+vertices[f.y].v+vertices[f.z].v+vertices[f.w].v));
		vec3 &f_new = face_vertices[face_vertices.size()-1];
		cout << "f_new: (" << f_new.x << ", " << f_new.y << ", " << f_new.z << ")" << endl;

		int e_id;
		e_id = add_edge(edges, f.x, f.y, i, vertices);
		//update_vertex(vertices, f.x, i, e_id);
		e_id = add_edge(edges, f.y, f.z, i, vertices);
		//update_vertex(vertices, f.y, i, e_id);
		e_id = add_edge(edges, f.z, f.w, i, vertices);
		//update_vertex(vertices, f.z, i, e_id);
		e_id = add_edge(edges, f.w, f.x, i, vertices);
		//update_vertex(vertices, f.w, i, e_id);

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
		if (e.face_ids.size() == 2)
			e_new = 1.f/4 * (vertices[e.v1].v + vertices[e.v2].v + face_vertices[e.face_ids[0]] + face_vertices[e.face_ids[1]]);
		else if (e.face_ids.size() == 1)
			e_new = edge_vertices[i];
		else
			cout << "Error: unhandles face number for edge!!!" << endl;

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

			v_new = 1.f/(relevant_edges+2) * (R + v.v+v.v);
		}
		v_news.push_back(v_new);
		cout << "v_new: (" << v_new.x << ", " << v_new.y << ", " << v_new.z << ")" << endl;
	}

	uint old_vertices = vertices.size();
	int off_vert = 0;
	int off_edge = off_vert + vertices.size();
	int off_face = off_edge + edges.size();

	vector<vec4> new_faces;
	vertices.clear();
	//faces.clear();

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
		vec4 &f = faces[i];
		
		new_faces.push_back(vec4(
			off_vert+f.x,
			off_edge+edges.get_id(f.x, f.y),
			off_face+i,
			off_edge+edges.get_id(f.x, f.w)
		));

		new_faces.push_back(vec4(
			off_edge+edges.get_id(f.y, f.x),
			off_vert+f.y,
			off_edge+edges.get_id(f.y, f.z),
			off_face+i
		));
		
		new_faces.push_back(vec4(
			off_face+i,
			off_edge+edges.get_id(f.z, f.y),
			off_vert+f.z,
			off_edge+edges.get_id(f.z, f.w)
		));
		
		new_faces.push_back(vec4(
			off_edge+edges.get_id(f.w, f.x),
			off_face+i,
			off_edge+edges.get_id(f.w, f.z),
			off_vert+f.w
		));
	}

	faces.clear();
	for (vec4 &f : new_faces) {
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

void write_obj(const vector<vertex> &vertices, const vector<vec4> &faces) {
	// Construct obj format

	// Write vertices
	ofstream outfile;
	outfile.open("out.obj");

	outfile << "o MyCube" << endl;
	for (uint i = 0; i < vertices.size(); i++) {
		const vec3 &v = vertices[i].v;
		outfile << "v " << v.x << " " << v.y << " " << v.z << endl;
	}

	// Write faces
	outfile << endl;
	for (uint i = 0; i < faces.size(); i++) {
		const vec4 &f = faces[i];
		outfile << "f " << f.x+1 << "/0/0 " << f.y+1 << "/0/0 " << f.z+1 << "/0/0 " << f.w+1 << "/0/0" << endl;
	}

	outfile.close();
}