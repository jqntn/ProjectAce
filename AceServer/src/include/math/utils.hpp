#pragma once

#include <cmath>

// Ensemble de fonctions utilitaires

/** Returns higher value in a generic way */
template< class T >
static constexpr T Max(const T A, const T B)
{
	return (B < A) ? A : B;
}

/** Returns lower value in a generic way */
template< class T >
static constexpr T Min(const T A, const T B)
{
	return (A < B) ? A : B;
}



/** Clamps X to be between Min and Max, inclusive */
template< class T >
constexpr T Clamp(const T X, const T MinValue, const T MaxValue)
{
	return Max(Min(X, MaxValue), MinValue);
}

// Le calcul des flottants n'étant pas exact, on utilise un epsilon plutôt que
// zéro pour vérifier certains calculs
template<typename T>
constexpr T Epsilon = T(0.0001f);

/** Interpolate float from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
template<typename T1, typename T2 = T1, typename T3 = T2, typename T4 = T3>
auto SmoothLerp(T1  Current, T2 Target, T3 DeltaTime, T4 InterpSpeed)
{
	using RetType = decltype(T1()* T2()* T3()* T4());

	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return static_cast<RetType>(Target);
	}

	// Distance to reach
	const RetType Dist = Target - Current;

	// If distance is too small, just set the desired location
	if ((Dist * Dist) < (1.e-8f))
	{
		return static_cast<RetType>(Target);
	}

	// Delta Move, Clamp so we do not over shoot.
	const RetType DeltaMove = Dist * Clamp<RetType>(DeltaTime * InterpSpeed, 0.f, 1.f);

	return Current + DeltaMove;
}

