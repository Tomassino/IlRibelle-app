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

// The main menu. This is a list of text items, each with its own MouseArea.
// This should take all the space of the parent item: the actual menu is only
// shown in the upper left part of the screen, but there is a mouse area that
// extends beyond the menu and that is used to capture clicks outside the menu
// and emit the closeMenu signal (the rationale is that whenever the user clicks
// a point outside the menu area the menu should be closed).
MouseArea {
	id: mainMenu

	// The width of the menu (only the actual menu, not the whole area
	// occupied by this item)
	property alias menuWidth: actualMenu.width

	// The height of the menu (only the actual menu, not the whole area
	// occupied by this item)
	property alias menuHeight: actualMenu.height

	// Whether clicks should be captured or not. Set to false if this item
	// is partially visible (e.g. if only the actual menu is hidden but part
	// of this item is still on screen)
	property alias captureClicks: mainMenu.enabled

	// The signal emitted when the menu should be closed
	signal closeMenu()

	// The signal emitted when the user requested to quit
	signal closeApplication()

	// The signal amitted when the user selects the about menu item
	signal showAbout()

	// This is the item with the actual menu
	Flickable {
		anchors {
			right: parent.right
			top: parent.top
		}
		width: Math.min(parent.width, actualMenu.width)
		height: Math.min(parent.height, actualMenu.height)
		contentWidth: actualMenu.width
		contentHeight: actualMenu.height

		Rectangle {
			id: actualMenu
			color: "#FF5656"
			height: layout.height
			width: layout.width

			// This is needed so that clicks in the area between menu items
			// are not considered as clicks outside menu (and so do not
			// cause the menu to be closed)
			MouseArea {
				anchors.fill: parent
			}

			ColumnLayout {
				id: layout

				Rectangle {
					Layout.preferredHeight: 1
					Layout.fillWidth: true
					color: "white"
				}

				MenuItem {
					text: qsTr("Update")
					textColor: "white"
					clickEnabled: mainMenu.captureClicks
					Layout.fillWidth: true

					onClicked: {
						// We directly use the updateNews() slot of the context object
						// and then we close the menu
						updateNews()
						closeMenu()
					}
				}

				Rectangle {
					Layout.preferredHeight: 1
					Layout.fillWidth: true
					color: "white"
				}

				MenuItem {
					text: qsTr("Clear cache")
					textColor: "white"
					clickEnabled: mainMenu.captureClicks
					Layout.fillWidth: true

					onClicked: {
						// We show the dialog to ask for confirmation
						// and then we close the menu
						clearCacheConfirmation.open()
						closeMenu()
					}
				}

				Rectangle {
					Layout.preferredHeight: 1
					Layout.fillWidth: true
					color: "white"
				}

				MenuItem {
					text: qsTr("Increase font size")
					textColor: "white"
					clickEnabled: mainMenu.captureClicks && canIncreaseFontSize
					Layout.fillWidth: true

					onClicked: increaseFontSize()
				}

				Rectangle {
					Layout.preferredHeight: 1
					Layout.fillWidth: true
					color: "white"
				}

				MenuItem {
					text: qsTr("Reduce font size")
					textColor: "white"
					clickEnabled: mainMenu.captureClicks && canDecreaseFontSize
					Layout.fillWidth: true

					onClicked: decreaseFontSize()
				}

				Rectangle {
					Layout.preferredHeight: 1
					Layout.fillWidth: true
					color: "white"
				}

				MenuItem {
					text: qsTr("About")
					textColor: "white"
					clickEnabled: mainMenu.captureClicks
					Layout.fillWidth: true

					onClicked: {
						// We emit the showAbout() signal and then we close the menu
						showAbout()
						closeMenu()
					}
				}

				Rectangle {
					Layout.preferredHeight: 1
					Layout.fillWidth: true
					color: "white"
				}

				MenuItem {
					text: qsTr("Quit")
					textColor: "white"
					clickEnabled: mainMenu.captureClicks
					Layout.fillWidth: true

					onClicked: closeApplication()
				}

				Rectangle {
					Layout.preferredHeight: 1
					Layout.fillWidth: true
					color: "white"
				}
			}
		}
	}

	// This is triggered when the user clicks outside the actual menu area
	onClicked: closeMenu()

	// The message box to ask the user whether to really clear cache or not
	MessageDialog {
		id: clearCacheConfirmation
		title: qsTr("Really clear cache?")
		text: qsTr("Do you really want to clear the cache? All news will be deleted and you will need to download them again.")
		standardButtons: StandardButton.Yes | StandardButton.No

		onYes: {
			clearAllNews()
		}
	}
}
