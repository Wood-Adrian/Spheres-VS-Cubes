#pragma once

//this is redundant but intellisense doesnt know hw_rvl will always be defined bc defined at top of main.cpp
#ifndef HW_RVL
#define HW_RVL
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <string>
#include <vector>
#include <memory>
#include <limits>

#include <ogcsys.h>
#include <gccore.h>
#include <grrlib.h>
#include <ogc/lwp_watchdog.h>

#include "sphere.hpp"
#include "vectorUtils.hpp"

enum class CubeTargeting {
	FIRST = 0,
	LAST,
	CLOSE,
	STRONG
};

struct projectileInfo {
	float speed;
	int pierce;
	float distanceLifetime;
	double damage;
};

struct projectileBeginInfo {
	guVector position;
	guVector direction;
};

class Cube {

private:
	
	struct cubeTypeInfo {
		u32 colour;
		float radius;
		float ROF; //rate of fire (s)
		int cost;
		projectileInfo projInfo;
	};

	//contains base stats of cubes and the projectiles they create
	static constexpr cubeTypeInfo typeInfo[] = {
		//{colour, radius, ROF, cost, {speed, pierce, distanceLifetime, damage}}
		{ RGBA(0xff, 0x33, 0x33, 0xff), 10.0f, 1.0f, 200, { 0.5f, 1, 15.0f, 20.0f } },
		{ RGBA(0x33, 0xff, 0x33, 0xff), 15.0f, 1.0f, 200, { 0.5f, 1, 15.0f, 20.0f } },
		{ RGBA(0x33, 0x33, 0xff, 0xff), 7.0f, 0.25f, 200, { 0.5f, 1, 15.0f, 20.0f } }
	};

	guVector position;
	float rotationHoz;
	guVector projectileDirection;
	u32 type;
	float radius; //distance cube can attack
	float ROF; //rate of fire (s)
	u64 lastFireTime;
	CubeTargeting targetMode;

public:

	//get colour from cube type NOTE: only supports solid colours
	//@param _type Type of cube (from cube object)
	static u32 Colour(u32 _type);

	//create new cube at specified position
	Cube(guVector _pos, u32 _type);

	//get position of cube
	guVector GetPosition();

	//get rotation of cube
	//@return rotationHoz of cube (needs to be negated for camera use!)
	float GetRotation();

	//get type of cube
	//@return type of cube
	u32 GetType();

	//get projectile info from cube
	projectileInfo GetProjectileInfo();

	//get info to begin moving projectile from cube
	projectileBeginInfo GetProjectileBeginInfo();

	//scan spheres to check if any are in range
	//NOTE: should become quadtree/octree for optimisation in future!
	//@return true if should shoot, false if either 1. no spheres or 2. too soon to shoot
	bool ScanSpheres(const std::vector<std::shared_ptr<Sphere>>& sphereList, u64 currentTime);

	//add time difference from when game is paused in some way
	//@param timeDifference difference in time from when first paused to unpause
	void AddTimeDifference(u64 timeDifference);

};