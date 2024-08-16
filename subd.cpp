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

vector<vec2> tcs;
vector<vector<int>> tc_face_map;

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
	c_id = creases.add(6, 2);
	creases.get(c_id).sharpness = .5f;
	c_id = creases.add(2, 0);
	creases.get(c_id).sharpness = .5f;

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
		//vertices[i].tc = glm::vec2(tc.x[0], tc.x[1]);
		tcs.push_back(glm::vec2(tc.x[0], tc.x[1]));
	}

	vector<vector<int>> faces;
	stack_t *data_f = (stack_t *)cfg->f->storage;
	for (int i = 0; i <= cfg->f->index; i++) {
		stack_t *v_list = data_f + i;
		printf("f ");
		vector<int> verts;
		vector<int> tc_face_list;
		for (int j = 0; j <= v_list->index; j++) {
			int *v_cfg = ((int **)v_list->storage)[j];
			verts.push_back(v_cfg[0]-1);
			tc_face_list.push_back(v_cfg[1]-1);
			printf("%s ", v_cfg_to_str(v_cfg));
		}
		printf("\n");
		faces.push_back(verts);
		tc_face_map.push_back(tc_face_list);
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
			e_new = e.sharpness * edge_vertices[i] + (1-e.sharpness) * calc_smooth_edge_vertex(e, vertices, face_vertices);
		}

		e_news.push_back(e_new);
		cout << "e_new: (" << e_new.x << ", " << e_new.y << ", " << e_new.z << ")" << endl;
	}

	// Calculate new vertex points
	vector<vec3> v_news;
	for (uint i = 0; i < vertices.size(); i++) {
		vertex &v = vertices[i];
		vector<edge *> sharp_edges;
		vector<int> sharp_edge_ids;
		float v_sharpness = 0;
		vec3 v_new;

		for (uint j = 0; j < v.edge_ids.size(); j++) {
			edge &v_edge = edges.get(v.edge_ids[j]);
			if (v_edge.sharpness > 0) {
				sharp_edges.push_back(&v_edge);
				sharp_edge_ids.push_back(j);
				v_sharpness += v_edge.sharpness;
			}
		}
		if (sharp_edge_ids.size() > 0)
			v_sharpness = 1.f/sharp_edge_ids.size() * v_sharpness;
		
		uint n_sharp_edges = sharp_edges.size();
		vec3 v_smooth = calc_vertex_vertex(v, edges, edge_vertices, face_vertices);
		if (n_sharp_edges <= 1) {
			// zero or one adjacent sharp edges -> smooth vertex rule
			v_new = v_smooth;
		}
		else if (n_sharp_edges == 2) {
			// two adjacent sharp edges -> crease rule (or blend between crease vertex and corner mask)
			vec3 e1 = calc_sharp_edge_vertex(*sharp_edges[0], vertices); //edge_vertices[sharp_edge_ids[0]];
			vec3 e2 = calc_sharp_edge_vertex(*sharp_edges[1], vertices); //edge_vertices[sharp_edge_ids[1]];
			//v_new = 1.f/8 * (6.f * v.v + e1 + e2);
			v_new = 1.f/4 * (2.f * v.v + e1 + e2);
		}
		else {
			// three or more adjacent sharp edges -> corner rule
			v_new = v.v;
		}

		if (n_sharp_edges > 1) {
			// TODO: check this interpolation step...
			//v_new = v_sharpness * v.v + (1 - v_sharpness) * v_new;
			//v_new = v_sharpness * v_new + (1 - v_sharpness) * v.v;
			v_new = v_sharpness * v_new + (1 - v_sharpness) * v_smooth;
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
	edge_list new_creases;
	vector<vec2> new_tcs;
	vector<vector<int>> new_tc_face_map;
	normals.clear();
	for (uint i = 0; i < faces.size(); i++) {
		vector<int> &f = faces[i];
		int off_tcs = new_tcs.size();
		vector<int> &tc_list = tc_face_map[i];
		
		int n = f.size();
		// Calculate texture coordinates of the face
		vec2 tc_face(0);
		for (int j = 0; j < n; j++) {
			vec2 current_tc = tcs[tc_list[j]];
			vec2 next_tc = tcs[tc_list[(j+1)%n]];
			tc_face += current_tc;

			new_tcs.push_back(tcs[tc_list[j]]);					// vertex tc
			new_tcs.push_back(.5f * (current_tc + next_tc));	// edge tc
		}
		tc_face /= n; // face tc
		new_tcs.push_back(tc_face);

		for (int j = 0; j < n; j++) {
			vector<int> new_f;
			int edge_vert1_id = f[(j+1)%n];
			int edge_vert2_id = f[((j-1)%n+n)%n];
			int v_vert_id = off_vert+f[j];
			int e_vert1_id = off_edge+edges.get_id(f[j], edge_vert1_id);
			int f_vert_id = off_face+i;
			int e_vert2_id = off_edge+edges.get_id(f[j], edge_vert2_id);
			new_f.push_back(v_vert_id);		// 1 vertex vertex,	e(4,1) -> calc. sharpness
			new_f.push_back(e_vert1_id);	// 2 edge vertex,	e(1,2) -> calc. sharpness
			new_f.push_back(f_vert_id);		// 3 face vertex,	e(2,3) -> smooth edge
			new_f.push_back(e_vert2_id);	// 4 edge vertex,	e(3,4) -> smooth edge
			new_faces.push_back(new_f);

			vector<int> tc_face;
			tc_face.push_back(off_tcs + j*2);			// vertex tc
			tc_face.push_back(off_tcs + j*2+1);			// edge tc
			tc_face.push_back(off_tcs + n*2);			// face tc
			tc_face.push_back(off_tcs + (j-1+n)%n * 2+1);	// edge tc
			new_tc_face_map.push_back(tc_face);

			if (!new_creases.exists(f[j], edge_vert1_id)) {
				float s = 0.f;

				vertex &v = vertices[v_vert_id];
				int e_id = edges.get_id(f[j], edge_vert1_id);
				edge &e = edges.get(e_id);
				float max_adjacent_sharpness = 0.f;
				for (uint k = 0; k < v.edge_ids.size(); k++) {
					if (v.edge_ids[k] != e_id) {
						edge &adj_edge = edges.get(v.edge_ids[k]);
						if (adj_edge.sharpness > max_adjacent_sharpness)
							max_adjacent_sharpness = adj_edge.sharpness;

					}
				}

				// TODO: only consider edges with sharpness?
				//s = 1.f/4 * (3*e.sharpness + max_adjacent_sharpness);
				s = e.sharpness;

				new_creases.add(v_vert_id, e_vert1_id, s);
			}
			// TODO: is this second case neccessary or is it guaranteed that the first case covers all new edges?
			if (!new_creases.exists(f[j], edge_vert2_id)) {
				// ...
			}

			glm::vec3 u = vertices[new_f[0]].v - vertices[new_f[1]].v;
			glm::vec3 v = vertices[new_f[2]].v - vertices[new_f[1]].v;
			glm::vec3 normal = glm::normalize(glm::cross(v, u));
			normals.push_back(normal);
		}
	}

	// Propagate new crease values
	creases.clear();
	for (int i = 0; i < new_creases.size(); i++) {
		edge &e = new_creases.get(i);
		if (e.sharpness > 0)
			creases.add(e.v1, e.v2, e.sharpness);
	}

	tcs.clear();
	for (vec2 tc : new_tcs)
		tcs.push_back(tc);

	faces.clear();
	for (vector<int> &f : new_faces) {
		faces.push_back(f);
	}

	tc_face_map.clear();
	for (vector<int> &tc_f : new_tc_face_map)
		tc_face_map.push_back(tc_f);

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
	return .5f * (vertices[e.v1].v + vertices[e.v2].v);
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
	vector<vector<int>> new_tc_face_map;

	for (uint i = 0; i < faces.size(); i++) {
		vector<int> &f = faces[i];
		vector<int> &f_tc = tc_face_map[i];
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

			// assign vertex points
			vector<int> new_f;
			new_f.push_back(f[i_x]);
			new_f.push_back(f[i_b]);
			new_f.push_back(f[i_a]);
			new_faces.push_back(new_f);

			// assign texture coordinates
			vector<int> new_f_tc;
			new_f_tc.push_back(f_tc[i_x]);
			new_f_tc.push_back(f_tc[i_b]);
			new_f_tc.push_back(f_tc[i_a]);
			new_tc_face_map.push_back(new_f_tc);

			// works only for planar polygons:
			new_normals.push_back(normals[i]);

			f.erase(f.begin() + i_x);
			f_tc.erase(f_tc.begin() + i_x);
			n = f.size();
		}
		new_faces.push_back(f);
		new_tc_face_map.push_back(f_tc);
		new_normals.push_back(normals[i]);
	}

	normals.clear();
	faces.clear();
	tc_face_map.clear();
	for (uint i = 0; i < new_faces.size(); i++) {
		faces.push_back(new_faces[i]);
		tc_face_map.push_back(new_tc_face_map[i]);
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
	if (has_normals) {
		outfile << endl;
		for (uint i = 0; i < normals.size(); i++) {
			const vec3 &n = normals[i];
			outfile << "vn " << n.x << " " << n.y << " " << n.z << endl;
		}
	}

	// Write texture coordinates
	if (has_tc) {
		outfile << endl;
		for (uint i = 0; i < tcs.size(); i++) {
			const vec2 &tc = tcs[i];
			outfile << "vt " << tc.x << " " << tc.y << endl;
		}
	}

	// Write faces
	outfile << endl;
	for (uint i = 0; i < faces.size(); i++) {
		const vector<int> &f = faces[i];
		outfile << "f";
		for (uint j = 0; j < f.size(); j++) {
			//int tc_index = f[j]+1;
			int tc_index = tc_face_map[i][j]+1;
			if (has_normals && has_tc)
				outfile << " " << f[j]+1  << "/" << tc_index << "/" << i+1;
			else if (has_normals)
				outfile << " " << f[j]+1  << "//" << i+1;
			else if (has_tc)
				outfile << " " << f[j]+1  << "/" << tc_index;
			else
				outfile << " " << f[j]+1;
		}

		outfile << endl;
	}

	outfile.close();
}