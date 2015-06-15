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

CentralSubItem {
	id: wrapperItem

	// The property with the source of the image to show
	property alias imageSource: image.source

	// The maximum zoom factor for the image
	property real maxImageZoom: 10

	Image {
		id: image

		x: (wrapperItem.width - width) / 2
		y: (wrapperItem.height - height) / 2
		horizontalAlignment: Image.AlignHCenter
		verticalAlignment: Image.AlignVCenter
		fillMode: Image.PreserveAspectFit
		width: sourceSize.width
		height: sourceSize.height
		transformOrigin: Item.Center

		// Here we calculare the dimension on screenof the image,
		// considering scale
		property real screenImageWidth: width * scale
		property real screenImageHeight: height * scale
	}

	PinchArea {
		id: pinchArea
		anchors.fill: parent
		enabled: mouseAreasEnabled

		// By using this the PinchArea will modify the scale property of
		// the image.
		pinch {
			target: image

			// We compute the minimum scale so that an image that is
			// bigger than the screen can be reduced to fit the
			// screen but not more, while allowing smaller images to
			// be visualized at a scale of 1
			minimumScale: Math.min(1, wrapperItem.width / image.sourceSize.width, wrapperItem.height / image.sourceSize.height)
			maximumScale: maxImageZoom
			minimumRotation: 0
			maximumRotation: 0
			dragAxis: Pinch.XAndYAxis

			// These properties have to be adapted so that the image
			// can be dragged up to the borders but not more. Here
			// we have to take into account the initial rectangle
			// of the image (width and height do not change) and the
			// current scale factor. Moreover if the image is
			// smaller than the screen, we fix its position
			minimumX: (wrapperItem.width > image.screenImageWidth) ? (wrapperItem.width - image.width) / 2 : (wrapperItem.width - image.screenImageWidth) / 2 + (wrapperItem.width - image.width) / 2
			maximumX: (wrapperItem.width > image.screenImageWidth) ? (wrapperItem.width - image.width) / 2 : (image.screenImageWidth - wrapperItem.width) / 2 + (wrapperItem.width - image.width) / 2
			minimumY: (wrapperItem.height > image.screenImageHeight) ? (wrapperItem.height - image.height) / 2 : (wrapperItem.height - image.screenImageHeight) / 2 + (wrapperItem.height - image.height) / 2
			maximumY: (wrapperItem.height > image.screenImageHeight) ? (wrapperItem.height - image.height) / 2 : (image.screenImageHeight - wrapperItem.height) / 2 + (wrapperItem.height - image.height) / 2
		}
	}
}
