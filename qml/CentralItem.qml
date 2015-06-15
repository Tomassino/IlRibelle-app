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

// The central item (i.e. the item occupying the majority of the screen). This
// is a StackView which displays the list of news on which other items are
// pushed. Here we take care of showing the news details when a news is
// selected, the main item also pushed other items (such as the configuration
// panel) when needed. This item should have focus to receive key events (e.g.
// to go back when back is pressed)
StackView {
	id: centralItem
	initialItem: listOfNews

	// The signal emitted when the user is at the starting item in the stack
	// and tries to pop again (e.g. by pressing the back button)
	signal attemptToPopLastItem()

	// The function popping one element from the stack. If there is only one
	// element in the stack the attemptToPopLastItem signal is emitted
	function goBack()
	{
		if (centralItem.depth > 1) {
			centralItem.pop()
		} else {
			attemptToPopLastItem()
		}
	}

	// The function to show the about screen
	function showAbout()
	{
		if ((aboutScreen.Stack.status !== Stack.Activating) && (aboutScreen.Stack.status !== Stack.Active)) {
			centralItem.push(aboutScreen)
		}
	}

	NewsList {
		id: listOfNews
		visible: true

		onNewsClicked: centralItem.push({item: news.roleValue("qmlItem"), properties: {news: news}})
	}

	AboutScreen {
		id: aboutScreen
		visible: false
	}

	delegate: StackViewDelegate {
		pushTransition: StackViewTransition {
			SequentialAnimation {
				PropertyAction {
					target: exitItem
					property: "mouseAreasEnabled"
					value: false
				}
				PropertyAction {
					target: enterItem
					property: "visible"
					value: false
				}
				NumberAnimation {
					target: exitItem
					property: "scaleFactor"
					duration: 400
					easing.type: Easing.InOutQuad
					from: 1.0
					to: 0.01
				}
				PropertyAction {
					target: exitItem
					property: "visible"
					value: false
				}
				PropertyAction {
					target: enterItem
					property: "scaleFactor"
					value: 0.01
				}
				PropertyAction {
					target: enterItem
					property: "mouseAreasEnabled"
					value: false
				}
				PropertyAction {
					target: enterItem
					property: "visible"
					value: true
				}
				NumberAnimation {
					target: enterItem
					property: "scaleFactor"
					duration: 400
					easing.type: Easing.InOutQuad
					from: 0.01
					to: 1.0
				}
				PropertyAction {
					target: enterItem
					property: "mouseAreasEnabled"
					value: true
				}
				ScriptAction {
					script: {
						// Here we call the backToNewsList() function of the controller object
						// to perform cleanup when the user goes back to the list of news
						if (centralItem.depth == 1) {
							backToNewsList()
						}
					}
				}
			}
		}
	}
}
