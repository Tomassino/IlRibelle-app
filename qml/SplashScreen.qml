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

// The initial splash screen
Rectangle {
	color: "white"

	// This property should be set to the toolbar used in the application,
	// so that we can modify the object covering it during the splash screen
	property var toolBar: null

	// A function to reset the source image (to free memory)
	function releaseImage()
	{
		image.source = ""
	}

	// This is to ignore clicks and touches when the splash screen is visible
	MouseArea {
		anchors.fill: parent
	}

	Rectangle {
		anchors {
			horizontalCenter: parent.horizontalCenter
			verticalCenter: parent.verticalCenter
		}
		width: Math.max(image.width, text.width)
		height: image.height + text.height

		Image {
			id: image

			fillMode: Image.PreserveAspectFit
			horizontalAlignment: Image.AlignHCenter
			verticalAlignment: Image.AlignVCenter
			anchors {
				horizontalCenter: parent.horizontalCenter
			}
			width: sourceSize.width
			height: sourceSize.height

			source: "qrc:///resources/header.png"
		}

		Text {
			id: text

			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
			anchors {
				horizontalCenter: image.horizontalCenter
				top: image.bottom
			}
			width: contentWidth
			height: contentHeight

			text: "Fondatore Massimo Fini\nDirettore Responsabile Valerio Lo Monaco"
			wrapMode: Text.WordWrap
		}
	}

	Timer {
		interval: 1500
		running: true
		repeat: false

		onTriggered: splashHidingAnimation.start()
	}

	// The animation to hide the splash screen
	SequentialAnimation {
		id: splashHidingAnimation
		running: false

		NumberAnimation {
			targets: (toolBar === null) ? splashScreen : [splashScreen, toolBar.splashRectangle]
			property: "opacity"
			easing.type: Easing.Linear
			duration: 1000
			from: 1
			to: 0
		}
		ScriptAction {
			script: {
				// Here we hide the splash screen and then release the splash image
				splashScreen.visible = false
				if (toolBar !== null) {
					toolBar.splashRectangle.visible = false
				}
				splashScreen.releaseImage()
			}
		}
	}
}
