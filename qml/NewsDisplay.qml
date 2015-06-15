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

// A simple item to show the details of the news. For the moment we only show
// the text of the news as rich text
CentralSubItem {
	id: newsDetail

	// This contains the whole news
	property var news

	ScrollView {
		anchors.fill: parent

		Flickable {
			id: flickableArea
			anchors.fill: parent
			contentWidth: newsTextItem.width
			contentHeight: newsTextItem.height
			flickableDirection: Flickable.VerticalFlick

			Text {
				id: newsTextItem
				wrapMode: Text.Wrap
				width: newsDetail.width
				textFormat: Text.RichText
				font.pointSize: fontSize
				text: news.description;
			}
		}
	}
}
