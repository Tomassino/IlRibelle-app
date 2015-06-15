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

#ifndef __DATA_AVAILABLE_NOTIFEE_H__
#define __DATA_AVAILABLE_NOTIFEE_H__

#include <QByteArray>
#include <QNetworkReply>
#include <QMap>

class NetworkManager;
namespace __internal {
	class NetworkReplyHandler;
}

/**
 * \brief The interface for classes that receive data from NetworkManager
 *
 * Classes that request data to a NetworkManager must implement this interface
 * so that they are notified when the data they requested arrives. Each request
 * of data to the NetworkManager has an associated request ID that is passed to
 * callbacks in this class. This class has a callback that is called every time
 * data is available (i.e. when the readyRead signal of the QNetworkReply is
 * emitted), You should read directly from the QNetworkReply when you implement
 * that function. The DataArrivedNotifee and AllDataArrivedNotifee subclasses
 * have functions that directly provide data in QByteArray (see description of
 * those classes)
 */
class DataAvailableNotifee
{
public:
	/**
	 * \brief Constructor
	 */
	DataAvailableNotifee();

	/**
	 * \brief Destructor
	 */
	virtual ~DataAvailableNotifee();

	/**
	 * \brief Returns the QNetworkReply from which data can be read for the
	 *        given request
	 *
	 * \param id the id of the request
	 * \return the QNetworkReply from which data can be read
	 */
	QNetworkReply* reply(int id)
	{
		return m_replies.value(id, nullptr);
	}

	/**
	 * \brief Returns the QNetworkReply from which data can be read for the
	 *        given request (const version)
	 *
	 * \param id the id of the request
	 * \return the QNetworkReply from which data can be read (const version)
	 */
	const QNetworkReply* reply(int id) const
	{
		return m_replies.value(id, nullptr);
	}

	/**
	 * \brief The function called when data is available
	 *
	 * When this function is called, you can read data from the reply
	 * object.
	 * \param id the request ID
	 */
	virtual void dataAvailable(int id) = 0;

	/**
	 * \brief The function called when all data is available
	 *
	 * \param id the request ID
	 */
	virtual void allDataAvailable(int id) = 0;

	/**
	 * \brief Re-implement to handle network errors
	 *
	 * Note that in case of errors during a request, NetworkManager calls
	 * both this function and emits its networkError signal
	 * \param id the request ID
	 * \param description the description of the error
	 */
	virtual void networkError(int id, const QString& description);

	/**
	 * \brief The function called after the request is completed and has
	 *        been deleted
	 *
	 * This function is called when a request has finished and has been
	 * deleted. Here you cannot read data (trying to get the reply with the
	 * reply() function will return nullptr), but you can enqueue a new
	 * request with the same id of the the request that has terminated. The
	 * default implementation does noting
	 * \param id the ID of the request that has finished
	 */
	virtual void requestCompleted(int id);

	/**
	 * \brief Interrupts the request with the given id
	 *
	 * \param id the request to interrupt
	 */
	void interruptRequest(int id);

private:
	/**
	 * \brief Sets the reply object for a request id. This is called by
	 *        NetworkManager
	 *
	 * This doesn't perform any check on whether the id is already used or
	 * not, please make sure the id is not used before calling this function
	 * \param id the if of the reply object
	 * \param reply the reply object
	 */
	void setReply(int id, QNetworkReply* reply);

	/**
	 * \brief Sets the object calling our callbacks for each id. This is
	 *        called by NetworkManager
	 *
	 * We need them to tell them not to call our callbacks when we are
	 * destroyed. This doesn't perform any check on whether the id is
	 * already used or not, please make sure the id is not used before
	 * calling this function
	 * \param id the id of the reply handler
	 * \param replyHandler the object calling out callbacks
	 */
	void setReplyHandler(int id, __internal::NetworkReplyHandler* replyHandler);

	/**
	 * \brief The object from which the reply can be read for every id
	 */
	QMap<int, QNetworkReply*> m_replies;

	/**
	 * \brief The object calling out callbacks
	 */
	QMap<int, __internal::NetworkReplyHandler*> m_replyHandlers;

	/**
	 * \brief NetworkManager is friend to access our private functions
	 */
	friend class NetworkManager;
};

/**
 * \brief The class which provides received data in a QByteArray as it arrives
 *
 * This is a subclass of DataAvailableNotifee with a callback which provides
 * data in a QByteArray. If you subclass this class, do not override
 * dataAvailable(), override dataArrived instead
 */
class DataArrivedNotifee : public DataAvailableNotifee
{
public:
	/**
	 * \brief Constructor
	 */
	DataArrivedNotifee();

	/**
	 * \brief Destructor
	 */
	virtual ~DataArrivedNotifee();

	/**
	 * \brief The function called when data is available
	 *
	 * Do not reimplement this function when inheriting from this class, use
	 * the dataArrived() method
	 * \param id the request ID
	 */
	virtual void dataAvailable(int id) final;

	/**
	 * \brief Implement to handle the arrived data
	 *
	 * This is called every time new data is available
	 * \param id the request ID
	 * \param data the data that has just arrived
	 */
	virtual void dataArrived(int id, const QByteArray& data) = 0;

};

/**
 * \brief The class which provides received data in a QByteArray when everything
 *        has been received
 *
 * This is a subclass of DataAvailableNotifee with a callback which provides
 * data in a QByteArray when all data has arrived. If you subclass this class,
 * do not override dataAvailable(), override allDataArrived instead
 */
class AllDataArrivedNotifee : public DataAvailableNotifee
{
public:
	/**
	 * \brief Constructor
	 */
	AllDataArrivedNotifee();

	/**
	 * \brief Destructor
	 */
	virtual ~AllDataArrivedNotifee();

	/**
	 * \brief The function called when data is available
	 *
	 * Do not reimplement this function when inheriting from this class, use
	 * the allDataArrived() method
	 * \param id the request ID
	 */
	virtual void dataAvailable(int id) final;

	/**
	 * \brief The function called when all data is available
	 *
	 * Do not reimplement this function when inheriting from this class, use
	 * the allDataArrived() method
	 * \param id the request ID
	 */
	virtual void allDataAvailable(int id) final;

	/**
	 * \brief Implement to handle the arrived data
	 *
	 * This is called once when all data has been received
	 * \param id the request ID
	 * \param data the data that has just arrived
	 */
	virtual void allDataArrived(int id, const QByteArray& data) = 0;

};

#endif
