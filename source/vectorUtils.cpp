#include "vectorUtils.hpp"

float VectorDistance(const guVector& vect1, const guVector& vect2) {
	return sqrtf(powf((vect1.x - vect2.x), 2) + powf((vect1.y - vect2.y), 2) + powf((vect1.z - vect2.z), 2));
}

guVector DirectionUnitVector(const guVector& location, const guVector& destination) {
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


bool VectorSurpassedPoint(const guVector& position, const guVector& destination, const guVector& direction) {
	guVector newDirection = DirectionUnitVector(position, destination);
	if (guVecDotProduct(&direction, &newDirection) <= 0) {
		return true;
	}
	return false;
}



//massive thanks to wikipedia and the source below for explaining this
//https://scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection.html

bool RayIntersectTriangle(const guVector& rayOrg, const guVector& rayDir, const guVector& tri1, const guVector& tri2, const guVector& tri3, guVector& pos) {
	constexpr float epsilon = std::numeric_limits<float>::epsilon();

	const guVector edge1 = guVector({	tri2.x - tri1.x,
										tri2.y - tri1.y, 
										tri2.z - tri1.z
									});
	const guVector edge2 = guVector({	tri3.x - tri1.x,
										tri3.y - tri1.y,
										tri3.z - tri1.z
									});

	//creates vector orthogonal (perpendicular) to the ray direction and one of the edges of the triangle
	//the P in equation 5 (website) (D x E2 (AC))
	guVector pVector;
	guVecCross(&rayDir, &edge2, &pVector);

	//if (near) 0, perpendicular, but because pVector perpendicular to rayDir, ray is parallel to trangle, i.e., never meets triangle
	f32 determinant = guVecDotProduct(&edge1, &pVector);
	if (fabsf(determinant) < epsilon) return false;

	//NOTE: the above could have been done like this:
	//guVecCross(&edge1, &edge2, &normal);
	//f32 determinant = guVecDotProduct(&rayDir, &normal);
	//however this causes the determinant to be negated compared to what is done (i could just negate it myself but the pVector above is also used below)

	float invDet = 1.0 / determinant;

	//the T in equation 5 (website) (O - A)
	guVector tVector;
	guVecSub(&rayOrg, &tri1, &tVector);
	//u is the second unknown
	float uVal = guVecDotProduct(&tVector, &pVector) * invDet;
	//if 0 <= u <= 1 (and not small enough for margin of error (wikipedia)).
	if ((uVal < 0 && fabsf(uVal) > epsilon) || (uVal > 1 && fabsf(uVal - 1) > epsilon)) return false;

	//the Q in equation 5 (website) (T x E1 (AB))
	guVector qVector;
	guVecCross(&tVector, &edge1, &qVector);
	//v is the third unknown
	float vVal = guVecDotProduct(&rayDir, &qVector) * invDet;
	//ensure  0<=v<=1 and u+1 <= 1
	if ((vVal < 0 && fabsf(vVal) > epsilon) || (uVal + vVal > 1 && fabsf(uVal + vVal - 1) > epsilon)) return false;

	//t is the first unknown (how far in rayDir you have to travel)
	float tVal = guVecDotProduct(&qVector, &edge2) * invDet;

	guVector rayDirScaledByT;
	guVecScale(&rayDir, &rayDirScaledByT, tVal);
	guVecAdd(&rayOrg, &rayDirScaledByT, &pos);

	if (tVal > epsilon) return true;

	return false;
}