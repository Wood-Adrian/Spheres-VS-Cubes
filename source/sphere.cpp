#include "sphere.hpp"

u32 Sphere::Colour(u32 _type) {
	if (_type >= sizeof(typeInfo)/sizeof(typeInfo[0]) || _type < 0) {
		_type = 0;
	}
	return typeInfo[_type].colour;
}

Sphere::Sphere(const std::vector<guVector>& _mapPoints, u32 _type) {
	Activate(_mapPoints, _type);
}

void Sphere::Activate(const std::vector<guVector>& _mapPoints, u32 _type) {
	position = _mapPoints[0];
	if (_type >= sizeof(typeInfo) / sizeof(typeInfo[0]) || _type < 0) {
		_type = 0;
	}
	type = _type;
	healthMax = typeInfo[type].maxHealth;
	health = healthMax;
	alive = true;
	speed = typeInfo[type].speed;
	nextPathPoint = 1;
	direction = DirectionUnitVector(position, _mapPoints[nextPathPoint]);
	distanceThroughPath = 0;
}

guVector Sphere::GetPosition() const {
	return position;
}

void Sphere::SetPosition(guVector _position) {
	position = _position;
}

u32 Sphere::GetType() const {
	return type;
}

void Sphere::SetType(u32 _type) {
	type = _type;
}

double Sphere::GetHealth() {
	return health;
}

bool Sphere::GetAlive() {
	return alive;
}

void Sphere::SetAlive(bool _alive) {
	alive = _alive;
}

guVector Sphere::GetDirection() {
	return direction;
}

float Sphere::GetDistanceThroughPath() {
	return distanceThroughPath;
}

int Sphere::GetKilledMoney() {
	return healthMax / 4;
}

bool Sphere::UpdatePosition(const std::vector<guVector>& _mapPoints, double deltaTime) {
	bool returnCanMove = true;
	float distanceLeftToTravel = speed * deltaTime;
	while (1) {
		guVector newPosition = { position.x += direction.x * distanceLeftToTravel,
									position.y += direction.y * distanceLeftToTravel,
									position.z += direction.z * distanceLeftToTravel };

		if (VectorSurpassedPoint(newPosition, _mapPoints[nextPathPoint], direction)) {
			distanceLeftToTravel -= VectorDistance(position, _mapPoints[nextPathPoint]);
			position = _mapPoints[nextPathPoint];

			//if no more points on path (hit the end)
			if (_mapPoints.size() <= ++nextPathPoint) {
				returnCanMove = false;
				break;
			}
			direction = DirectionUnitVector(position, _mapPoints[nextPathPoint]);
		}
		else {
			position = newPosition;
			returnCanMove = true;
			break;
		}
	}

	//update distanceThroughPath
	float distanceFromPrevPoint = VectorDistance(position, _mapPoints[nextPathPoint - 1]);
	float distanceToNextPoint = VectorDistance(position, _mapPoints[nextPathPoint]);
	distanceThroughPath = (nextPathPoint - 1) + (((distanceFromPrevPoint - distanceToNextPoint) / (distanceFromPrevPoint + distanceToNextPoint)) * 0.5f) + 0.5f;

	alive = returnCanMove;
	return returnCanMove;
}

bool Sphere::Hit(double healthDrop) {
	health -= healthDrop;
	alive = (health <= 0) ? false : true;
	return alive;
}

