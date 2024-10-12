#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <string>
#include <vector>

#include <ogcsys.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include <ogc/lwp_watchdog.h>

#include "vectorUtils.hpp"


class Sphere {

private:

	struct typeInfoStruct {
		u32 colour;
		float speed;
		double maxHealth;
	};

	//contains base stats of spheres
	static constexpr typeInfoStruct typeInfo[] = {
		//invisible (do not use!!!)
		{ RGBA(0x00,0x00,0x00,0xFF), 0.0625f, 100},
		//red
		{ RGBA(0xFF,0x11,0x11,0xFF), 0.0625f, 100},
		//yellow
		{ RGBA(0xFF,0xFF,0x11,0xFF), 0.0625f, 100},
		//green
		{ RGBA(0x11,0xFF,0x11,0xFF), 0.09375f, 100},
		//blue
		{ RGBA(0x11,0x11,0xFF,0xFF), 0.125f, 100},
		//orange
		{ RGBA(0xFF,0x99,0x33,0xFF), 0.1825f, 100},
		//pink
		{ RGBA(0xFF,0xAA,0xFF,0xFF), 0.0625f, 100},
	};


	guVector position;
	u32 type;
	double healthMax;
	double health;
	bool alive;
	float speed;
	size_t nextPathPoint;
	guVector direction;
	//integer part is number of lines of path completed (i.e. nextPathPoint-1)
	//decimal part is percentage of distance travelled in current path
	float distanceThroughPath;

public:

	//get colour from sphere type NOTE: only supports solid colours
	//@param _type Type of sphere (from sphere object)
	static u32 Colour(u32 _type);

	//constructor to specify attributes
	//@param _mapPoints The points of the path that the sphere should follow
	//@param _position Position of sphere
	//@param _type Type of sphere
	Sphere(const std::vector<guVector>& _mapPoints, u32 _type = 0);

	//Sets attributes as given from the constructor/when directly called
	//@param _mapPoints The points of the path that the sphere should follow
	//@param _position Position of sphere
	//@param _type Type of sphere
	void Activate(const std::vector<guVector>& _mapPoints, u32 _type = 0);

	//get current position
	//@return guVector of position (struct containing x,y,z)
	guVector GetPosition() const;

	//set current sphere position
	//@param _position Position of sphere
	void SetPosition(guVector _position);

	//get type of sphere
	//@return Type of sphere
	u32 GetType() const;

	//set type of sphere
	//@param _type Type of sphere
	void SetType(u32 _type);

	//get remaining health of sphere
	double GetHealth();

	//get if sphere is currently alive
	//@return true if sphere is alive, false if dead
	bool GetAlive();

	//set if sphere should be alive or not
	//@param _alive Is alive?
	void SetAlive(bool _alive);

	//get direction that sphere is travelling in
	guVector GetDirection();

	//@return distanceThroughPath, i.e. number representing distance through the path
	float GetDistanceThroughPath();

	//return amount of money to receive when getting killed
	//@return Money received when killing this sphere
	int GetKilledMoney();

	//update the sphere's position to move it forwards for the next frame, autoadjusts direction vector as needed
	//@param _mapPoints Points of the path that the sphere should follow
	//@param deltaTime Delta time of last frame (should be ~1 at 60fps)
	//@return true if still has more to travel, false if sphere has surpassed end
	bool UpdatePosition(const std::vector<guVector>& _mapPoints, double deltaTime);

	//Deal with hit damage
	//@param healthDrop amount to drop health by (i.e. damage of projectile)
	//@return true if sphere still alive, false if killed
	bool Hit(double healthDrop);
};
