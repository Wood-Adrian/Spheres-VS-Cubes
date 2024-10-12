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

#include "sphere.hpp"

class SpherePool {

private:

	std::list<std::shared_ptr<Sphere>> spherePool;

public:

	//default constructor
	SpherePool();

	//destructor, only call when no spheres are in main (i.e. all spheres returned)
	~SpherePool();

	//returns sphere pointer, either from pool or newly generated
	//@param position Position of new sphere
	//@param type Type of new sphere
	//@return Pointer to new sphere object in memory
	std::shared_ptr<Sphere> NewSphere(const std::vector<guVector>& mapPoints, u32 type = 0);

	//give back the sphere to the pool
	//@param sphere Sphere to be returned
	void ReturnSphere(std::shared_ptr<Sphere> sphere);
};