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
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.3

// The bar with buttons on top of the screen. This contains two buttons: one
// to go back to the previous screen and one to show the menu. Signals are
// emitted when buttons are clicked. This also contains a white rectangle
// covering the toolbar which is animated together with the splash screen
ToolBar {
	id: toolBar

	// The distance of buttons from the border. This is the value of the
	// anchors.margin property of buttons
	property alias buttonDistanceFromBorder: layout.spacing

	// The rectangle covering the toolbar at the begining
	property var splashRectangle: coveringRectangle

	// The visible property of the convering rectangle
	property alias splashRectangleVisible: coveringRectangle.visible

	// The signal emitted when the back button is clicked
	signal backClicked()

	// The signal emitted when the menu button is clicked
	signal mainMenuClicked()

	// Setting the background to white
	style: ToolBarStyle {
		background: Rectangle {
			color: "white"
		}
	}

	// When the component is complete, we ask the context object to generate the icons for buttons
	// by setting the preferred height
	Component.onCompleted: {
		// Requesting the images (we use anonymous functions as callbacks)
		getPNGFromVectorial(":/resources/busyIndicator.svg", busyIndicator.busyIndicatorIconHeight, true, function(filename) { busyIndicator.busyIndicatorIcon = filename });

		var toolbarIconHeight = toolBar.height - (2 * buttonDistanceFromBorder);
		getPNGFromVectorial(":/resources/back.svg", toolbarIconHeight, true, function(filename) { back.source = filename });
		getPNGFromVectorial(":/resources/menu.svg", toolbarIconHeight, true, function(filename) { mainMenu.source = filename });
	}

	Item {
		id: wrapperItem
		anchors.fill: parent
		implicitWidth: layout.implicitWidth
		implicitHeight: 8*mm

		// This is shown to cover the tool bar when the splash screen is visible
		Rectangle {
			id: coveringRectangle
			anchors.fill: parent
			z: 100
			color: "white"
			visible: false

			// This is to ignore clicks and touches when the splash screen is visible
			MouseArea {
				anchors.fill: parent
			}
		}

		RowLayout {
			id: layout
			anchors.fill: parent

			Image {
				id: back
				Layout.minimumHeight: toolBar.height - (2 * buttonDistanceFromBorder)
				Layout.minimumWidth: Layout.minimumHeight

				MouseArea {
					anchors.fill: parent
					onClicked: backClicked()
				}
			}

			Item {
				id: topBarImage
				Layout.fillWidth: true
				Layout.fillHeight: true

				Image {
					id: headerImage
					source: "qrc:///resources/header.png"
					anchors.fill: parent
					fillMode: Image.PreserveAspectFit
					horizontalAlignment: Image.AlignHCenter
					verticalAlignment: Image.AlignVCenter
				}

				BusyIndicator {
					id: busyIndicator

					running: networkRequestsRunning
					x: topBarImage.width - imageSize
					y: bigIndicator ? ((topBarImage.height - imageSize) / 2) : 0
					z: 10

					property bool bigIndicator: ((topBarImage.width - headerImage.paintedWidth) > (2 * topBarImage.height)) ? true : false
					property real imageSize: bigIndicator ? (topBarImage.height * 0.9) : (topBarImage.height * 0.5)

					// Sets the height of the busy indicator icon. The busy icon renders horribly
					// if it is too small
					property real busyIndicatorIconHeight: (busyIndicator.imageSize < 20) ? 20 : busyIndicator.imageSize

					// The image to use as the busy indicator
					property string busyIndicatorIcon

					style: BusyIndicatorStyle {
						indicator: Image {
							sourceSize.width: control.imageSize
							sourceSize.height: control.imageSize
							width: control.imageSize
							height: control.imageSize
							// This is set by the controller after the image has been generated
							source: busyIndicator.busyIndicatorIcon
							visible: control.running

							// The animator to rotate the image
							RotationAnimator on rotation {
								from: 0;
								to: 360;
								duration: 1000
								easing.type: Easing.Linear
								loops: Animation.Infinite
								running: control.running
							}
						}
					}
				}
			}

			Image {
				id: mainMenu
				Layout.minimumHeight: toolBar.height - (2 * buttonDistanceFromBorder)
				Layout.minimumWidth: Layout.minimumHeight

				MouseArea {
					anchors.fill: parent
					onClicked: mainMenuClicked()
				}
			}
		}
	}
}
