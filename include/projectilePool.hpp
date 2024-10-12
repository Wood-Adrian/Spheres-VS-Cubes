#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <string>
#include <vector>
#include <list>
#include <memory>

#include "projectile.hpp"

class ProjectilePool {

private:

	std::list<std::shared_ptr<Projectile>> projectilePool;

public:

	//default constructor
	ProjectilePool();

	//destructor, only call when no projectiles are in main (i.e. all projectiles returned)
	~ProjectilePool();

	//returns projectile pointer, either from pool or newly generated
	//@param _position Position of new projectile
	//@param _direction Direction of new projectile
	//@param _speed Speed of new projectile (per frame movement)
	//@param _pierce Pierce of new projectile (no. spheres that can be hit before disappearing)
	//@param _distanceLifetime Max distance projectile can travel
	//@return Pointer to new projectile object in heap
	std::shared_ptr<Projectile> NewProjectile(guVector _position, guVector _direction, float _speed, int _pierce, float _distanceLifetime, double _damage);

	//give back the projectile to the pool
	//@param projectile Projectile to be returned
	void ReturnProjectile(std::shared_ptr<Projectile> projectile);
};