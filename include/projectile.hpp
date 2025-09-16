#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

#include <ogcsys.h>
#include <gccore.h>
#include <grrlib.h>

#include "sphere.hpp"
#include "vectorUtils.hpp"

class Projectile {
	
private:

	guVector position;
	guVector direction;
	float distanceLifetime;
	float distanceTravelled;
	float speed;
	u32 pierce;
	double damage;
	std::vector<std::shared_ptr<Sphere>> spheresHit;
	bool alive;

public:

	//constructor, gives everything to Activate()
	//@param _position Position of projectile
	//@param _direction Direction projectile will travel
	//@param _speed Speed of projectile to travel each frame
	//@param _pierce Total number of spheres that projectile is allowed to hit
	//@param _distanceLifetime Total distance the projectile is allowed to travel before despawning
	Projectile(guVector _position, guVector _direction, float _speed, int _pierce, float _distanceLifetime, double _damage);

	//activates the projectile (only to be called in pool before being sent out)
	void Activate(guVector _position, guVector _direction, float _speed, int _pierce, float _distanceLifetime, double _damage);

	//get position of projectile
	//@return vector of position
	guVector GetPosition() const;

	double GetDamage() const;

	bool GetAlive() const;

	//updates the position of the projectile based on direction and speed
	//@param deltaTime current delta time of the frame
	//@return true if still alive, false if projectile expired (distanceLifetime)
	bool UpdatePosition(double deltaTime);

	//check if projectile collides with given sphere
	//@return true if projectile should damage sphere, false if either 1. do not collide, or 2. already collided before
	bool IsCollide(std::shared_ptr<Sphere> sphere);

	//add sphere to hit list
	//@return true if projectile can hit more spheres, false if projectile dead
	bool SphereHit(std::shared_ptr<Sphere> sphere);

};