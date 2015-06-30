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

#ifndef __IL_RIBELLE_CHANNEL_H__
#define __IL_RIBELLE_CHANNEL_H__

#include "include/news.h"
#include "include/channel.h"
#include "include/defaultchannelcompleter.h"
#include "include/standardroles.h"

/**
 * \brief The namespace with additional roles for news from www.ilribelle.com
 */
namespace IlRibelleRoles
{
	/**
	 * \brief The url of the main image of the article
	 */
	DEFINE_ROLE(mainImageUrl, toUrl)

	/**
	 * \brief The name of the file of the main image of the article
	 */
	DEFINE_ROLE(mainImageFile, toString)

	/**
	 * \brief Whether the news has a linked audio resource
	 */
	DEFINE_ROLE(hasAudioResource, toBool)

	/**
	 * \brief The URL of the audio resource
	 */
	DEFINE_ROLE(audioResourceUrl, toUrl)

	/**
	 * \brief The path where the audio resource could be saved
	 */
	DEFINE_ROLE(audioResourcePath, toString)

	/**
	 * \brief The title of the audio resource
	 */
	DEFINE_ROLE(audioResourceTitle, toString)

	/**
	 * \brief The author of the audio resource
	 */
	DEFINE_ROLE(audioResourceAuthor, toString)

	/**
	 * \brief The length in milliseconds of the audio resource
	 */
	DEFINE_ROLE(audioResourceDuration, toUInt)

	/**
	 * \brief Whether the news has a livestream link
	 */
	DEFINE_ROLE(hasLivestreamLink, toBool)

	/**
	 * \brief The URL of the livestream link
	 */
	DEFINE_ROLE(livestreamUrl, toUrl)

	/**
	 * \brief Additional iframes urls
	 *
	 * These are the links found inside iframe tags
	 */
	DEFINE_ROLE(iframeUrls, toStringList)
}

/**
 * \brief The list with additional roles for news from www.ilribelle.com
 */
using IlRibelleRolesList = RolesList<IlRibelleRoles::mainImageUrl, IlRibelleRoles::mainImageFile, IlRibelleRoles::hasAudioResource, IlRibelleRoles::audioResourceUrl, IlRibelleRoles::audioResourcePath, IlRibelleRoles::audioResourceTitle, IlRibelleRoles::audioResourceAuthor, IlRibelleRoles::audioResourceDuration, IlRibelleRoles::hasLivestreamLink, IlRibelleRoles::livestreamUrl, IlRibelleRoles::iframeUrls>;

/**
 * \brief The type for the news of www.ilibelle.com
 */
using IlRibelleNews = News<RolesList<StandardNewsRoles, IlRibelleRolesList>>;

/**
 * \brief The type for the channel of www.ilribelle.com
 */
using IlRibelleChannel = Channel<StandardChannelRoles, IlRibelleNews>;

#endif
