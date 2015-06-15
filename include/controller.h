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

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QJSValue>
#include <QMap>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QtDebug>
#include <QStringList>
#include <QScreen>
#include <memory>
#include "include/channel.h"
#include "include/channelupdater.h"
#include "include/newslistmodel.h"
#include "include/iconsgenerator.h"
#include "include/utilities.h"
#include "include/networkmanager.h"
#include "include/rolesqmlaccessor.h"

/**
 * \brief The class that controls what happends behind the scenes
 *
 * This is responsible of managing the local storage, configuration, syncing
 * data ad so on. The properties of this class are meant to be exposed to QML.
 *
 * The parameters this class stores and reads are:
 * 	- lastUpdate: the date and time of the last time news were updated
 * 	- ttl: the number of minutes between updates. If -1 uses the channel ttl
 * 	       value
 * 	- keepNewsForDays: the number of days old news are kept. If 0 news are
 * 	                   not stored
 *	- fontSize: the size of fonts used in the application
 *
 * News are stored in binary JSON format (Qt format) in a file called "storage"
 * in the writable QStandardPaths::DataLocation. This class is also responsible
 * for generating all the icons at the correct resolution from the svg files
 * stored as resources once the application starts
 */
class Controller : public QObject
{
	Q_OBJECT
	Q_PROPERTY(AbstractRolesQMLAccessor* channel READ channel NOTIFY channelChanged)
	Q_PROPERTY(AbstractNewsListModel* newsModel READ newsModel NOTIFY newsModelChanged)
	Q_PROPERTY(int ttl READ ttl WRITE setTtl NOTIFY ttlChanged)
	Q_PROPERTY(unsigned int keepNewsForDays READ keepNewsForDays WRITE setKeepNewsForDays NOTIFY keepNewsForDaysChanged)
	Q_PROPERTY(bool networkRequestsRunning READ networkRequestsRunning NOTIFY networkRequestsRunningChanged)
	Q_PROPERTY(bool canIncreaseFontSize READ canIncreaseFontSize NOTIFY canIncreaseFontSizeChanged)
	Q_PROPERTY(bool canDecreaseFontSize READ canDecreaseFontSize NOTIFY canDecreaseFontSizeChanged)
	Q_PROPERTY(qreal fontSize READ fontSize NOTIFY fontSizeChanged)
	Q_PROPERTY(QString aboutText READ aboutText NOTIFY aboutTextChanged)
	Q_PROPERTY(qreal mm READ mm NOTIFY mmChanged)

public:
	/**
	 * \brief Constructor
	 *
	 * The template parameter is the type of the channel
	 * \param dummy needed to resolve the template type for the channel
	 *              (there is no way to call template constructor with
	 *              explicit template parameters)
	 * \param dummy2 needed to resolve the template type for the channel
	 *               updater (there is no way to call template constructor
	 *               with explicit template parameters)
	 * \param channelName the name of the channel. This is used to generate
	 *                    the path for the channel data directory
	 * \param channelURL the url of the channel (i.e. of the RSS feed for
	 *                   the channel)
	 * \param aboutTextFilename the path of the HTML page to use on the
	 *                          about page
	 * \param parent the parent object
	 */
	template <class ChannelType, class ChannelUpdaterType>
	Controller(Type2Type<ChannelType> dummy, Type2Type<ChannelUpdaterType> dummy2, QString channelName, QUrl channelURL, QString aboutTextFilename, QObject* parent = nullptr)
	      : QObject(parent)
	      , m_aboutTextFilename(aboutTextFilename)
	      , m_settings()
	      , m_channel(std::make_unique<ChannelType>(channelURL, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + channelName))
	      , m_channelUpdater(std::make_unique<ChannelUpdaterType>(static_cast<ChannelType*>(m_channel.get()), this)) // The cast here won't fail for sure
	      , m_channelRolesQMLAccessor(std::make_unique<RolesQMLAccessor<ChannelType>>(static_cast<ChannelType*>(m_channel.get()), nullptr)) // The cast here won't fail for sure
	      , m_newsModel(std::make_unique<NewsListModel<ChannelType>>(static_cast<ChannelType*>(m_channel.get()))) // The cast here won't fail for sure
	      , m_iconsGenerator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/icons")
	      , m_updateTimer()
	      , m_networkRequestsRunning(false)
	      , m_lastPNGGenerationIndex(0)
	      , m_PNGGenerationMap()
	      , m_canIncreaseFontSize(false)
	      , m_canDecreaseFontSize(false)
	      , m_initialFontSize(-1.0)
	      , m_mm(qApp->primaryScreen()->physicalDotsPerInch() / 25.4)
	{
		Q_UNUSED(dummy)
		Q_UNUSED(dummy2)

		// Reloading data from disk if the storage file exists
		const QString dataFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, "storednews.dat", QStandardPaths::LocateFile);
		if (!dataFile.isEmpty()) {
			QFile file(dataFile);
			if (file.open(QIODevice::ReadOnly)) {
				QJsonDocument document = QJsonDocument::fromBinaryData(file.readAll());
				if (document.isObject()) {
					if (!m_channel->load(document.object())) {
						qDebug() << "Error reloading data from" << dataFile << "(Channel::load returned false)";
					}
				} else {
					qDebug() << "Error reloading data from" << dataFile << "(JSon document is not an object)";
				}
			} else {
				qDebug() << "Error reloading data from" << dataFile << "(cannot open file with JSon data)";
			}
		} else {
			qDebug() << "No previous data found";
		}

		// Connecting signals
		connect(m_channelUpdater.get(), &ChannelUpdaterType::error, this, &Controller::error);
		connect(&m_updateTimer, &QTimer::timeout, this, &Controller::updateNews);
		connect(&(NM::instance()), &NetworkManager::networkRequestsStarted, this, &Controller::setNetworkRequestsRunning);
		connect(&(NM::instance()), &NetworkManager::networkRequestsEnded, this, &Controller::unsetNetworkRequestsRunning);
		connect(&(NM::instance()), &NetworkManager::networkError, this, &Controller::networkError);
		connect(&m_iconsGenerator, &IconsGenerator::iconGenerated, this, &Controller::iconGenerated);

		// Starting the thread of the icon generator
		m_iconsGenerator.start();

		// Now updating news from the net...
		updateNews();

		// ... and then starting the timer for the news updates
		setTimerInterval();
		m_updateTimer.start();

		emit channelChanged();
		emit newsModelChanged();
		emit aboutTextChanged();
		emit mmChanged();
	}

	/**
	 * \brief Destructor
	 */
	virtual ~Controller();

	/**
	 * \brief Returns the QObject with the dynamic properties of the channel
	 *
	 * \return the QObject with the dynamic properties of the channel
	 */
	AbstractRolesQMLAccessor* channel()
	{
		return m_channelRolesQMLAccessor.get();
	}

	/**
	 * \brief Returns the model for the list of news
	 *
	 * \return the model for the list of news
	 */
	AbstractNewsListModel* newsModel()
	{
		return m_newsModel.get();
	}

	/**
	 * \brief Returns the time in minutes between two successive updates of
	 *        news
	 *
	 * \return the time in minutes between two successive updates of news
	 */
	int ttl() const;

	/**
	 * \brief Sets the time in minutes between two successive updates of
	 *        news
	 *
	 * \param t the time in minutes between two successive updates of news.
	 *          If -1 uses the ttl value from the channel
	 */
	void setTtl(int t);

	/**
	 * \brief Returns the number of days old news are kept. If 0 news are
	 *        not stored
	 *
	 * \return the number of days old news are kept. If 0 news are not
	 *         stored
	 */
	unsigned int keepNewsForDays() const;

	/**
	 * \brief Sets the number of days old news are kept. If 0 news are not
	 *        stored
	 *
	 * \param k the number of days old news are kept. If 0 news are not
	 *          stored
	 */
	void setKeepNewsForDays(unsigned int keepNewsForDays);

	/**
	 * \brief Returns true if there are network requests running
	 *
	 * \return true if there are network requests running, false otherwise
	 */
	bool networkRequestsRunning() const;

	/**
	 * \brief Returns the current busy indicator icon height
	 *
	 * \return the current busy indicator icon height
	 */
	int busyIndicatorIconHeight() const;

	/**
	 * \brief Sets the value of the busy indicator icon height property
	 *
	 * \param busyIndicatorIconHeight the new value of the busy indicator
	 *                                icon height property
	 */
	void setBusyIndicatorIconHeight(int busyIndicatorIconHeight);

	/**
	 * \brief Returns the path of the icon for the busy indicator
	 *
	 * \return the path of the icon for the busy indicator
	 */
	QString busyIndicatorIcon() const;

	/**
	 * \brief Returns the current toolbar icons height
	 *
	 * \return the current toolbar icons height
	 */
	int toolbarIconHeight() const;

	/**
	 * \brief Sets the value of the toolbar icon height property
	 *
	 * \param toolbarIconHeight the new value of the toolbar icon height
	 *                          property
	 */
	void setToolbarIconHeight(int toolbarIconHeight);

	/**
	 * \brief Returns the path of the icon for the toolbar back button
	 *
	 * \return the path of the icon for the toolbar back button
	 */
	QString toolbarBackIcon();

	/**
	 * \brief Returns the path of the icon for the toolbar menu button
	 *
	 * \return the path of the icon for the toolbar menu button
	 */
	QString toolbarMenuIcon();

	/**
	 * \brief Returns true if font size can be increased
	 *
	 * \return true if font size can be increased
	 */
	bool canIncreaseFontSize() const;

	/**
	 * \brief Returns true if font size can be decreased
	 *
	 * \return true if font size can be decreased
	 */
	bool canDecreaseFontSize() const;

	/**
	 * \brief Returns the current font size
	 *
	 * \return the current font size
	 */
	qreal fontSize() const;

	/**
	 * \brief Reads the about text from file and returns it
	 *
	 * \return the about text
	 */
	QString aboutText() const;

	/**
	 * \brief Returns the number of points in a millimiter
	 *
	 * This is exposed in QML via the mm property. Multiply lengths in
	 * millimiters by this to get the length in points, usable them as
	 * absolute lengths of items
	 * \return the number of points in a millimiter
	 */
	qreal mm() const
	{
		return m_mm;
	}

signals:
	/**
	 * \brief The signal emitted when there is a network error
	 *
	 * \param reason the description of the error
	 */
	void networkError(QString reason);

	/**
	 * \brief The signal emmitted when the channel changes
	 */
	void channelChanged();

	/**
	 * \brief The signal emitted when the newsModel changes
	 */
	void newsModelChanged();

	/**
	 * \brief The signal emitted when the ttl changes
	 */
	void ttlChanged();

	/**
	 * \brief The signal emitted when keepNewsForDays changes
	 */
	void keepNewsForDaysChanged();

	/**
	 * \brief The signal emitted when the networkRequestsRunning property
	 *        changes
	 */
	void networkRequestsRunningChanged();

	/**
	 * \brief The signal emitted when font size can be increased again
	 */
	void canIncreaseFontSizeChanged();

	/**
	 * \brief The signal emitted when font size can be decreased again
	 */
	void canDecreaseFontSizeChanged();

	/**
	 * \brief The signal emitted when the font size changes
	 */
	void fontSizeChanged();

	/**
	 * \brief The signal emitted when the about text changes
	 */
	void aboutTextChanged();

	/**
	 * \brief The signal emitted when the mm property changes
	 */
	void mmChanged();

public slots:
	/**
	 * \brief Updates news from the net
	 */
	void updateNews();

	/**
	 * \brief Removes all news
	 */
	void clearAllNews();

	/**
	 * \brief Performs the final cleanup
	 */
	void finalize();

	/**
	 * \brief Increases the font size by one point
	 */
	void increaseFontSize();

	/**
	 * \brief Decreases the font size by one point
	 */
	void decreaseFontSize();

	/**
	 * \brief Sets the initial font size
	 *
	 * This is called by QML when components are created so that we know
	 * from what font size we start
	 * \param fontSize the initial font size
	 */
	void setInitialFontSize(qreal fontSize);

	/**
	 * \brief Generates a PNG image from a vectorial image
	 *
	 * This function takes the path to an svg image and the desired
	 * dimension and generates a PNG image of that dimension. The callback
	 * QML method is called when generation is finished. This function keeps
	 * the aspect ration of the SVG image. The PNG image is generated only
	 * once for each SVG and resolution, subsequent calls to this function
	 * will return the already generated image
	 * \param sourceImage the path to the source SVG image
	 * \param imageDim the width or height of the requested PNG image.
	 *                 Whether this is the width or height depends on the
	 *                 dimIsHeight parameter
	 * \param dimIsHeight if true the imageDim is the height of the
	 *                    requested image, if false the width. The other
	 *                    dimension is computed so that the aspect ratio of
	 *                    the source SVG image is kept
	 * \param callback a QML javascript function that is called when the
	 *                 image has been generated. The callback must have a
	 *                 single string parameter which will be set to the path
	 *                 of the generated image or the empty string in case of
	 *                 errors
	 */
	void getPNGFromVectorial(QString sourceImage, int imageDim, bool dimIsHeight, QJSValue callback);

	/**
	 * \brief Returns a news (the RolesQMLAccessor for the news) from the
	 *        given URL
	 *
	 * If the url points to a page the channel can parse, it returns an
	 * instance of a RolesQMLAccessor with the news. Otherwise returns a
	 * null pointer
	 * \param newsUrl the url of the news to return
	 * \return an instance of RolesQMLAccessor for the news or nullptr if
	 *         the url points to a page the channel cannot parse
	 */
	QObject* getNewsForURL(QString newsUrl);

	/**
	 * \brief The function to call when the user comes back to the list of
	 *        news
	 *
	 * This function should be called from QML whenever the user comes back
	 * to the list of news. Here we perform some cleanup.
	 */
	void backToNewsList();

private slots:
	/**
	 * \brief The slot called in case of errors
	 */
	void error(QString error);

	/**
	 * \brief Sets to true the flag indicating that there are network
	 *        requests running
	 */
	void setNetworkRequestsRunning();

	/**
	 * \brief Sets to false the flag indicating that there are network
	 *        requests running
	 */
	void unsetNetworkRequestsRunning();

	/**
	 * \brief The slot called when an icon has been generated
	 *
	 * \param index the index of the icon that has been generated
	 * \param filename the name of the file with the generated icon
	 */
	void iconGenerated(unsigned int index, QString filename);

private:
	/**
	 * \brief Sets the interval of the timer to the value of ttl
	 */
	void setTimerInterval();

	/**
	 * \brief The path of the file with text for the about page
	 */
	const QString m_aboutTextFilename;

	/**
	 * \brief The object with application settings
	 */
	QSettings m_settings;

	/**
	 * \brief The channel with the news
	 *
	 * We have to dynamically allocate the channel because we need to
	 * destroy it before deleting singletons
	 */
	std::unique_ptr<AbstractChannel> m_channel;

	/**
	 * \brief The object updating the channel
	 *
	 * We have to dynamically allocate the channel updater because we need
	 * to destroy it before deleting singletons
	 */
	std::unique_ptr<AbstractChannelUpdater> m_channelUpdater;

	/**
	 * \brief The object exposing roles to QML
	 */
	std::unique_ptr<AbstractRolesQMLAccessor> m_channelRolesQMLAccessor;

	/**
	 * \brief The model of the list of news
	 *
	 * This is exposed as a read-only property. We have to dynamically
	 * allocate the channel because we need to destroy it before deleting
	 * singletons
	 */
	std::unique_ptr<AbstractNewsListModel> m_newsModel;

	/**
	 * \brief The object that generates the icons
	 *
	 * This is the object that generates the icons in a separate thread.
	 * A signal is emitted for every image that is generated
	 */
	IconsGenerator m_iconsGenerator;

	/**
	 * \brief The timer for the automatic update
	 */
	QTimer m_updateTimer;

	/**
	 * \brief This is true if any network request is running
	 */
	bool m_networkRequestsRunning;

	/**
	 * \brief The last index used to request PNG images generation
	 *
	 * This is incremeted for each request to have a unique id for each
	 * request
	 */
	unsigned int m_lastPNGGenerationIndex;

	/**
	 * \brief The map with PNG generation requests and the callback to call
	 *        when the geration finishes
	 */
	QMap<unsigned int, QJSValue> m_PNGGenerationMap;

	/**
	 * \brief This is true when font size can be increased
	 */
	bool m_canIncreaseFontSize;

	/**
	 * \brief This is true when font size can be decreased
	 */
	bool m_canDecreaseFontSize;

	/**
	 * \brief The initial font size
	 *
	 * This is set when the setInitialFontSize() function is called. It is
	 * set to a negative value by the constructor. The current font size is
	 * stored in the settimngs (with the name "fontSize")
	 */
	qreal m_initialFontSize;

	/**
	 * \brief How many points are there in a millimiter
	 *
	 * This is exposed in QML via the mm property. Multiply lengths in
	 * millimiters by this to get the length in points, usable them as
	 * absolute lengths of items
	 */
	const qreal m_mm;
};

#endif
