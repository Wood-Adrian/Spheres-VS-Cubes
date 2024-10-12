#include "cube.hpp"

u32 Cube::Colour(u32 _type) {
	if (_type >= sizeof(typeInfo) / sizeof(typeInfo[0]) || _type < 0) {
		_type = 0;
	}
	return typeInfo[_type].colour;
}

Cube::Cube(guVector _pos, u32 _type) {
	position = _pos;
	if (_type >= sizeof(typeInfo) / sizeof(typeInfo[0]) || _type < 0) {
		_type = 0;
	}
	type = _type;

	targetMode = CubeTargeting::FIRST;
	rotationHoz = 0;
	projectileDirection = { 0,0,0 };
	lastFireTime = 0;

	radius = typeInfo[type].radius;
	ROF = typeInfo[type].ROF;

}

guVector Cube::GetPosition() {
	return position;
}

float Cube::GetRotation() {
	return rotationHoz;
}

u32 Cube::GetType() {
	return type;
}

projectileInfo Cube::GetProjectileInfo() {
	return typeInfo[type].projInfo;
}

projectileBeginInfo Cube::GetProjectileBeginInfo() {
	return { position, projectileDirection };
}

bool Cube::ScanSpheres(const std::vector<std::shared_ptr<Sphere>>& sphereList, u64 currentTime) {
	//if not enough time has passed since last shot, return early
	//note: expanded secs_to_ticks to remove early u64 conversion
	if ((u64)(ROF * (u64)60'750'000) + lastFireTime > currentTime) return false;

	//first targeting
	float distancePathMost = -1;
	//last targeting
	float distancePathLeast = std::numeric_limits<float>::max();
	//close targeting
	float distanceCube = radius+1;
	//strong targeting
	u32 sphereType = -1;

	guVector _projectileDirection = { 0,0,0 };

	bool sphereFound = false;

	auto it = sphereList.begin();

	while (it != sphereList.end()) {
		float sphereDistanceFromCube = VectorDistance((*it)->GetPosition(), position);
		if (sphereDistanceFromCube > radius) {
			it++;
			continue;
		}
		sphereFound = true;

		bool updateProjectileDirection = false;
		float sphereDistanceThroughPath = (*it)->GetDistanceThroughPath();

		switch (targetMode) {
		case CubeTargeting::FIRST:

			if (sphereDistanceThroughPath > distancePathMost) {
				distancePathMost = sphereDistanceThroughPath;
				updateProjectileDirection = true;
			}

			break;
		case CubeTargeting::LAST:

			if (sphereDistanceThroughPath < distancePathLeast) {
				distancePathLeast = sphereDistanceThroughPath;
				updateProjectileDirection = true;
			}

			break;
		case CubeTargeting::CLOSE:

			if (sphereDistanceFromCube < distanceCube) {
				distanceCube = sphereDistanceFromCube;
				updateProjectileDirection = true;
			}

			break;
		//strong has special case of defaulting to first targeting if 2 spheres have same strength
		case CubeTargeting::STRONG:
			
			u32 currentType = (*it)->GetType();
			if (currentType > sphereType) {
				sphereType = currentType;
				distancePathMost = sphereDistanceThroughPath;
				updateProjectileDirection = true;
			}
			else if (currentType == sphereType && sphereDistanceThroughPath > distancePathMost) {
				distancePathMost = sphereDistanceThroughPath;
				updateProjectileDirection = true;
			}
			break;
		}

		if (updateProjectileDirection) {

			guVector spherePos = (*it)->GetPosition();
			guVector sphereDir = (*it)->GetDirection();
			//sum of position and direction, so cubes shoot infront to miss less
			guVector projDir;
			guVecAdd(&spherePos, &sphereDir, &projDir);
			_projectileDirection = DirectionUnitVector(position, projDir);
		}

		it++;
	}

	if (!sphereFound) return false;
	
	//prepare cube to get ready to fire projectile
	projectileDirection = _projectileDirection;
	guVector cubeDirection = { projectileDirection.x, 0, projectileDirection.z };
	guVecNormalize(&cubeDirection);
	rotationHoz = (cubeDirection.z > 0) ? 2 * M_PI - acosf(cubeDirection.x) : acosf(cubeDirection.x);
	lastFireTime = currentTime;
	return true;
}

void Cube::AddTimeDifference(u64 timeDifference) {
	lastFireTime += timeDifference;
}