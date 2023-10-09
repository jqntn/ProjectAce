#pragma once

#include <cmath>

constexpr float Pi = 3.14159265358979323846f;
constexpr float Deg2Rad = 1.f / 180.f * Pi;
constexpr float Rad2Deg = 1.f / Pi * 180.f;

inline float DegreesToRadians(float deg)
{
	return deg * Deg2Rad;
}

inline float RadiansToDegrees(float rad)
{
	return rad * Rad2Deg;
}