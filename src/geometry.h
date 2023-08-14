#pragma once

#include <math.h>
#include "vec3.h"
#include "utils.h"

// particles
const int particles_limit = 5;
const float particles_rad_min = 0.02f;
const float particles_rad_max = 0.06f;
const float particles_vel_min = 1.0f;
const float particles_vel_max = 3.0f;
const float particles_lifetime = 8.0f;
const float particles_decay_rate = 0.95f;


vec3 rotate(const vec3& p, double phi) {
	return vec3(cos(phi) * p.x() - sin(phi) * p.y(), sin(phi) * p.x() + cos(phi) * p.y(), 0.0);
}

class Object {
public:
	Object() {}
	Object(uint32_t color) : color(color) {}

	virtual bool in(const point3& p) { return false; }

public:
	uint32_t color = 0x000000;
};

class Sphere : public Object {
public:
	Sphere() : Object() {}
	Sphere(point3 origin, double radius, uint32_t color)
		: orig(origin)
		, rad(radius)
		, Object(color) {}
	Sphere(point3 origin, double radius, vec3 vel, uint32_t color)
		: orig(origin)
		, rad(radius)
		, velocity(vel)
	  , Object(color) {}

	bool in(const point3& p) {
		return (p.x() - orig.x()) * (p.x() - orig.x()) + (p.y() - orig.y()) * (p.y() - orig.y()) < rad * rad;
	}

	void update(float dt) {
		orig += velocity * dt;
	}

public:
	point3 orig = point3(0, 0, 0);
	double rad = 1.0;
	vec3 velocity = vec3(0, 0, 0);
};

class Rectangle : public Object {
public:
	Rectangle() : Object() {}
	Rectangle(point3 left_corner, point3 right_corner, uint32_t color)
		: l_corner(left_corner)
		, r_corner(right_corner)
		, orig((left_corner + right_corner) / 2)
	  , Object(color)
		, len(right_corner.x() - left_corner.x()) {}
	Rectangle(point3 left_corner, point3 right_corner, vec3 vel, uint32_t color)
		: l_corner(left_corner)
		, r_corner(right_corner)
		, orig((left_corner + right_corner) / 2)
		, velocity(vel)
	  , Object(color)
		, len(right_corner.x() - left_corner.x()) {}

	bool in(const point3& p) {
		return (l_corner.x() < p.x() && p.x() < r_corner.x()) && (l_corner.y() < p.y() && p.y() < r_corner.y());
	}

	void update(float dt) {
		l_corner += velocity * dt;
		r_corner += velocity * dt;
		orig += velocity * dt;
	}

public:
	point3 l_corner = point3(-0.5, -0.5, 0);
	point3 r_corner = point3(0.5, 0.5, 0);
	point3 orig = point3(0, 0, 0);
	vec3 velocity = vec3(0, 0, 0);
	double len = 1.0;
};

class Explosion: public Object {
public:
	Explosion(point3 origin, uint32_t color) : origin(origin), Object(color) {
		for (auto& p : particles) {
			p.orig = origin + vec3(randrange(-0.05, 0.05), randrange(-0.05, 0.05), 0);
			p.rad = randrange(particles_rad_min, particles_rad_max);
			p.velocity = random_in_unit_disk() * randrange(particles_vel_min, particles_vel_max);
		}
	}

	bool in(const point3& pos) {
		for (auto& p : particles) {
			if (p.in(pos))
				return true;
		}
		return false;
	}

	void update(float dt) {
		if (lifetime > particles_lifetime)
			alive = false;
		for (auto& p : particles) {
			p.rad *= particles_decay_rate;
			p.update(dt);
			lifetime += dt;
		}
	}

public:
	Sphere particles[particles_limit];
	point3 origin;
	bool alive = true;
	float lifetime = 0.0;
};

class Trail : public Object {

};