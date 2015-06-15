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

#ifndef __MISC_NATIVE_H__
#define __MISC_NATIVE_H__

#include <QObject>
#include <QtQml>
#include <QEvent>

#ifdef MISC_NATIVE_NO_JNI_ONLOAD_DEFINITION
using JavaVM = struct _JavaVM;
/**
 * \brief This function registers native method of the Java binding class
 *
 * \param vm the Java virtual machine instance
 * \param reserved unused
 * \return the requested version of the JNI
 */
int miscNative_registerJavaNativeMethods(JavaVM* vm, void* reserved);
#endif

namespace MiscNative_internal
{
	class MiscNativeImpl;
}

/**
 * \brief The MiscNative singleton allows to access some native functionalities
 *        directly from C++/QML code with platform abstraction
 *
 * This is a singleton. Its status indicates whether an action is being
 * performed or not and whether it has finished successfully or with an error.
 * For actions that are instantaneous the status changes during the function
 * call, otherwise it can change at a later time. For the moment only posting a
 * share intent is supported.
 * The supported platforms are Android and Desktop (the implementation for the
 * latter is mostly to test stuffs, not doing anything real)
 */
class MiscNative : public QObject
{
	Q_OBJECT
	Q_ENUMS(Status)
	Q_PROPERTY(Status status READ status NOTIFY statusChanged)

public:
	/**
	 * \brief The possible states
	 */
	enum Status {
		Idle, /**< Not performing any request */
		Acting, /**< An action is being executed */
		ActionFinished, /**< An action has completed successfully */
		ActionError /**< An error occurred while performing an action */
	};

	/**
	 * \brief Returns the only instance of this class
	 *
	 * \return the only instance of this class
	 */
	static MiscNative* instance();

private:
	/**
	 * \brief Constructor
	 *
	 * This is private because this must be a singleton
	 * \param parent the parent object
	 */
	MiscNative(QObject* parent = nullptr);

	/**
	 * \brief Destructor
	 */
	virtual ~MiscNative();

public:
	/**
	 * \brief Copy constructor deleted
	 */
	MiscNative(const MiscNative&) = delete;

	/**
	 * \brief Copy assignment operator deleted
	 */
	MiscNative& operator=(const MiscNative&) = delete;

	/**
	 * \brief Move constructor deleted
	 */
	MiscNative(MiscNative&&) = delete;

	/**
	 * \brief Move assignment operator deleted
	 */
	MiscNative& operator=(MiscNative&&) = delete;

	/**
	 * \brief Returns the current status
	 *
	 * \return the current status
	 */
	Status status() const
	{
		return m_status;
	}

public slots:
	/**
	 * \brief Posts a share intent to share a link
	 *
	 * If we are doing something else, this does nothing
	 * \param link a link to share
	 * \param subject the subject for the link to share
	 * \param shareCaption the caption of the share dialog
	 */
	void share(QString link, QString subject, QString shareCaption);

signals:
	/**
	 * \brief The signal emitted whenever status changes
	 */
	void statusChanged();

	/**
	 * \brief The signal emitted when the current operation completes
	 */
	void operationDone();

	/**
	 * \brief The signal emitted in case of error
	 *
	 * \param reason the explanation of the error
	 */
	void error(QString reason);

private:
	/**
	 * \brief The function receiving and dealing with custom events
	 *
	 * This is used to receive and execute a CommandEvent
	 * \param event the event we have reveived
	 */
	virtual void customEvent(QEvent* event) override;

	/**
	 * \brief Sets the new status and emits the statusChanged signal
	 *
	 * \param newStatus the new status
	 */
	void setStatus(Status newStatus);

	/**
	 * \brief The function called by the platform-specific implementation
	 *        when an action completed successfully
	 */
	void actionCompleted();

	/**
	 * \brief The function called by the platform-specific implementation
	 *        when there was an error
	 *
	 * \param reason the error message
	 */
	void actionError(QString reason);

	/**
	 * \brief The current status
	 */
	Status m_status;

	/**
	 * \brief Platform-specific implementation
	 *
	 * This is not deleted in the destructor, delete in other way!
	 */
	MiscNative_internal::MiscNativeImpl* m_impl;

	/**
	 * \brief The class with internal implementation is friend to call
	 *        private function
	 */
	friend class MiscNative_internal::MiscNativeImpl;
};

/**
 * \brief The function returning the instance of the MiscNative singleton
 *
 * \param engine the qml engine
 * \param scriptEngine the javascript engine
 * \return an instance of QTwitter
 */
QObject* miscNativeSingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine);

#endif
