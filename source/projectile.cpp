#include "projectile.hpp"

Projectile::Projectile(guVector _position, guVector _direction, float _speed, int _pierce, float _distanceLifetime, double _damage) {
	Activate(_position, _direction, _speed, _pierce, _distanceLifetime, _damage);
}

void Projectile::Activate(guVector _position, guVector _direction, float _speed, int _pierce, float _distanceLifetime, double _damage) {
	position = _position;
	direction = _direction;
	speed = _speed;
	pierce = _pierce;
	distanceLifetime = _distanceLifetime;
	distanceTravelled = 0;
	alive = true;
	damage = _damage;
	spheresHit.clear();
}

guVector Projectile::GetPosition() const {
	return position;
}

double Projectile::GetDamage() const {
	return damage;
}

bool Projectile::GetAlive() const {
	return alive;
}

bool Projectile::UpdatePosition(double deltaTime) {
	float distance = speed * deltaTime;
	position = {	position.x += direction.x * distance,
					position.y += direction.y * distance,
					position.z += direction.z * distance };

	distanceTravelled += distance;
	if (distanceTravelled > distanceLifetime) {
		alive = false;
	}
	return alive;
}

//NOTE: update this once spheres have different radii
bool Projectile::IsCollide(std::shared_ptr<Sphere> sphere) {
	//if sphere already been hit, don't collide
	if (std::find(spheresHit.begin(), spheresHit.end(), sphere) != spheresHit.end()) return false;
	//sphere radius is 1.0, proj radius is 0.125
	if (VectorDistance(sphere->GetPosition(), position) <= 1.125f) return true;
	return false;
}


bool Projectile::SphereHit(std::shared_ptr<Sphere> sphere) {
	spheresHit.push_back(sphere);
	if (spheresHit.size() >= pierce) {
		alive = false;
	}
	return alive;
}