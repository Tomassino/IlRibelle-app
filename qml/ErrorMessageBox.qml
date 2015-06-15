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

// A simple box to show error messages on top of the screen which disappear
// after a period of time. You should not set the height or width of this, only
// its x and y position and the parentWidth property
Rectangle {
	id: errorMessageBox
	visible: false
	color: "black"
	height: errorText.height + textBorder
	width: 2 * (parentWidth / 3)
	radius: textBorder
	opacity: 0.9

	// The total width of the parent item. We will set our width as a
	// portion of this width
	property real parentWidth

	// The border around text
	property real textBorder: width * 0.05

	// The text to display
	property alias errorMessage: errorText.text

	// The time after which the box disappears
	property alias interval: errorTimer.interval

	Text {
		id: errorText

		height: contentHeight
		width: parent.width - errorMessageBox.textBorder
		x: (parent.width - width) / 2
		y: (parent.height - height) / 2
		color: "white"
		font.pointSize: fontSize
		wrapMode: Text.Wrap
	}

	// The timer to hide the message
	Timer {
		id: errorTimer
		interval: 3000
		running: false
		repeat: false

		onTriggered: errorBoxHidingAnimation.start()
	}

	// The animation to hide us
	SequentialAnimation {
		id: errorBoxHidingAnimation
		running: false

		NumberAnimation {
			targets: errorMessageBox
			property: "opacity"
			easing.type: Easing.Linear
			duration: 1000
			from: 0.9
			to: 0
		}
		ScriptAction {
			script: {
				errorMessageBox.visible = false
				errorMessageBox.opacity = 0.9
			}
		}
	}

	// When we become visible, we also start the timer to hide us
	onVisibleChanged: {
		if (visible) {
			errorTimer.start()
		}
	}
}
