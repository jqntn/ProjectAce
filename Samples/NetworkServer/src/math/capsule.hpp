#pragma once

#include <math/sphere.hpp>
#include <math/vector3.hpp>
#include <cmath>
#include <ostream>

// Classe gérant des capsules (segment avec rayon)
struct Capsule
{
	Vector3 start;
	Vector3 end;
	float radius;

	// Constructeur par défaut (initialisation à zéro)
	Capsule() :
	radius(0.f)
	{
	}

	// Constructeur avec valeurs
	Capsule(const Vector3& s, const Vector3& e, float r) :
	start(s),
	end(e),
	radius(r)
	{
	}

	// Constructeur avec centre + hauteur (aligné sur Y)
	Capsule(const Vector3& c, float h, float r) :
	start(c - Vector3(0, h * 0.5f, 0)),
	end(c + Vector3(0, h * 0.5f, 0)),
	radius(r)
	{
	}

	// Constructeur par copie (généré par le compilateur)
	Capsule(const Capsule&) = default;

	// Opérateur d'assignation par copie (généré par le compilateur)
	Capsule& operator=(const Capsule&) = default;

	// Renvoie vrai s'il y a une intersection entre une capsule et une sphère
	static bool Intersects(const Capsule& capsule, const Sphere& sphere)
	{
		// Est-ce que la distance (au carré) entre le segment formé par la capsule et la position de la sphère
		// est inférieur au radius de la capsule et de la sphère (au carré)
		float distSq = Vector3::SegmentDistance(capsule.start, capsule.end, sphere.center);
		float radius = capsule.radius + sphere.radius;
		return distSq <= radius * radius;
	}

	// Renvoie vrai s'il y a une intersection entre deux capsules
	static bool Intersects(const Capsule& c1, const Capsule& c2)
	{
		// Est-ce que la distance (au carré) entre les deux segments est inférieur à la somme des radius au carré
		float d1, d2;
		Vector3 p1, p2;
		float distSq = Vector3::ClosestPoint(c1.start, c1.end, c2.start, c2.end, d1, d2, p1, p2);
		float radius = c1.radius + c2.radius;
		return distSq <= radius * radius;
	}

	// Opérateurs arithmétiques

	// Comparaison
	bool operator==(const Capsule& capsule) const
	{
		return start == capsule.start && end == capsule.end && radius == capsule.radius;
	}

	bool operator!=(const Capsule& capsule) const
	{
		return !operator==(capsule);
	}
};

inline std::ostream& operator<<(std::ostream& oss, const Capsule& capsule)
{
	return oss << "Capsule(" << capsule.start << " -> " << capsule.end << "; " << capsule.radius << ")";
}
