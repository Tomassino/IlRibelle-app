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

import QtQuick 2.4

// The base for all items that are shown inside the CentralItem. This contains
// what is needed to animate transitions between items
Rectangle {
	id: centralSubItem

	// The property to set the scale factor of this item. This is used to
	// animate transitions between items in the StackView
	property real scaleFactor: 1

	// This is set to false when mouse areas should be disables (e.g. during
	// animations)
	property bool mouseAreasEnabled: true

	transform: Scale {
		id: centralSubItemScale

		origin.x: centralSubItem.width / 2
		origin.y: centralSubItem.height / 2
		xScale: centralSubItem.scaleFactor
		yScale: centralSubItem.scaleFactor
	}
}
