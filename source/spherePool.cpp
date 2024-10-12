#include "spherePool.hpp"

SpherePool::SpherePool() {
}

SpherePool::~SpherePool() {
	for (std::list<std::shared_ptr<Sphere>>::iterator it = spherePool.begin(); it != spherePool.end(); ++it) {
		if (*it) {
			(*it).reset();
		}
	}
	spherePool.clear();
}

//returns sphere pointer, either from pool or newly generated
std::shared_ptr<Sphere> SpherePool::NewSphere(const std::vector<guVector>& mapPoints, u32 type) {
	if (spherePool.empty()) {
		return std::make_shared<Sphere>(mapPoints, type);
	}
	std::shared_ptr<Sphere> spherePointer = spherePool.back();
	spherePool.pop_back();
	spherePointer->Activate(mapPoints, type);
	return spherePointer;
}

void SpherePool::ReturnSphere(std::shared_ptr<Sphere> sphere) {
	spherePool.push_back(sphere);
}