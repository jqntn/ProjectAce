#pragma once

#include <cmath>

// Ensemble de fonctions utilitaires

// Borne une valeur entre un minimum et un maximum
template<typename T> T Clamp(T value, T min, T max)
{
	if (value > max)
		return max;
	else if (value < min)
		return min;
	else
		return value;
}

// Le calcul des flottants n'étant pas exact, on utilise un epsilon plutôt que zéro pour vérifier certains calculs
template<typename T> constexpr T Epsilon = T(0.0001f);