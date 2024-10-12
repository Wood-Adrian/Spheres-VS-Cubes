#include "projectilePool.hpp"

ProjectilePool::ProjectilePool() {
}

ProjectilePool::~ProjectilePool() {
	for (auto it = projectilePool.begin(); it != projectilePool.end(); ++it) {
		if (*it) {
			(*it).reset();
		}
	}
	projectilePool.clear();
}

//returns projectile pointer, either from pool or newly generated
std::shared_ptr<Projectile> ProjectilePool::NewProjectile(guVector _position, guVector _direction, float _speed, int _pierce, float _distanceLifetime, double _damage) {
	if (projectilePool.empty()) {
		return std::make_shared<Projectile>(_position, _direction, _speed, _pierce, _distanceLifetime, _damage);
	}
	std::shared_ptr<Projectile> projectilePointer = projectilePool.back();
	projectilePool.pop_back();
	projectilePointer->Activate(_position, _direction, _speed, _pierce, _distanceLifetime, _damage);
	return projectilePointer;
}

void ProjectilePool::ReturnProjectile(std::shared_ptr<Projectile> projectile) {
	projectilePool.push_back(projectile);
}