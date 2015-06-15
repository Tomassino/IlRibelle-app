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

#include "include/utilities.h"
#include <QStringList>

QDateTime dateTimeFromRssString(const QString& str)
{
	// We try conversion using the Qt::RFC2822Date format, if it fails we try to change the
	// year to 4 digits
	QDateTime dt = QDateTime::fromString(str, Qt::RFC2822Date);

	if (!dt.isValid()) {
		// Trying to add two digits to the year (we add "20") and then converting again
		QString newStr;

		const int commaPosition = str.indexOf(QChar(','));
		const QString beforeComma = (commaPosition == -1) ? QString() : str.left(commaPosition + 1);
		const QString afterComma = (commaPosition == -1) ? str : str.mid(commaPosition + 2);

		// There is no day before the comma, year should be the third field
		QStringList fields = afterComma.split(" ", QString::SkipEmptyParts);
		if (fields.size() < 3) {
			fields[2] = "20" + fields[2];
		} else {
			// We don't know how to fix the date
			return QDateTime();
		}

		// Generating the new string
		newStr = beforeComma + " " + fields.join(" ");

		// Trying ocnversion one more time
		dt = QDateTime::fromString(newStr, Qt::RFC2822Date);
	}

	return dt;
}

const QEvent::Type CommandEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

CommandEvent::CommandEvent(const std::function<void()>& command)
	: QEvent(eventType)
	, m_command(command)
{
}

void CommandEvent::executeCommand()
{
	m_command();
}

CommandEventReceiverClass::CommandEventReceiverClass()
	: QObject()
{
}

CommandEventReceiverClass::~CommandEventReceiverClass()
{
}

bool CommandEventReceiverClass::event(QEvent* e)
{
	CommandEvent* ce = dynamic_cast<CommandEvent*>(e);
	if (ce != nullptr) {
		ce->accept();

		// Executing command
		ce->executeCommand();

		return true;
	} else {
		return QObject::event(e);
	}
}
