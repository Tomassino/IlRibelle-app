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
import QtQuick.Controls 1.2

CentralSubItem {
	id: aboutTextContainer
	color: "white"

	ScrollView {
		anchors.fill: parent

		Flickable {
			id: flickableArea
			anchors.fill: parent
			anchors.leftMargin: 1*mm
			anchors.rightMargin: 1*mm
			contentWidth: credits.width
			contentHeight: credits.height
			flickableDirection: Flickable.VerticalFlick

			Text {
				id: credits
				width: aboutTextContainer.width - (flickableArea.anchors.leftMargin + flickableArea.anchors.rightMargin)
				text: aboutText
				wrapMode: Text.Wrap
				textFormat: Text.RichText
				font.pointSize: fontSize

				onLinkActivated: {
					if (!mouseAreasEnabled) {
						return;
					}

					Qt.openUrlExternally(link)
				}
			}
		}
	}
}
