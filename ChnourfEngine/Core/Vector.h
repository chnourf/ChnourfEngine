#pragma once
#include "glm\glm.hpp"


template <class T> struct Vector2
{
	Vector2(){}

	Vector2(const T& anX, const T& anY) :
		x(anX),
		y(anY)
	{}

	Vector2(const T& anX):
		x(anX),
		y(anY)
	{}

	T x;
	T y;
};

template <class T> struct Vector3
{
	Vector3() {}
	Vector3(const T& anX, const T& anY, const T& aZ) :
		x(anX),
		y(anY),
		z(aZ)
	{}

	Vector3(const glm::vec3& aVec3):
		x(aVec3.x),
		y(aVec3.y),
		z(aVec3.z)
	{}

	Vector3(const T& anX):
		x(anX),
		y(anX),
		z(anX)
	{}

	inline void Normalize()
	{
		T squareNorm = x*x + y*y + z*z;
		if (squareNorm > 0)
		{
			const T invNorm = T(1) / (T)sqrt(squareNorm);
			x *= invNorm;
			y *= invNorm;
			z *= invNorm;
		}
	}

	T x;
	T y;
	T z;
};

template <class T>
bool operator==(const Vector2<T>& l, const Vector2<T>& r)
{
	return l.x == r.x && l.y == r.y;
}

template <class T>
Vector2<T> operator-(const Vector2<T>& l, const Vector2<T>& r)
{
	return Vector2<T>(l.x - r.x, l.y - r.y);
}

template <class T>
Vector3<T> operator+(const Vector3<T>& l, const Vector3<T>& r)
{
	return Vector3<T>(l.x + r.x, l.y + r.y, l.z + r.z);
}

template <class T>
Vector3<T> operator-(const Vector3<T>& l, const Vector3<T>& r)
{
	return Vector3<T>(l.x - r.x, l.y - r.y, l.z - r.z);
}

typedef Vector2<float>  vec2f;
typedef Vector2<int>  vec2i;

typedef Vector3<float>  vec3f;
typedef Vector3<int>  vec3i;