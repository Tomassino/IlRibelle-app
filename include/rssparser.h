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

#ifndef __RSS_PARSER_H__
#define __RSS_PARSER_H__

#include <QUrl>
#include <QList>
#include <QXmlStreamReader>
#include "include/dataavailablenotifee.h"
#include "include/standardroles.h"

class AbstractChannel;
class AbstractChannelUpdater;

/**
 * \brief The class downloading and parsing an rss feed
 *
 * This class downloads and parses an rss feed. It is initialized with the
 * channel to update, which is actually updated when the fetch() function is
 * called. The channel is updated with information and news from the rss feed
 * as the xml file is parsed
 */
class RssParser : private DataAvailableNotifee
{
private:
	/**
	 * \brief The enum with possible states
	 */
	enum class States {
		NotStarted, /// We have not started reading
		Started, /// We have just started and have to read the rss tag
		RssRead, /// We have just read the rss tag and wait for channel
		ReadingChannel, /// We are reading the channel section
		ReadingNews, /// We are reading a news (item)
		FinishedReadingChannel, /// We have finished reading the
					/// channel, now we expect the closing
					/// rss tag
		FinishedReadingRss, /// We have finished reading the rss, now
				    /// we expect the end of the document
		DocumentFinished /// The rss document is finished
	};

public:
	/**
	 * \brief Constructor
	 *
	 * \param channel the channel to update
	 * \param channelUpdater the object updating the channel
	 */
	RssParser(AbstractChannel* channel, AbstractChannelUpdater* channelUpdater);

	/**
	 * \brief Destructor
	 */
	virtual ~RssParser();

	/**
	 * \brief Downloads the rss xml file and parses it
	 *
	 * This is asyncronous, the function returns immediately.
	 */
	void fetch();

private:
	/**
	 * \brief The function called when data is available
	 *
	 * When this function is called, you can read data from the reply
	 * object.
	 * \param id the request ID
	 */
	virtual void dataAvailable(int id) override;

	/**
	 * \brief The function called when all data is available
	 *
	 * \param id the request ID
	 */
	virtual void allDataAvailable(int id) override;

	/**
	 * \brief Re-implement to handle network errors
	 *
	 * Note that in case of errors during a request, NetworkManager calls
	 * both this function and emits its networkError signal
	 * \param id the request ID
	 * \param description the description of the error
	 */
	virtual void networkError(int id, const QString& description) override;

	/**
	 * \brief The function called to start reading the document
	 */
	void startReadingDocument();

	/**
	 * \brief The function called to read a specific tag
	 *
	 * \param tag the tag to read
	 * \param nextState the state to change to if the tag ids found
	 */
	void readTag(QString tag, States nextState);

	/**
	 * \brief The function called to read information about the channel
	 */
	void readChannelInfo();

	/**
	 * \brief The function called to read information about the items
	 */
	void readItemInfo();

	/**
	 * \brief The function called to read the closing rss tag
	 */
	void readClosingRssTag();

	/**
	 * \brief The function that reads the end of the document
	 */
	void readEndDocument();

	/**
	 * \brief Reads a channel element
	 *
	 * \return true if we have read and item element and so should switch to
	 *         reading the news
	 */
	bool readChannelElementAndCheckItem();

	/**
	 * \brief The function reading data from channel tags
	 */
	void readChannelTagData();

	/**
	 * \brief Reads an item element
	 */
	void readItemElement();

	/**
	 * \brief The function reading data from item tags
	 */
	void readItemTagData();

	/**
	 * \brief The channel to update
	 */
	AbstractChannel* const m_channel;

	/**
	 * \brief The object updating the channel
	 */
	AbstractChannelUpdater* const m_channelUpdater;

	/**
	 * \brief The XML stream reader to read the rss
	 */
	QXmlStreamReader m_reader;

	/**
	 * \brief The possible states
	 */
	States m_status;

	/**
	 * \brief The prefix for the http://purl.org/dc/elements/1.1/ namespace
	 */
	QString m_dcNamespacePrefix;

	/**
	 * \brief The stack of opened tags inside channel
	 */
	QStringList m_openedTags;

	/**
	 * \brief The categories of the channel
	 */
	QStringList m_channelCategories;

	/**
	 * \brief The current news roles being read
	 *
	 * When we finish reading the news, we add it to the channel
	 */
	Roles<StandardNewsRoles> m_currentNewsRoles;

	/**
	 * \brief The categories of the news
	 */
	QStringList m_newsCategories;

	/**
	 * \brief If true the guid of the news is a permalink
	 */
	bool m_guidIsPermalink;
};

#endif
