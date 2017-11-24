#pragma once
#include <cmath>

const unsigned int locPictureDimension = 2048u;
const unsigned int MetersPerPixel = 128;
const float locMapSize = (float)MetersPerPixel * (float)locPictureDimension;
const unsigned int locGridNumOfElements = 128;
const float locDistanceBetweenElements = locMapSize / (float)(locGridNumOfElements);
const unsigned int locDepth = 6u;
const float gain = 0.5f;
const float lacunarity = 1.90f;
const float locSeaLevel = 0.45f;
const float rainfallDiffusionCoefficient = 0.1f;
const float rainfallMountainTransmissionRate = 0.4f;
const float windNoisePatternSize = locMapSize / 5.f;
const unsigned int riverNumber = pow(locGridNumOfElements / 16u, 2u);