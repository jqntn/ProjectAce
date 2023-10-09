#pragma once

#include <math/utils.hpp>
#include <cmath>
#include <ostream>

struct Vector3;

inline Vector3 operator*(float n, const Vector3& vec);
inline Vector3 operator/(float n, const Vector3& vec);

// Classe gérant des vecteurs à trois dimensions
struct Vector3
{
	float x;
	float y;
	float z;

	// Constructeur par défaut (initialisation à zéro)
	Vector3() :
	x(0.f),
	y(0.f),
	z(0.f)
	{
	}

	// Constructeur avec valeurs
	Vector3(float X, float Y, float Z) :
	x(X),
	y(Y),
	z(Z)
	{
	}

	// Constructeur par copie (généré par le compilateur)
	Vector3(const Vector3&) = default;

	// Opérateur d'assignation par copie (généré par le compilateur)
	Vector3& operator=(const Vector3&) = default;

	// Calcul de la longueur: float l = vec.Length()
	float Length() const
	{
		return std::sqrt(SquaredLength());
	}

	// Calcul de la longueur au carré: float l = vec.SquaredLength()
	float SquaredLength() const
	{
		return DotProduct(*this, *this);
	}

	// Calcule les points de proximité entre deux segments, retournant leur distance au carré (et d'autres infos)
	// Utilisé pour l'intersection de capsules
	static float ClosestPoint(const Vector3& start1, const Vector3& end1, const Vector3& start2, const Vector3& end2, float& dist1, float& dist2, Vector3& p1, Vector3& p2)
	{
		// Vient du livre "Real Time Collision Detection" (parce qu'il faut pas déconner)
		Vector3 d1 = end1 - start1; // Direction vector of segment S1
		Vector3 d2 = end2 - start2; // Direction vector of segment S2
		Vector3 r = start1 - start2;
		float a = DotProduct(d1, d1); // Squared length of segment S1, always nonnegative
		float e = DotProduct(d2, d2); // Squared length of segment S2, always nonnegative
		float f = DotProduct(d2, r);

		// Check if either or both segments degenerate into points
		if (a <= Epsilon<float> && e <= Epsilon<float>)
		{
			// Both segments degenerate into points
			dist1 = dist2 = 0.0f;
			p1 = start1;
			p2 = start2;

			return DotProduct(p1 - p2, p1 - p2);
		}

		if (a <= Epsilon<float>)
		{
			// First segment degenerates into a point
			dist1 = 0.0f;
			dist2 = f / e; // s = 0 => t = (b*s + f) / e = f / e
			dist2 = Clamp(dist2, 0.0f, 1.0f);
		}
		else
		{
			float c = DotProduct(d1, r);
			if (e <= Epsilon<float>)
			{
				// Second segment degenerates into a point
				dist2 = 0.0f;
				dist1 = Clamp(-c / a, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
			}
			else
			{
				// The general nondegenerate case starts here
				float b = DotProduct(d1, d2);
				float denom = a * e - b * b; // Always nonnegative
				// If segments not parallel, compute closest point on L1 to L2 and
				// clamp to segment S1. Else pick arbitrary s (here 0)
				if (denom != 0.0f)
					dist1 = Clamp((b * f - c * e) / denom, 0.0f, 1.0f);
				else 
					dist1 = 0.0f;

				// Compute point on L2 closest to S1(s) using
				// t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
				dist2 = (b * dist1 + f) / e;
				// If t in [0,1] done. Else clamp t, recompute s for the new value
				// of t using s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1)= (t*b - c) / a
				// and clamp s to [0, 1]
				if (dist2 < 0.0f)
				{
					dist2 = 0.0f;
					dist1 = Clamp(-c / a, 0.0f, 1.0f);
				}
				else if (dist2 > 1.0f)
				{
					dist2 = 1.0f;
					dist1 = Clamp((b - c) / a, 0.0f, 1.0f);
				}
			}
		}

		p1 = start1 + d1 * dist1;
		p2 = start2 + d2 * dist2;
		return DotProduct(p1 - p2, p1 - p2);
	}

	// Produit vectoriel: Vec3 r = Vec3::CrossProduct(v1, v2)
	static Vector3 CrossProduct(const Vector3& v1, const Vector3& v2)
	{
		return Vector3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
	}

	// Calcule la distance entre deux positions: float d = Vector3::Distance(v1, v2)
	static float Distance(const Vector3& v1, const Vector3& v2)
	{
		return (v1 - v2).Length();
	}

	static Vector3 Reflect(const Vector3& incident, const Vector3& normal)
	{
		return incident - 2.f * DotProduct(incident, normal) * normal;
	}

	// Calcule la distance (au carré) entre un segment et une position: float sqDist = Vector3::Distance(start, end, p)
	static float SegmentDistance(const Vector3& start, const Vector3& end, const Vector3& position)
	{
		Vector3 ab = end - start;
		Vector3 ac = position - start;
		Vector3 bc = position - end;
		// Projection du point sur le segment
		float e = DotProduct(ac, ab);

		// Au début du segment, distance avec le start
		if (e <= 0.f)
			return DotProduct(ac, ac);

		// Fin du segment, distance avec end
		float f = DotProduct(ab, ab);
		if (e >= f)
			return DotProduct(bc, bc);

		// Projection sur le segment lui-même
		return DotProduct(ac, ac) - e * e / f;
	}

	// Produit scalaire: float r = Vec3::DotProduct(v1, v2)
	static float DotProduct(const Vector3& v1, const Vector3& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	// Normalisation d'un vecteur: vec2 norm = Vec3::Normalize(vec)
	static Vector3 Normalize(const Vector3& vec)
	{
		return vec / vec.Length();
	}

	// Opérateurs arithmétiques

	// Moins unaire
	Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}

	// Addition
	Vector3& operator+=(const Vector3& vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;

		return *this;
	}

	Vector3 operator+(const Vector3& vec) const
	{
		Vector3 result(*this);
		result += vec;

		return result;
	}

	// Soustraction
	Vector3& operator-=(const Vector3& vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;

		return *this;
	}

	Vector3 operator-(const Vector3& vec) const
	{
		Vector3 result(*this);
		result -= vec;

		return result;
	}

	// Multiplication par un nombre
	Vector3& operator*=(float n)
	{
		x *= n;
		y *= n;
		z *= n;

		return *this;
	}

	Vector3 operator*(float n) const
	{
		Vector3 result(*this);
		result *= n;

		return result;
	}

	// Multiplication par un vecteur (multiplication des composants)
	Vector3& operator*=(const Vector3& vec)
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;

		return *this;
	}

	Vector3 operator*(const Vector3& vec) const
	{
		Vector3 result(*this);
		result *= vec;

		return result;
	}

	// Division par un nombre
	Vector3& operator/=(float n)
	{
		x /= n;
		y /= n;
		z /= n;

		return *this;
	}

	Vector3 operator/(float n) const
	{
		Vector3 result(*this);
		result /= n;

		return result;
	}

	// Comparaison
	bool operator==(const Vector3& vec) const
	{
		return x == vec.x && y == vec.y && z == vec.z;
	}

	bool operator!=(const Vector3& vec) const
	{
		return !operator==(vec);
	}
};

// float * vec3
inline Vector3 operator*(float n, const Vector3& vec)
{
	return Vector3(n * vec.x, n * vec.y, n * vec.z);
}

// float / vec3
inline Vector3 operator/(float n, const Vector3& vec)
{
	return Vector3(n / vec.x, n / vec.y, n / vec.z);
}

inline std::ostream& operator<<(std::ostream& oss, const Vector3& vec)
{
	return oss << "Vec3(" << vec.x << "; " << vec.y << "; " << vec.z << ")";
}
