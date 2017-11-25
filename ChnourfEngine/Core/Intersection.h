#pragma once
#include "Vector.h"
#include <algorithm>

struct AABB
{
	AABB(){}

	AABB(const vec3f& aMin, const vec3f aMax) :
		myMin(aMin),
		myMax(aMax)
	{
	}

	float GetRadius() const
	{
		auto x = pow(myMax.x - myMin.x, 2);
		auto y = pow(myMax.y - myMin.y, 2);
		auto z = pow(myMax.z - myMin.z, 2);

		return sqrt(x + y + z) / 2;
	}

	vec3f myMin;
	vec3f myMax;
};

struct Ray
{
	Ray(){}

	Ray(const vec3f& anOrigin, const vec3f aDirection) :
		myOrigin(anOrigin),
		myDirection(aDirection)
	{
	}

	vec3f myOrigin;
	vec3f myDirection;
};

struct Plane
{
	Plane(){}

	Plane(const vec3f& aNormal, float aDistanceFromOrigin) :
		myNormal(aNormal),
		myDistanceFromOrigin(aDistanceFromOrigin)
	{}

	vec3f myNormal;
	float myDistanceFromOrigin;
};

struct Frustum
{
	Frustum() {}

	Frustum(const Plane& aNear, const Plane& aFar, const Plane& aLeft, const Plane& aRight, const Plane& aBottom, const Plane& aTop) :
		myNear(aNear),
		myFar(aFar),
		myLeft(aLeft),
		myRight(aRight),
		myBottom(aBottom),
		myTop(aTop)
	{}

	Plane myNear;
	Plane myFar;
	Plane myLeft;
	Plane myRight;
	Plane myBottom;
	Plane myTop;
};

// Solution by txutxi, uses only one vertex
inline bool AABBvsFrustum(const AABB& anAABB, const Frustum& aFrustum)
{
	// Indexed for the 'index trick' later
	vec3f box[] = { anAABB.myMin, anAABB.myMax };

	// We have 6 planes defining the frustum
	static const int NUM_PLANES = 6;
	const Plane *planes[NUM_PLANES] =
	{ &aFrustum.myNear, &aFrustum.myLeft, &aFrustum.myRight, &aFrustum.myBottom, &aFrustum.myTop, &aFrustum.myFar };

	// We only need to do 6 point-plane tests
	for (int i = 0; i < NUM_PLANES; ++i)
	{
		// This is the current plane
		const Plane &p = *planes[i];

		// p-vertex selection (with the index trick)
		// According to the plane normal we can know the
		// indices of the positive vertex
		const int px = static_cast<int>(p.myNormal.x > 0.0f);
		const int py = static_cast<int>(p.myNormal.y > 0.0f);
		const int pz = static_cast<int>(p.myNormal.z > 0.0f);

		// Dot product
		// project p-vertex on plane normal
		// (How far is p-vertex from the origin)
		const float dp =
			(p.myNormal.x*box[px].x) +
			(p.myNormal.y*box[py].y) +
			(p.myNormal.z*box[pz].z);

		// Doesn't intersect if it is behind the plane
		if (dp < -p.myDistanceFromOrigin)
		{ 
			return false;
		}
	}
	return true;
}

inline bool IsPointInsideAABB(const AABB& anAABB, const vec3f& aPoint)
{
	return aPoint.x >= anAABB.myMin.x && aPoint.y >= anAABB.myMin.y && aPoint.z >= anAABB.myMin.z
		&& aPoint.x <= anAABB.myMax.x && aPoint.y <= anAABB.myMax.y && aPoint.z <= anAABB.myMax.z;
}

inline bool AABBvsRay(const AABB& anAABB, const Ray& aRay)
{
	float tmin = -INFINITY, tmax = INFINITY;

	if (aRay.myDirection.x != 0.f) {
		float tx1 = (anAABB.myMin.x - aRay.myOrigin.x) / aRay.myDirection.x;
		float tx2 = (anAABB.myMax.x - aRay.myOrigin.x) / aRay.myDirection.x;

		tmin = std::max(tmin, std::min(tx1, tx2));
		tmax = std::min(tmax, std::max(tx1, tx2));
	}

	if (aRay.myDirection.y != 0.0) {
		float ty1 = (anAABB.myMin.y - aRay.myOrigin.y) / aRay.myDirection.y;
		float ty2 = (anAABB.myMax.y - aRay.myOrigin.y) / aRay.myDirection.y;

		tmin = std::max(tmin, std::min(ty1, ty2));
		tmax = std::min(tmax, std::max(ty1, ty2));
	}

	if (aRay.myDirection.z != 0.0) {
		float tz1 = (anAABB.myMin.z - aRay.myOrigin.z) / aRay.myDirection.z;
		float tz2 = (anAABB.myMax.z - aRay.myOrigin.z) / aRay.myDirection.z;

		tmin = std::max(tmin, std::min(tz1, tz2));
		tmax = std::min(tmax, std::max(tz1, tz2));
	}

	return tmax >= tmin;
}