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

#include "include/networkmanager.h"
#include <QCoreApplication>
#include <QDebug>

namespace __internal {
	NetworkReplyHandler::NetworkReplyHandler(NetworkManager* manager, const QNetworkRequest& request, int id, DataAvailableNotifee* notifee, QObject* parent)
		: QObject(parent)
		, m_manager(manager)
		, m_request(request)
		, m_id(id)
		, m_notifee(notifee)
	{
//qDebug() << ((unsigned long) this) << m_id << "NetworkReplyHandler" << __func__;

		QNetworkReply* const reply = m_notifee->reply(m_id);

		// Connecting signals from reply to our slots
		connect(reply, &QNetworkReply::downloadProgress, this, &NetworkReplyHandler::downloadProgress);
		connect(reply, &QNetworkReply::uploadProgress, this, &NetworkReplyHandler::uploadProgress);
		connect(reply, &QNetworkReply::readyRead, this, &NetworkReplyHandler::readyRead);
		connect(reply, &QNetworkReply::finished, this, &NetworkReplyHandler::finished);
		connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &NetworkReplyHandler::error);
		connect(reply, &QNetworkReply::sslErrors, this, &NetworkReplyHandler::sslErrors);
	}

	NetworkReplyHandler::~NetworkReplyHandler()
	{
//qDebug() << ((unsigned long) this) << m_id << "NetworkReplyHandler" << __func__;
	}

	void NetworkReplyHandler::clearNotifee()
	{
//qDebug() << ((unsigned long) this) << m_id << "NetworkReplyHandler" << __func__;

		m_notifee = nullptr;
	}

	void NetworkReplyHandler::stopRequest()
	{
//qDebug() << ((unsigned long) this) << m_id << "NetworkReplyHandler" << __func__;
		// Telling the network manager that we have finished or job (in any case). This will
		// also delete us and the reply
		m_manager->replyHandlerFinished(this);
	}

	void NetworkReplyHandler::downloadProgress(qint64 /*bytesReceived*/, qint64 /*bytesTotal*/)
	{
		// Unused for the moment
	}

	void NetworkReplyHandler::uploadProgress(qint64 /*bytesSent*/, qint64 /*bytesTotal*/)
	{
		// Unused for the moment
	}

	void NetworkReplyHandler::readyRead()
	{
//qDebug() << ((unsigned long) this) << m_id << "NetworkReplyHandler" << __func__;

		if (!m_notifee) {
			return;
		}

		QNetworkReply* const reply = m_notifee->reply(m_id);

qDebug() << "NetworkReplyHandler" << m_id << "Request for " << reply->url() << " retcode" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		// Checking that there was no error
		if (reply->error() == QNetworkReply::NoError) {
			QVariant vRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
			if (vRedirectUrl.isValid()) {
				// Telling the manager we were redirected
//				qDebug() << "Request redirected to url" << vRedirectUrl;
				m_manager->replyRedirected(this, vRedirectUrl.toUrl());
			} else {
				// Calling notifee callback
				m_notifee->dataAvailable(m_id);
			}
		}
	}

	void NetworkReplyHandler::finished()
	{
//qDebug() << ((unsigned long) this) << m_id << "NetworkReplyHandler" << __func__;

		bool skipReplyHandlerFinished = false;

		if (m_notifee) {
			QNetworkReply* const reply = m_notifee->reply(m_id);

//qDebug() << "NetworkReplyHandler - finished -" << m_id << "Request for " << reply->url() << " retcode" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

			// Checking that there was no error
			if (reply->error() == QNetworkReply::NoError) {
				QVariant vRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
				if (vRedirectUrl.isValid()) {
					// Telling the manager we were redirected
//					qDebug() << "Request redirected to url" << vRedirectUrl;
					m_manager->replyRedirected(this, vRedirectUrl.toUrl());

					// If we have been redirected, we skip the call to m_manager->replyHandlerFinished
					// at the end of this function because the replyRedirected call already takes
					// care of scheduling us for removal
					skipReplyHandlerFinished = true;
				} else {
					// Calling notifee callback
					m_notifee->allDataAvailable(m_id);
				}
			}
		}

		// Finally telling the network manager that we have finished or job
		if (!skipReplyHandlerFinished) {
			m_manager->replyHandlerFinished(this);
		}
	}

	void NetworkReplyHandler::error(QNetworkReply::NetworkError code)
	{
//qDebug() << ((unsigned long) this) << m_id << "NetworkReplyHandler" << __func__;

		if (!m_notifee) {
			return;
		}

	#warning FORSE IN ALCUNI CASI NON SI DEVE SOLLEVARE ERRORE (AD ESEMPIO QNetworkReply::TemporaryNetworkFailureError, SE C È UNA REDIRECTION, CONTROLLARE ALTRI)
		// Telling the newtork manager that an error occurred
		m_manager->replyError(this, code);

		// Also telling the notifee
		m_notifee->networkError(m_id, tr("Error handling the request"));

		// There will be a call to NetworkReplyHandler::finished() that will chedule us for removal
	}

	void NetworkReplyHandler::sslErrors(const QList<QSslError>& errors)
	{
//qDebug() << ((unsigned long) this) << m_id << "NetworkReplyHandler" << __func__;

		if (!m_notifee) {
			return;
		}

		// Telling the newtork manager that an error occurred
		m_manager->replySslErrors(this, errors);

		// Also telling the notifee
		m_notifee->networkError(m_id, tr("Error in ssl session initialization when handling the request"));

		// There will be a call to NetworkReplyHandler::finished() that will chedule us for removal
	}
}

NetworkManager::NetworkManager(QObject* parent)
	: QObject(parent)
	, m_manager(new QNetworkAccessManager(this))
	, m_replyHandlers()
{
}

NetworkManager::~NetworkManager()
{
	// Deleting all reply handlers
	for (auto it = m_replyHandlers.begin(); it != m_replyHandlers.end(); ++it) {
		// Removing the reply handler from the notifee
		if ((*it)->notifee() != nullptr) {
			(*it)->notifee()->setReply((*it)->id(), nullptr);
			(*it)->notifee()->setReplyHandler((*it)->id(), nullptr);
		}

		delete *it;
	}

	// m_manger will be deleted by QObject destructor because we are its parent
}

bool NetworkManager::getFile(QUrl url, DataAvailableNotifee* notifee, int id)
{
//qDebug() << ((unsigned long) this) << id << "NetworkManager" << __func__ << url;

	// Checking that a request with the same id doesn't exist
	if (notifee->reply(id) != nullptr) {
		return false;
	}

	// Creating a new request
	const QNetworkRequest request(url);
	QNetworkReply* reply = m_manager->get(request);

	// If there were no requests running, emitting the networkRequestsStarted() signal
	if (m_replyHandlers.isEmpty()) {
		emit networkRequestsStarted();
	}

	// Setting the reply in the notifee
	notifee->setReply(id, reply);

	// Creating the handler for the request and adding it to the set
	__internal::NetworkReplyHandler* handler = new __internal::NetworkReplyHandler(this, request, id, notifee);
	m_replyHandlers.insert(handler);

	// Telling the notifee which is the reply handler object
	notifee->setReplyHandler(id, handler);

	return true;
}

void NetworkManager::replyRedirected(__internal::NetworkReplyHandler* replyHandler, QUrl newUrl)
{
//qDebug() << ((unsigned long) this) << replyHandler->id() << "NetworkManager" << __func__;

	// Here we start a new request with the same id of the one that was redirected. Here we
	// never emit the networkRequestsStarted signal (the old request was running)

	const auto id = replyHandler->id();
	DataAvailableNotifee* const notifee = replyHandler->notifee();
	QNetworkReply* const oldReply = replyHandler->notifee()->reply(id);

	// Creating a new request
	const QNetworkRequest request(newUrl);
	QNetworkReply* const newReply = m_manager->get(request);

	notifee->setReply(id, newReply);

	// Creating the handler for the request and adding it to the set
	__internal::NetworkReplyHandler* newHandler = new __internal::NetworkReplyHandler(this, request, id, notifee);
	m_replyHandlers.insert(newHandler);

	// Telling the notifee which is the reply handler object and clearing it in
	// the old reply handler
	notifee->setReplyHandler(id, newHandler);
	replyHandler->clearNotifee();

	// Removing the handler from the set and scheduling for deletion, both the old reply
	// and the old reply handler
	m_replyHandlers.remove(replyHandler);
	auto command = [oldReply, replyHandler](){ delete replyHandler; delete oldReply; };
	QCoreApplication::postEvent(&(CommandEventReceiver::instance()), new CommandEvent(std::move(command)));
}

void NetworkManager::replyError(__internal::NetworkReplyHandler* replyHandler, QNetworkReply::NetworkError code)
{
//qDebug() << ((unsigned long) this) << replyHandler->id() << "NetworkManager" << __func__;

	QString errorMessage;

	switch (code) {
		case QNetworkReply::NoError:
			errorMessage = tr("No error but error handler called (probably a bug)");
			break;
		case QNetworkReply::ConnectionRefusedError:
			errorMessage = tr("The remote server refused the connection");
			break;
		case QNetworkReply::RemoteHostClosedError:
			errorMessage = tr("The remote server closed the connection prematurely");
			break;
		case QNetworkReply::HostNotFoundError:
			errorMessage = tr("The remote host name was not found");
			break;
		case QNetworkReply::TimeoutError:
			errorMessage = tr("The connection to the remote server timed out");
			break;
		case QNetworkReply::OperationCanceledError:
			errorMessage = tr("The operation was canceled");
			break;
		case QNetworkReply::SslHandshakeFailedError:
			errorMessage = tr("The SSL/TLS handshake failed");
			break;
		case QNetworkReply::TemporaryNetworkFailureError:
			errorMessage = tr("The connection was broken due to disconnection from the network (connection will be retried automatically)");
			break;
		case QNetworkReply::NetworkSessionFailedError:
			errorMessage = tr("The connection was broken due to disconnection from the network");
			break;
		case QNetworkReply::BackgroundRequestNotAllowedError:
			errorMessage = tr("The background request is not currently allowed");
			break;
		case QNetworkReply::ProxyConnectionRefusedError:
			errorMessage = tr("The connection to the proxy server was refused");
			break;
		case QNetworkReply::ProxyConnectionClosedError:
			errorMessage = tr("The proxy server closed the connection prematurely");
			break;
		case QNetworkReply::ProxyNotFoundError:
			errorMessage = tr("The proxy host name was not found");
			break;
		case QNetworkReply::ProxyTimeoutError:
			errorMessage = tr("The connection to the proxy timed out");
			break;
		case QNetworkReply::ProxyAuthenticationRequiredError:
			errorMessage = tr("The proxy requires authentication");
			break;
		case QNetworkReply::ContentAccessDenied:
			errorMessage = tr("The access to the remote content was denied");
			break;
		case QNetworkReply::ContentOperationNotPermittedError:
			errorMessage = tr("The operation requested is not permitted");
			break;
		case QNetworkReply::ContentNotFoundError:
			errorMessage = tr("The remote content was not found");
			break;
		case QNetworkReply::AuthenticationRequiredError:
			errorMessage = tr("Authentication failed");
			break;
		case QNetworkReply::ContentReSendError:
			errorMessage = tr("The request needed to be sent again, but this failed");
			break;
		case QNetworkReply::ContentConflictError:
			errorMessage = tr("Conflict with the current state of the resource");
			break;
		case QNetworkReply::ContentGoneError:
			errorMessage = tr("The requested resource is no longer available");
			break;
		case QNetworkReply::InternalServerError:
			errorMessage = tr("Internal server error");
			break;
		case QNetworkReply::OperationNotImplementedError:
			errorMessage = tr("The server does not support the functionality required");
			break;
		case QNetworkReply::ServiceUnavailableError:
			errorMessage = tr("The server is unable to handle the request at this time");
			break;
		case QNetworkReply::ProtocolUnknownError:
			errorMessage = tr("Unknown protocol");
			break;
		case QNetworkReply::ProtocolInvalidOperationError:
			errorMessage = tr("The requested operation is invalid for this protocol");
			break;
		case QNetworkReply::UnknownNetworkError:
			errorMessage = tr("Unknown network-related error");
			break;
		case QNetworkReply::UnknownProxyError:
			errorMessage = tr("Unknown proxy-related error");
			break;
		case QNetworkReply::UnknownContentError:
			errorMessage = tr("Unknown error related to the remote content");
			break;
		case QNetworkReply::ProtocolFailure:
			errorMessage = tr("A breakdown in protocol was detected");
			break;
		case QNetworkReply::UnknownServerError:
			errorMessage = tr("Unknown error related to the server response");
			break;
		default:
			errorMessage = tr("Unknown network error");
			break;
	}

	emit networkError(errorMessage);
}

void NetworkManager::replySslErrors(__internal::NetworkReplyHandler* replyHandler, const QList<QSslError>& /*errors*/)
{
//qDebug() << ((unsigned long) this) << replyHandler->id() << "NetworkManager" << __func__;

	const QString errorMessage = tr("Error in ssl session initialization when handling a request");

	emit networkError(errorMessage);
}

void NetworkManager::replyHandlerFinished(__internal::NetworkReplyHandler* replyHandler)
{
//qDebug() << ((unsigned long) this) << replyHandler->id() << "NetworkManager" << __func__;

	const auto id = replyHandler->id();

	// Removing the handler from the set
	m_replyHandlers.remove(replyHandler);

	DataAvailableNotifee* const notifee = replyHandler->notifee();

	// Taking the reply here because after the next if there could be a new request with the same id
	QNetworkReply* const reply = notifee ? notifee->reply(id) : nullptr;

	// Setting the reply and reply handler of the notifee to null and calling requestCompleted()
	if (notifee) {
		replyHandler->clearNotifee();

		notifee->setReply(id, nullptr);
		notifee->setReplyHandler(id, nullptr);

		notifee->requestCompleted(id);
	}

	// Scheduling for deletion, both the reply and the reply handler
	auto command = [reply, replyHandler](){ delete replyHandler; delete reply; };
	QCoreApplication::postEvent(&(CommandEventReceiver::instance()), new CommandEvent(std::move(command)));

	// If no requests are running emitting the networkRequestsEnded() signal
	if (m_replyHandlers.isEmpty()) {
		emit networkRequestsEnded();
	}
}


//PROBLEMA: CERTE NEWS, ANCHE SE COMPLETE, NON VENGONO MARCATE COME TALI NELLA GUI E QUINDI NON È POSSIBILE VISUALIZZARLE...
