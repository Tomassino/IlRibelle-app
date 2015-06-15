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
import QtMultimedia 5.4
import com.ilribelle 1.0

// This is the player for audio resources from Raz24. Here we use a
// RemoteFileProvider object to get the resource (either directly remotely or
// via a downloaded file)
Rectangle {
	id: mainItem

	// We use dummy button to compute our height
	height: dummyButton.height * 2.1

	// The remote url of the audio resource
	property url remoteUrl

	// The file where the audio resource can be saved
	property string filePath

	// The signal emitted when there is an error
	signal error(string reason)

	// A dummy button (never shown), just to get the default height
	Button {
		id: dummyButton
		visible: false
		text: "Dummy"
	}

	// The object playing the audio
	Audio {
		id: player

		autoLoad: true
		autoPlay: false
		volume: 1.0

		onErrorChanged: showError()

		function showError()
		{
			if (error === Audio.NoError) {
				console.log("Audio error: NoError")
			} else if (error === Audio.ResourceError) {
				console.log("Audio error: ResourceError")
			} else if (error === Audio.FormatError) {
				console.log("Audio error: FormatError")
			} else if (error === Audio.NetworkError) {
				console.log("Audio error: NetworkError")
			} else if (error === Audio.AccessDenied) {
				console.log("Audio error: AccessDenied")
			} else if (error === Audio.ServiceMissing) {
				console.log("Audio error: ServiceMissing")
			} else {
				console.log("Audio error: Unknown error")
			}

			console.log("Audio errorString" + errorString)
		}
	}

	// The panel to choose whether to download the file or directly play it
	Item {
		id: actionSelectionPanel
		anchors.fill: parent
		visible: (!internal.forceShowPlayer) &&
			 ((internal.curProvider === null) ||
			  (internal.curProvider.status === RemoteFileProvider.NoDownload))

		// Here I use items around buttons so that buttons get the same size no matter the
		// size of the text
		RowLayout {
			anchors.fill: parent

			Item {
				Layout.fillWidth: true
				Layout.fillHeight: true

				Button {
					anchors.fill: parent
					text: qsTr("Download")

					onClicked: {
						if (internal.curProvider !== null) {
							internal.curProvider.startDownload()
						}
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.fillHeight: true

				Button {
					anchors.fill: parent
					text: qsTr("Listen directly")

					onClicked: internal.forceShowPlayer = true
				}
			}
		}
	}

	// The panel to show download progress and to control it
	Item {
		id: downloadPanel
		anchors.fill: parent
		visible: (!internal.forceShowPlayer) &&
			 (internal.curProvider !== null) &&
			 ((internal.curProvider.status === RemoteFileProvider.Downloading) ||
			  (internal.curProvider.status === RemoteFileProvider.DownloadInterrupted))

		// Here I use items around buttons so that buttons get the same size no matter the
		// size of the text
		GridLayout {
			anchors.fill: parent
			columns: 2
			flow: GridLayout.LeftToRight

			ProgressBar {
				id: downloadProgressBar

				Layout.columnSpan: 2
				Layout.fillWidth: true
				visible: true
				minimumValue: 0
				maximumValue: 100
				orientation: Qt.Horizontal
				indeterminate: false
			}

			Item {
				Layout.fillWidth: true
				Layout.fillHeight: true

				Button {
					anchors.fill: parent
					text: ((internal.curProvider !== null) && (internal.curProvider.status === RemoteFileProvider.Downloading)) ? qsTr("Pause Download") : qsTr("Resume Download")

					onClicked: {
						if (internal.curProvider !== null) {
							if (internal.curProvider.status === RemoteFileProvider.Downloading) {
								internal.curProvider.interruptDownload()
							} else {
								internal.curProvider.startDownload()
							}
						}
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.fillHeight: true

				Button {
					anchors.fill: parent
					text: qsTr("Cancel Download")

					onClicked: {
						if (internal.curProvider !== null) {
							internal.curProvider.removeDownload()
						}
					}
				}
			}
		}
	}

	// The panel to show the audio player
	Item {
		id: playerPanel
		anchors.fill: parent
		visible: (internal.forceShowPlayer) ||
			 ((internal.curProvider !== null) &&
			  (internal.curProvider.status === RemoteFileProvider.Downloaded))


		// Here I use items around buttons so that buttons get the same size no matter the
		// size of the text
		GridLayout {
			anchors.fill: parent
			columns: 2
			flow: GridLayout.LeftToRight

			Item {
				Layout.columnSpan: 2
				Layout.fillWidth: true
				Layout.fillHeight: true

				RowLayout {
					anchors.fill: parent

					Text {
						id: audioCurTime

						text: internal.formatTime(player.position)
					}

					Slider {
						id: audioPositionSlider

						Layout.fillWidth: true
						visible: true
						minimumValue: 0
						maximumValue: player.duration
						orientation: Qt.Horizontal
						tickmarksEnabled: false
						enabled: player.seekable
						value: player.position
					}

					Text {
						id: audioTotalLength

						text: internal.formatTime(player.duration)
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.fillHeight: true

				Button {
					anchors.fill: parent
					text: (player.playbackState === Audio.PlayingState) ? qsTr("Pause") : qsTr("Play")

					onClicked: {
						if (player.playbackState === Audio.PlayingState) {
							player.pause()
						} else {
							player.play()
						}
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.fillHeight: true

				Button {
					anchors.fill: parent
					text: qsTr("Stop")

					onClicked: {
						internal.forceShowPlayer = false
						player.stop()
					}
				}
			}
		}
	}

	// This object contains internal data
	QtObject {
		id: internal

		// When true the player is shown regardless of the status of the audio provider
		property bool forceShowPlayer: false

		// The RemoteFileProvider object we are currenty using
		property var curProvider: null

		// The function called when there is and error in an audio provider
		function providerError()
		{
			if (curProvider.error === RemoteFileProvider.NoError) {
				return
			}

			// Emitting the error signal. If this is called by a provider that is not the current
			// one also remove the provider at the end
			if (curProvider.error === RemoteFileProvider.NetworkError) {
				mainItem.error(qsTr("Network error while getting ") + curProvider.audioResourceTitle + qsTr(" from the audio provider"))
			} else if (curProvider.error === RemoteFileProvider.PartFileRemovedError) {
				mainItem.error(qsTr("Removed partial file when getting ") + curProvider.audioResourceTitle + qsTr(" from the audio provider"))
			} else if (curProvider.error === RemoteFileProvider.ChangeUrlOrFileWhileDownloading) {
				mainItem.error(qsTr("INTERNAL ERROR: changed url or file when getting ") + curProvider.audioResourceTitle + qsTr(" from the audio provider"))
			} else if (curProvider.error === RemoteFileProvider.CannotCreateFile) {
				mainItem.error(qsTr("The audio provider could not create file when getting ") + curProvider.audioResourceTitle)
			} else {
				mainItem.error(qsTr("Unknown error from the audio provider for ") + curProvider.audioResourceTitle)
			}
		}

		// This function takes a time in milliseconds and returns it in
		// format mm:ss
		function formatTime(curTime)
		{
			var s = Math.round((curTime / 1000) % 60)
			var m = Math.round(curTime / 60000)

			var t
			if (s < 10) {
				t = ":0" + s.toString()
			} else {
				t = ":" + s.toString()
			}
			if (m < 10) {
				t = "0" + m.toString() + t
			} else {
				t = m.toString() + t
			}

			return t;
		}
	}

	// When the remote url changes, we change the current provider to one that handles the
	// new remote url
	onRemoteUrlChanged: {
		// Disconnecting signal of old provider, if it exists
		if (internal.curProvider !== null) {
			internal.curProvider.onErrorChanged.disconnect(internal.providerError)
		}

		// Getting the remote file provider
		internal.curProvider = RemoteFileProviderFactory.getRemoteFileProvider(mainItem, remoteUrl)

		// We set this to false to revert to the standard behaviour on panel visibility
		internal.forceShowPlayer = false

		// Now connecting the error callback
		internal.curProvider.onErrorChanged.connect(internal.providerError)

		// Finally binding some properties
		internal.curProvider.remoteUrl = Qt.binding(function() { return mainItem.remoteUrl })
		internal.curProvider.filePath = Qt.binding(function() { return mainItem.filePath })
		player.source = Qt.binding(function() { return internal.curProvider.resourceUrl })
		downloadProgressBar.value = Qt.binding(function() { return internal.curProvider.downloadProgress })
	}

	Component.onDestruction: {
		// Explicitly disconnecting the signal, this doesn't seem to happend
		// automatically (perhaps a bug?)
		if (internal.curProvider !== null) {
			internal.curProvider.onErrorChanged.disconnect(internal.providerError)
		}
	}
}
