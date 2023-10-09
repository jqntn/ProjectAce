#pragma once

#include <optional>

// Simple classe pour permettre d'appeler une lambda dans un destructeur (simili-defer du Go)
template<typename F>
class ExitCapsule
{
	public:
		ExitCapsule(F functor) :
		m_functor(std::move(functor))
		{
			// On r�cup�re le foncteur (typiquement une lambda) en param�tre et on se l'approprie
		}

		~ExitCapsule()
		{
			// On d�clenche l'appel de fonction � la destruction
			if (m_functor)
				(*m_functor)();
		}

		// Permet d'�viter l'appel de la fonction
		void Reset()
		{
			m_functor.reset();
		}

	private:
		std::optional<F> m_functor;
};

template<typename F>
ExitCapsule(F) -> ExitCapsule<F>;
