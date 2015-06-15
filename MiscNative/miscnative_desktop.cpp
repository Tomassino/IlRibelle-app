/******************************************************************************
 * IlRibelle.com                                                              *
 * Copyright (C) 2014                                                         *
 * Tomassino Ferrauto <t_ferrauto@yahoo.it>                                   *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

#include "MiscNative/miscnative.h"
#include <random>

namespace MiscNative_internal
{
	// This helper class is needed to simulate actions on desktop
	class MiscNativeImpl : public QObject
	{
	public:
		MiscNativeImpl(MiscNative* miscNative)
			: QObject(miscNative)
			, m_miscNative(miscNative)
			, m_timerId(-1)
			, m_randomGenerator((std::random_device())())
		{
		}

		void waitAndConcludeAction()
		{
			// Starting a timer with a random delay
			std::uniform_int_distribution<> dis(1000, 3000);
			m_timerId = startTimer(dis(m_randomGenerator));
		}

	protected:
		void timerEvent(QTimerEvent*)
		{
			// Stopping timer
			killTimer(m_timerId);

			// Ending with error with a .5 probability
			std::uniform_real_distribution<> dis(0.0, 1.0);
			if (dis(m_randomGenerator) < 0.5) {
				m_miscNative->actionCompleted();
			} else {
				m_miscNative->actionError("Random error");
			}
		}

	private:
		MiscNative* m_miscNative;
		int m_timerId;
		std::mt19937 m_randomGenerator;
	};
}

void MiscNative::share(QString, QString, QString)
{
	// If we are already doing something, doing nothing
	if (m_status == Acting) {
		return;
	}

	// Creating the implementation if it doesn't exists yet
	if (m_impl == nullptr) {
		m_impl = new MiscNative_internal::MiscNativeImpl(this);
	}

	// Setting the status to Acting
	setStatus(Acting);

	// Simulating performing the action
	m_impl->waitAndConcludeAction();
}
