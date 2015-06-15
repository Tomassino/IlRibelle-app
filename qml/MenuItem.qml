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
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import com.ilribelle 1.0

// An item in the main menu. This is a Rectangle with an overlapping MouseArea
// and text inside. This exposes the clicked signal which is emitted when the
// internal mouse area is clicked
Rectangle {
	id: menuItem
	color: "transparent"
	border.color: "transparent"
	border.width: 2*mm
	implicitWidth: textItem.contentWidth + 2*mm
	implicitHeight: textItem.contentHeight + 2*mm

	// The property with the item text
	property alias text: textItem.text

	// The color of text
	property alias textColor: textItem.color

	// If set to true, clicks are captured, otherwise they aren't
	property alias clickEnabled: menuMouseArea.enabled

	// The signal emitted when the item is clicked
	signal clicked()

	Text {
		id: textItem
		font.pointSize: fontSize

		anchors.fill: parent
		anchors.margins: 1*mm
	}

	MouseArea {
		id: menuMouseArea

		anchors.fill: parent

		onClicked: menuItem.clicked()
	}
}
