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

#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <QObject>
#include <QString>
#include <QUrl>
#include <QList>
#include <QStringList>
#include <QVariant>
#include <QJsonObject>
#include <QSet>
#include <QHash>
#include <QDynamicPropertyChangeEvent>
#include <QApplication>
#include <array>
#include <memory>
#include "include/news.h"
#include "include/standardroles.h"
#include "include/utilities.h"

class RssParser;
template <class, class>
class AllNewsCompleter;
class AbstractRolesQMLAccessor;

/**
 * \brief The abstract base class for a channel
 *
 * This is needed because we cannot use Q_OBJECT in template class. Do not use
 * this class directly, use the Channel class
 */
class AbstractChannel : public QObject
{
	Q_OBJECT

public:
	/**
	 * \brief Constructor
	 *
	 * \@param parent the parent object
	 */
	AbstractChannel(QObject* parent = nullptr)
		: QObject(parent)
	{
	}

	/**
	 * \brief Destructor
	 */
	virtual ~AbstractChannel()
	{
	}

	/**
	 * \brief Reads the channel from a JSON object
	 *
	 * \param obj the JSON object from which we read
	 * \return false in case of error, true if the news was read correctly
	 */
	virtual bool load(const QJsonObject& obj) = 0;

	/**
	 * \brief Writes the channel as a JSON object
	 *
	 * \return the JSON object representing the news
	 */
	virtual QJsonObject save() const = 0;

	/**
	 * \brief Returns the directory where channel data is stored
	 *
	 * \return the directory where channel data is stored
	 */
	virtual QString dataDir() const = 0;

	/**
	 * \brief Returns a reference to the standardRoles for the channel
	 *
	 * \return a reference to the standardRoles for the channel
	 */
	virtual StandardChannelRoles& standardRoles() = 0;

	/**
	 * \brief Returns a const reference to the standardRoles for the channel
	 *
	 * \return a const reference to the standardRoles for the channel
	 */
	virtual const StandardChannelRoles& standardRoles() const = 0;

	/**
	 * \brief Adds a news
	 *
	 * This function is used by the rss parser. The Channel class has
	 * functions which allows adding news having more roles than the
	 * standard ones
	 * \param roles the object with roles data for the news to add
	 */
	virtual void addStandardNews(const StandardNewsRoles& roles) = 0;

	/**
	 * \brief Returns the number of news
	 *
	 * \return the number of news
	 */
	virtual int numNews() const = 0;

	/**
	 * \brief Returns a reference to the standard roles for the i-th news
	 *
	 * \param i the index of the news to return
	 * \return a reference to the roles of the i-th news in the list
	 */
	virtual StandardNewsRoles& standardNews(int i) = 0;

	/**
	 * \brief Returns a reference to the standard roles for the i-th news
	 *        (const version)
	 *
	 * \param i the index of the news to return
	 * \return a const reference to the roles of the i-th news in the list
	 */
	virtual const StandardNewsRoles& standardNews(int i) const = 0;

	/**
	 * \brief Returns the index of the the news with the given ID
	 *
	 * IDs are unsigned int uniquely associated to a news. They are not
	 * guaranteed to be sequential, but it is guaranteed that the ID is
	 * unique (different news have different IDs) and permanet during the
	 * same execution (a news will always have the same ID from the
	 * beginning of the execution of a program to its end; the ID may change
	 * in different executions). This function returns -1 if no news exists
	 * with the given ID
	 * \param id the ID of the news to return
	 * \return the index of the news or -1 if no news exists with the given
	 *         id
	 */
	virtual int newsIndexByID(unsigned int id) const = 0;

	/**
	 * \brief Removes all news that are older than the given date
	 *
	 * \param date news older than this date are removed
	 */
	virtual void deleteNews(const QDateTime& date) = 0;

	/**
	 * \brief Removes all news
	 */
	virtual void clearAllNews() = 0;

	/**
	 * \brief Returns a name for a file of a news that is guaranteed to be
	 *        unique
	 *
	 * This does not create the file, it only returns an absolute filename
	 * that is guaranteed to be unique. This function also adds the file to
	 * the list of files for the news
	 * \param i the index of the news
	 * \param ext the extension to use for the file (the "." is added by
	 *            this function)
	 * \return the absolute filename of a file that is guaranteed to be
	 *         unique
	 */
	virtual QString createFileForNews(int i, QString ext) = 0;

	/**
	 * \brief Deletes all files attached to a news
	 *
	 * This also clears the list of files attached to the news.
	 * \param i the index of the news whose files to delete
	 */
	virtual void deleteAllFilesForNews(int i) = 0;

	/**
	 * \brief Returns a name for a file of the channel that is guaranteed to
	 *        be unique
	 *
	 * This does not create the file, it only returns an absolute filename
	 * that is guaranteed to be unique. This function also adds the file to
	 * the list of files for the channel
	 * \param ext the extension to use for the file (the "." is added by
	 *            this function)
	 * \return the absolute filename of a file that is guaranteed to be
	 *         unique
	 */
	virtual QString createFileForChannel(QString ext) = 0;

	/**
	 * \brief Deletes all files attached to the channel
	 *
	 * This also clears the list of files attached to the channel
	 */
	virtual void deleteAllFilesForChannel() = 0;

	/**
	 * \brief Deletes all files attached to all news and to the channel
	 */
	virtual void deleteAllExternalFiles() = 0;

	/**
	 * \brief Deletes all files in the data directory that do not belog to
	 *        the channel or any news
	 *
	 * This does not remove sub-directories of the data dir
	 */
	virtual void deleteUnknownFiles() = 0;

	/**
	 * \brief Returns the ID of the news with the given URL
	 *
	 * This returns an invalid ID if no news with the given URL exists
	 * \param newsUrl the url of the news whose ID to find
	 * \return the ID of the news or an invalid ID if no news with the given
	 *         URL esists
	 */
	virtual unsigned int newsIDForURL(QUrl newsUrl) = 0;

	/**
	 * \brief Returns a roles qml accessor for a temporary news
	 *
	 * This function return the qml accessor for a temporary news. See the
	 * Channel class documentation for more information on temporary news
	 * \param newsUrl the url of the news to return
	 * \return a qml roles accessor for the temporary news. Note that the
	 *         lifetime of the returned object is managed by this class, you
	 *         must not delete it externally
	 */
	virtual AbstractRolesQMLAccessor* getTemporaryNewsFromUrl(QUrl newsUrl) = 0;

	/**
	 * \brief Cleans the cache of temporary news
	 *
	 * This function removes temporary news in excess from the cache. See
	 * the Channel class documentation for more information on temporary
	 * news and their cache.
	 */
	virtual void cleanTemporaryNewsCache() = 0;

signals:
	/**
	 * \brief The signal emitted when a new news is about to be added
	 *
	 * This is always followed by the newsAdded signal
	 * \param index the index where the news will be added
	 */
	void aboutToAddNews(int index);

	/**
	 * \brief The signal emitted when a news has been added
	 *
	 * This is always preceded by the abouToAddNews signal
	 */
	void newsAdded();

	/**
	 * \brief The signal emitted when some news are about to be deleted
	 *
	 * This is always followed by the newsDeleted signal
	 * \param startIndex the starting index of the news to be deleted
	 * \param endIndex the ending index of the news to be deleted
	 */
	void aboutToDeleteNews(int startIndex, int endIndex);

	/**
	 * \brief The signal emitted when news have been deletes
	 *
	 * This is always preceded by the aboutToDeleteNews signal
	 */
	void newsDeleted();

	/**
	 * \brief The signal emitted when a news has been changed
	 *
	 * \param index the index of the news that has been updated
	 * \param roles the list of roles that have changed
	 */
	void newsUpdated(int index, const QVector<int>& roles = QVector<int>());

	/**
	 * \brief The signal emitted when we are starting to fetch new data
	 *
	 * This is always followed by the dataUpdated signal
	 */
	void aboutToUpdateData();

	/**
	 * \brief The signal emitted when we have finished updating data
	 *
	 * This is always preceded by the aboutToUpdateData signal
	 */
	void dataUpdated();
};

/**
 * \brief The class storing news for channel and channel information
 *
 * This class is responsible for keeping the list of news of a single channel,
 * updating them from the web and performing serialization to JSON. News are
 * fetched by a NewsUpdater instance using RSS. The channel must be initialized
 * with an URL from which the rss can be fetched and a directory where data is
 * stored. The channel information are fetched from rss. This class also manages
 * data files for all news in the channel. The directory for data passed to the
 * constructor must be writable (it is created if it doesn't exists) and must be
 * only used by one channel (i.e. different channels must have different data
 * directories). The files created by the createFileForNews() and
 * createFileForChannel() are unique and their name is simply a number followed
 * by the extension (if you need to create a file in the data directory do not
 * use a simple number as the file name to avoid clashes with files created
 * here). This class has two template parameters: Roles that is the RolesList
 * with the Roles for the channel and NewsType that is the type of news stored
 * here. The requirements for these template parameters are:
 *	- Role: this must contain StandardChannelRoles.
 *	- NewsType: this must contain StandardNewsRoles.
 * The channel can also return a temporary news. Temporary news are news that
 * are not in the standard list, they are cached but not remembered across
 * executions. Moreover, if a temporary news is requested for the same URL of a
 * news already in the list, the news is nevertheless downloaded again. The
 * cache for temporary news is simply a queue, the oldest news (the one accessed
 * less recently) is deleted if the number of cached news grows over a certain
 * limit (see the temporaryNewsCacheSize constructor parameter). To remove news
 * in excess, however, the cleanTemporaryNewsCache() function must be explicitly
 * called (this also removes the roles qml accessors for the news). The function
 * getTemporaryNewsFromUrl() can be used to get a qml accessor for a temporary
 * news with the given URL. A final note on temporary news: the cache size
 * should be kept small, the search of news in the cache is made in linear time.
 * Note that this class is NOT THREAD SAFE.
 */
template <class RolesListType, class NewsType>
class Channel : public AbstractChannel, public Roles<RolesListType>
{
	// Roles must contain StandardChannelRoles
	static_assert(std::is_convertible<RolesListType, StandardChannelRoles>::value, "The list of roles in Channel must contain the StandardChannelRoles list");
	// NewsType must contain StandardNewsRoles
	static_assert(std::is_convertible<NewsType, StandardNewsRoles>::value, "The list of roles in News must contain the StandardNewsRoles list");

public:
	/**
	 * \brief The type of news used here
	 */
	using StoredNewsType = NewsType;

public:
	/**
	 * \brief Constructor
	 *
	 * \param url the url from which the rss is fetched
	 * \param dataDir the absolute path to the directory where we can store
	 *                data
	 * \param temporaryNewsCacheSize the size of the cache for temporary
	 *                               news (see class description for more
	 *                               information)
	 * \param parent the parent object
	 */
	Channel(QUrl url, QString dataDir, int temporaryNewsCacheSize = 5, QObject* parent = nullptr);

	/**
	 * \brief Destructor
	 */
	virtual ~Channel();

	/**
	 * \brief Reads the channel from a JSON object
	 *
	 * \param obj the JSON object from which we read
	 * \return false in case of error, true if the news was read correctly
	 */
	virtual bool load(const QJsonObject& obj) override;

	/**
	 * \brief Writes the channel as a JSON object
	 *
	 * \return the JSON object representing the news
	 */
	virtual QJsonObject save() const override;

	/**
	 * \brief The function called by the ChannelUpdater when it is starting
	 *        the update of the channel and news
	 *
	 * This simply emits the aboutToUpdateData signal
	 */
	void startUpdatingData()
	{
		emit aboutToUpdateData();
	}

	/**
	 * \brief The function called by the ChannelUpdater when it has finished
	 *        updating the channel and news
	 *
	 * This simply emits the dataUpdated signal
	 */
	void finishedUpdatingData()
	{
		emit dataUpdated();
	}

	/**
	 * \brief Returns the directory where channel data is stored
	 *
	 * \return the directory where channel data is stored
	 */
	virtual QString dataDir() const override
	{
		return m_dataDir;
	}

	/**
	 * \brief Returns a reference to the standardRoles for the channel
	 *
	 * \return a reference to the standardRoles for the channel
	 */
	virtual StandardChannelRoles& standardRoles() override
	{
		return *this;
	}

	/**
	 * \brief Returns a const reference to the standardRoles for the channel
	 *
	 * \return a const reference to the standardRoles for the channel
	 */
	virtual const StandardChannelRoles& standardRoles() const override
	{
		return *this;
	}

	/**
	 * \brief Adds a news
	 *
	 * This function is used by the rss parser
	 * \param roles the object with roles data for the news to add
	 */
	virtual void addStandardNews(const StandardNewsRoles& roles) override;

	/**
	 * \brief Adds a news
	 *
	 * This function adds a news to the list. Here you can pass any
	 * RolesList provided that it can be converted to the RolesList of
	 * NewsType used here
	 * \param roles the object with roles data for the news to add
	 */
	template <class NewsRoles>
	void addNews(const NewsRoles& roles);

	/**
	 * \brief Returns the list of news
	 *
	 * News are ordered in descending order by date.
	 * \return a const reference to the list of news
	 */
	const QList<NewsType*>& newsList() const
	{
		return m_news;
	}

	/**
	 * \brief Returns the number of news
	 *
	 * \return the number of news
	 */
	virtual int numNews() const override
	{
		return m_news.size();
	}

	/**
	 * \brief Returns a reference to the standard roles for the i-th news
	 *
	 * \param i the index of the news to return
	 * \return a reference to the roles of the i-th news in the list
	 */
	virtual StandardNewsRoles& standardNews(int i) override
	{
		return *(m_news[i]);
	}

	/**
	 * \brief Returns a reference to the standard roles for the i-th news
	 *        (const version)
	 *
	 * \param i the index of the news to return
	 * \return a const reference to the roles of the i-th news in the list
	 */
	virtual const StandardNewsRoles& standardNews(int i) const override
	{
		return *(m_news[i]);
	}

	/**
	 * \brief Returns a reference to the i-th news
	 *
	 * \param i the index of the news to return
	 * \return a reference to the i-th news in the list
	 */
	NewsType& news(int i)
	{
		return *(m_news[i]);
	}

	/**
	 * \brief Returns a reference to the i-th news (const version)
	 *
	 * \param i the index of the news to return
	 * \return a const reference to the i-th news in the list
	 */
	const NewsType& news(int i) const
	{
		return *(m_news[i]);
	}

	/**
	 * \brief Returns the index of the the news with the given ID
	 *
	 * IDs are unsigned int uniquely associated to a news. They are not
	 * guaranteed to be sequential, but it is guaranteed that the ID is
	 * unique (different news have different IDs) and permanet during the
	 * same execution (a news will always have the same ID from the
	 * beginning of the execution of a program to its end; the ID may change
	 * in different executions). This function returns -1 if no news exists
	 * with the given ID
	 * \param id the ID of the news to return
	 * \return the index of the news or -1 if no news exists with the given
	 *         id
	 */
	int newsIndexByID(unsigned int id) const override
	{
		return m_newsIdToIndex.value(id, -1);
	}

	/**
	 * \brief Removes all news that are older than the given date
	 *
	 * \param date news older than this date are removed
	 */
	virtual void deleteNews(const QDateTime& date) override;

	/**
	 * \brief Removes all news
	 */
	virtual void clearAllNews() override;

	/**
	 * \brief Returns a name for a file of a news that is guaranteed to be
	 *        unique
	 *
	 * This does not create the file, it only returns an absolute filename
	 * that is guaranteed to be unique. This function also adds the file to
	 * the list of files for the news
	 * \param i the index of the news
	 * \param ext the extension to use for the file (the "." is added by
	 *            this function)
	 * \return the absolute filename of a file that is guaranteed to be
	 *         unique
	 */
	virtual QString createFileForNews(int i, QString ext) override;

	/**
	 * \brief Deletes all files attached to a news
	 *
	 * This also clears the list of files attached to the news.
	 * \param i the index of the news whose files to delete
	 */
	virtual void deleteAllFilesForNews(int i) override;

	/**
	 * \brief Returns a name for a file of the channel that is guaranteed to
	 *        be unique
	 *
	 * This does not create the file, it only returns an absolute filename
	 * that is guaranteed to be unique. This function also adds the file to
	 * the list of files for the channel
	 * \param ext the extension to use for the file (the "." is added by
	 *            this function)
	 * \return the absolute filename of a file that is guaranteed to be
	 *         unique
	 */
	virtual QString createFileForChannel(QString ext) override;

	/**
	 * \brief Deletes all files attached to the channel
	 *
	 * This also clears the list of files attached to the channel
	 */
	virtual void deleteAllFilesForChannel() override;

	/**
	 * \brief Deletes all files attached to all news and to the channel
	 */
	virtual void deleteAllExternalFiles() override;

	/**
	 * \brief Deletes all files in the data directory that do not belog to
	 *        the channel or any news
	 *
	 * This does not remove sub-directories of the data dir
	 */
	virtual void deleteUnknownFiles() override;

	/**
	 * \brief Returns the ID of the news with the given URL
	 *
	 * This returns an invalid ID if no news with the given URL exists
	 * \param newsUrl the url of the news whose ID to find
	 * \return the ID of the news or an invalid ID if no news with the given
	 *         URL esists
	 */
	virtual unsigned int newsIDForURL(QUrl newsUrl) override;

	/**
	 * \brief Returns a roles qml accessor for a temporary news
	 *
	 * This function return the qml accessor for a temporary news. See class
	 * documentation for more information on temporary news
	 * \param newsUrl the url of the news to return
	 * \return a qml roles accessor for the temporary news. Note that the
	 *         lifetime of the returned object is managed by this class, you
	 *         must not delete it externally
	 */
	virtual AbstractRolesQMLAccessor* getTemporaryNewsFromUrl(QUrl newsUrl) override;

	/**
	 * \brief Cleans the cache of temporary news
	 *
	 * This function removes temporary news in excess from the cache. See
	 * class documentation for more information on temporary news and their
	 * cache.
	 */
	virtual void cleanTemporaryNewsCache() override;

private:
	/**
	 * \brief The function called when a news changes
	 *
	 * This emits the newsUpdated() signal
	 * \param roleIndex the index of the role of the news that has changed
	 * \param id the id of the news whose role has changed
	 */
	void newsDataChanged(int roleIndex, unsigned int id);

	/**
	 * \brief Creates a news
	 *
	 * The news is returned in an unique_ptr. This function assigns an
	 * unique ID to the news. The news is not inserted in the list, its ID
	 * is not insered in the m_newsIdToIndex map and its URL is not inserted
	 * in the m_newsURLToId map
	 * \return a new news
	 */
	std::unique_ptr<NewsType> createNews();

	/**
	 * \brief Inserts the news in the list
	 *
	 * This also fills the map with the ID and index of the news
	 * \param news the news to add. The pointer is
	 */
	void insertNews(std::unique_ptr<NewsType>&& news);

	/**
	 * \brief The absolute path to the directory with data for the channel
	 *
	 * This directory must be only used by this channel. This is not stored
	 * in the JSON data stream, it must be provided to the channel
	 * constructor
	 */
	const QString m_dataDir;

	/**
	 * \brief The maximum size of the temporary news cache
	 *
	 * This is the size of the cache that is enforced when the function
	 * cleanTemporaryNewsCache() is called. During normal operation the
	 * cache can grow past this size
	 */
	const int m_temporaryNewsCacheSize;

	/**
	 * \brief An index to create unique files
	 *
	 * This index is increased monotonically and is used to generate unique
	 * filenames inside the data dir. It is stored in the JSON data stream
	 */
	int m_fileCreationIndex;

	/**
	 * \brief The list of news
	 */
#warning QUI USARE LISTA DI unique_ptr A NEWS
	QList<NewsType*> m_news;

	/**
	 * \brief The map from news ids to news index in the m_news list
	 */
	QHash<unsigned int, int> m_newsIdToIndex;

	/**
	 * \brief The map from the url of a news to its id
	 */
	QHash<QUrl, unsigned int> m_newsURLToId;

	/**
	 * \brief The current maximum ID of news
	 *
	 * This is used to assign a unique id to news
	 */
	unsigned int m_maxNewsID;

	/**
	 * \brief If true the callback for news changes does nothing
	 */
	bool m_ignoreNewsCallback;

	/**
	 * \brief A structure with a news and its accessor
	 *
	 * This is used to keep a temporary news and its accessor
	 */
	struct NewsAndAccessor {
#warning QUI USARE unique_ptr PER NEWS E ACCESSOR
		/**
		 * \brief The news
		 */
		NewsType* news;

		/**
		 * \brief The qml accessor for the news
		 */
		AbstractRolesQMLAccessor* accessor;
	};

	/**
	 * \brief The cache of temporary news
	 *
	 * News more recently requested are at the beginning of the list
	 */
	QList<NewsAndAccessor> m_temporaryNews;
};

// Implementation of template functions of Channel
#include <QDebug>
#include <QJsonValue>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QSet>
#include <memory>
#include "include/rssparser.h"
#include "include/allnewscompleter.h"
#include "include/rolesqmlaccessor.h"

template <class RolesListType, class NewsType>
Channel<RolesListType, NewsType>::Channel(QUrl url, QString dataDir, int temporaryNewsCacheSize, QObject* parent)
	: AbstractChannel(parent)
	, Roles<RolesListType>()
	, m_dataDir(dataDir)
	, m_temporaryNewsCacheSize(temporaryNewsCacheSize)
	, m_fileCreationIndex(0)
	, m_news()
	, m_newsIdToIndex()
	, m_newsURLToId()
	, m_maxNewsID(0)
	, m_ignoreNewsCallback(false)
	, m_temporaryNews()
{
	// Setting the URL role
	this->template setData<ChannelRoles::siteUrl>(url);

	// Creating the data directory if it doesn't exist
	QFileInfo dinfo(m_dataDir);
	if (!dinfo.exists()) {
		if (!QDir().mkpath(m_dataDir)) {
			qDebug() << "Cannot create data directory" << m_dataDir;
		}
	} else if (!dinfo.isDir() || !dinfo.isWritable()) {
		qDebug() << m_dataDir << "is not a directory or is not writable";
	}
}

template <class RolesListType, class NewsType>
Channel<RolesListType, NewsType>::~Channel()
{
	// Deleting all news
	for (auto n: m_news) {
		delete n;
	}

	// Deleting all temporary news
	for (auto t: m_temporaryNews) {
		delete t.accessor;
		delete t.news;
	}

}

template <class RolesListType, class NewsType>
bool Channel<RolesListType, NewsType>::load(const QJsonObject& obj)
{
	// Iterating the JSON object and converting key to a Role. If conversion fails, returning false
	for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
		// Skipping the news property, we will read it later. We also read fileCreationIndex directly
		if (it.key() == "news") {
			continue;
		} else if (it.key() == "fileCreationIndex") {
			m_fileCreationIndex = it.value().toInt(m_fileCreationIndex);

			continue;
		}

		const int r = this->getRoleIndexFromName(it.key().toLatin1());

		if (r == -1) {
			return false;
		}

		this->setVariantData(r, it.value().toVariant());
	}

	// Now loading all news
	QJsonValue news = obj.value("news");
	if (!news.isArray()) {
		return false;
	}

	// Adding news
	QJsonArray newsArray = news.toArray();
	for (auto it = newsArray.constBegin(); it != newsArray.constEnd(); ++it) {
		if (!(*it).isObject()) {
			return false;
		}

		// Creating a news. We use a unique_ptr so that if we have to exit because
		// of problems loading the news, memory is automatically released
		std::unique_ptr<NewsType> n = createNews();
		if (!n->load((*it).toObject())) {
			return false;
		}

		// Inserting the news
		insertNews(std::move(n));
	}

	return true;
}

template <class RolesListType, class NewsType>
QJsonObject Channel<RolesListType, NewsType>::save() const
{
	// Building a QVariantMap from the values we have
	QVariantMap map;

	for (int i = 0; i < Roles<RolesListType>::numRoles(); ++i) {
		map[this->getRoleNameFromIndex(i)] = this->getVariantData(i);
	}

	// Adding to the map the file creation index
	map["fileCreationIndex"] = m_fileCreationIndex;

	// Creating the JSON object
	QJsonObject obj = QJsonObject::fromVariantMap(map);

	// Now adding all news
	QJsonArray news;
	for (auto it = m_news.constBegin(); it != m_news.constEnd(); ++it) {
		news.append((*it)->save());
	}
	obj.insert("news", news);

	return obj;
}

template <class RolesListType, class NewsType>
void Channel<RolesListType, NewsType>::addStandardNews(const StandardNewsRoles& roles)
{
	// Creating a news
	std::unique_ptr<NewsType> n = createNews();

	// Copying data
	(static_cast<StandardNewsRoles&>(*n)).copyDataFromOtherRolesList(roles);

	// Inserting the news
	insertNews(std::move(n));
}

template <class RolesListType, class NewsType>
template <class NewsRoles>
void Channel<RolesListType, NewsType>::addNews(const NewsRoles& roles)
{
	// Creating a news
	std::unique_ptr<NewsType> n = createNews();

	// Copying data
	n->copyDataFromOtherRolesList(roles);

	// Inserting the news
	insertNews(std::move(n));
}

template <class RolesListType, class NewsType>
void Channel<RolesListType, NewsType>::deleteNews(const QDateTime& date)
{
	if (m_news.isEmpty()) {
		return;
	}

	// News are ordered by descending date, we can use a binary search to find the first news
	// before date
	int startIndex;
	if (m_news.first()->template getData<NewsRoles::pubDate>() <= date) {
		startIndex = 0;
	} else if (m_news.last()->template getData<NewsRoles::pubDate>() > date) {
		return;
	} else {
		int minIndex = 0;
		int maxIndex = m_news.size() - 1;
		while ((maxIndex - minIndex) > 1) {
			const int index = (minIndex + maxIndex) / 2;

			if (m_news[index]->template getData<NewsRoles::pubDate>() > date) {
				minIndex = index;
			} else {
				maxIndex = index;
			}
		}
		startIndex = maxIndex;
	}

	// Deleting
	emit aboutToDeleteNews(startIndex, m_news.size() - 1);
	while (m_news.size() > startIndex) {
		// Removing all files for the news
		deleteAllFilesForNews(m_news.size() - 1);

		// Removing the ID and URL from maps
		m_newsIdToIndex.remove(m_news.last()->id());
		m_newsURLToId.remove(m_news.last()->template getData<NewsRoles::link>());

		// Removing the news
		delete m_news.takeLast();
	}
	emit newsDeleted();
}

template <class RolesListType, class NewsType>
void Channel<RolesListType, NewsType>::clearAllNews()
{
	// If there is no news, exiting
	if (m_news.isEmpty()) {
		return;
	}

	// Removing all news
	emit aboutToDeleteNews(0, m_news.size() - 1);

	// First removing all files for all news
	for (int i = 0; i < m_news.size(); ++i) {
		deleteAllFilesForNews(i);
	}

	// Now deleting all news and clearing the list
	for (auto n: m_news) {
		delete n;
	}
	m_news.clear();
	m_newsIdToIndex.clear();
	m_newsURLToId.clear();

	emit newsDeleted();
}

template <class RolesListType, class NewsType>
QString Channel<RolesListType, NewsType>::createFileForNews(int i, QString ext)
{
	// Generating the unique filename
	const QString filename = m_dataDir + "/" + QString::number(m_fileCreationIndex) + "." + ext;

	// Incrementing the index
	++m_fileCreationIndex;

	// Adding the file to the files for the news
	auto attachedFiles = m_news[i]->template getData<NewsRoles::attachedFiles>();
	attachedFiles.append(filename);
	m_news[i]->template setData<NewsRoles::attachedFiles>(attachedFiles);

	return filename;
}

template <class RolesListType, class NewsType>
void Channel<RolesListType, NewsType>::deleteAllFilesForNews(int i)
{
	const auto& filesToDelete = m_news[i]->template getData<NewsRoles::attachedFiles>();

	for (auto& f: filesToDelete) {
		if (!QFile::remove(f)) {
			qDebug() << "In deleteAllFilesForNews(): cannot remove file" << f;
		}
	}

	// Resetting the list of files attached to the news
	m_news[i]->template setData<NewsRoles::attachedFiles>(QStringList());
}

template <class RolesListType, class NewsType>
QString Channel<RolesListType, NewsType>::createFileForChannel(QString ext)
{
	// Generating the unique filename
	const QString filename = m_dataDir + "/" + QString::number(m_fileCreationIndex) + "." + ext;

	// Incrementing the index
	++m_fileCreationIndex;

	// Adding the file to the files for the channel
	auto attachedFiles = this->template getData<ChannelRoles::attachedFiles>();
	attachedFiles.append(filename);
	this->template setData<ChannelRoles::attachedFiles>(attachedFiles);

	return filename;
}

template <class RolesListType, class NewsType>
void Channel<RolesListType, NewsType>::deleteAllFilesForChannel()
{
	const QStringList& filesToDelete = this->template getData<ChannelRoles::attachedFiles>();

	for (auto& f: filesToDelete) {
		if (!QFile::remove(f)) {
			qDebug() << "In deleteAllFilesForChannel(): cannot remove file" << f;
		}
	}

	// Resetting the list of files attached to the channel
	this->template setData<ChannelRoles::attachedFiles>(QStringList());
}

template <class RolesListType, class NewsType>
void Channel<RolesListType, NewsType>::deleteAllExternalFiles()
{
	// First removing all files for all news
	for (int i = 0; i < m_news.size(); ++i) {
		deleteAllFilesForNews(i);
	}

	// Now removing all files for the channel
	deleteAllFilesForChannel();
}

template <class RolesListType, class NewsType>
void Channel<RolesListType, NewsType>::deleteUnknownFiles()
{
	// First of all creating a set with all files in all news and in the channel
	QSet<QString> filesToKeep;

	// Adding files for all news
	for (const auto n: m_news) {
		const QStringList& l = n->template getData<NewsRoles::attachedFiles>();

		for (const auto& f: l) {
			filesToKeep.insert(f);
		}
	}

	// Adding files of the channel
	const QStringList& l = this->template getData<ChannelRoles::attachedFiles>();

	for (const auto& f: l) {
		filesToKeep.insert(f);
	}

	// Now listing all files in the directory. We do not take directories into account
	QDir dataDir = QDir(m_dataDir);
	const QStringList fileInDir = dataDir.entryList(QDir::Files | QDir::Hidden);

	// Finally removing all files that are not in the set of files to keep
	for (auto f: fileInDir) {
		if (!filesToKeep.contains(m_dataDir + "/" + f)) {
			qDebug() << "Deleting unknown file" << f;
			dataDir.remove(f);
		}
	}
}

template <class RolesListType, class NewsType>
unsigned int Channel<RolesListType, NewsType>::newsIDForURL(QUrl newsUrl)
{
	auto it = m_newsURLToId.find(newsUrl);
	if (it == m_newsURLToId.end()) {
		// An invalid ID (the +1 is just to be sure...)
		return m_maxNewsID + 1;
	} else {
		return it.value();
	}
}

template <class RolesListType, class NewsType>
AbstractRolesQMLAccessor* Channel<RolesListType, NewsType>::getTemporaryNewsFromUrl(QUrl newsUrl)
{
	// A check just for debug
#ifndef QT_NO_DEBUG
	if (m_newsURLToId.contains(newsUrl)) {
		qDebug() << "Requested a temporay news with the same url of a news in the list. Url:" << newsUrl;
	}
#endif

	// The cache should be rather small, we do not use any map-like structure to look for the
	// news in the cache
	int newsIndex = 0;
	for (; (newsIndex < m_temporaryNews.size()) && (m_temporaryNews[newsIndex].news->template getData<NewsRoles::link>() != newsUrl); ++newsIndex);

	if (newsIndex < m_temporaryNews.size()) {
		// Moving the news on top of the list
		auto n = m_temporaryNews.takeAt(newsIndex);
		m_temporaryNews.push_front(n);

		return m_temporaryNews.first().accessor;
	} else {
		// The news is not in the cache, creating it

		// For the moment we return nullptr, we need to modify channel to do things the right way
		return nullptr;
	}
}

template <class RolesListType, class NewsType>
void Channel<RolesListType, NewsType>::cleanTemporaryNewsCache()
{
	// Removing news in excess
	while (m_temporaryNews.size() > m_temporaryNewsCacheSize) {
		delete m_temporaryNews.last().accessor;
		delete m_temporaryNews.last().news;

		m_temporaryNews.pop_back();
	}
}

template <class RolesListType, class NewsType>
void Channel<RolesListType, NewsType>::newsDataChanged(int roleIndex, unsigned int id)
{
	if (m_ignoreNewsCallback) {
		return;
	}

	// Getting the index of the news
	const int index = m_newsIdToIndex.value(id, -1);

	if (Q_UNLIKELY(index == -1)) {
		// This should never happend, but just in case...
		qDebug() << "INTERNAL ERROR: a news changed but we cannot find it in the list, id:" << id;

		return;
	}

	// If publication date or the title changes, we have to remove the news from the
	// list and add it again, because its position could have changed. If the link
	// changed, we have to update the m_newsURLToId map
	if ((m_news[index]->template getIndex<NewsRoles::pubDate>() == roleIndex) ||
	    (m_news[index]->template getIndex<NewsRoles::title>() == roleIndex)){
		// Removing the news from the list. We also need to emit signals to tell the model
		// what is happening
		emit aboutToDeleteNews(index, index);

		std::unique_ptr<NewsType> n(m_news[index]);
		m_news.removeAt(index);

		emit newsDeleted();

		// Adding the news again in its correct position
		insertNews(std::move(n));
	} else {
		// Checking if we have to update the m_newsURLToId map
		if (m_news[index]->template getIndex<NewsRoles::link>() == roleIndex) {
			// We have to cycle the map...
			auto it = m_newsURLToId.begin();
			for (; (it != m_newsURLToId.end()) && (it.value() != id); ++it);

			// If we could not find the news in the map, there was an error
			if (Q_UNLIKELY(it == m_newsURLToId.end())) {
#warning QUI FALLIRE (qFatal) APPENA È STATO SCRITTO ANCHE IL CODICE PER SCARICARE NEWS NON NELLA LISTA DA RSS
				qDebug() << "The news with id" << id << "could not be found when updating the m_newsURLToId map";
			}

			m_newsURLToId.erase(it);
			m_newsURLToId[m_news[index]->template getData<NewsRoles::link>()] = id;
		}

		// Emitting signal
		emit newsUpdated(index, QVector<int>() << roleIndex);
	}
}

template <class RolesListType, class NewsType>
std::unique_ptr<NewsType> Channel<RolesListType, NewsType>::createNews()
{
	// The id of the new news
	const int newsId = m_maxNewsID++;

	// Creating the new news. We don't set the callback because which callback to set and
	// when depends on where the news is put
	return std::make_unique<NewsType>(newsId);
}

template <class RolesListType, class NewsType>
void Channel<RolesListType, NewsType>::insertNews(std::unique_ptr<NewsType>&& news)
{
	// The index where we will add the news
	int destIndex = 0;

	// We have to add the news while keeping the list ordered in descending order
	if ((m_news.isEmpty()) || (*news > *(m_news.first()))) {
		destIndex = 0;
	} else if (*news < *(m_news.last())) {
		destIndex = m_news.size();
	} else if ((*news == *(m_news.first())) || (*news == *(m_news.last()))) {
		// The news is already there, not adding it
		return;
	} else {
		// News are ordered by date, so we can use a binary search to find the inserition point
		int minIndex = 0;
		int maxIndex = m_news.size() - 1;
		while ((maxIndex - minIndex) > 1) {
			const int index = (minIndex + maxIndex) / 2;

			// If we found another news equal to the one to insert, we do nothing
			if (*news == *(m_news[index])) {
				return;
			} else if (*(m_news[index]) > *news) {
				minIndex = index;
			} else {
				maxIndex = index;
			}
		}

		destIndex = maxIndex;
	}

	// Last check if the news is equal to the one at destIndex and if not, adding it
	if ((destIndex >= m_news.size()) || (*news != *(m_news[destIndex]))) {
		// Setting the callback for the news to our function
		news->addCallbackForSetData([this, newsId = news->id()](int roleIndex) { this->newsDataChanged(roleIndex, newsId); });

		// Emitting signals before and after the news ahs been added
		emit aboutToAddNews(destIndex);

		const unsigned int id = news->id();
		NewsType* const addedNews = news.release();
		m_news.insert(destIndex, addedNews);
		// Putting the news id and index in the map
		m_newsIdToIndex[id] = destIndex;
		// Also saving the association between news link and id
		m_newsURLToId[addedNews->template getData<NewsRoles::link>()] = id;

		// We have to modify the complete and qmlItem property so that if they are invalid, we set
		// them to default values. We do this without triggering the callback
		m_ignoreNewsCallback = true;
		if (!addedNews->data(addedNews->template getIndex<NewsRoles::qmlItem>()).isValid()) {
			addedNews->template setData<NewsRoles::qmlItem>(QUrl("qrc:///qml/NewsDisplay.qml"));
		}
		if (!addedNews->data(addedNews->template getIndex<NewsRoles::complete>()).isValid()) {
			addedNews->template setData<NewsRoles::complete>(false);
		}
		m_ignoreNewsCallback = false;

		emit newsAdded();
	}
}

#endif
