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
#include "include/utilities.h"

MiscNative* MiscNative::instance()
{
	static MiscNative* miscNative = new MiscNative();
	return miscNative;
}

MiscNative::MiscNative(QObject* parent)
	: QObject(parent)
	, m_status(Idle)
	, m_impl(nullptr)
{
}

MiscNative::~MiscNative()
{
	// Nothing to do here
}

void MiscNative::customEvent(QEvent* event)
{
	CommandEvent* c = dynamic_cast<CommandEvent*>(event);
	if (c != nullptr) {
		// Accepting the event and executing the command
		c->accept();
		c->executeCommand();
	}
}

void MiscNative::setStatus(Status newStatus)
{
	m_status = newStatus;

	emit statusChanged();
}

void MiscNative::actionCompleted()
{
	setStatus(ActionFinished);
	emit operationDone();
}

void MiscNative::actionError(QString reason)
{
	setStatus(ActionError);
	emit error(reason);
}

QObject* miscNativeSingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)
	return MiscNative::instance();
}
