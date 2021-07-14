#pragma once

#include <algorithm>
#include "vector.h"
#include "body.h"

class Octant {
public:
	int count = 0;
	double m_sum = 0.0;
	Vector pos_avg = Vector(0.0, 0.0, 0.0);

private:
	Vector range_min;
	Vector range_max;
	Vector range_center;

	Body *body = nullptr;
	Octant *children[8] = { nullptr };

	double width;

public:
	Octant(const Vector& range_min, const Vector& range_max) {
		this->range_min = range_min;
		this->range_max = range_max;
		this->range_center = (range_min + range_max) * 0.5;
		this->width = std::max(std::max(range_max.x - range_min.x, range_max.y - range_min.y), range_max.z - range_min.z);
	}

	~Octant() {
		for (int i = 0; i < 8; ++i) {
			if (children[i] != nullptr) {
				delete children[i];
			}
		}
	}

	void insert(Body *b) {
		if (count == 0) {
			this->body = b;
		} else if (count == 1) {
			_insert_into_children(this->body);
			_insert_into_children(b);
			this->body = nullptr;
		} else {
			_insert_into_children(b);
		}
		++count;
	}

	void compute_mass_distribution() {
		if (count == 0) {
			m_sum = 0.0;
			pos_avg = Vector(0.0, 0.0, 0.0);
		} else if (count == 1) {
			m_sum = body->m;
			pos_avg = Vector(body->pos);
		} else {
			for (int i = 0; i < 8; ++i) {
				if (children[i] != nullptr) {
					children[i]->compute_mass_distribution();
					m_sum += children[i]->m_sum;
					pos_avg += children[i]->pos_avg * children[i]->m_sum;
				}
			}
			pos_avg *= 1.0 / m_sum;
		}
	}

	Vector get_acceleration(Body *b, double theta) {
		Vector acc;
		if (count == 1) {
			acc = b->acceleration(*(this->body));
		} else if (count > 1) {
			Vector diff = (b->pos - pos_avg);
			double dist = diff.length();
			double quotient = this->width / dist;

			if (quotient < theta) {
				acc = diff * (m_sum / (dist * dist * dist + EPS) * -KAPPA);
			} else {
				acc = Vector(0.0, 0.0, 0.0);
				for (int i = 0; i < 8; ++i) {
					if (children[i] != nullptr) {
						acc += children[i]->get_acceleration(b, theta);
					}
				}
			}
		}
		return acc;
	}

private:
	void _insert_into_children(Body *b) {
		int idx = 0;
		if (b->pos.x > range_center.x) idx |= 1;
		if (b->pos.y > range_center.y) idx |= 2;
		if (b->pos.z > range_center.z) idx |= 4;

		if (children[idx] == nullptr) {
			Vector child_min = Vector(
				(idx & 1) ? range_center.x : range_min.x,
				(idx & 2) ? range_center.y : range_min.y,
				(idx & 4) ? range_center.z : range_min.z
			);
			Vector child_max = Vector(
				(idx & 1) ? range_max.x : range_center.x,
				(idx & 2) ? range_max.y : range_center.y,
				(idx & 4) ? range_max.z : range_center.z
			);
			children[idx] = new Octant(child_min, child_max);
		}
		children[idx]->insert(b);
	}
};