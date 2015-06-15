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

#ifndef __NEWS_H__
#define __NEWS_H__

#include <QString>
#include <QUrl>
#include <QDateTime>
#include <QByteArray>
#include <QList>
#include <QStringList>
#include <QJsonObject>
#include <QVariant>
#include <QDebug>
#include <array>
#include <type_traits>
#include "include/utilities.h"
#include "include/standardroles.h"

/**
 * \brief The class modelling a single news
 *
 * This class models a single news. The roles of the news are given by the Roles
 * parameter, which must always contain StandardNewsRoles. External files
 * belonging to this news are listed here in the AttachedFiles role.
 * Manipulating the list of attached files (adding or removing a file) does not
 * influence the actual files. You must ensure the list and the actual files are
 * syncronized externally, e.g. removing all files when the news is removed.
 * Consider using function in channel to manipulate the files attached to a news
 * because they ensure consistency between the list stored here and the files on
 * disk. This class also stores the URL of the QML Item that is used to show the
 * news and a flag signalling if the news is complete (i.e. all data relative to
 * the news has been downloaded and the news is ready to be read) or not. Data
 * is stored using a list of QVariant, instead of having separated fields of the
 * native type. This is so because the model we use to communicate with the GUI
 * uses roles (i.e. indexes in a list of QVariants) to request data. This class
 * also has functions to serialize data as a JSON object (which is easy to store
 * and fast to encode/decode). Finally, each news has an unsigned integer id
 * that is set in the constructor and cannot be changed. It should be unique,
 * meaning that during program execution no two news should have the same id (it
 * is not stored, though, so the same news can have different ids in different
 * executions)
 */
template <class RolesListType>
class News : public Roles<RolesListType>
{
	// RolesListType must contain StandardNewsRoles
	static_assert(std::is_convertible<RolesListType, StandardNewsRoles>::value, "The list of roles in News must contain the StandardNewsRoles list");

public:
	/**
	 * \brief Constructor
	 *
	 * \param id the news ID
	 */
	News(unsigned int id)
		: Roles<RolesListType>()
		, m_id(id)
	{
	}

	/**
	 * \brief Copy constructor
	 *
	 * \param other the news to copy
	 */
	News(const News<RolesListType>& other)
		: Roles<RolesListType>(other)
	{
	}

	/**
	 * \brief Move constructor
	 *
	 * \param other the news to move here
	 */
	News(News<RolesListType>&& other)
		: Roles<RolesListType>(std::move(other))
	{
	}

	/**
	 * \brief Destructor
	 */
	~News()
	{
		// Nothing to do here
	}

	/**
	 * \brief Equality operator
	 *
	 * Two news are considered equal if they have the same publication date
	 * and the same title
	 * \return true if the two news are equal
	 */
	bool operator==(const News<RolesListType>& other) const
	{
		const auto& thisPubDate = this->template getData<NewsRoles::pubDate>();
		const auto& otherPubDate = other.template getData<NewsRoles::pubDate>();
		const auto& thisTitle = this->template getData<NewsRoles::title>();
		const auto& otherTitle = other.template getData<NewsRoles::title>();
		return (thisPubDate == otherPubDate) && (thisTitle == otherTitle);
	}

	/**
	 * \brief Disequality operator
	 *
	 * Two news are considered equal if they have the same publication date
	 * and the same title
	 * \return true if the two news are equal
	 */
	bool operator!=(const News<RolesListType>& other) const
	{
		return !operator==(other);
	}

	/**
	 * \brief Less-than operator
	 *
	 * First the publication dates are compared, then, if they are the same,
	 * the titles are compared
	 * \return true if this news is less than the other
	 */
	bool operator<(const News<RolesListType>& other) const
	{
		const auto& thisPubDate = this->template getData<NewsRoles::pubDate>();
		const auto& otherPubDate = other.template getData<NewsRoles::pubDate>();
		const auto& thisTitle = this->template getData<NewsRoles::title>();
		const auto& otherTitle = other.template getData<NewsRoles::title>();
		return (thisPubDate < otherPubDate) ||
		       ((thisPubDate == otherPubDate) && (thisTitle < otherTitle));
	}

	/**
	 * \brief Greater-than operator
	 *
	 * First the publication dates are compared, then, if they are the same,
	 * the titles are compared
	 * \return true if this news is greater than tnewshe other
	 */
	bool operator>(const News<RolesListType>& other) const
	{
		const auto& thisPubDate = this->template getData<NewsRoles::pubDate>();
		const auto& otherPubDate = other.template getData<NewsRoles::pubDate>();
		const auto& thisTitle = this->template getData<NewsRoles::title>();
		const auto& otherTitle = other.template getData<NewsRoles::title>();
		return (thisPubDate > otherPubDate) ||
		       ((thisPubDate == otherPubDate) && (thisTitle > otherTitle));
	}

	/**
	 * \brief Returns the news id
	 *
	 * \return the news id
	 */
	unsigned int id()
	{
		return m_id;
	}

	/**
	 * \brief Clears this news
	 *
	 * All roles are reset, but no callback is called
	 */
	void reset()
	{
		this->Roles<RolesListType>::reset(true);
	}

	/**
	 * \brief Reads the news from a JSON object
	 *
	 * This resets the news before reading
	 * \param obj the JSON object from which we read
	 * \return false in case of error, true if the news was read correctly
	 */
	bool load(const QJsonObject& obj)
	{
		reset();

		// Iterating the JSON object and converting key to a Role. If conversion fails, returning false
		for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
			const int r = this->getRoleIndexFromName(it.key().toLatin1());

			if (r == -1) {
				return false;
			}

			this->setVariantData(r, it.value().toVariant());
		}

		return true;
	}

	/**
	 * \brief Writes the news as a JSON object
	 *
	 * \return the JSON object representing the news
	 */
	QJsonObject save() const
	{
		// Building a QVariantMap from the values we have
		QVariantMap map;

		for (int i = 0; i < Roles<RolesListType>::numRoles(); ++i) {
			map[this->getRoleNameFromIndex(i)] = this->getVariantData(i);
		}

		// Now building a JSON object from the variant map
		return QJsonObject::fromVariantMap(map);
	}

	/**
	 * \brief Returns data for the Role with the given index
	 *
	 * \param i the index of the Role
	 * \return data for the role
	 */
	QVariant data(unsigned int i) const
	{
		return (i < this->numRoles()) ? this->getVariantData(i) : QVariant();
	}

private:
	/**
	 * \brief The news ID
	 */
	unsigned int m_id;
};

#endif
