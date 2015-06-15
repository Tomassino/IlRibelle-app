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

#ifndef __ALL_NEWS_COMPLETER_H__
#define __ALL_NEWS_COMPLETER_H__

#include <QSet>
#include <functional>
#include <QHash>
#include <QCoreApplication>
#include "include/utilities.h"

/**
 * \brief The class completing all news of a channel
 *
 * This class completes all news from the given channel. The template parameter
 * Channel is the Channel class (must be a template instantiation of the Channel
 * class). Instances of this class call the workFinishedCallback functional when
 * all news have been completed. After calling start(), you should wait for the
 * functional to  be called before deleting this object. If you call start more
 * than once while news are already been completed, nothing happends. If you
 * call start again after the functional has been called, news are completed
 * again (only news that are not yet complete)
 */
template <class ChannelType, class NewsCompleter>
class AllNewsCompleter
{
public:
	/**
	 * \brief Constructor
	 *
	 * \param channel the channel with the news to complete
	 * \param workFinishedCallback the functional to call when all news have
	 *                             been completed
	 * \param maxParallelNews the maximum number of news that are completed
	 *                        in parallel. If negative all news are
	 *                        completed in parallel
	 */
	AllNewsCompleter(ChannelType* channel, const std::function<void()>& workFinishedCallback, int maxParallelNews = 3);

	/**
	 * \brief Destructor
	 */
	~AllNewsCompleter();

	/**
	 * \brief Starts completing news
	 *
	 * This function actually starts completing news. If news are already
	 * been completed, this does nothing
	 */
	void start();

private:
	/**
	 * \brief The function called when parsing of a news is completed
	 *
	 * If there are no more news to complete, call the workFinishedCallback
	 * functional
	 * \param id the id of the news that was parsed
	 */
	void parsingCompleted(unsigned int id);

	/**
	 * \brief The function which starts the completion process for one news
	 */
	void completeNextNews();

	/**
	 * \brief A helper function to extract the type of the News from the
	 *        channel
	 *
	 * This function is not implemented because is only used as the
	 * parameter of decltype to get the type of the news from Channel
	 * \param dummy a dummy parameter for template resolutions
	 */
	template <class Roles, class NewsType, template <class, class> class ChannelType_t>
	static NewsType newsTypeForChannel(Type2Type<ChannelType_t<Roles, NewsType>> dummy);

	/**
	 * \brief The type of news in the channel
	 */
	using News = decltype(newsTypeForChannel(Type2Type<ChannelType>()));

	/**
	 * \brief The channel to complete
	 */
	ChannelType* const m_channel;

	/**
	 * \brief The functional called when all news have been completed
	 */
	std::function<void()> m_workFinishedCallback;

	/**
	 * \brief The maximum number of news that are completed in parallel
	 *
	 * If negative all news are completed in parallel
	 */
	const int m_maxParallelNews;

	/**
	 * \brief The set of news that will be completed. Elements of the set
	 *        are news ids
	 */
	QSet<int> m_newsToComplete;

	/**
	 * \brief The number of news being currently completed
	 */
	int m_numNewsBeingCompleted;

	/**
	 * \brief The map of active news completers
	 *
	 * This stores the news completer for the given news id. We need this to
	 * destroy the news completers that have finished their job
	 */
	QHash<int, NewsCompleter*> m_activeNewsCompleters;
};

// Implemetation of template functions

template <class ChannelType, class NewsCompleter>
AllNewsCompleter<ChannelType, NewsCompleter>::AllNewsCompleter(ChannelType* channel, const std::function<void()>& workFinishedCallback, int maxParallelNews)
	: m_channel(channel)
	, m_workFinishedCallback(workFinishedCallback)
	, m_maxParallelNews(maxParallelNews)
	, m_newsToComplete()
	, m_numNewsBeingCompleted(0)
	, m_activeNewsCompleters()
{
}

template <class ChannelType, class NewsCompleter>
AllNewsCompleter<ChannelType, NewsCompleter>::~AllNewsCompleter()
{
	// Nothing to do here
}

template <class ChannelType, class NewsCompleter>
void AllNewsCompleter<ChannelType, NewsCompleter>::start()
{
	// Checking if news are already been completed
	if (m_numNewsBeingCompleted != 0) {
		return;
	}

	m_newsToComplete.clear();
	// Filling the set of news to complete with all news not already completed
	for (int i = 0; i < m_channel->numNews(); ++i) {
		if (!m_channel->news(i).template getData<NewsRoles::complete>()) {
			m_newsToComplete.insert(m_channel->news(i).id());
		}
	}

	// Checking that we have at least one news to complete
	if (m_newsToComplete.isEmpty()) {
		// Nothing to do, we can call the callback
		m_workFinishedCallback();

		return;
	}

	// Requesting data for the news to complete. We do at most m_maxParallelNews, but if
	// m_maxParallelNews is negative, we request all news now
	const auto newsToRequest = (m_maxParallelNews < 0) ? m_newsToComplete.size() : m_maxParallelNews;
	while ((m_numNewsBeingCompleted != newsToRequest) && (!m_newsToComplete.isEmpty())) {
		completeNextNews();
	}
}

template <class ChannelType, class NewsCompleter>
void AllNewsCompleter<ChannelType, NewsCompleter>::parsingCompleted(unsigned int id)
{
	// Scheduling the news completer for removal
	QCoreApplication::postEvent(&(CommandEventReceiver::instance()), new CommandEvent([ptr = m_activeNewsCompleters[id]]() { delete ptr; }));

	m_activeNewsCompleters.remove(id);

	// Here we decrement the number of news being completed
	--m_numNewsBeingCompleted;

	// Checking if we have completed everything
	if (m_newsToComplete.isEmpty() && (m_numNewsBeingCompleted == 0)) {
		// Ok, we can call the callback
		m_workFinishedCallback();

		return;
	} else if (!m_newsToComplete.isEmpty()) {
		// If we get here we can enqueue another request
		completeNextNews();
	}
}

template <class ChannelType, class NewsCompleter>
void AllNewsCompleter<ChannelType, NewsCompleter>::completeNextNews()
{
	// Extracting the first value from the set
	const int newsId = *(m_newsToComplete.begin());
	m_newsToComplete.erase(m_newsToComplete.begin());

	// Creating the object that will get and parse the webpage
	News& news = m_channel->news(m_channel->newsIndexByID(newsId));
	auto callback = [this, newsId]() { this->parsingCompleted(newsId); };
	NewsCompleter* newsCompleter = new NewsCompleter(m_channel, &news, callback);

	// Storin the news completer to be able to delete it when done
	m_activeNewsCompleters[newsId] = newsCompleter;

	// Incrementing the number of requests
	++m_numNewsBeingCompleted;

	// Starting parser
	newsCompleter->start();
}

#endif
