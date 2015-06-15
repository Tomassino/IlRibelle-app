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

#ifndef __STANDARD_ROLES_H__
#define __STANDARD_ROLES_H__

#include "include/roles.h"

/**
 * \brief The namespace with standard roles for news
 *
 * This namespace contains the definitions of all standard roles for a News
 */
namespace NewsRoles {
	/**
	 * \brief The tile of the news
	 */
	DEFINE_ROLE(title, toString)

	/**
	 * \brief The URL of the news
	 */
	DEFINE_ROLE(link, toUrl)

	/**
	 * \brief The news synopsis
	 */
	DEFINE_ROLE(description, toString)

	/**
	 * \brief Email address of the author of the news
	 */
	DEFINE_ROLE(authorEMail, toString)

	/**
	 * \brief The categories for the news
	 */
	DEFINE_ROLE(categories, toStringList)

	/**
	 * \brief The URL of a media object that is attached to the news
	 */
	DEFINE_ROLE(enclosureUrl, toUrl)

	/**
	 * \brief The length of a media object that is attached to the news
	 */
	DEFINE_ROLE(enclosureLength, toUInt)

	/**
	 * \brief The mime type of a media object that is attached to the news
	 */
	DEFINE_ROLE(enclosureType, toString)

	/**
	 * \brief The permanent link to the news
	 */
	DEFINE_ROLE(permalink, toUrl)

	/**
	 * \brief Indicates when the news was published
	 */
	DEFINE_ROLE(pubDate, toDateTime)

	/**
	 * \brief The news creator
	 */
	DEFINE_ROLE(creator, toString)

	/**
	 * \brief The QML Item that is used to disply this news (an  url from
	 *        where the item is loaded)
	 */
	DEFINE_ROLE(qmlItem, toUrl)

	/**
	 * \brief The list of files attached to this news (e.g. images)
	 */
	DEFINE_ROLE(attachedFiles, toStringList)

	/**
	 * \brief Whether the news is complete
	 */
	DEFINE_ROLE(complete, toBool)
}

/**
 * \brief The standard list of roles for a news
 *
 * All news must have these roles
 */
using StandardNewsRoles = RolesList<NewsRoles::title, NewsRoles::link, NewsRoles::description, NewsRoles::authorEMail, NewsRoles::categories, NewsRoles::enclosureUrl, NewsRoles::enclosureLength, NewsRoles::enclosureType, NewsRoles::permalink, NewsRoles::pubDate, NewsRoles::creator, NewsRoles::qmlItem, NewsRoles::attachedFiles, NewsRoles::complete>;

/**
 * \brief The namespace with standard roles for channels
 *
 * This namespace contains the definitions of all standard roles for a Channel
 */
namespace ChannelRoles {
	/**
	 * \brief The site URL
	 */
	DEFINE_ROLE(siteUrl, toUrl)

	/**
	 * \brief The title of the channel
	 */
	DEFINE_ROLE(title, toString)

	/**
	 * \brief The url of the website corresponding to the channel
	 */
	DEFINE_ROLE(link, toUrl)

	/**
	 * \brief The description of the channel
	 */
	DEFINE_ROLE(description, toString)

	/**
	 * \brief The language of the channel
	 */
	DEFINE_ROLE(language, toString)

	/**
	 * \brief The copyright notice for content in the channel
	 */
	DEFINE_ROLE(copyright, toString)

	/**
	 * \brief The email address for person responsible for editorial content
	 */
	DEFINE_ROLE(managingEditor, toString)

	/**
	 * \brief The email address for person responsible for technical issues
	 *        relating to the channel
	 */
	DEFINE_ROLE(webMaster, toString)

	/**
	 * \brief The publication date for the content in the channel
	 */
	DEFINE_ROLE(pubDate, toDateTime)

	/**
	 * \brief The last time the content of the channel changed
	 */
	DEFINE_ROLE(lastBuildDate, toDateTime)

	/**
	 * \brief The categories for the channel
	 */
	DEFINE_ROLE(categories, toStringList)

	/**
	 * \brief The time-to-live in minutes of the channel (i.e. for how long
	 *        the channel should be cached)
	 */
	DEFINE_ROLE(ttl, toUInt)

	/**
	 * \brief The URL of the image that represents the channel
	 */
	DEFINE_ROLE(imageUrl, toUrl)

	/**
	 * \brief The description of the link of the image
	 */
	DEFINE_ROLE(imageDescription, toString)

	/**
	 * \brief The list of files attached to this channel
	 */
	DEFINE_ROLE(attachedFiles, toStringList)
}

/**
 * \brief The standard list of roles for a channel
 *
 * All channels must have these roles
 */
using StandardChannelRoles = RolesList<ChannelRoles::siteUrl, ChannelRoles::title, ChannelRoles::link, ChannelRoles::description, ChannelRoles::language, ChannelRoles::copyright, ChannelRoles::managingEditor, ChannelRoles::webMaster, ChannelRoles::pubDate, ChannelRoles::lastBuildDate, ChannelRoles::categories, ChannelRoles::ttl, ChannelRoles::imageUrl, ChannelRoles::imageDescription, ChannelRoles::attachedFiles>;

#endif
