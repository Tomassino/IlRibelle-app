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

#ifndef __ROLES_TO_PROPERTIES_WRAPPER_H__
#define __ROLES_TO_PROPERTIES_WRAPPER_H__

#include <QObject>
#include <QVariant>
#include "roles.h"

/**
 * \brief The abstract class to access roles from QML
 *
 * We need this because we cannot use Q_OBJECT in template classes
 */
class AbstractRolesQMLAccessor : public QObject
{
	Q_OBJECT

public:
	/**
	 * \brief Constructor
	 *
	 * \param parent the parent of the object
	 */
	AbstractRolesQMLAccessor(QObject* parent = nullptr)
		: QObject(parent)
		, m_qmlReadOnlyRoles(true)
	{
	}

	/**
	 * \brief Destructor
	 */
	virtual ~AbstractRolesQMLAccessor()
	{
	}

	/**
	 * \brief Sets whether roles are read-only from QML or not
	 *
	 * \param ro if true roles are read-only from QML
	 */
	void setQMLReadOnlyRoles(bool ro)
	{
		m_qmlReadOnlyRoles = ro;
	}

	/**
	 * \brief Returns the value of a role
	 *
	 * \param roleName the name of the role whose value to return
	 * \return the value of a role. An invalid value is returned if the
	 *         role does not exists
	 */
	Q_INVOKABLE virtual QVariant roleValue(QByteArray roleName) = 0;

public slots:
	/**
	 * \brief Sets the value of a role
	 *
	 * \param roleName the name of the role whose value to set
	 * \param value the new value of the role
	 * \return false if the role doesn't exists
	 */
	virtual bool setRoleValue(QByteArray roleName, QVariant value) = 0;

signals:
	/**
	 * \brief The signal emitted when a role changes
	 *
	 * \param roleName the name of the role that changed
	 */
	void roleChanged(QByteArray roleName);

protected:
	/**
	 * \brief If true roles are read-only from QML (true by default)
	 */
	bool m_qmlReadOnlyRoles;
};

/**
 * \brief The class that allows to access roles from QML
 *
 * This wrapper to a Roles template instantiation that adds functions to get and
 * set roles from QML and to receive notifications whena role changes. This
 * class needs to be registered to QML to be used (AbstractRolesQMLAccessor
 * should actually be registered). Roles are by default read-only from QML (i.e.
 * calling setRoleValue does nothing and returns false), if you want to have
 * them read-write, call setQMLReadOnlyRoles(false)
 *
 * Developer note: I thought that using QObject dynamic properties could have
 *                 been a better way to expose roles to QML, but dynamic
 *                 properties are not seen from QML...
 */
template<class RolesType>
class RolesQMLAccessor : public AbstractRolesQMLAccessor
{
public:
	/**
	 * \brief Constructor
	 *
	 * \param rolesObj the object whose roles will be exposed as dynamic
	 *                 properties
	 * \param parent the parent of the object
	 */
	RolesQMLAccessor(RolesType* rolesObj, QObject* parent = nullptr);

	/**
	 * \brief Destructor
	 */
	virtual ~RolesQMLAccessor();

	/**
	 * \brief Returns the value of a role
	 *
	 * \param roleName the name of the role whose value to return
	 * \return the value of a role. An invalid value is returned if the
	 *         role does not exists
	 */
	virtual QVariant roleValue(QByteArray roleName) override;

	/**
	 * \brief Sets the value of a role
	 *
	 * \param roleName the name of the role whose value to set
	 * \param value the new value of the role
	 * \return false if the role doesn't exists
	 */
	virtual bool setRoleValue(QByteArray roleName, QVariant value) override;

private:
	/**
	 * \brief The function called when a role in the wrapped roles list
	 *        changes
	 *
	 * This syncs the dynamic property associated to the role that changed
	 * \param roleIndex the index of the role that has changed
	 */
	void roleDataChanged(int roleIndex);

	/**
	 * \brief The object whose roles we expose as dynamic properties
	 */
	RolesType* const m_rolesObj;

	/**
	 * \brief The id of the callback we register (to remove it in the
	 *        destructor)
	 */
	const int m_callbackId;
};

// Implementation of template functions

template<class RolesType>
RolesQMLAccessor<RolesType>::RolesQMLAccessor(RolesType* rolesObj, QObject* parent)
	: AbstractRolesQMLAccessor(parent)
	, m_rolesObj(rolesObj)
	, m_callbackId(m_rolesObj->addCallbackForSetData([this](int roleIndex) { this->roleDataChanged(roleIndex); }))
{
}

template<class RolesType>
RolesQMLAccessor<RolesType>::~RolesQMLAccessor()
{
	// Removing the callback
	m_rolesObj->removeCallbackForSetData(m_callbackId);
}

template<class RolesType>
QVariant RolesQMLAccessor<RolesType>::roleValue(QByteArray roleName)
{
	const int roleIndex = m_rolesObj->getRoleIndexFromName(roleName);

	if (roleIndex == -1) {
		return QVariant();
	} else {
		return m_rolesObj->getVariantData(roleIndex);
	}
}

template<class RolesType>
bool RolesQMLAccessor<RolesType>::setRoleValue(QByteArray roleName, QVariant value)
{
	if (m_qmlReadOnlyRoles) {
		return false;
	}

	const int roleIndex = m_rolesObj->getRoleIndexFromName(roleName);

	if (roleIndex == -1) {
		return false;
	}

	// If value is not actually changing, ending here and not emitting the roleChanged signal
	if (m_rolesObj->getVariantData(roleIndex) == value) {
		return true;
	}

	// Changing data. We ask to ignore the callback, we know data is
	// changing and can emit the signal by ourself
	m_rolesObj->setVariantData(roleIndex, value, true);

	emit roleChanged(roleName);

	return true;
}

template<class RolesType>
void RolesQMLAccessor<RolesType>:: roleDataChanged(int roleIndex)
{
	emit roleChanged(m_rolesObj->getRoleNameFromIndex(roleIndex));
}

#endif
