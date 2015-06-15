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
import com.ilribelle 1.0

// A simple item to show the details of the news. For the moment we only show
// the text of the news as rich text
CentralSubItem {
	id: newsDetail

	// This contains the whole news
	property var news

	ColumnLayout {
		spacing: 0
		anchors.fill: parent

		ScrollView {
			Layout.fillWidth: true
			Layout.fillHeight: true
			//Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

			Flickable {
				id: flickableArea
				anchors.fill: parent
				anchors.leftMargin: 1*mm
				anchors.rightMargin: 1*mm
				contentWidth: newsTextItem.width
				contentHeight: newsTextItem.height
				flickableDirection: Flickable.VerticalFlick

				Text {
					id: newsTextItem
					wrapMode: Text.Wrap
					width: newsDetail.width - (flickableArea.anchors.leftMargin + flickableArea.anchors.rightMargin)
					textFormat: Text.RichText
					font.pointSize: fontSize
					baseUrl: "http://www.ilribelle.com"

					onLinkActivated: {
						if (!mouseAreasEnabled) {
							return;
						}

						if (link.search(/^image\+/) != -1) {
							newsDetail.Stack.view.push({item: "qrc:///qml/IlRibelleImage.qml", properties: {imageSource: link.slice(6)}})
						} else {
							var actualLink = link

							// If the link starts with /, it is interpreted as absolute in the
							// resource tree (because this is in the resources). However, as it
							// comes from a webpage, we should interpret it as relative to the
							// base Url
							if (link.trim().slice(0, 1) === "/") {
								actualLink = baseUrl + link
							}

							var newsToShow = getNewsForURL(actualLink)

							if (newsToShow === null) {
								Qt.openUrlExternally(actualLink)
							} else {
								newsDetail.Stack.view.push({item: newsToShow.roleValue("qmlItem"), properties: {news: newsToShow}})
							}
						}
					}

					onWidthChanged: parseNewsText()
				}
			}
		}

		RowLayout {
			Layout.fillWidth: true

			// A button to share the news with other means
			Button {
				id: otherShareButton
				visible: true
				text: qsTr("Share news")
				height: 50

				Layout.fillWidth: true

				onClicked: MiscNative.share(news.roleValue("link"), news.roleValue("title"), qsTr("Share link"))
			}
		}

		// We use a loader to avoid unnecessary loading of a rather "heavy" component
		Loader {
			id: audioPlayerLoader

			Layout.fillWidth: true

			source: news.roleValue("hasAudioResource") ? "qrc:///qml/RazAudioPlayer.qml" : ""

			// If we load the player, we also set some of its properties
			onLoaded: {
				item.remoteUrl = newsDetail.news.roleValue("audioResourceUrl")
				item.filePath = newsDetail.news.roleValue("audioResourcePath")

				item.color = "white"
				height = item.height
			}
		}
	}

	// The function called to parse the content of newsText and substitute
	// tags with actual values
	function parseNewsText()
	{
		if (news === null) {
			return;
		}

		var tmp = news.roleValue("description").replace(/<\$BIGIMAGEWIDTH\$>/g, newsTextItem.width / 2)
		newsTextItem.text = tmp.replace(/<\$SMALLIMAGEWIDTH\$>/g, newsTextItem.width / 4)

		if (news.roleValue("hasLivestreamLink")) {
			newsTextItem.text = newsTextItem.text + "<br/><h3><center><a href=\"" + news.roleValue("livestreamUrl") + "\">" + qsTr("Watch the stream") + "</a></center></h3>"
		}
	}

	onNewsChanged: parseNewsText()

	function sharingOk()
	{
		console.log("Sharing OK!")
	}

	function sharingFailed()
	{
		console.log("Sharing Failed!")
	}

	// We need to connect signals from Facebook and Twitter
	Component.onCompleted: {
		MiscNative.operationDone.connect(sharingOk)
		MiscNative.error.connect(sharingFailed)
	}
}
