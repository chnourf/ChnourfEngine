#pragma once



template <class T> struct Vector2
{
	Vector2(){}
	Vector2(const T& anX, const T& anY) :
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

	T x;
	T y;
	T z;
};

template <class T>
bool operator==(const Vector2<T>& l, const Vector2<T>& r)
{
	return l.x == r.x && l.y == r.y;
}

typedef Vector2<float>  vec2f;
typedef Vector2<int>  vec2i;

typedef Vector3<float>  vec3f;
typedef Vector3<int>  vec3i;