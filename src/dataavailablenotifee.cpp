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

#include "include/dataavailablenotifee.h"
#include "include/networkmanager.h"

DataAvailableNotifee::DataAvailableNotifee()
	: m_replies()
	, m_replyHandlers()
{
}

DataAvailableNotifee::~DataAvailableNotifee()
{
	// Telling all reply handlers that we have been deleted
	for (auto h: m_replyHandlers.values()) {
		h->clearNotifee();
	}
}

void DataAvailableNotifee::networkError(int /*id*/, const QString& /*description*/)
{
}

void DataAvailableNotifee::requestCompleted(int /*id*/)
{
}

void DataAvailableNotifee::interruptRequest(int id)
{
	if (m_replyHandlers.contains(id)) {
		m_replyHandlers[id]->stopRequest();
	}
}

void DataAvailableNotifee::setReply(int id, QNetworkReply* reply)
{
	if (reply == nullptr) {
		m_replies.remove(id);
	} else {
		m_replies[id] = reply;
	}
}

void DataAvailableNotifee::setReplyHandler(int id, __internal::NetworkReplyHandler* replyHandler)
{
	if (replyHandler == nullptr) {
		m_replyHandlers.remove(id);
	} else {
		m_replyHandlers[id] = replyHandler;
	}
}

DataArrivedNotifee::DataArrivedNotifee()
	: DataAvailableNotifee()
{
}

DataArrivedNotifee::~DataArrivedNotifee()
{
	// Nothing to do here
}

void DataArrivedNotifee::dataAvailable(int id)
{
	// Reading data and calling the callback
	QByteArray data = reply(id)->readAll();

	dataArrived(id, data);
}

AllDataArrivedNotifee::AllDataArrivedNotifee()
	: DataAvailableNotifee()
{
}

AllDataArrivedNotifee::~AllDataArrivedNotifee()
{
	// Nothing to do here
}

void AllDataArrivedNotifee::dataAvailable(int /*id*/)
{
	// Here we don't do anything, we read all data in allDataAvailable
}

void AllDataArrivedNotifee::allDataAvailable(int id)
{
	// Reading all data and calling the callback
	QByteArray data = reply(id)->readAll();

	allDataArrived(id, data);
}
