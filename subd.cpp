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
void subdivide(vector<vertex> &vertices, vector<vector<int>> &faces, vector<vec3> &normals);
void triangulate(const vector<vertex> &vertices, vector<vector<int>> &faces, vector<vec3> &normals);
void write_obj(const vector<vertex> &vertices, const vector<vector<int>> &faces, const vector<vec3> &normals, const std::string name);

vec3 calc_smooth_edge_vertex(const edge &e, const vector<vertex> &vertices, const vector<vec3> &face_vertices);
vec3 calc_sharp_edge_vertex(const edge &e, const vector<vertex> &vertices);
vec3 calc_vertex_vertex(const vertex &v, edge_list &edges, const vector<vec3> &edge_vertices, const vector<vec3> &face_vertices);

bool has_normals;
bool has_tc;
bool triagnulation = true;

edge_list creases;

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

	// Crease configuration
	int c_id = creases.add(0, 4);
	creases.get(c_id).sharpness = 1.f;
	c_id = creases.add(4, 6);
	creases.get(c_id).sharpness = 1.f;

	// Parse obj file
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

	vector<vec3> normals;
	vec3_t *data_normals = (vec3_t *)cfg->vn->storage;
	for (int i = 0; i <= cfg->vn->index; i++) {
		vec3_t normal = data_normals[i];
		printf("vn %s\n", vec3_to_str(normal));
		normals.push_back(glm::vec3(normal.x[0], normal.x[1], normal.x[2]));
	}

	vec2_t *data_texcoords = (vec2_t *)cfg->vt->storage;
	for (int i = 0; i <= cfg->vt->index; i++) {
		vec2_t tc = data_texcoords[i];
		printf("vt %s\n", vec2_to_str(tc));
		//...
		//vertices[i].tc = glm::vec2(tc.x[0], tc.x[1]);
	}

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

	has_normals = cfg->vn->index != -1;
	has_tc = cfg->vt->index != -1;

	printf("\n");
	printf("PARSING DONE\n");

	for (int i = 0; i < subd_level; i++)
		subdivide(vertices, faces, normals);

	if (triagnulation)
		triangulate(vertices, faces, normals);

	std::string name = std::string(cfg->name) + "_" + std::to_string(subd_level);
	write_obj(vertices, faces, normals, name);

	cout << "Name: " << name << endl;

	return 0;
}

void subdivide(vector<vertex> &vertices, vector<vector<int>> &faces, vector<vec3> &normals) {
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

	// Assign edge crease sharpness
	for (int i = 0; i < creases.size(); i++) {
		edge &c = creases.get(i);
		edge &e = edges.get(edges.get_id(c.v1, c.v2));
		e.sharpness = c.sharpness;
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

		if (e.sharpness <= 0) {
			// Smooth edge

			e_new = calc_smooth_edge_vertex(e, vertices, face_vertices);
		}
		else if(e.sharpness >= 1.f) {
			// Infinitely Sharp edge

			e_new = edge_vertices[i];
		}
		else {
			// Semi-sharp edge

			e_new = vec3(0);
		}

		e_news.push_back(e_new);
		cout << "e_new: (" << e_new.x << ", " << e_new.y << ", " << e_new.z << ")" << endl;
	}

	// Calculate new vertex points
	vector<vec3> v_news;
	for (uint i = 0; i < vertices.size(); i++) {
		vertex &v = vertices[i];
		vector<edge *> sharp_edges;
		vec3 v_new;

		for (uint j = 0; j < v.edge_ids.size(); j++) {
			edge &v_edge = edges.get(v.edge_ids[j]);
			if (v_edge.sharpness > 0)
				sharp_edges.push_back(&v_edge);
		}
		
		uint n_sharp_edges = sharp_edges.size();
		if (n_sharp_edges <= 1) {
			// zero or one adjacent sharp edges -> smooth vertex rule
			v_new = calc_vertex_vertex(v, edges, edge_vertices, face_vertices);
		}
		else if (n_sharp_edges == 2) {
			// two adjacent sharp edges -> crease rule (or blend between crease vertex and corner mask)
			vec3 e1 = calc_sharp_edge_vertex(*sharp_edges[0], vertices);
			vec3 e2 = calc_sharp_edge_vertex(*sharp_edges[1], vertices);
			v_new = 1.f/8 * (6.f * v.v + e1 + e2);
		}
		else {
			// three or more adjacent sharp edges -> corner rule
			v_new = v.v;
		}

		v_news.push_back(v_new);
		cout << "v_new: (" << v_new.x << ", " << v_new.y << ", " << v_new.z << ")" << endl;
	}

	uint old_vertices = vertices.size();
	int off_vert = 0;
	int off_edge = off_vert + vertices.size();
	int off_face = off_edge + edges.size();

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
	vector<vector<int>> new_faces;
	normals.clear();
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

			glm::vec3 u = vertices[new_f[0]].v - vertices[new_f[1]].v;
			glm::vec3 v = vertices[new_f[2]].v - vertices[new_f[1]].v;
			glm::vec3 normal = glm::normalize(glm::cross(v, u));
			normals.push_back(normal);
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
		e_id = edges.add(a, b);
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

// Calculate smooth edge vertex
vec3 calc_smooth_edge_vertex(const edge &e, const vector<vertex> &vertices, const vector<vec3> &face_vertices) {
	int n_faces = e.face_ids.size();
	if (n_faces == 2)
		return 1.f/4 * (vertices[e.v1].v + vertices[e.v2].v + face_vertices[e.face_ids[0]] + face_vertices[e.face_ids[1]]);
	else if (n_faces == 1)
		//return edge_vertices[i];
		return calc_sharp_edge_vertex(e, vertices);
	else if (n_faces > 2) {
		// TODO: verify if this case is equivalent to literature!
		// Mine: implicit handling of an edge with n_faces > 2
		vec3 e_new = vertices[e.v1].v + vertices[e.v2].v;
		for (int j = 0; j < n_faces; j++)
			e_new += face_vertices[e.face_ids[j]];

		return e_new / (float)(2+n_faces);
	}
	else
		cout << "Error: unhandled face number for edge!!!" << endl;

	return vec3(0);
}

// Calculate current / sharp edge vertex
vec3 calc_sharp_edge_vertex(const edge &e, const vector<vertex> &vertices) {
	return .5f * vertices[e.v1].v + vertices[e.v2].v;
}

vec3 calc_vertex_vertex(const vertex &v, edge_list &edges, const vector<vec3> &edge_vertices, const vector<vec3> &face_vertices) {
	uint n = v.edge_ids.size();
	if (n == v.face_ids.size()) {
		vec3 Q(0), R(0);
		for (uint j = 0; j < n; j++)
			Q += face_vertices[v.face_ids[j]];

		Q /= n;

		for (uint j = 0; j < n; j++)
			R += edge_vertices[v.edge_ids[j]];

		R /= n;

		return 1.f/n * (Q + 2.f*R + (n-3.f)*v.v);
	}
	else {
		int relevant_edges = 0;
		vec3 R(0);
		for (uint j = 0; j < v.edge_ids.size(); j++) {
			// check if edge is at a hole
			edge &e = edges.get(v.edge_ids[j]);
			if (e.face_ids.size() != 1)
				continue;

			relevant_edges++;
			R += edge_vertices[v.edge_ids[j]];
		}

		// TODO: verify if this case is equivalent to literature!
		// Mine: double weight the vertex position here
		return 1.f/(relevant_edges+2) * (R + v.v+v.v);
	}
}

// ear cutting triangulation
void triangulate(const vector<vertex> &vertices, vector<vector<int>> &faces, vector<vec3> &normals) {
	vector<vector<int>> new_faces;
	vector<vec3> new_normals;

	for (uint i = 0; i < faces.size(); i++) {
		vector<int> &f = faces[i];
		int n = f.size();
 		int j = -1;
		while (n > 3) {
			j++;
			int i_x = j%n;
			int i_a = ((j-1) % n + n) % n;
			int i_b = (j+1) % n;
			vec3 x = vertices[f[i_x]].v;
			vec3 a = vertices[f[i_a]].v;
			vec3 b = vertices[f[i_b]].v;
			vec3 to_a = a - x;
			vec3 to_b = b - x;

			// test if angle at x is convex inside the polygon, otherwise continue
			float d = glm::determinant(glm::mat3x3(to_b, to_a, normals[i]));
			if (d <= 0)
				continue;

			// test if the triangle x-a-b is an ear (all other points of the polygon are outside this triangle),
			// otherwise continue
			// 1. calc normal (cross(to_a, to_b))
			// 2. calc "normal of normal and to_a" (cross(normal, to_a))
			// 3. test for each other point if its left or right of to_a
			vec3 normal = cross(to_b, to_a);
			vec3 b_to_a = a - b;
			vec3 v = cross(normal, b_to_a);

			int i_k = (i_b+1)%n;
			bool skip_x = false;
			while (i_k != i_a) {
				vec3 b_to_k = vertices[f[i_k]].v - b;
				if (dot(v, b_to_k) >= 0) {
					skip_x = true;
					break;
				}
				i_k = (i_k+1)%n;
			}
			if (skip_x)
				continue;

			vector<int> new_f;
			new_f.push_back(f[i_x]);
			new_f.push_back(f[i_b]);
			new_f.push_back(f[i_a]);
			new_faces.push_back(new_f);

			// works only for planar polygons:
			new_normals.push_back(normals[i]);

			f.erase(f.begin() + i_x);
			n = f.size();
		}
		new_faces.push_back(f);
		new_normals.push_back(normals[i]);
	}

	normals.clear();
	faces.clear();
	for (uint i = 0; i < new_faces.size(); i++) {
		faces.push_back(new_faces[i]);
		normals.push_back(new_normals[i]);
	}
}

// Construct obj format
void write_obj(const vector<vertex> &vertices, const vector<vector<int>> &faces, const vector<vec3> &normals, const std::string name) {
	ofstream outfile;
	outfile.open("out/out_" + name + ".obj");
	outfile << "o " << name << endl;

	// Write vertices
	for (uint i = 0; i < vertices.size(); i++) {
		const vec3 &v = vertices[i].v;
		outfile << "v " << v.x << " " << v.y << " " << v.z << endl;
	}

	// Write normals
	outfile << endl;
	for (uint i = 0; i < normals.size(); i++) {
		const vec3 &n = normals[i];
		outfile << "vn " << n.x << " " << n.y << " " << n.z << endl;
	}

	// Write texture coordinates
	// ...

	// Write faces
	outfile << endl;
	for (uint i = 0; i < faces.size(); i++) {
		const vector<int> &f = faces[i];
		outfile << "f";
		for (uint j = 0; j < f.size(); j++) {
			//if (has_normals && has_tc)
			//	outfile << " " << f[j]+1  << "/0/0";
			if (has_normals)
				outfile << " " << f[j]+1  << "//" << i+1;
			//else if (has_tc)
			//	outfile << " " << f[j]+1  << "/0/0";
			else
				outfile << " " << f[j]+1;
		}

		outfile << endl;
	}

	outfile.close();
}