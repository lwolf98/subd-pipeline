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

int edge_list::add(int a, int b) {
	normalize_edge_order(a, b);
	edges.push_back(edge(a,b));
	return size()-1;
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