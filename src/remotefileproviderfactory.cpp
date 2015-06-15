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

#include "include/remotefileproviderfactory.h"
#include <functional>

RemoteFileProviderFactory::RemoteFileProviderFactory(QObject* parent)
	: QObject(parent)
	, m_fileProviders()
	, m_providersForOwner()
	, m_ownersForProvider()
{
}

RemoteFileProviderFactory::~RemoteFileProviderFactory()
{
	// Nothing to do here, all audio providers are our children and so will
	// be deleted by QObject destructor
}

RemoteFileProvider* RemoteFileProviderFactory::getRemoteFileProvider(QObject* owner, QUrl remoteUrl)
{
	RemoteFileProvider* p = nullptr;

	// Checking if provider already exists and returning it in case
	if (m_fileProviders.contains(remoteUrl)) {
		p = m_fileProviders[remoteUrl];
	} else {
		p = new RemoteFileProvider(this);
		p->setRemoteUrl(remoteUrl);
		m_fileProviders[remoteUrl] = p;

		// Connecting the signal for status change
		connect(p, &RemoteFileProvider::statusChanged,
			std::bind(&RemoteFileProviderFactory::providerChangedState, this, p));
	}

	// If the owner already owns some provider, we do not need to connects its destroyed signal,
	// otherwise connecting it
	if (!m_providersForOwner.contains(owner)) {
		connect(owner, &QObject::destroyed,
			std::bind(&RemoteFileProviderFactory::ownerDeleted, this, owner));
	}

	// Adding owner to the set of owners for the file provider and the provider to the set of
	// owned providers
	m_providersForOwner[owner].insert(p);
	m_ownersForProvider[p].insert(owner);

	return p;
}

void RemoteFileProviderFactory::ownerDeleted(QObject* owner)
{
	// Safety check
	if (Q_UNLIKELY(!m_providersForOwner.contains(owner))) {
		qDebug() << "INTERNAL ERROR: unknown owner of RemoteFileProvider deleted";

		return;
	}

	// We have to check, for all providers owned by owner, if they can be deleted or not
	for (RemoteFileProvider* p: m_providersForOwner[owner]) {
		// Few safety checks
		if (Q_UNLIKELY(!m_ownersForProvider.contains(p) || !m_ownersForProvider[p].contains(owner))) {
			qDebug() << "INTERNAL ERROR: inconsistent data structures in RemoteFileProviderFactory";

			return;
		}

		m_ownersForProvider[p].remove(owner);
		// Checking if we have to delete the provider
		if (m_ownersForProvider[p].isEmpty() && (p->status() != RemoteFileProvider::Downloading)) {
			m_ownersForProvider.remove(p);
			m_fileProviders.remove(p->remoteUrl());
			delete p;
		}
	}

	// Removing the owner
	m_providersForOwner.remove(owner);
}

void RemoteFileProviderFactory::providerChangedState(RemoteFileProvider* provider)
{
	// Safety check
	if (Q_UNLIKELY(!m_ownersForProvider.contains(provider))) {
		qDebug() << "INTERNAL ERROR: unknown owner of RemoteFileProvider deleted";

		return;
	}

	// We are only interested in the new status being Downloaded or DownloadInterrupted
	if ((provider->status() != RemoteFileProvider::Downloaded) ||
	    (provider->status() != RemoteFileProvider::DownloadInterrupted)) {
		return;
	}

	// Checking if the provider has no owners and, if so, deleting it
	if (m_ownersForProvider[provider].isEmpty()) {
		m_ownersForProvider.remove(provider);
		m_fileProviders.remove(provider->remoteUrl());
		delete provider;
	}
}

QObject* remoteFileProviderFactorySingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)

	return new RemoteFileProviderFactory();
}
