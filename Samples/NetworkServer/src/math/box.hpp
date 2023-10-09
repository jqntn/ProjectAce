#pragma once

#include <math/vector3.hpp>
#include <cmath>
#include <ostream>

// Classe gérant des boites alignées (sans rotation), avec un centre et des dimensions
struct Box
{
	Vector3 center;
	Vector3 dims;

	// Constructeur par défaut (initialisation à zéro)
	Box() = default;

	// Constructeur avec valeurs
	Box(const Vector3& c, const Vector3& d) :
	center(c),
	dims(d)
	{
	}

	// Constructeur par copie (généré par le compilateur)
	Box(const Box&) = default;

	// Opérateur d'assignation par copie (généré par le compilateur)
	Box& operator=(const Box&) = default;

	// Renvoie le point minimal (souvent considéré comme origine dans certains calculs/moteurs)
	Vector3 Mins() const
	{
		return center - dims * 0.5f;
	}

	// Renvoie le point maximal
	Vector3 Maxs() const
	{
		return center + dims * 0.5f;
	}

	// Construit une boite plus classique depuis un point d'origine et des dimensions
	static Box FromOriginDims(const Vector3& origin, const Vector3& dims)
	{
		return Box(origin + dims * 0.5f, dims);
	}

	// Renvoie vrai si les boites sont en intersection: if (Box::Intersects(b1, b2))
	// Calcule de façon optionnelle la boite de collision
	static bool Intersects(const Box& b1, const Box& b2, Box* intersection = nullptr)
	{
		Vector3 b1Mins = b1.Mins();
		Vector3 b1Maxs = b1.Maxs();
		Vector3 b2Mins = b2.Mins();
		Vector3 b2Maxs = b2.Maxs();

		float left = std::max(b1Mins.x, b2Mins.x);
		float right = std::min(b1Maxs.x, b2Maxs.x);
		if (left >= right)
			return false;

		float top = std::max(b1Mins.y, b2Mins.y);
		float bottom = std::min(b1Maxs.y, b2Maxs.y);
		if (top >= bottom)
			return false;

		float up = std::max(b1Mins.z, b2Mins.z);
		float down = std::min(b1Maxs.z, b2Maxs.z);
		if (up >= down)
			return false;

		if (intersection)
			*intersection = FromOriginDims(Vector3(left, top, up), Vector3(right - left, bottom - top, down - up));

		return true;
	}

	// Opérateurs arithmétiques

	// Comparaison
	bool operator==(const Box& box) const
	{
		return center == box.center && dims == box.dims;
	}

	bool operator!=(const Box& box) const
	{
		return !operator==(box);
	}
};

inline std::ostream& operator<<(std::ostream& oss, const Box& box)
{
	return oss << "Box(" << box.center << "; " << box.dims << ")";
}
