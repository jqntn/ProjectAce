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
			// On récupère le foncteur (typiquement une lambda) en paramètre et on se l'approprie
		}

		~ExitCapsule()
		{
			// On déclenche l'appel de fonction à la destruction
			if (m_functor)
				(*m_functor)();
		}

		// Permet d'éviter l'appel de la fonction
		void Reset()
		{
			m_functor.reset();
		}

	private:
		std::optional<F> m_functor;
};

template<typename F>
ExitCapsule(F) -> ExitCapsule<F>;
