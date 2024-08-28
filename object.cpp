#include <iostream>
#include <fstream>
#include "object.h"

using namespace glm;
using namespace std;

void object::init_object(cfg_t *cfg) {
	// Meta data
	mesh.has_normals = cfg->vn->index != -1;
	mesh.has_texture = cfg->vt->index != -1;
	if (cfg->mtllib_path != NULL)
		mtllib_path = std::string(cfg->mtllib_path);
	
	if (cfg->material != NULL)
		material = std::string(cfg->material);

	source_path = cfg->source_path;
	std::string obj_name = name;
	if (cfg->name != NULL) {
		name = cfg->name;
	}
	else {
		std::string filename = source_path.substr(source_path.find_last_of("/") + 1);
		int pos = filename.find_last_of('.');
		name = filename.substr(0, pos);
	}

	// Parse obj file
	vec3_t *data = (vec3_t *)cfg->v->storage;
	for (int i = 0; i <= cfg->v->index; i++) {
		vec3_t val = data[i];
		printf("v %s\n", vec3_to_str(val));
		mesh.vertices.push_back(vertex(val));
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
		mesh.tex_coords.push_back(glm::vec2(tc.x[0], tc.x[1]));
	}

	crease_t *data_creases = (crease_t *)cfg->creases->storage;
	for (int i = 0; i <= cfg->creases->index; i++) {
		crease_t crease = data_creases[i];
		printf("crease %d-%d: s = %f\n", crease.a, crease.b, crease.s);
		mesh.creases.add(crease.a, crease.b, crease.s);
	}

	stack_t *data_f = (stack_t *)cfg->f->storage;
	for (int i = 0; i <= cfg->f->index; i++) {
		stack_t *v_list = data_f + i;
		face face;
		if (mesh.has_normals)
			face.normal = normals[i];
		else
			face.normal = vec3(0);

		printf("f ");
		for (int j = 0; j <= v_list->index; j++) {
			int *v_cfg = ((int **)v_list->storage)[j];
			vertex_config v;
			v.pos = v_cfg[0]-1;
			v.tc = v_cfg[1]-1;
			printf("%s ", v_cfg_to_str(v_cfg));

			face.verts.push_back(v);
		}
		printf("\n");
		mesh.faces.push_back(face);
	}
	
}

// Construct obj format
void object::write_obj(std::string outfile_path) {
	ofstream outfile;
	outfile.open(outfile_path);
	if (has_mtllib())
		outfile << "mtllib " << mtllib_path << endl << endl;

	outfile << "o " << name << endl;

	// Write vertices
	for (uint i = 0; i < mesh.vertices.size(); i++) {
		const vec3 &v = mesh.vertices[i].v;
		outfile << "v " << v.x << " " << v.y << " " << v.z << endl;
	}

	// Write normals
	if (mesh.has_normals) {
		outfile << endl;
		for (uint i = 0; i < mesh.normals.size(); i++) {
			const vec3 &n = mesh.normals[i];
			outfile << "vn " << n.x << " " << n.y << " " << n.z << endl;
		}
	}

	// Write texture coordinates
	if (mesh.has_texture) {
		outfile << endl;
		for (uint i = 0; i < mesh.tex_coords.size(); i++) {
			const vec2 &tc = mesh.tex_coords[i];
			outfile << "vt " << tc.x << " " << tc.y << endl;
		}
	}

	// Write faces
	outfile << endl;
	if (has_material())
		outfile << "usemtl " << material << endl;

	for (uint i = 0; i < mesh.faces.size(); i++) {
		const face &f = mesh.faces[i];
		outfile << "f";
		for (uint j = 0; j < f.verts.size(); j++) {
			const vertex_config &v_cfg = f.verts[j];
			int tc_index = v_cfg.tc+1;
			int pos_index = v_cfg.pos+1;
			int normal_index = i+1;
			if (mesh.has_normals && mesh.has_texture)
				outfile << " " << pos_index  << "/" << tc_index << "/" << normal_index;
			else if (mesh.has_normals)
				outfile << " " << pos_index << "//" << normal_index;
			else if (mesh.has_texture)
				outfile << " " << pos_index  << "/" << tc_index;
			else
				outfile << " " << pos_index;
		}

		outfile << endl;
	}

	outfile.close();
}

void mesh::subdivide() {
	mesh new_mesh;

	// Gather face vertices and edges and update vertex information
	edge_list edges;
	vector<vec3> face_vertices;
	for (uint i = 0; i < faces.size(); i++) {
		face& f = faces[i];
		vec3 f_new(0);
		for (uint j = 0; j < f.verts.size(); j++)
			f_new += vertices[f.verts[j].pos].v;

		f_new = 1.f/f.size() * f_new;
		face_vertices.push_back(f_new);
		cout << "f_new: (" << f_new.x << ", " << f_new.y << ", " << f_new.z << ")" << endl;

		for (uint j = 0; j < f.size(); j++)
			add_edge(edges, f.verts[j].pos, f.verts[(j+1)%f.size()].pos, i);

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
			e_new = calc_smooth_edge_vertex(e, face_vertices);
		}
		else if(e.sharpness >= 1.f) {
			// Infinitely Sharp edge
			e_new = edge_vertices[i];
		}
		else {
			// Semi-sharp edge
			e_new = e.sharpness * edge_vertices[i] + (1-e.sharpness) * calc_smooth_edge_vertex(e, face_vertices);
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
			vec3 e1 = calc_sharp_edge_vertex(*sharp_edges[0]); //edge_vertices[sharp_edge_ids[0]];
			vec3 e2 = calc_sharp_edge_vertex(*sharp_edges[1]); //edge_vertices[sharp_edge_ids[1]];
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
	normals.clear();
	for (uint i = 0; i < faces.size(); i++) {
		face &f = faces[i];
		int off_tcs = new_mesh.tex_coords.size();
		
		int n = f.size();
		// Calculate texture coordinates of the face
		if (has_texture) {
			vec2 tc_face(0);
			for (int j = 0; j < n; j++) {
				vec2 current_tc = tex_coords[f.verts[j].tc];
				vec2 next_tc = tex_coords[f.verts[(j+1)%n].tc];
				tc_face += current_tc;

				new_mesh.tex_coords.push_back(current_tc);						// vertex tc
				new_mesh.tex_coords.push_back(.5f * (current_tc + next_tc));	// edge tc
			}
			tc_face /= n; // face tc
			new_mesh.tex_coords.push_back(tc_face);
		}

		for (int j = 0; j < n; j++) {
			face new_f;
			int edge_vert1_id = f.verts[(j+1)%n].pos;
			int edge_vert2_id = f.verts[((j-1)%n+n)%n].pos;
			int v_vert_id = off_vert+f.verts[j].pos;
			int e_vert1_id = off_edge+edges.get_id(f.verts[j].pos, edge_vert1_id);
			int f_vert_id = off_face+i;
			int e_vert2_id = off_edge+edges.get_id(f.verts[j].pos, edge_vert2_id);
			for (int k = 0; k < 4; k++)
				new_f.verts.push_back(vertex_config());

			new_f.verts[0].pos = v_vert_id;		// 1 vertex vertex,	e(4,1) -> calc. sharpness
			new_f.verts[1].pos = e_vert1_id;	// 2 edge vertex,	e(1,2) -> calc. sharpness
			new_f.verts[2].pos = f_vert_id;		// 3 face vertex,	e(2,3) -> smooth edge
			new_f.verts[3].pos = e_vert2_id;	// 4 edge vertex,	e(3,4) -> smooth edge

			new_f.verts[0].tc = off_tcs + j*2;				// vertex tc
			new_f.verts[1].tc = off_tcs + j*2+1;			// edge tc
			new_f.verts[2].tc = off_tcs + n*2;				// face tc
			new_f.verts[3].tc = off_tcs + (j-1+n)%n * 2+1;	// edge tc
			
			new_mesh.faces.push_back(new_f);

			if (!new_mesh.creases.exists(f.verts[j].pos, edge_vert1_id)) {
				float s = 0.f;

				vertex &v = vertices[v_vert_id];
				int e_id = edges.get_id(f.verts[j].pos, edge_vert1_id);
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

				new_mesh.creases.add(v_vert_id, e_vert1_id, s);
			}
			// TODO: is this second case neccessary or is it guaranteed that the first case covers all new edges?
			if (!new_mesh.creases.exists(f.verts[j].pos, edge_vert2_id)) {
				// ...
			}
			
			glm::vec3 u = vertices[new_f.verts[0].pos].v - vertices[new_f.verts[1].pos].v;
			glm::vec3 v = vertices[new_f.verts[2].pos].v - vertices[new_f.verts[1].pos].v;
			glm::vec3 normal = glm::normalize(glm::cross(v, u));
			normals.push_back(normal);
		}
	}

	// Propagate new crease values
	creases.clear();
	for (int i = 0; i < new_mesh.creases.size(); i++) {
		edge &e = new_mesh.creases.get(i);
		if (e.sharpness > 0)
			creases.add(e.v1, e.v2, e.sharpness);
	}

	tex_coords.clear();
	for (vec2 tc : new_mesh.tex_coords)
		tex_coords.push_back(tc);

	faces.clear();
	for (face &f : new_mesh.faces) {
		faces.push_back(f);
	}

	// Calculate adjacent edges and faces
	edges.clear();
	for (uint i = 0; i < faces.size(); i++) {
		face& f = faces[i];
		for (uint j = 0; j < f.size(); j++)
			add_edge(edges, f.verts[j].pos, f.verts[(j+1)%f.size()].pos, i);
	}
}

int mesh::add_edge(edge_list &edges, int a, int b, int f_id) {
	int e_id = edges.get_id(a, b);
	if (e_id == -1) {
		e_id = edges.add(a, b);
	}
	edge &e = edges.get(e_id);

	update_vertex(vertices[a], f_id, e_id);
	update_vertex(vertices[b], f_id, e_id);

	vector<int> &face_ids = e.face_ids;
	if (!e.face_exists(f_id)) {
		face_ids.push_back(f_id);
	}

	return e_id;
}

void mesh::update_vertex(vertex &v, int f_id, int e_id) {
	if (!v.edge_exists(e_id))
		v.edge_ids.push_back(e_id);

	if (!v.face_exists(f_id))
		v.face_ids.push_back(f_id);
}

// Calculate smooth edge vertex
vec3 mesh::calc_smooth_edge_vertex(const edge &e, const vector<vec3> &face_vertices) {
	int n_faces = e.face_ids.size();
	if (n_faces == 2)
		return 1.f/4 * (vertices[e.v1].v + vertices[e.v2].v + face_vertices[e.face_ids[0]] + face_vertices[e.face_ids[1]]);
	else if (n_faces == 1)
		//return edge_vertices[i];
		return calc_sharp_edge_vertex(e);
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
vec3 mesh::calc_sharp_edge_vertex(const edge &e) {
	return .5f * (vertices[e.v1].v + vertices[e.v2].v);
}

vec3 mesh::calc_vertex_vertex(const vertex &v, edge_list &edges, const vector<vec3> &edge_vertices, const vector<vec3> &face_vertices) {
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
void mesh::triangulate() {
	mesh new_mesh;

	for (uint i = 0; i < faces.size(); i++) {
		face &f = faces[i];
		//vector<int> &f_tc = tc_face_map[i];
		int n = f.verts.size();
 		int j = -1;
		while (n > 3) {
			j++;
			int i_x = j%n;
			int i_a = ((j-1) % n + n) % n;
			int i_b = (j+1) % n;
			vec3 x = vertices[f.verts[i_x].pos].v;
			vec3 a = vertices[f.verts[i_a].pos].v;
			vec3 b = vertices[f.verts[i_b].pos].v;
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
				vec3 b_to_k = vertices[f.verts[i_k].pos].v - b;
				if (dot(v, b_to_k) >= 0) {
					skip_x = true;
					break;
				}
				i_k = (i_k+1)%n;
			}
			if (skip_x)
				continue;

			// create new face (triangle)
			face new_f;
			new_f.normal = normals[i];
			new_f.verts.push_back(vertex_config());
			new_f.verts.push_back(vertex_config());
			new_f.verts.push_back(vertex_config());

			// assign vertex points
			new_f.verts[0].pos = f.verts[i_x].pos;
			new_f.verts[1].pos = f.verts[i_b].pos;
			new_f.verts[2].pos = f.verts[i_a].pos;

			// assign texture coordinates
			new_f.verts[0].tc = f.verts[i_x].tc;
			new_f.verts[1].tc = f.verts[i_b].tc;
			new_f.verts[2].tc = f.verts[i_a].tc;

			// works only for planar polygons:
			new_mesh.normals.push_back(normals[i]);
			new_mesh.faces.push_back(new_f);

			f.verts.erase(f.verts.begin() + i_x);
			n = f.verts.size();
		}
		new_mesh.faces.push_back(f);
		new_mesh.normals.push_back(normals[i]);
	}

	normals.clear();
	faces.clear();
	for (uint i = 0; i < new_mesh.faces.size(); i++) {
		faces.push_back(new_mesh.faces[i]);
		normals.push_back(new_mesh.normals[i]);
	}
}