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

#include "include/roleshelpers.h"

void callSetDataCallbacks(const RolesCallbackStorage* s, int roleIndex)
{
	for (const auto& c: s->m_setDataCallbacks) {
		c(roleIndex);
	}
}

RolesCallbackStorage::RolesCallbackStorage()
	: m_setDataCallbacks()
	, m_nextCallbackID(0)
{
}

int RolesCallbackStorage::addCallbackForSetData(const std::function<void(int)>& setDataCallback)
{
	const int callbackID = m_nextCallbackID;
	m_setDataCallbacks[callbackID] = setDataCallback;

	m_nextCallbackID++;
	return callbackID;
}

bool RolesCallbackStorage::removeCallbackForSetData(int id)
{
	if (!m_setDataCallbacks.contains(id)) {
		return false;
	}

	m_setDataCallbacks.remove(id);

	return true;
}
