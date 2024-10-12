#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <string>

#include <ogcsys.h>
#include <gccore.h>

#include "vectorUtils.hpp"

struct CameraData {
	guVector pos;
	guVector direction;
	float hoz;
	float ver;
	float FOV;
};

class Camera {

private:

	guVector pos;
	guVector targetPos;
	guVector direction;
	float rotationHoz;
	float targetHoz;
	float rotationVer;
	float targetVer;
	float FOV;

	void UpdateDirectionValue();

public:

	//contructor
	Camera(guVector _pos, float _rotationHoz, float _rotationVer, float _FOV);

	//get current camera data
	const CameraData GetCameraData();

	//set camera target position
	void SetPositionTarget(guVector _targetPos);
	//set camera rotation target
	void SetRotationTarget(float _targetHoz, float _targetVer);
	//set camera FOV;
	void SetFOV(float _FOV);

	//rotate target of camera by given amount
	void RotateCamera(float _rotateHoz, float _rotateVer);
	//move target of camera position by given values
	void MoveCamera(guVector _pos);
	//move camera up (0-1)
	void MoveUp(float distance);
	//move camera down (0-1)
	void MoveDown(float distance);
	//move camera left (0-1)
	void MoveLeft(float distance);
	//move camera right (0-1)
	void MoveRight(float distance);
	//move camera forwards (0-1)
	void MoveForward(float distance);
	//move camera backwards (0-1)
	void MoveBackward(float distance);
	//update position/rotation to get closer to target (run every frame before render
	void UpdateCameraValues(void);

};
