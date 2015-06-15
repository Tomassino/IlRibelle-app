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

#ifndef __DEFAULT_CHANNEL_COMPLETER_H__
#define __DEFAULT_CHANNEL_COMPLETER_H__

#include <functional>
#include <Qt>

/**
 * \brief The default channel completer
 *
 * Channel completers are objects which take a channel and fill all the fields
 * in the channel that couldn't be filled by the RssParser (because the needed
 * information was not in the rss stream) or modify fields that have already
 * been filled. They are used as a template parameter of the Channel class. All
 * completers must satisfy the following requirements:
 *	- they must have a constructor taking two parameters, namely a pointer
 *	  to the channel and a std::function<void()> object. The latter must be
 *	  called when the channel completer has finished completing the channel.
 *	  After that functional is called, the channel completer can be
 *	  destroyed at any time;
 *	- they must also have a start() function that is called to start
 *	  completing the channel and that can be called multiple times to
 *	  complete the channel once more.
 * This class is the default channel completer that does nothing (the start()
 * function simply calls the functor)
 */
class DefaultChannelCompleter
{
public:
	/**
	 * \brief Constructor
	 *
	 * \param channel the channel to complete
	 * \param workFinishedCallback the functor called when this has finished
	 *                             its work
	 */
	template<class ChannelType>
	DefaultChannelCompleter(ChannelType* channel, const std::function<void()>& workFinishedCallback)
		: m_workFinishedCallback(workFinishedCallback)
	{
		Q_UNUSED(channel)
	}

	/**
	 * \brief Destructor
	 */
	~DefaultChannelCompleter()
	{
	}

	/**
	 * \brief Starts completing news
	 *
	 * This simply calls the functor
	 */
	void start()
	{
		m_workFinishedCallback();
	}

private:
	/**
	 * \brief The functor to call to signal that we have finished our work
	 */
	std::function<void()> m_workFinishedCallback;
};

#endif
