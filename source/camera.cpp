#include "camera.hpp"

Camera::Camera(guVector _pos, float _rotationHoz, float _rotationVer, float _FOV) {
	pos = _pos;
	rotationHoz = _rotationHoz;
	rotationVer = _rotationVer;
	FOV = _FOV;
	
	targetPos = pos;
	targetHoz = rotationHoz;
	targetVer = rotationVer;

	UpdateDirectionValue();
}

const CameraData Camera::GetCameraData() {
	return { pos, direction, rotationHoz, rotationVer, FOV };
}

void Camera::SetPositionTarget(guVector _targetPos) {
	targetPos = _targetPos;
}

void Camera::SetFOV(float _FOV) {
	FOV = _FOV;
}

void Camera::SetRotationTarget(float _targetHoz, float _targetVer) {
	float camVerPrev = targetVer;

	targetHoz = _targetHoz;
	targetVer = _targetVer;

	if (targetVer < -(M_PI / 2)) targetVer = camVerPrev;
	if (targetVer > (M_PI / 2)) targetVer = camVerPrev;
}

void Camera::RotateCamera(float _rotateHoz, float _rotateVer) {
	float camVerPrev = targetVer;

	targetHoz += _rotateHoz / 16.0f;
	targetVer += _rotateVer / 16.0f;

	if (targetVer < -(M_PI / 2)) targetVer = camVerPrev;
	if (targetVer > (M_PI / 2)) targetVer = camVerPrev;
}

void Camera::MoveCamera(guVector pos) {
	guVecAdd(&targetPos, &pos, &targetPos);
}

void Camera::MoveUp(float distance) {
	targetPos.y += distance / 16.0f;
}

void Camera::MoveDown(float distance) {
	targetPos.y -= distance / 16.0f;
}

void Camera::MoveLeft(float distance) {
	targetPos.x -= (cosf(rotationHoz) / 16) * distance;
	targetPos.z -= (sinf(rotationHoz) / 16) * distance;
}

void Camera::MoveRight(float distance) {
	targetPos.x += (cosf(rotationHoz) / 16) * distance;
	targetPos.z += (sinf(rotationHoz) / 16) * distance;
}

void Camera::MoveForward(float distance) {
	guVector scaledDistance;
	guVecScale(&direction, &scaledDistance, 0.0625f * distance);
	guVecAdd(&targetPos, &scaledDistance, &targetPos);
}

void Camera::MoveBackward(float distance) {
	guVector scaledDistance;
	guVecScale(&direction, &scaledDistance, -0.0625f * distance);
	guVecAdd(&targetPos, &scaledDistance, &targetPos);
}

void Camera::UpdateCameraValues() {

	//so that anything can happen to the values beforehand and only 1 place has to check the target values before being used
	//note: targetVer is validated on change, unlike targetHoz which is validated on use
	while (targetHoz < 0) targetHoz += 2 * M_PI;
	while (targetHoz > 2 * M_PI) targetHoz -= 2 * M_PI;

	//rotation (hoz)
	if (rotationHoz != targetHoz) {

		//if 1, camera moves clockwise (right), if -1 camera moves anticlockwise (left)
		float swapDirection;
		float rotationHozDifference = fabs(rotationHoz - targetHoz);

		bool currentLargerThanTarget = (rotationHoz > targetHoz) ? true : false;
		bool differenceAbovePi = (rotationHozDifference > M_PI) ? true : false;
		
		//if difference above pi, current rotation must eventually pass through 0
		if (differenceAbovePi) {
			rotationHozDifference = (2 * M_PI) - rotationHozDifference;
		}

		//refer to that one painting i made for reasons bools are this way
		if (currentLargerThanTarget == differenceAbovePi) {
			swapDirection = 1.0f;
		}
		else {
			swapDirection = -1.0f;
		}

		if (rotationHozDifference <= 0.005f) {
			rotationHoz = targetHoz;
		}
		else if (rotationHozDifference < 0.007692f) {
			rotationHoz += (rotationHozDifference - 0.005f) * swapDirection;
		}
		else {
			rotationHoz += (rotationHozDifference * 0.35f) * swapDirection;
		}
		/*original values, doesnt feel very slidy
		if (rotationHozDifference <= 0.015625f) {
			rotationHoz = targetHoz;
		}
		else if (rotationHozDifference < 0.03125f) {
			rotationHoz += (rotationHozDifference - 0.015625f) * swapDirection;
		}
		else {
			rotationHoz += (rotationHozDifference * 0.5f) * swapDirection;
		*/
	}

	//rotation (ver)

	float camVerPrev = rotationVer;
	if (rotationVer != targetVer) {

		float rotationVerDifference = rotationVer - targetVer;

		if (fabs(rotationVerDifference) <= 0.005f) {
			rotationVer = targetVer;
		}
		else if (fabs(rotationVerDifference) < 0.007692f) {
			rotationVer -= rotationVerDifference + ((rotationVerDifference < 0) ? 0.005f : -0.005f);
		}
		else {
			rotationVer -= rotationVerDifference * 0.35f;
		}
	}

	//fix values
	while (rotationHoz < 0) rotationHoz += 2 * M_PI;
	while (rotationHoz > 2 * M_PI) rotationHoz -= 2 * M_PI;
	while (rotationVer < -(M_PI / 2)) rotationVer = camVerPrev;
	while (rotationVer > (M_PI / 2)) rotationVer = camVerPrev;

	UpdateDirectionValue();


	//position

	float positionDistance = VectorDistance(pos, targetPos);
	guVector directionToTarget = DirectionUnitVector(pos, targetPos);
	guVector scaledVector;
	if (positionDistance <= 0.005f) {
		pos = targetPos;
	}
	else if (positionDistance < 0.00792f) {
		c_guVecScale(&directionToTarget, &scaledVector, positionDistance - 0.005f);
		c_guVecAdd(&pos, &scaledVector, &pos);
	}
	else {
		c_guVecScale(&directionToTarget, &scaledVector, positionDistance * 0.35f);
		c_guVecAdd(&pos, &scaledVector, &pos);
	}
}

//change camera direction
void Camera::UpdateDirectionValue() {
	direction = DirectionUnitVector(pos, { pos.x + (sinf(rotationHoz) * cosf(rotationVer)), pos.y + sinf(rotationVer), pos.z - (cosf(rotationHoz) * cosf(rotationVer)) });
}