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

#ifndef __REMOTE_FILE_PROVIDER_H__
#define __REMOTE_FILE_PROVIDER_H__

#include <QObject>
#include <QUrl>
#include <QString>
#include <QFileSystemWatcher>
#include <QFile>
#include <QSet>
#include "include/dataavailablenotifee.h"

/**
 * \brief The class allowing to download a remote file
 *
 * This class allows to download a remote file. It checks if the file has aready
 * been download and, if so, provides the url of the file instead of the remote
 * one. For the moment resuming interrupted downloads do not work (it will
 * probably be added later). While downloading, data is stored in a file that
 * has the same path of the destination file with the .part suffix added. This
 * class should be registered with the QML engine so that it can be used from
 * QML. When this class is destroyed unfinished downloads are interrupted. You
 * should create instances using RemoteFileProviderFactory so that download can
 * continue in background
 */
class RemoteFileProvider : public QObject, private DataArrivedNotifee
{
	Q_OBJECT
	Q_ENUMS(States Error)
	Q_PROPERTY(States status READ status NOTIFY statusChanged)
	Q_PROPERTY(QUrl remoteUrl READ remoteUrl WRITE setRemoteUrl NOTIFY remoteUrlChanged)
	Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
	Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)
	Q_PROPERTY(QUrl resourceUrl READ resourceUrl NOTIFY resourceUrlChanged)
	Q_PROPERTY(Error error READ error NOTIFY errorChanged)

public:
	/**
	 * \brief The possible states of the object
	 */
	enum States {
		NoDownload, /// No download is running and the file hasn't been
			    /// downloaded yet
		Downloading, /// We are downloading the file
		DownloadInterrupted, /// The download has been interrupted and
				     /// the file is partially downloaded
		Downloaded /// The file has been downloaded
	};

	/**
	 * \brief The possible errors
	 */
	enum Error {
		NoError, /// No error
		NetworkError, /// Error downloading the file
		PartFileRemovedError, /// The file with partially downloaded
				      /// data has been removed
		ChangeUrlOrFileWhileDownloading, /// Attempt to change the
						 /// resource url or the
						 /// destination file while
						 /// downloading
		CannotCreateFile /// Cannot create the resource destination file
				 /// or the file with partial download
	};

public:
	/**
	 * \brief Constructor
	 *
	 * \param the parent object
	 */
	RemoteFileProvider(QObject* parent = nullptr);

	/**
	 * \brief Destructor
	 */
	virtual ~RemoteFileProvider();

	/**
	 * \brief Returns the current status of the object
	 *
	 * \return the current status of the object
	 */
	States status() const
	{
		return m_status;
	}

	/**
	 * \brief Returns the remote url from where the file is downloaded
	 *
	 * \return the remote url from where the file is downloaded
	 */
	QUrl remoteUrl() const
	{
		return m_remoteUrl;
	}

	/**
	 * \brief Sets the remote url from where the file is downloaded
	 *
	 * An attempt to change this while the status is not NoDownload will
	 * result in a ChangeUrlOrFileWhileDownloading error (the url will NOT
	 * be changed)
	 * \param url the remote url from where the file is downloaded
	 */
	void setRemoteUrl(QUrl url);

	/**
	 * \brief Returns the path of the file where the remote resource is
	 *        downloaded
	 *
	 * \return the path of the file where the remote resource is downloaded
	 */
	QString filePath() const
	{
		return m_filePath;
	}

	/**
	 * \brief Sets the path of the file where the remote resource is
	 *        downloaded
	 *
	 * An attempt to change this while the status is not NoDownload will
	 * result in a ChangeUrlOrFileWhileDownloading error (the file path will
	 * NOT be changed). The directory where the file is to be created must
	 * exist before calling this function. If the file exists when this
	 * function is called this also changes the status to Downloaded
	 * \param filePath the path of the file where the remote resource is
	 *                 downloaded
	 */
	void setFilePath(QString filePath);

	/**
	 * \brief Returns the current progress status of download
	 *
	 * This is a number from 0 to 100 where 0 means that nothing has been
	 * downloaded yet and 100 means that the file is fully donwloaded. The
	 * value is valid only if the status id Downloading or
	 * DownloadInterrupted
	 * \return the download progress status between 0 and 100
	 */
	int downloadProgress()
	{
		return m_downloadProgress;
	}

	/**
	 * \brief Returns the URL to use to access the resource
	 *
	 * This is equal to the remote url if the file hasn't been downloaded,
	 * it is equal to the file path if the file has been downloaded
	 * \return the URL to use to access the resource
	 */
	QUrl resourceUrl();

	/**
	 * \brief Reurns the current error status
	 *
	 * \return the current error status
	 */
	Error error()
	{
		return m_error;
	}

public slots:
	/**
	 * \brief Starts the download or resumes it if it has been interrupted
	 *
	 * \note For the moment resuming simply restarts the download from the
	 *       beginnig
	 */
	void startDownload();

	/**
	 * \brief Interrupts the download
	 *
	 * This does not remove the downloaded file
	 */
	void interruptDownload();

	/**
	 * \brief Interrupts the download if running and removes the downloaded
	 *        file
	 *
	 * This resets the status to NoDownload
	 */
	void removeDownload();

signals:
	/**
	 * \brief The signal emitted when the status of this object changes
	 */
	void statusChanged();

	/**
	 * \brief The signal emitted when the remote url changes
	 */
	void remoteUrlChanged();

	/**
	 * \brief The signal emitted when the path of the file where the remote
	 *        resource is downloaded changes
	 */
	void filePathChanged();

	/**
	 * \brief The signal emitted when the download progress status changes
	 */
	void downloadProgressChanged();

	/**
	 * \brief The signal emitted when the URL to use to access the resource
	 *        changes
	 */
	void resourceUrlChanged();

	/**
	 * \brief The signal emitted when the error status changes
	 *
	 * \note This is emitted also when the error status changes back to
	 *       NoError
	 */
	void errorChanged();

private slots:
	/**
	 * \brief The slot called when the downloaded file changes
	 *
	 * This is used to change status in case the file is removed
	 * \param path the path of the file that changed
	 */
	void downloadedFileChanged(const QString& path);

private:
	/**
	 * \brief The function called to handle the arrived data
	 *
	 * This is called every time new data is available
	 * \param id the request ID
	 * \param data the data that has just arrived
	 */
	virtual void dataArrived(int id, const QByteArray& data);

	/**
	 * \brief The function called when all data is available
	 *
	 * \param id the request ID
	 */
	virtual void allDataAvailable(int id);

	/**
	 * \brief Called in case of network errors
	 *
	 * \param id the request ID
	 * \param description the description of the error
	 */
	virtual void networkError(int id, const QString& description);

	/**
	 * \brief Sets the status flag and emits the changed signal
	 *
	 * This also emits the resourceUrlChanged() signal if the new status
	 * is Downloaded
	 * param status the new status
	 */
	void setStatus(States status);

	/**
	 * \brief Sets the error flag and emits the changed signal
	 *
	 * The signal is always emitted, even if the actual value of the error
	 * variable hasn't changed (e.g. because the previous error status was
	 * the same as the new error status). The only exception is that if the
	 * previous error status was NoError and the new one is still NoError,
	 * no signal is emitted
	 * \param error the new error code
	 */
	void setError(Error error);

	/**
	 * \brief The current status
	 */
	States m_status;

	/**
	 * \brief The remote url from which the remote resource can be
	 *        downloaded
	 */
	QUrl m_remoteUrl;

	/**
	 * \brief The path of the file where the remote resource is downloaded
	 */
	QString m_filePath;

	/**
	 * \brief The current download progress status
	 *
	 * This is a number from 0 to 100
	 */
	int m_downloadProgress;

	/**
	 * \brief The current error status
	 */
	Error m_error;

	/**
	 * \brief The object to watch for changes in the downloaded file
	 *
	 * We use this to change status if the downloaded file is deleted
	 */
	QFileSystemWatcher m_fileWatcher;

	/**
	 * \brief The file where partially downloaded data is written
	 */
	QFile m_partFile;
};

#endif
