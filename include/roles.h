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

#ifndef __ROLES_H__
#define __ROLES_H__

#include <QVariant>
#include <QHash>
#include <array>
#include <utility>
#include <functional>
#include "include/roleshelpers.h"
#include "include/utilities.h"

/**
 * \file roles.h
 *
 * This file contains classes and helpers to create custom Roles for News or
 * Channel. A role is a simple class with an index (which is not fixed and may
 * change when the Role is used in different News or Channels), a name (a fixed
 * C string) and a QVariant field with data. Roles can be combined to create a
 * RolesList. A RolesList object has methods to get/set the value of for a role
 * or to get the index and name by specifying the Role name (i.e. the class
 * name). There are other helper classes which permit to access data for a role
 * in a list using the index. To define a new role you can use the DEFINE_ROLE
 * macro (see description). Roles, however, are classes, so you can also define
 * them explicitly in case you need particular data structures. What follows is
 * what all roles must have to be used with RolesList and RolesVectors:
 *
 * \code
 *	class RoleName
 *	{
 *	public:
 *		using RoleType = <data_type_for_role>;
 *
 *		// The index parameter must be stored in i
 *		RoleName(int index);
 *
 *		RoleName(const RoleName& other);
 *
 *		// This is not strictly necessary
 *		RoleName(RoleName&& other);
 *
 *		// Returns the value of the role
 *		RoleType toRoleType() const;
 *
 *		// Sets the value of the role
 *		void fromRoleType(const RoleType& d);
 *
 *		// This is the role name used in qml
 *		static constexpr const char* s = "role_name";
 *
 *		// The index of the role (set in the constructor)
 *		const int i;
 *
 *		// This is data exposed to qml (it must always be present)
 *		QVariant v;
 *	};
 * \endcode
 */

/**
 * \brief The class modelling a list of roles with access vectors
 *
 * This class puts together a RolesVector and RolesList. You should use this
 * instead of inheriting from those two classes directly. It also allows setting
 * the callback after creation
 */
template <class RolesListType>
class Roles : public RolesCallbackStorage, public RolesVectors<RolesListType>, public RolesListType
{
public:
	/**
	 * \brief Constructor
	 */
	Roles()
		: RolesCallbackStorage()
		, RolesVectors<RolesListType>(static_cast<RolesCallbackStorage*>(this))
		, RolesListType(static_cast<RolesCallbackStorage*>(this), RolesVectors<RolesListType>::m_roleValues.data())
	{
	}
};

#endif
