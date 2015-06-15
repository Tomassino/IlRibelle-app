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

#include "include/controller.h"

namespace {
	// Default values for some parameters
	const int defaultTTL = -1;
	const int fallbackTTL = 60; // This is used if the TTL from the channel is 0
	const unsigned int defaultKeepNewsForDays = 60;
	const qreal maxFontSize = 32.0;
}

Controller::~Controller()
{
	// Terminating the thread that generates the icons and deleting it
	if (m_iconsGenerator.isRunning()) {
		// We ignore the signals from the icon generator
		m_iconsGenerator.disconnect();

		// Asking to terminate the icons generation and waiting for the thread to finish
		m_iconsGenerator.interruptGeneration();
		m_iconsGenerator.wait();
	}

	// Here we delete the channel, channel updater, channel QObject properties object and model
	// explicitly because they must be destroyed before the singletons are destroyed
	m_newsModel.reset();
	m_channelRolesQMLAccessor.reset();
	m_channelUpdater.reset();
	m_channel.reset();

	// Deleting singletons
	NM::deleteInstance();
}

int Controller::ttl() const
{
	const int confTtl = m_settings.value("ttl", defaultTTL).toInt();

	if (confTtl == -1) {
		if (m_channel->standardRoles().getData<ChannelRoles::ttl>() == 0) {
			return fallbackTTL;
		} else {
			return m_channel->standardRoles().getData<ChannelRoles::ttl>();
		}
	} else {
		return confTtl;
	}
}

void Controller::setTtl(int t)
{
	const int confTtl = m_settings.value("ttl", defaultTTL).toInt();

	if (confTtl != t) {
		m_settings.setValue("ttl", t);

		// Updating the timer interval and emitting the signal
		setTimerInterval();
		emit ttlChanged();
	}
}

unsigned int Controller::keepNewsForDays() const
{
	return m_settings.value("keepNewsForDays", defaultKeepNewsForDays).toUInt();
}

void Controller::setKeepNewsForDays(unsigned int k)
{
	if (keepNewsForDays() != k) {
		m_settings.setValue("keepNewsForDays", k);

		emit keepNewsForDaysChanged();
	}
}

bool Controller::networkRequestsRunning() const
{
	return m_networkRequestsRunning;
}

bool Controller::canIncreaseFontSize() const
{
	return m_canIncreaseFontSize;
}

bool Controller::canDecreaseFontSize() const
{
	return m_canDecreaseFontSize;
}

qreal Controller::fontSize() const
{
	return m_settings.value("fontSize", m_initialFontSize).toReal();
}

QString Controller::aboutText() const
{
	// The file with the about text is stored the resources
	QFile aboutFile(m_aboutTextFilename);

	if (!aboutFile.open(QIODevice::ReadOnly)) {
		qDebug() << "Cannot read the file with the about text" << aboutFile.fileName();
	}

	return aboutFile.readAll();
}

void Controller::updateNews()
{
	// Storing the value of ttl before updating. If it changes and we are using it, emitting
	// the ttlChanged signal
	const unsigned int prevTtl = m_channel->standardRoles().getData<ChannelRoles::ttl>();

	// Updating news
	m_channelUpdater->update();

	// Checking if ttl changed and we are using the channel ttl and emitting a signal if this is true
	if ((prevTtl != m_channel->standardRoles().getData<ChannelRoles::ttl>()) && (m_settings.value("ttl", defaultTTL).toInt() == -1)) {
		// We also have to change the timer interval
		setTimerInterval();
		emit ttlChanged();
	}

	// Saving the time of the last update
	m_settings.setValue("lastUpdate", QDateTime::currentDateTime());
}

void Controller::clearAllNews()
{
	m_channelUpdater->clearAllNews();
}

void Controller::finalize()
{
	// Removing old news
	const int k = keepNewsForDays();
	const QDateTime date = QDateTime::currentDateTime().addDays(-k);
	m_channel->deleteNews(date);

	// Saving news if we can
	const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	if (!dataDir.isEmpty()) {
		// Checking the directory exists, otherwise trying to create it
		if (!QFileInfo(dataDir).exists()) {
			// Creating the directory
			QDir d = QDir::root();
			d.mkpath(dataDir);
		}
		if (QFileInfo(dataDir).isDir()) {
			// Opening the storage file for writing
			QFile file(dataDir + "/storednews.dat");
			if (file.open(QIODevice::WriteOnly)) {
				QJsonDocument document(m_channel->save());
				file.write(document.toBinaryData());
			} else {
				qDebug() << "Could not write data to" << file.fileName();
			}
		} else {
			qDebug() << "Cannot create data in storage dir" << dataDir;
		}
	} else {
		qDebug() << "Cannot find storage dir" << dataDir;
	}

	// Also forcing sync of settings, just to be sure
	m_settings.sync();

	// Finally removing all files that do not belong to the channel from the channel dataDir
	m_channel->deleteUnknownFiles();
}

void Controller::increaseFontSize()
{
	qreal fontSize = m_settings.value("fontSize", m_initialFontSize).toReal();

	if (fontSize < maxFontSize) {
		fontSize += 1.0;

		if (!m_canDecreaseFontSize) {
			m_canDecreaseFontSize = true;

			emit canDecreaseFontSizeChanged();
		}

		// Checking if font size can be increased further
		if (fontSize >= maxFontSize) {
			m_canIncreaseFontSize = false;

			emit canIncreaseFontSizeChanged();
		}

		// Storing font size
		m_settings.setValue("fontSize", fontSize);

		emit fontSizeChanged();
	}
}

void Controller::decreaseFontSize()
{
	qreal fontSize = m_settings.value("fontSize", m_initialFontSize).toReal();

	if (fontSize > m_initialFontSize) {
		fontSize -= 1.0;

		if (!m_canIncreaseFontSize) {
			m_canIncreaseFontSize = true;

			emit canIncreaseFontSizeChanged();
		}

		// Checking if font size can be increased further
		if (fontSize <= m_initialFontSize) {
			m_canDecreaseFontSize = false;

			emit canDecreaseFontSizeChanged();
		}

		// Storing font size
		m_settings.setValue("fontSize", fontSize);

		emit fontSizeChanged();
	}
}

void Controller::setInitialFontSize(qreal fontSize)
{
	// Saving the initial font size and allowing resizing
	m_initialFontSize = fontSize;

	qreal currFontSize = m_settings.value("fontSize", m_initialFontSize).toReal();

	// The curent font size must never be less than m_initialFontSize
	if (currFontSize < m_initialFontSize) {
		m_settings.setValue("fontSize", m_initialFontSize);
		currFontSize = m_initialFontSize;

		emit fontSizeChanged();
	}

	m_canIncreaseFontSize = false;
	m_canDecreaseFontSize = false;

	if (currFontSize > m_initialFontSize) {
		m_canDecreaseFontSize = true;
	}
	if (currFontSize < maxFontSize) {
		m_canIncreaseFontSize = true;
	}

	// Emitting the signals for increase and decrease fonts flags change
	emit canDecreaseFontSizeChanged();
	emit canIncreaseFontSizeChanged();
}

void Controller::getPNGFromVectorial(QString sourceImage, int imageDim, bool dimIsHeight, QJSValue callback)
{
	// Getting the index for this conversion request
	const unsigned int requestIndex = m_lastPNGGenerationIndex++;

	// Adding the index to the map
	m_PNGGenerationMap[requestIndex] = callback;

	// Submitting the request
	m_iconsGenerator.convertIcon(sourceImage, requestIndex, imageDim, dimIsHeight ? IconsGenerator::SpecifiedDimension::Height : IconsGenerator::SpecifiedDimension::Width);
}

QObject* Controller::getNewsForURL(QString newsUrl)
{
	auto accessorFromListModel = m_newsModel->getAccessorForNewsUrl(newsUrl);

	if (!accessorFromListModel) {
		// The news is not in the list recent news, we have to ask Channel to create
		// a temporary news (he caches recently requested news)
		return m_channel->getTemporaryNewsFromUrl(newsUrl);
	} else {
		return accessorFromListModel;
	}
}

void Controller::backToNewsList()
{
	// Cleaning the cache of temporary news
	m_channel->cleanTemporaryNewsCache();
}

void Controller::error(QString errorString)
{
	qDebug() << errorString;
}

void Controller::setNetworkRequestsRunning()
{
	if (!m_networkRequestsRunning) {
		m_networkRequestsRunning = true;

		emit networkRequestsRunningChanged();
	}
}

void Controller::unsetNetworkRequestsRunning()
{
	if (m_networkRequestsRunning) {
		m_networkRequestsRunning = false;

		emit networkRequestsRunningChanged();
	}
}

void Controller::iconGenerated(unsigned int index, QString filename)
{
	// Performing some safety checks
	if (Q_UNLIKELY(!m_PNGGenerationMap.contains(index))) {
		qDebug() << "INTERNAL ERROR: no icon with index" << index << "- Filename is" << filename;

		return;
	}

	if (!m_PNGGenerationMap[index].isCallable()) {
		qDebug() << "INTERNAL ERROR: the callback for index" << index << "is not callable!" << "- Filename is" << filename;

		return;
	}

	// Calling the QML callback. We have to add file:// because qml expects an URL
	QJSValue ret = m_PNGGenerationMap[index].call(QJSValueList() << ("file://" + filename));

	// Checking the callback was fine
	if (ret.isError()) {
		qDebug() << "INTERNAL ERROR: the callback for index" << index << "with filename" << filename << "returned error:" << ret.toString();
	}
}

void Controller::setTimerInterval()
{
	m_updateTimer.setInterval(ttl() * 60 * 1000);
}
