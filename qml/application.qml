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
import QtQuick.Controls 1.3

// The main item. It contains the top bar, the central item, the menu and the
// configuration panel (which is shown in the central item when selected)
ApplicationWindow {
	id: mainItem
	width: 480
	height: 640
	visible: true

	function closeApplication()
	{
		// For the moment we just quit, perhaps we should ask the user to confirm
		Qt.quit()
	}

	// This is connected to the networkError signal of the context object in
	// the Component.onCompleted slot
	function networkError(reason) {
		errorMessageBox.errorMessage = qsTr("Network error: ") + reason
		errorMessageBox.visible = true;
	}

	toolBar: TopBar {
		id: topBar
		//height: 30*mm

		splashRectangleVisible: true

		onBackClicked: {
			centralItem.goBack()
			if (mainMenu.menuVisible) {
				mainMenu.menuVisible = false;
			}
		}

		onMainMenuClicked: mainMenu.menuVisible = !mainMenu.menuVisible
	}

	Rectangle {
		id: mainWindowItem
		anchors.fill: parent
		color: "white"
		// We do all key processing here
		focus: true

		// This element is invisible and will never be made visible. We only
		// use it to get the default text size
		Text {
			id: dummyText
			text: "dummy"
			visible: false
		}

		CentralItem {
			id: centralItem
			anchors.fill: parent
			visible: true

			onAttemptToPopLastItem: mainItem.closeApplication()
		}

		MainMenu {
			id: mainMenu
			x: menuVisible ? 0 : menuWidth + 1
			y: 0
			width: mainWindowItem.width
			height: mainWindowItem.height
			z: 10
			captureClicks: menuVisible
			property bool menuVisible: false

			Behavior on x {
				NumberAnimation {
					duration: 400
					easing.type: Easing.InOutQuad
				}
			}

			onCloseMenu: menuVisible = false
			onCloseApplication: mainItem.closeApplication()
			onShowAbout: centralItem.showAbout()
		}

		Keys.onMenuPressed: mainMenu.menuVisible = !mainMenu.menuVisible

		Keys.onBackPressed: {
			if (mainMenu.menuVisible) {
				mainMenu.menuVisible = false
			} else {
				centralItem.goBack()
			}
		}

		Keys.onReleased: {
			if (event.key === Qt.Key_Backspace) {
				if (mainMenu.menuVisible) {
					mainMenu.menuVisible = false;
				} else {
					centralItem.goBack()
				}

				// Accepting event to prevent propagation
				event.accepted = true
			}
		}
	}

	// The splash screen
	SplashScreen {
		id: splashScreen
		visible: true
		anchors.fill: parent
		z: 100
		toolBar: mainItem.toolBar
	}

	// The rectangle to show error messages
	ErrorMessageBox {
		id: errorMessageBox
		visible: false
		x: (parent.width - width) / 2
		y: 2 * (parent.height / 3)
		parentWidth: parent.width
		z: 50
	}

	// We need this to tell the context object which is the starting font size
	// and to connect the networkError signal from the context object
	Component.onCompleted: {
		setInitialFontSize(dummyText.font.pointSize)

		onNetworkError.connect(networkError);
	}
}
