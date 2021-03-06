#pragma once
#include <vector>

// COPYRIGHT 2002 KEN PERLIN.

#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H


class PerlinNoise {
	// The permutation vector
	std::vector<int> p;
public:
	// Initialize with the reference values for the permutation vector
	PerlinNoise();
	// Generate a new permutation vector based on the value of seed
	PerlinNoise(unsigned int seed);
	// Get a noise value, for 2D images z can have any value
	double noise(double x, double y, double z = 0) const;

private:
	double fade(double t) const;
	double lerp(double t, double a, double b) const;
	double grad(int hash, double x, double y, double z) const;
};

#endif