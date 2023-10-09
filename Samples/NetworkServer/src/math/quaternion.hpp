#pragma once

#include <math/angles.hpp>
#include <math/vector3.hpp>
#include <cmath>
#include <ostream>

// Classe gérant des quaternions (utilisés pour représenter une rotation)
struct Quaternion
{
	float x;
	float y;
	float z;
	float w;

	// Constructeur par défaut (rotation identitaire : pas de rotation)
	Quaternion() :
	x(0.f),
	y(0.f),
	z(0.f),
	w(1.f)
	{
	}

	// Constructeur prenant des valeurs
	Quaternion(float X, float Y, float Z, float W) :
	x(X),
	y(Y),
	z(Z),
	w(W)
	{
	}

	// Constructeur par copie (généré par le compilateur)
	Quaternion(const Quaternion&) = default;

	// Opérateur d'assignation par copie (généré par le compilateur)
	Quaternion& operator=(const Quaternion&) = default;

	// Calcul de la longueur: float l = quat.Length()
	float Length() const
	{
		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	// Applique la rotation du quaternion à un vecteur: Vec3 rotatedVec = quat.Rotate(vec);
	Vector3 Rotate(const Vector3& vec) const
	{
		// Calcul plus rapide que la version classique (q x v x q*)
		// https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion

		// Extract the vector part of the quaternion
		Vector3 u(x, y, z);

		// Extract the scalar part of the quaternion
		float s = w;

		// Do the math
		return 2.0f * Vector3::DotProduct(u, vec) * u
		       + (s * s - Vector3::DotProduct(u, u)) * vec
		       + 2.0f * s * Vector3::CrossProduct(u, vec);
	}

	// Calcule la rotation en degrés sur un axe sous la forme d'un quaternion
	static Quaternion AngleAxis(float degrees, Vector3 axis)
	{
		float radians = degrees * Deg2Rad;
		radians *= 0.5f;

		float sina, cosa;
		sina = std::sin(radians);
		cosa = std::cos(radians);

		axis *= sina;

		return Quaternion(axis.x, axis.y, axis.z, cosa);
	}

	// Calcule la rotation inverse d'un quaternion normalisé: Quaternion invQuat = Quaternion::Conjugate(quat);
	static Quaternion Conjugate(const Quaternion& quat)
	{
		return Quaternion(quat.w, -quat.x, -quat.y, -quat.z);
	}

	// Converti des angles d'euler en quaternion
	static Quaternion FromEulerAngles(float yaw, float pitch, float roll)
	{
		yaw *= Deg2Rad;
		pitch *= Deg2Rad;
		roll *= Deg2Rad;

		double yawOver2 = yaw * 0.5f;
		float cosYawOver2 = float(std::cos(yawOver2));
		float sinYawOver2 = float(std::sin(yawOver2));

		double pitchOver2 = pitch * 0.5f;
		float cosPitchOver2 = float(std::cos(pitchOver2));
		float sinPitchOver2 = float(std::sin(pitchOver2));

		double rollOver2 = roll * 0.5f;
		float cosRollOver2 = float(std::cos(rollOver2));
		float sinRollOver2 = float(std::sin(rollOver2));

		Quaternion result;
		result.w = cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2;
		result.x = sinYawOver2 * cosPitchOver2 * cosRollOver2 + cosYawOver2 * sinPitchOver2 * sinRollOver2;
		result.y = cosYawOver2 * sinPitchOver2 * cosRollOver2 - sinYawOver2 * cosPitchOver2 * sinRollOver2;
		result.z = cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2;

		return result;
	}

	// Calcule la rotation permettant de passer d'une direction (vecteur normalisé) é une autre: Quaternion quat = Quaternion::RotationBetween(from, to)
	static Quaternion RotationBetween(const Vector3& from, const Vector3& to)
	{
		// Based on: http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors
		float norm = std::sqrt(from.SquaredLength() * to.SquaredLength());

		Vector3 crossProduct = Vector3::CrossProduct(from, to);

		Quaternion result;
		result.w = norm + Vector3::DotProduct(from, to);
		result.x = crossProduct.x;
		result.y = crossProduct.y;
		result.z = crossProduct.z;

		return Normalize(result);
	}

	// Calcule un quaternion normalisé: Quaternion normalizedQuat = Quaternion::Normalize(quat)
	static Quaternion Normalize(const Quaternion& quat)
	{
		float length = quat.Length();
		return Quaternion(quat.x / length, quat.y / length, quat.z / length, quat.w / length);
	}

	// Combine la rotation d'un quaternion avec un autre (l'ordre est important)
	Quaternion operator*(const Quaternion& quat)
	{
		Quaternion result;
		result.w = w * quat.w - x * quat.x - y * quat.y - z * quat.z;
		result.x = w * quat.x + x * quat.w + y * quat.z - z * quat.y;
		result.y = w * quat.y + y * quat.w + z * quat.x - x * quat.z;
		result.z = w * quat.z + z * quat.w + x * quat.y - y * quat.x;

		return result;
	}
};

inline std::ostream& operator<<(std::ostream& oss, const Quaternion& quat)
{
	return oss << "Quaternion(" << quat.x << "; " << quat.y << "; " << quat.z << " | " << quat.w << ")";
}
