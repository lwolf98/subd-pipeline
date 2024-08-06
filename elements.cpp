#include "elements.h"

bool edge::face_exists(int id) const {
	for (int i : face_ids)
		if (i == id)
			return true;

	return false;
}

void normalize_edge_order(int &a, int &b) {
	if (a > b) {
		int tmp = b;
		b = a;
		a = tmp;
	}
}

int edge_list::add(int a, int b, float sharpness) {
	normalize_edge_order(a, b);
	edges.push_back(edge(a, b, sharpness));
	return size()-1;
}

int edge_list::add(int a, int b) {
	return add(a, b, 0.f);
}

void edge_list::clear() {
	edges.clear();
}

int edge_list::get_id(int a, int b) const {
	normalize_edge_order(a, b);
	for (int i = 0; i < size(); i++) {
		const edge &e = edges[i];
		if (e.v1 == a && e.v2 == b)
			return i;
	}
	
	return -1;
}

edge& edge_list::get(int id) {
	return edges[id];
}

int edge_list::size() const {
	return edges.size();
}

bool edge_list::exists(int a, int b) const {
	return get_id(a, b) != -1;
}


bool vertex::edge_exists(int id) const {
	for (int i : edge_ids)
		if (i == id)
			return true;

	return false;
}

bool vertex::face_exists(int id) const {
	for (int i : face_ids)
		if (i == id)
			return true;

	return false;
}

/*glm::vec3 edge::current_edge_vertex(const std::vector<glm::vec3> &vertices) {
	return 1.f/2 * (vertices[v1].v+vertices[v2].v);
}

glm::vec3 edge::smooth_edge_vertex(const std::vector<glm::vec3> &vertices) {
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
}

glm::vec3 edge::sharp_edge_vertex(const std::vector<glm::vec3> &vertices) {
	return current_edge_vertex(vertices);
}*/