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

#include "include/remotefileprovider.h"
#include "include/networkmanager.h"
#include <QFileInfo>

RemoteFileProvider::RemoteFileProvider(QObject* parent)
	: QObject(parent)
	, DataArrivedNotifee()
	, m_status(NoDownload)
	, m_remoteUrl()
	, m_filePath()
	, m_downloadProgress(0)
	, m_error(NoError)
	, m_fileWatcher()
	, m_partFile()
{
	// Connecting the signal from the file watcher
	connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &RemoteFileProvider::downloadedFileChanged);
}

RemoteFileProvider::~RemoteFileProvider()
{
	if (Downloading == m_status) {
		interruptRequest(17);
	}
}

void RemoteFileProvider::setRemoteUrl(QUrl url)
{
	if (NoDownload != m_status) {
		setError(ChangeUrlOrFileWhileDownloading);

		return;
	}

	// Resetting error
	setError(NoError);

	if (url != m_remoteUrl) {
		m_remoteUrl = url;

		emit remoteUrlChanged();

		// We can get here only if the file is not downloaded, so we have to also signal
		// the change in the audioResource property
		emit resourceUrlChanged();
	}
}

void RemoteFileProvider::setFilePath(QString filePath)
{
	if (NoDownload != m_status) {
		setError(ChangeUrlOrFileWhileDownloading);

		return;
	}

	// Resetting error
	setError(NoError);

	if (filePath != m_filePath) {
		// Removing the old file from the list of watched files
		if (!m_filePath.isEmpty()) {
			m_fileWatcher.removePath(m_filePath);
		}

		m_filePath = filePath;

		emit filePathChanged();

		// Checking if the file exists and, if so, changing status and adding the file to the
		// list of files to watch
		QFileInfo info(m_filePath);
		if (info.isFile() && info.isReadable()) {
			m_fileWatcher.addPath(m_filePath);

			// Here we also have to check if the .part file exists: if so, the download is partial
			QFileInfo info(m_filePath + ".part");
			if (info.isFile() && info.isReadable()) {
				setStatus(DownloadInterrupted);
			} else {
				setStatus(Downloaded);
			}
		}
	}
}

QUrl RemoteFileProvider::resourceUrl()
{
	QUrl retUrl = m_remoteUrl;

	if (m_status == Downloaded) {
		// Adding a safety check, just to be sure
		QFileInfo info(m_filePath);
		if (info.isFile() && info.isReadable()) {
			retUrl = QUrl("file://" + m_filePath);
		} else {
			// We should not get here (because we watch the file for changes). If we do,
			// we change the status and return the remote url
			QFileInfo info(m_filePath + ".part");
			if (info.isFile() && info.isReadable()) {
				setStatus(DownloadInterrupted);
			} else {
				setStatus(NoDownload);
			}
		}
	}

	return retUrl;
}

void RemoteFileProvider::startDownload()
{
	if (!m_remoteUrl.isValid()) {
		return;
	}

	if ((Downloading == m_status) || (Downloaded == m_status)) {
		return;
	}

	// For the moment, there is no resume support

	// Resetting the progress indicator
	m_downloadProgress = 0;
	emit downloadProgressChanged();

	// Creating the output file. Here we just create an empty file
	QFile destFile(m_filePath);
	if (!destFile.open(QFile::WriteOnly | QFile::Truncate)) {
		setError(CannotCreateFile);

		return;
	}

	// Creating the part file
	m_partFile.setFileName(m_filePath + ".part");
	if (!m_partFile.open(QFile::WriteOnly | QFile::Truncate)) {
		setError(CannotCreateFile);

		return;
	}

	// We use 17 as the ID of all our requests (which are never parallel)
	const bool ret = NM::instance().getFile(m_remoteUrl, this, 17);

	// We chack the unlikely event that ret is false, just for debug purpouse
	if (Q_UNLIKELY(!ret)) {
		qDebug() << "Internal error, a request with the id already exists in RemoteFileProvider";
	}

	setStatus(Downloading);
}

void RemoteFileProvider::interruptDownload()
{
	if (Downloading == m_status) {
		interruptRequest(17);

		setStatus(DownloadInterrupted);
		m_partFile.close();
	}
}

void RemoteFileProvider::removeDownload()
{
	// Interrupting download if running
	interruptDownload();

	// Removing both the audio file and the part file
	QFile::remove(m_filePath);
	QFile::remove(m_filePath + ".part");

	// Setting the status to NoDownload and resetting the error flag
	setStatus(NoDownload);
	setError(NoError);
}

void RemoteFileProvider::downloadedFileChanged(const QString& path)
{
	if (path == m_filePath) {
		// We only need to know if the file has been removed to change the status
		if (!QFileInfo(m_filePath).exists()) {
			setStatus(NoDownload);
		}
	} else {
		qDebug() << "Received notification for unexpected file" << path;
	}
}

void RemoteFileProvider::dataArrived(int /*id*/, const QByteArray& data)
{
	// Appending to the part file
	m_partFile.write(data);

	// Update the download progress
	const float partSize = m_partFile.size();
	const float totalSize = reply(17)->header(QNetworkRequest::ContentLengthHeader).toInt();
	m_downloadProgress = int((partSize / totalSize) * 100.0);
	emit downloadProgressChanged();
}

void RemoteFileProvider::allDataAvailable(int /*id*/)
{
	// Setting download progress to 100
	m_downloadProgress = 100;
	emit downloadProgressChanged();

	// Closing the part file and renaming it
	m_partFile.close();
	QFile::remove(m_filePath);

	if (!QFile::rename(m_filePath + ".part", m_filePath)) {
		setError(CannotCreateFile);
	} else {
		setStatus(Downloaded);
	}
}

void RemoteFileProvider::networkError(int id, const QString& description)
{
	qDebug() << "Network error in RemoteFileProvider for url" << m_remoteUrl << "id" << id << "reason" << description;

	// Setting the error status
	setError(NetworkError);
}

void RemoteFileProvider::setStatus(States status)
{
	if (m_status != status) {
		m_status = status;

		emit statusChanged();

		// Checking if we also have to emit the signal for audioResouce change
		if (m_status == Downloaded){
			emit resourceUrlChanged();
		}
	}
}

void RemoteFileProvider::setError(Error error)
{
	if ((NoError == m_error) && (NoError == error)) {
		return;
	}

	m_error = error;

	emit errorChanged();
}
