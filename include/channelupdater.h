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

#ifndef __CHANNEL_UPDATER_H__
#define __CHANNEL_UPDATER_H__

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <array>
#include <memory>
#include "include/utilities.h"

class RssParser;

template <class, class>
class AllNewsCompleter;

/**
 * \brief The abstract base class for channel updater
 *
 * This is needed because we cannot use Q_OBJECT in template class. Do not use
 * this class directly, use the ChannelUpdater class
 */
class AbstractChannelUpdater : public QObject
{
	Q_OBJECT

public:
	/**
	 * \brief Constructor
	 *
	 * \@param parent the parent object
	 */
	AbstractChannelUpdater(QObject* parent = nullptr)
		: QObject(parent)
	{
	}

	/**
	 * \brief Destructor
	 */
	virtual ~AbstractChannelUpdater()
	{
	}

	/**
	 * \brief The function called by the rss parser when parsing has
	 *        finished
	 *
	 * \param success set to true if reading was successful, to false
	 *                otherwise
	 * \param reason if reading waas not successful, the description of the
	 *               error
	 */
	virtual void allNewsReceived(bool success, QString reason = QString()) = 0;

	/**
	 * \brief Updates the channel reading data from the rss url
	 */
	virtual void update() = 0;

	/**
	 * \brief Removes all news
	 */
	virtual void clearAllNews() = 0;

signals:
	/**
	 * \brief The signal emitted in case of error while updating the channel
	 *
	 * \param reasong a description of the error
	 */
	void error(QString reason);
};

/**
 * \brief The class updating the channel and news
 *
 * This class is responsible for updating a channel and its news. It fetches the
 * rss feed, parses it and then completes both the channel and its news using
 * the completers. This class has three template parameters: ChannelType that is
 * the type of the channel, ChannelCompleter that is the  type of the class
 * responsible of completing the channel (i.e. to add more information to the
 * one extracted from the RSS feed) and NewsCompleter that is the type of the
 * class responsible of completing news (i.e. to add more information to the one
 * extracted from the RSS). The requirements for these template parameters are:
 *	- ChannelType: this must be a template instantiation of the Chanel
 *	  class.
 *	- ChannelCompleter: this must have a constructor taking two parameters,
 *	  namely a pointer to the channel and a std::function<void()> object.
 *	  The latter must be called when the channel completer has finished
 *	  completing the channel. After that functional is called, the channel
 *	  completer can be destroyed at any time. It must also have a start()
 *	  function that is called to start completing the channel and that can
 *	  be called multiple times to complete the channel once more.
 *	- NewsCompleter: this must have a constructor taking three parameters,
 *	  namely a pointer to the channel, a pointer to the news to complete and
 *	  a std::function<void()> object. The latter must be called when the
 *	  news completer has finished completing the news. After that functional
 *	  is called, the news completer can be destroyed at any time. It must
 *	  also have a start() function that is called to start completing the
 *	  news and that can be called multiple times to complete the news once
 *	  more.
 * Note that this class is NOT THREAD SAFE, so the channel and news completers
 * must not call functions of channel or news from a different thread.
 */
template <class ChannelType, class ChannelCompleter, class NewsCompleter>
class ChannelUpdater : public AbstractChannelUpdater
{
public:
	/**
	 * \brief Constructor
	 *
	 * \param channel the channel to update
	 * \param parent the parent object
	 */
	ChannelUpdater(ChannelType* channel, QObject* parent = nullptr);

	/**
	 * \brief Destructor
	 */
	virtual ~ChannelUpdater();

	/**
	 * \brief Copy constructor is disabled
	 */
	ChannelUpdater(const ChannelUpdater&) = delete;

	/**
	 * \brief Move constructor is disabled
	 */
	ChannelUpdater(ChannelUpdater&&) = delete;

	/**
	 * \brief Copy operator is disabled
	 */
	ChannelUpdater& operator=(const ChannelUpdater&) = delete;

	/**
	 * \brief Move operator is disabled
	 */
	ChannelUpdater& operator=(ChannelUpdater&&) = delete;

	/**
	 * \brief The function called by the rss parser when parsing has
	 *        finished
	 *
	 * \param success set to true if reading was successful, to false
	 *                otherwise
	 * \param reason if reading was not successful, the description of the
	 *               error
	 */
	virtual void allNewsReceived(bool success, QString reason = QString()) override;

	/**
	 * \brief Updates the channel reading data from the rss url
	 */
	virtual void update() override;

	/**
	 * \brief Removes all news
	 */
	virtual void clearAllNews() override;

private:
	/**
	 * \brief The function called when the ChannelCompleter has finished its
	 *        job
	 */
	void channelCompleterFinished();

	/**
	 * \brief The function called when the completer for all news has
	 *        finished its job
	 */
	void allNewsCompleterFinished();

	/**
	 * \brief The channel we update
	 */
	ChannelType* const m_channel;

	/**
	 * \brief The RSS parser
	 *
	 * The parser exists only when updating the list of news
	 */
	std::unique_ptr<RssParser> m_parser;

	/**
	 * \brief  The channel completer
	 *
	 * The channel completer exists only when completing the channel
	 */
	std::unique_ptr<ChannelCompleter> m_channelCompleter;

	/**
	 * \brief  The object completing all news
	 *
	 * This exists only when completing news
	 */
	std::unique_ptr<AllNewsCompleter<ChannelType, NewsCompleter>> m_allNewsCompleter;

	/**
	 * \brief If true performs an update as soon as all news are completed
	 */
	bool m_updateWhenNewsCompleted;

	/**
	 * \brief If true news are deleted as soon as news update or completion
	 *        finishes
	 */
	bool m_clearNewsWhenPossible;
};

// Implementation of template functions of ChannelUpdater
#include "include/rssparser.h"
#include "include/allnewscompleter.h"

template <class ChannelType, class ChannelCompleter, class NewsCompleter>
ChannelUpdater<ChannelType, ChannelCompleter, NewsCompleter>::ChannelUpdater(ChannelType* channel, QObject* parent)
	: AbstractChannelUpdater(parent)
	, m_channel(channel)
	, m_parser()
	, m_channelCompleter()
	, m_allNewsCompleter()
	, m_updateWhenNewsCompleted(false)
	, m_clearNewsWhenPossible(false)
{
}

template <class ChannelType, class ChannelCompleter, class NewsCompleter>
ChannelUpdater<ChannelType, ChannelCompleter, NewsCompleter>::~ChannelUpdater()
{
	// Nothing to do here
}

template <class ChannelType, class ChannelCompleter, class NewsCompleter>
void ChannelUpdater<ChannelType, ChannelCompleter, NewsCompleter>::allNewsReceived(bool success, QString reason)
{
	// All data received, telling the channel (so that it can emit the signal) and scheduling the
	// parser for removal (using a command event)
	m_channel->finishedUpdatingData();
	QCoreApplication::postEvent(this, new CommandEvent([ptr = m_parser.release()]() { delete ptr; } ));

	// If there was an error, also emitting the error signal
	if (!success) {
		emit error(reason);
	}

	// If the complete removal of all news was scheduled, removing everything and returning
	if (m_clearNewsWhenPossible) {
		clearAllNews();

		return;
	}

	// Completing the channel. News will be completed when the channel is complete
	m_channelCompleter = std::make_unique<ChannelCompleter>(this, [this]() { this->channelCompleterFinished(); });
	m_channelCompleter->start();
}

template <class ChannelType, class ChannelCompleter, class NewsCompleter>
void ChannelUpdater<ChannelType, ChannelCompleter, NewsCompleter>::update()
{
	// If there are news to be completed or are already updating, we skip the update now and
	// schedule it for when all news have been completed
	if (m_parser || m_channelCompleter || m_allNewsCompleter) {
		m_updateWhenNewsCompleted = true;

		return;
	}

	m_updateWhenNewsCompleted = false;

	// Creating a parser and starting it
	m_parser = std::make_unique<RssParser>(m_channel, this);

	// Signalling we are about to update data
	m_channel->startUpdatingData();

	m_parser->fetch();
}

template <class ChannelType, class ChannelCompleter, class NewsCompleter>
void ChannelUpdater<ChannelType, ChannelCompleter, NewsCompleter>::clearAllNews()
{
	// If we are fetching news or completing some news, we have to schedule the removal
	// of all news for later
	if (m_parser || m_channelCompleter || m_allNewsCompleter) {
		m_clearNewsWhenPossible = true;

		return;
	}

	m_clearNewsWhenPossible = false;

	// Removing all news
	m_channel->clearAllNews();
}

template <class ChannelType, class ChannelCompleter, class NewsCompleter>
void ChannelUpdater<ChannelType, ChannelCompleter, NewsCompleter>::channelCompleterFinished()
{
	// Scheduling the removal of the channel completer
	QCoreApplication::postEvent(&(CommandEventReceiver::instance()), new CommandEvent([ptr = m_channelCompleter.release()]() { delete ptr; }));

	bool completeNews = true;

	// If the complete removal of all news was scheduled, doing it
	if (m_clearNewsWhenPossible) {
		clearAllNews();

		completeNews = false;
	}

	// If an updated was scheduled, doing it
	if (m_updateWhenNewsCompleted) {
		update();

		completeNews = false;
	}

	// Completing all news if we have to
	if (completeNews) {
		m_allNewsCompleter = std::make_unique<AllNewsCompleter<ChannelType, NewsCompleter>>(m_channel, [this]() { this->allNewsCompleterFinished(); });
		m_allNewsCompleter->start();
	}
}

template <class ChannelType, class ChannelCompleter, class NewsCompleter>
void ChannelUpdater<ChannelType, ChannelCompleter, NewsCompleter>::allNewsCompleterFinished()
{
	// Scheduling the removal of the all news completer
	QCoreApplication::postEvent(this, new CommandEvent([ptr = m_allNewsCompleter.release()]() { delete ptr; }));

	// If the complete removal of all news was scheduled, doing it
	if (m_clearNewsWhenPossible) {
		clearAllNews();
	}

	// If an updated was scheduled, doing it
	if (m_updateWhenNewsCompleted) {
		update();
	}
}

#endif
