#pragma once

#include <math/box.hpp>
#include <math/capsule.hpp>
#include <math/sphere.hpp>
#include <math/vector3.hpp>
#include <cmath>
#include <ostream>

// Classe gérant un rayon
struct Ray
{
	Vector3 direction;
	Vector3 origin;

	// Constructeur avec valeurs
	Ray(const Vector3& o, const Vector3& d) :
	direction(d),
	origin(o)
	{
	}

	// Constructeur par copie (généré par le compilateur)
	Ray(const Ray&) = default;

	// Opérateur d'assignation par copie (généré par le compilateur)
	Ray& operator=(const Ray&) = default;

	// Renvoie le point situé à une certaine distance sur ce rayon: Vector3 p = ray.At(d)
	Vector3 At(float dist) const
	{
		return origin + dist * direction;
	}

	// Calcule la distance minimale entre un rayon et une position: float f = Ray::Distance(r, v)
	static float Distance(const Ray& r, const Vector3& v)
	{
		// https://answers.unity.com/questions/62644/distance-between-a-ray-and-a-point.html
		return Vector3::CrossProduct(r.direction, v - r.origin).Length();
	}

	// Renvoie vrai en cas d'intersection entre un rayon et une boite (ainsi que la distance d'entrée/sortie)
	// La distance peut être négative si le rayon démarre à l'intérieur de la box
	static bool Intersects(const Ray& r, const Box& b, float& tmin, float& tmax)
	{
		// https://gamedev.stackexchange.com/a/18459
		Vector3 invDir = 1.f / r.direction;
		Vector3 t0s = (b.Mins() - r.origin) * invDir;
		Vector3 t1s = (b.Maxs() - r.origin) * invDir;

		tmin = std::max(std::max(std::min(t0s.x, t1s.x), std::min(t0s.y, t1s.y)), std::min(t0s.z, t1s.z));
		tmax = std::min(std::min(std::max(t0s.x, t1s.x), std::max(t0s.y, t1s.y)), std::max(t0s.z, t1s.z));
		if (tmax < 0.f)
			return false; //< intersection par l'arrière (AABB est derrière le rayon)

		if (tmin > tmax)
			return false; //< pas d'intersection

		return true;
	}

	// Renvoie vrai en cas d'intersection entre une capsule infinie (rayon avec radius) et une sphère: if (Ray::Intersects(r, r_radius, s, &intersection))
	static bool Intersects(const Ray& r, float rayRadius, const Sphere& s, Vector3* intersection = nullptr)
	{
		Vector3 rayToSphere = r.origin - s.center;

		// Cas spécial : le rayon commence à l'intérieur de la sphère
		if (rayToSphere.SquaredLength() < s.radius * s.radius)
		{
			if (intersection)
				*intersection = r.origin;

			return true;
		}

		Vector3 n = NearestPoint(r, s.center);

		// Vecteur perpendiculaire entre le rayon et l'origine de la sphère
		Vector3 p = n - s.center;
		float dRaySphere = p.Length();
		p /= dRaySphere;

		if (dRaySphere >= s.radius + rayRadius)
			return false;

		Vector3 ro = r.origin - p * std::min<float>(dRaySphere, rayRadius);

		return Intersects(Ray(ro, r.direction), s, intersection);
	}

	// Renvoie vrai en cas d'intersection entre un rayon et une capsule: if (Ray::Intersects(r, c, &p1, &p2, &n1, &n2)
	// La fonction sort les deux points d'intersection (entrée/sortie) ainsi que les deux normales de collision
	static bool Intersects(const Ray& ray, const Capsule& capsule, Vector3& p1, Vector3& p2, Vector3& n1, Vector3& n2)
	{
		// http://pastebin.com/2XrrNcxb

		// Substituting equ. (1) - (6) to equ. (I) and solving for t' gives:
		//
		// t' = (t * dot(AB, d) + dot(AB, AO)) / dot(AB, AB); (7) or
		// t' = t * m + n where 
		// m = dot(AB, d) / dot(AB, AB) and 
		// n = dot(AB, AO) / dot(AB, AB)
		//
		Vector3 AB = capsule.end - capsule.start;
		Vector3 AO = ray.origin - capsule.start;

		float AB_dot_d = Vector3::DotProduct(AB, ray.direction);
		float AB_dot_AO = Vector3::DotProduct(AB, AO);
		float AB_dot_AB = Vector3::DotProduct(AB, AB);
	
		float m = AB_dot_d / AB_dot_AB;
		float n = AB_dot_AO / AB_dot_AB;

		// Substituting (7) into (II) and solving for t gives:
		//
		// dot(Q, Q)*t^2 + 2*dot(Q, R)*t + (dot(R, R) - r^2) = 0
		// where
		// Q = d - AB * m
		// R = AO - AB * n
		Vector3 Q = ray.direction - (AB * m);
		Vector3 R = AO - (AB * n);

		float a = Vector3::DotProduct(Q, Q);
		float b = 2.0f * Vector3::DotProduct(Q, R);
		float c = Vector3::DotProduct(R, R) - (capsule.radius * capsule.radius);

		if (a < Epsilon<float>)
		{
			// Special case: AB and ray direction are parallel. If there is an intersection it will be on the end spheres...
			// NOTE: Why is that?
			// Q = d - AB * m =>
			// Q = d - AB * (|AB|*|d|*cos(AB,d) / |AB|^2) => |d| == 1.0
			// Q = d - AB * (|AB|*cos(AB,d)/|AB|^2) =>
			// Q = d - AB * cos(AB, d) / |AB| =>
			// Q = d - unit(AB) * cos(AB, d)
			//
			// |Q| == 0 means Q = (0, 0, 0) or d = unit(AB) * cos(AB,d)
			// both d and unit(AB) are unit vectors, so cos(AB, d) = 1 => AB and d are parallel.
			// 
			Sphere sphereA, sphereB;
			sphereA.center = capsule.start;
			sphereA.radius = capsule.radius;
			sphereB.center = capsule.end;
			sphereB.radius = capsule.radius;

			float atmin, atmax, btmin, btmax;
			if (!Intersects(ray, sphereA, atmin, atmax) ||
			    !Intersects(ray, sphereB, btmin, btmax))
			{
				// No intersection with one of the spheres means no intersection at all...
				return false;
			}

			if (atmin < btmin)
			{
				p1 = ray.origin + (ray.direction * atmin);
				n1 = p1 - capsule.start;
				n1 = Vector3::Normalize(n1);
			}
			else
			{
				p1 = ray.origin + (ray.direction * btmin);
				n1 = p1 - capsule.end;
				n1 = Vector3::Normalize(n1);
			}

			if (atmax > btmax)
			{
				p2 = ray.origin + (ray.direction * atmax);
				n2 = p2 - capsule.start;
				n2 = Vector3::Normalize(n2);
			}
			else
			{
				p2 = ray.origin + (ray.direction * btmax);
				n2 = p2 - capsule.end;
				n2 = Vector3::Normalize(n2);
			}

			return true;
		}

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
		{
			// The ray doesn't hit the infinite cylinder defined by (A, B).
			// No intersection.
			return false;
		}

		float tmin = (-b - std::sqrt(discriminant)) / (2.f * a);
		float tmax = (-b + std::sqrt(discriminant)) / (2.f * a);
		if (tmin > tmax)
			std::swap(tmin, tmax);

		// Now check to see if K1 and K2 are inside the line segment defined by A,B
		float t_k1 = tmin * m + n;
		if (t_k1 < 0.f)
		{
			// On sphere (A, r)...
			Sphere s;
			s.center = capsule.start;
			s.radius = capsule.radius;

			float stmin, stmax;
			if (!Intersects(ray, s, stmin, stmax))
				return false;
			
			p1 = ray.origin + (ray.direction * stmin);
			n1 = p1 - capsule.start;
			n1 = Vector3::Normalize(n1);
		}
		else if (t_k1 > 1.f)
		{
			// On sphere (B, r)...
			Sphere s;
			s.center = capsule.end;
			s.radius = capsule.radius;

			float stmin, stmax;
			if (!Intersects(ray, s, stmin, stmax))
				return false;
			
			p1 = ray.origin + (ray.direction * stmin);
			n1 = p1 - capsule.end;
			n1 = Vector3::Normalize(n1);
		}
		else
		{
			// On the cylinder...
			p1 = ray.origin + (ray.direction * tmin);

			Vector3 k1 = capsule.start + AB * t_k1;
			n1 = p1 - k1;
			n1 = Vector3::Normalize(n1);
		}

		float t_k2 = tmax * m + n;
		if (t_k2 < 0.f)
		{
			// On sphere (A, r)...
			Sphere s;
			s.center = capsule.start;
			s.radius = capsule.radius;

			float stmin, stmax;
			if (!Intersects(ray, s, stmin, stmax))
				return false;
			
			p2 = ray.origin + (ray.direction * stmax);
			n2 = p2 - capsule.start;
			n2 = Vector3::Normalize(n2);
			return true;
		}
		else if(t_k2 > 1.0f)
		{
			// On sphere (B, r)...
			Sphere s;
			s.center = capsule.end;
			s.radius = capsule.radius;

			float stmin, stmax;
			if (!Intersects(ray, s, stmin, stmax))
				return false;

			p2 = ray.origin + (ray.direction * stmax);
			n2 = p2 - capsule.end;
			n2 = Vector3::Normalize(n2);
			return true;
		}
		else
		{
			p2 = ray.origin + (ray.direction * tmax);

			Vector3 k2 = capsule.start + AB * t_k2;
			n2 = p2 - k2;
			n2 = Vector3::Normalize(n2);
			return true;
		}
	}

	// Renvoie vrai en cas d'intersection entre un rayon et une sphère: if (Ray::Intersects(r, s, &intersection))
	static bool Intersects(const Ray& r, const Sphere& s, Vector3* intersection = nullptr)
	{
		// https://gamedev.stackexchange.com/questions/96459/fast-ray-sphere-collision-code
		Vector3 m = r.origin - s.center;
		float b = Vector3::DotProduct(m, r.direction);
		float c = Vector3::DotProduct(m, m) - s.radius * s.radius;

		// Exit if r�s origin outside s (c > 0) and r pointing away from s (b > 0) 
		if (c > 0.0f && b > 0.0f) 
			return false;

		float discr = b * b - c;

		// A negative discriminant corresponds to ray missing sphere 
		if (discr < 0.0f) 
			return false;

		if (intersection)
		{
			// Ray now found to intersect sphere, compute smallest t value of intersection
			float t = -b - std::sqrt(discr);

			// If t is negative, ray started inside sphere so clamp t to zero 
			if (t < 0.0f)
				t = 0.0f;

			*intersection = r.At(t);
		}

		return true;
	}

	// Renvoie vrai en cas d'intersection entre un rayon et une sphère: if (Ray::Intersects(r, s, &intersection))
	// Renvoie les deux distances d'intersection (entrée et sortie)
	static bool Intersects(const Ray& r, const Sphere& s, float& tmin, float& tmax)
	{
		// https://gist.github.com/jdryg/ecde24d34aa0ce2d4d87
		Vector3 CO = r.origin - s.center;

		float a = Vector3::DotProduct(r.direction, r.direction);
		float b = 2.f * Vector3::DotProduct(CO, r.direction);
		float c = Vector3::DotProduct(CO, CO) - (s.radius * s.radius);

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			return false;

		discriminant = std::sqrt(discriminant);

		tmin = (-b - discriminant) / (2.f * a);
		tmax = (-b + discriminant) / (2.f * a);
		if (tmin > tmax)
			std::swap(tmin, tmax);

		if (tmin < 0.f)
			return false; //< behind origin

		return true;
	}

	// Renvoie le point (sur le rayon) le plus proche d'un autre point
	static Vector3 NearestPoint(const Ray& r, const Vector3& point)
	{
		Vector3 lhs = point - r.origin;
		float d = std::max<float>(0.f, Vector3::DotProduct(lhs, r.direction));

		return r.At(d);
	}

	// Opérateurs arithmétiques

	// Comparaison
	bool operator==(const Ray& ray) const
	{
		return origin == ray.origin && direction == ray.direction;
	}

	bool operator!=(const Ray& ray) const
	{
		return !operator==(ray);
	}
};

inline std::ostream& operator<<(std::ostream& oss, const Ray& ray)
{
	return oss << "Ray(" << ray.origin << "; " << ray.direction << ")";
}
