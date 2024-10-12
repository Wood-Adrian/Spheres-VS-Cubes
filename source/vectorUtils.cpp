#include "vectorUtils.hpp"

//finds distance between 2 points
float VectorDistance(guVector vect1, guVector vect2) {
	return sqrtf(powf((vect1.x - vect2.x), 2) + powf((vect1.y - vect2.y), 2) + powf((vect1.z - vect2.z), 2));
}

//get the unit vector for the direction to go from location->destination
guVector DirectionUnitVector(guVector location, guVector destination) {
	float distance = VectorDistance(location, destination);
	if (distance == 0) {
		return { 0,0,0 };
	}
	return guVector({
		(destination.x - location.x) / distance,
		(destination.y - location.y) / distance,
		(destination.z - location.z) / distance
		});
}

//return true if position has surpassed destination in the direction of direction
bool VectorSurpassedPoint(guVector position, guVector destination, guVector direction) {
	guVector newDirection = DirectionUnitVector(position, destination);
	if (guVecDotProduct(&direction, &newDirection) <= 0) {
		return true;
	}
	return false;
}