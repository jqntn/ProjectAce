#pragma once

#include <math/angles.hpp>
#include <cmath>
#include <ostream>
#include <string>

// Classe gérant des vecteurs à deux dimensions
struct Vector2 {
	float x;
	float y;

	// Constructeur par défaut (initialisation à zéro)
	Vector2() :
	x(0.f),
	y(0.f)
	{
	}

	// Constructeur avec valeurs
	Vector2(float X, float Y) :
	x(X),
	y(Y)
	{
	}

	// Constructeur par copie (généré par le compilateur)
	Vector2(const Vector2&) = default;

	// Opérateur d'assignation par copie (généré par le compilateur)
	Vector2& operator=(const Vector2&) = default;

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

	// Calcule l'angle (en degrés) entre deux directions (vecteurs normalisés) en degrés
	static float AngleBetween(const Vector2& v1, const Vector2& v2)
	{
		return RadiansToDegrees(std::atan2(v2.y, v2.x) - std::atan2(v1.y, v1.x));
	}

	// Produit scalaire: float r = Vec2::DotProduct(v1, v2)
	static float DotProduct(const Vector2& v1, const Vector2& v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	// Normalisation d'un vecteur: vec2 norm = Vec2::Normalize(vec)
	static Vector2 Normalize(const Vector2& vec)
	{
		return vec / vec.Length();
	}

	// Distance entre deux vecteurs
	static float Distance(const Vector2& vec1, const Vector2& vec2) {
		return (vec1 - vec2).Length();
	}

	// Opérateurs arithmétiques

	// Moins unaire
	Vector2 operator-() const
	{
		return Vector2(-x, -y);
	}

	// Addition
	Vector2& operator+=(const Vector2& vec)
	{
		x += vec.x;
		y += vec.y;

		return *this;
	}

	Vector2 operator+(const Vector2& vec) const
	{
		Vector2 result(*this);
		result += vec;

		return result;
	}

	// Soustraction
	Vector2& operator-=(const Vector2& vec)
	{
		x -= vec.x;
		y -= vec.y;

		return *this;
	}

	Vector2 operator-(const Vector2& vec) const
	{
		Vector2 result(*this);
		result -= vec;

		return result;
	}

	// Multiplication par un nombre
	Vector2& operator*=(float n)
	{
		x *= n;
		y *= n;

		return *this;
	}

	Vector2 operator*(float n) const
	{
		Vector2 result(*this);
		result *= n;

		return result;
	}

	// Multiplication par un vecteur (multiplication des composants)
	Vector2& operator*=(const Vector2& vec)
	{
		x *= vec.x;
		y *= vec.y;

		return *this;
	}

	Vector2 operator*(const Vector2& vec) const
	{
		Vector2 result(*this);
		result *= vec;

		return result;
	}

	// Division par un nombre
	Vector2& operator/=(float n)
	{
		x /= n;
		y /= n;

		return *this;
	}

	Vector2 operator/(float n) const
	{
		Vector2 result(*this);
		result /= n;

		return result;
	}

	// Comparaison
	bool operator==(const Vector2& vec) const
	{
		return x == vec.x && y == vec.y;
	}

	bool operator!=(const Vector2& vec) const
	{
		return !operator==(vec);
	}
};

// float * vec2
inline Vector2 operator*(float n, const Vector2& vec)
{
	return Vector2(n * vec.x, n * vec.y);
}

// float / vec2
inline Vector2 operator/(float n, const Vector2& vec)
{
	return Vector2(n / vec.x, n / vec.y);
}

inline std::ostream& operator<<(std::ostream& oss, const Vector2& vec)
{
	return oss << "Vec2(" << vec.x << "; " << vec.y << ")";
}
