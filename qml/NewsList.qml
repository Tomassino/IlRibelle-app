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
import QtQuick.Layouts 1.1

// The list of news. This is basically a ListView (the ScrollView is just to
// decorate the ListView). When a news is clicked the newsClicked() signal is
// emitted
CentralSubItem {
	id: listOfNews

	// Emitted when a news is selected. news is the model item data for the
	// news
	signal newsClicked(var news)

	ScrollView {
		anchors.fill: parent

		ListView {
			id: listOfNewsView
			model: newsModel

			// This property stores the item that has changed color
			property var itemChangedColor: null

			// When the list becomes visible, we reset the color of
			// the delegate which changed color before
			onVisibleChanged: {
				if (itemChangedColor != null) {
					itemChangedColor.color = itemChangedColor.normalColor
				}
			}

			// The delegate showing a news. It contains a rectangle
			// with text and a mouse area to capture clicks.
			delegate: Rectangle {
				id: itemDelegate
				width: parent.width
				height: titleRectangle.height + containersBorders
				color: "white"

				// The size of borders
				property real containersBorders: width * 0.02

				Rectangle {
					id: titleRectangle
					x: (parent.width - width) / 2
					y: (parent.height - height) / 2
					width: parent.width - containersBorders
					height: textContainer.height + containersBorders
					color: normalColor
					radius: containersBorders

					// The normal color of the delegate
					property color normalColor: "lightgray"

					// The color when the item is selected
					property color selectedColor: "#FF5656"

					Text {
						id: textContainer
						x: (parent.width - width) / 2
						y: (parent.height - height) / 2
						width: parent.width - containersBorders
						height: contentHeight

						text: (new Date(pubDate)).toLocaleDateString() + "<br/><b>"  + title + "</b><br/><i>" + creator + "</i>"
						font.pointSize: fontSize
						wrapMode: Text.Wrap
						textFormat: Text.RichText
						color: complete ? "black" : "darkGray"
					}
				}

				MouseArea {
					anchors.fill: parent

					// We enable clicking on the news only after it has been comepleted
					enabled: complete && mouseAreasEnabled

					onClicked: {
						titleRectangle.color = titleRectangle.selectedColor
						listOfNewsView.itemChangedColor = titleRectangle
						listOfNews.newsClicked(model.roles)
					}
				}
			}
		}
	}
}
