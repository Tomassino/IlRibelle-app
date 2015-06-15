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

#ifndef __REMOTE_FILE_PROVIDER_FACTORY_H__
#define __REMOTE_FILE_PROVIDER_FACTORY_H__

#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>
#include <QQmlListProperty>
#include <QList>
#include <QUrl>
#include <QSet>
#include "include/remotefileprovider.h"

/**
 * \brief A factory for RemoteFileProvider
 *
 * This class creates and manages the life of RemoteFileProvider instances. It
 * should be registered as a singleton in QML and used in QML to get objects of
 * type RemoteFileProvider.
 */
class RemoteFileProviderFactory : public QObject
{
	Q_OBJECT

public:
	/**
	 * \brief Constructor
	 *
	 * \param parent the parent object
	 */
	RemoteFileProviderFactory(QObject* parent = nullptr);

	/**
	 * \brief Destructor
	 */
	virtual ~RemoteFileProviderFactory();

public slots:
	/**
	 * \brief The function to get a RemoteFileProvider
	 *
	 * This function returns a RemoteFileProvider for the given url. If an
	 * object already exists in the list for that url, that object is
	 * returned, otherwise a new object is created. The owner object is
	 * needed because the returned object will be destroyed when caller is
	 * destroyed unless it is downloading a file. It is not to be confused
	 * with the parent of the RemoteFileProvider instance which is always
	 * set to this. If the provider has more than one owner, it is destroyed
	 * when all owners have been destroyed
	 * \param owner the object that requested a RemoteFileProvider
	 *              instance. The returned object is destroyed when owner is
	 *              destroyed unless it is downloading a file
	 * \param remoteUrl the url of the file
	 * \return the RemoteFileProvider instance for the given remote url
	 */
	RemoteFileProvider* getRemoteFileProvider(QObject* owner, QUrl remoteUrl);

private:
	/**
	 * \brief The function called when the owner of a RemoteFileProvider
	 *        is deleted
	 *
	 * \param owner the owner about to be deleted
	 */
	void ownerDeleted(QObject* owner);

	/**
	 * \brief The function called when a file provider changes its status
	 *
	 * If the provider has no owner and it has finished the download, the
	 * provider is deleted
	 * \param provider the provider that changed state
	 */
	void providerChangedState(RemoteFileProvider* provider);

	/**
	 * \brief The map of file providers
	 */
	QHash<QUrl, RemoteFileProvider*> m_fileProviders;

	/**
	 * \brief The map from owners to RemoteFileProviders objects
	 */
	QHash<QObject*, QSet<RemoteFileProvider*>> m_providersForOwner;

	/**
	 * \brief The map from RemoteFileProviders objects to owners
	 */
	QHash<RemoteFileProvider*, QSet<QObject*>> m_ownersForProvider;
};

/**
 * \brief The function returning an instance of the RemoteFileProviderFactory
 *
 * \param engine the qml engine
 * \param scriptEngine the javascript engine
 * \return an instance of RemoteFileProviderFactory
 */
QObject* remoteFileProviderFactorySingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine);

#endif
