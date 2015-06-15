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

#ifndef __NETWORK_MANAGER_H__
#define __NETWORK_MANAGER_H__

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QSet>
#include "include/dataavailablenotifee.h"
#include "include/utilities.h"

class NetworkManager;

namespace __internal {
	/**
	 * \brief An internal class which keeps information about a request and
	 *        handles signals from the reply
	 *
	 * This class is used internally by NetworkManager to keep the
	 * information about a request and to receive signals of the reply,
	 * communicating the object that started the request when data has
	 * arrived
	 */
	class NetworkReplyHandler : public QObject
	{
		Q_OBJECT

	public:
		/**
		 * \brief Constructor
		 *
		 * \param manager the network manager
		 * \param request the original request
		 * \param id the id of the request
		 * \param notifee the object to be notified of replies
		 * \param parent the parent object
		 */
		NetworkReplyHandler(NetworkManager* manager, const QNetworkRequest& request, int id, DataAvailableNotifee* notifee, QObject* parent = nullptr);

		/**
		 * \brief Destructor
		 */
		~NetworkReplyHandler();

		/**
		 * \brief Sets the notifee to nullptr
		 *
		 * Use this when we should no longer interact with the notifee
		 */
		void clearNotifee();

		/**
		 * \brief Returns the notifee
		 *
		 * \return the notifee
		 */
		DataAvailableNotifee* notifee()
		{
			return m_notifee;
		}

		/**
		 * \brief Returns the notifee (const version)
		 *
		 * \return the notifee (const version)
		 */
		const DataAvailableNotifee* notifee() const
		{
			return m_notifee;
		}

		/**
		 * \brief Stops the request by deleting it
		 *
		 * This also causes the notifee not to be updated anymore and
		 * this object to be scheduled for removal
		 */
		void stopRequest();

		/**
		 * \brief Returns the request id
		 *
		 * \return the request id
		 */
		int id() const
		{
			return m_id;
		}

	private:
		/**
		 * \brief The slot called with updates on the download progress
		 *
		 * \param bytesReceived the number of received bytes
		 * \param bytesTotal the total number of bytes to receive
		 */
		void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

		/**
		 * \brief The slot called with updates on the upload progress
		 *
		 * \param bytesSent the number of sent bytes
		 * \param bytesTotal the total number of bytes to send
		 */
		void uploadProgress(qint64 bytesSent, qint64 bytesTotal);

		/**
		 * \brief The slot called whenever there is data to read
		 */
		void readyRead();

		/**
		 * \brief The slot called when the reply process has finished
		 */
		void finished();

		/**
		 * \brief The slot called when there is a network error
		 *
		 * \param code the error code
		 */
		void error(QNetworkReply::NetworkError code);

		/**
		 * \brief The slot called when there is an error setting up an ssl/tsl
		 *        session
		 *
		 * \param errors the list of errors that were encoutered
		 */
		void sslErrors(const QList<QSslError>& errors);

		/**
		 * \brief The network manager
		 */
		NetworkManager* const m_manager;

		/**
		 * \brief The original request
		 */
		const QNetworkRequest m_request;

		/**
		 * \brief The id of the request
		 */
		const int m_id;

		/**
		 * \brief The object to be notified of replies
		 */
		DataAvailableNotifee* m_notifee;
	};
}

/**
 * \brief The class managing network accesses and download of files
 *
 * This class handles download of files from the net and all other network
 * related stuffs. Only one instance of this class should exist (even though
 * having multiple instances is possible). Use NM to access the singleton
 */
class NetworkManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * \brief Constructor
	 *
	 * \param parent the parent QObject
	 */
	NetworkManager(QObject* parent = nullptr);

	/**
	 * \brief Destructor
	 */
	virtual ~NetworkManager();

	/**
	 * \brief Downloads a file from the network
	 *
	 * \param url the url of the file to downnload
	 * \param notifee the object that is notified when the file arrives
	 * \param id the id of the request. This will be passed to callbacks
	 * \return false if a request with the same id already exists (in this
	 *         case the request is discarded), true otherwise
	 */
	bool getFile(QUrl url, DataAvailableNotifee* notifee, int id);

signals:
	/**
	 * \brief The sigal emitted when there are network errors
	 *
	 * If the error was generated by a request, also the networkError
	 * callback of the DataAvailableNotifee is called
	 * \param description the description of the error
	 */
	void networkError(QString description);

	/**
	 * \brief The signal emitted when a request is enqueued
	 *
	 * This signal is emitted when a request is added and there were no
	 * requests running
	 */
	void networkRequestsStarted();

	/**
	 * \brief The signal emitted when all runnning requests have finished
	 */
	void networkRequestsEnded();

private:
	/**
	 * \brief Called by NetworkReplyHandler when a reply returns a redirect
	 *
	 * This will enqueue a new request and delete the reply handler that
	 * received the redirect
	 * \param replyHandler the object that was dealing with the request
	 *                     that received a redirec
	 * \param newUrl the url to which the request was redirected
	 */
	void replyRedirected(__internal::NetworkReplyHandler* replyHandler, QUrl newUrl);

	/**
	 * \brief Called by NetworkReplyHandler when a network error happends
	 *
	 * \param replyHandler the object that was dealing with the request
	 *                     that generated the error
	 * \param code the error code
	 */
	void replyError(__internal::NetworkReplyHandler* replyHandler, QNetworkReply::NetworkError code);

	/**
	 * \brief Called by NetworkReplyHandler when there is an error setting
	 *        up an ssl/tsl session
	 *
	 * \param replyHandler the object that was dealing with the request
	 *                     that generated the error
	 * \param errors the list of errors that were encoutered
	 */
	void replySslErrors(__internal::NetworkReplyHandler* replyHandler, const QList<QSslError>& errors);

	/**
	 * \brief Called by a NetworkReplyHandler object when it has finished
	 *        its work
	 *
	 * This removes the handler from the list and schedules it for removal
	 * \param replyHandler the handler that has finished its job
	 */
	void replyHandlerFinished(__internal::NetworkReplyHandler* replyHandler);

	/**
	 * \brief The network access manager
	 */
	QNetworkAccessManager* const m_manager;

	/**
	 * \brief The set of reply handlers for active requests
	 */
	QSet<__internal::NetworkReplyHandler*> m_replyHandlers;

	/**
	 * \brief NetworkReplyHandler is friend to call private functions
	 */
	friend class __internal::NetworkReplyHandler;
};

/**
 * \brief The singleton of the NetworkManager
 */
using NM = Singleton<NetworkManager>;

#endif
