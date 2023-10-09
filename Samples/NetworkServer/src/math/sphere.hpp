#pragma once

#include <math/vector3.hpp>
#include <cmath>
#include <ostream>

// Classe gérant des sphères (position + rayon)
struct Sphere
{
	Vector3 center;
	float radius;

	// Constructeur par défaut (initialisation à zéro)
	Sphere() :
	radius(0.f)
	{
	}

	// Constructeur avec valeurs
	Sphere(const Vector3& c, float r) :
	center(c),
	radius(r)
	{
	}

	// Constructeur par copie (généré par le compilateur)
	Sphere(const Sphere&) = default;

	// Opérateur d'assignation par copie (généré par le compilateur)
	Sphere& operator=(const Sphere&) = default;

	// Calcule la distance entre la surface de deux sphères (négatif si intersection): float d = Sphere::Distance(s1, s2)
	static float Distance(const Sphere& s1, const Sphere& s2)
	{
		return Vector3::Distance(s1.center, s2.center) - s1.radius - s2.radius;
	}

	// Renvoie vrai si les sphères sont en intersection: if (Sphere::Intersects(s1, s2))
	static bool Intersects(const Sphere& s1, const Sphere& s2)
	{
		return Distance(s1, s2) < 0.f;
	}

	// Opérateurs arithmétiques

	// Comparaison
	bool operator==(const Sphere& sphere) const
	{
		return center == sphere.center && radius == sphere.radius;
	}

	bool operator!=(const Sphere& sphere) const
	{
		return !operator==(sphere);
	}
};

inline std::ostream& operator<<(std::ostream& oss, const Sphere& sphere)
{
	return oss << "Sphere(" << sphere.center << "; " << sphere.radius << ")";
}
