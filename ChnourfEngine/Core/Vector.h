#pragma once
#include "glm\glm.hpp"

template <class T> struct Vector2
{
	Vector2():
		x(T(0)),
		y(T(0))
	{}

	Vector2(const T& anX, const T& anY) :
		x(anX),
		y(anY)
	{}

	Vector2(const T& anX):
		x(anX),
		y(anX)
	{}

	inline void Normalize()
	{
		T squareNorm = x*x + y*y;
		if (squareNorm > 0)
		{
			const T invNorm = T(1) / (T)sqrt(squareNorm);
			x *= invNorm;
			y *= invNorm;
		}
	}

	inline static T Distance(const Vector2<T>& aVector, const Vector2<T>& anOther)
	{
		return sqrt(pow(aVector.x - anOther.x, 2) + pow(aVector.y - anOther.y, 2));
	}

	inline static float Cross(const Vector2<T>& aVector, const Vector2<T>& anOther)
	{
		return aVector.x * anOther.y - anOther.x * aVector.y;
	}

	inline static float Dot(const Vector2<T>& aVector, const Vector2<T>& anOther)
	{
		return aVector.x * anOther.x + aVector.y * anOther.y;
	}

	void operator=(const Vector2<T>& anOther)
	{
		x = anOther.x;
		y = anOther.y;
	}

	T x;
	T y;
};

template <class T>
Vector2<T> operator+(const Vector2<T>& l, const Vector2<T>& r)
{
	return Vector2<T>(l.x + r.x, l.y + r.y);
}

template <class T>
Vector2<T> operator-(const Vector2<T>& l, const Vector2<T>& r)
{
	return Vector2<T>(l.x - r.x, l.y - r.y);
}

template <class T>
bool operator==(const Vector2<T>& l, const Vector2<T>& r)
{
	return l.x == r.x && l.y == r.y;
}

template <class T> struct Vector3
{
	Vector3():
		x(T(0)),
		y(T(0)),
		z(T(0))
	{}

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

	Vector3(const Vector2<T> aVec2) :
		x(aVec2.x),
		y(T(0)),
		z(aVec2.y)
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

	void operator=(const Vector3<T>& anOther)
	{
		x = anOther.x;
		y = anOther.y;
		z = anOther.z;
	}

	T x;
	T y;
	T z;
};

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

template <class T>
bool operator==(const Vector3<T>& l, const Vector3<T>& r)
{
	return l.x == r.x && l.y == r.y && l.z == r.z;
}

typedef Vector2<float>  vec2f;
typedef Vector2<int>  vec2i;

typedef Vector3<float>  vec3f;
typedef Vector3<int>  vec3i;