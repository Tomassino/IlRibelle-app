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

#ifndef __NEWS_LIST_MODEL_H__
#define __NEWS_LIST_MODEL_H__

#include <QAbstractListModel>
#include <QModelIndex>
#include <QList>
#include "include/channel.h"
#include "include/rolesqmlaccessor.h"

/**
 * \brief This is needed because moc does not support template classes
 *
 * See class NewsListModel for information
 */
class AbstractNewsListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	/**
	 * \brief Constructor
	 *
	 * \param parent the parent object
	 */
	AbstractNewsListModel(QObject* parent)
		: QAbstractListModel(parent)
	{
	}

	/**
	 * \brief Destructor
	 */
	virtual ~AbstractNewsListModel()
	{
	}

	/**
	 * \brief Returns the roles qml accessor for the news with the given
	 *        url
	 *
	 * If the news is not in our list, this function returns nullptr
	 * \param newsUrl the url of the news to return
	 * \return the roles qml accessor for the news or nullptr if the news is
	 *         not in our list
	 */
	virtual AbstractRolesQMLAccessor* getAccessorForNewsUrl(QUrl newsUrl) = 0;

private slots:
	/**
	 * \brief The slot called when a new news is about to be added
	 *
	 * This calls beginInsertRows() to inform views that data is about to
	 * change
	 * \param index the index where the news will be added
	 */
	virtual void aboutToAddNews(int index) = 0;

	/**
	 * \brief The slot called when a news has been added
	 *
	 * This calls endInsertRows() to inform views that data has been changed
	 */
	virtual void newsAdded() = 0;

	/**
	 * \brief The slot calles when some news are about to be deleted
	 *
	 * This is calls beginRemoveRows() to inform views that data is about to
	 * change
	 * \param startIndex the starting index of the news to be deleted
	 * \param endIndex the ending index of the news to be deleted
	 */
	virtual void aboutToDeleteNews(int startIndex, int endIndex) = 0;

	/**
	 * \brief The slot called when news have been deletes
	 *
	 * This calls endRemoveRows() to inform views that data has been changed
	 */
	virtual void newsDeleted() = 0;

	/**
	 * \brief The slot called when a news has been changed
	 *
	 * \param index the index of the news that has been updated
	 * \param roles the list of roles that have changed
	 */
	virtual void newsUpdated(int index, const QVector<int>& roles) = 0;
};

/**
 * \brief The model for the list of news of a single channel
 *
 * This is the model we use to display the list of news. The template parameter
 * ChannelType is the type of the channel. This provides a special role named
 * "roles" (whose id is Qt::UserRole) that can be used to get the roles qml
 * accessor object for the news
 */
template <class ChannelType>
class NewsListModel : public AbstractNewsListModel
{
public:
	/**
	 * \brief The type of news used here
	 */
	using NewsType = typename ChannelType::StoredNewsType;

public:
	/**
	 * \brief Constructor
	 *
	 * \param channel the channel whose news we show. This is also the
	 *                parent object
	 */
	NewsListModel(ChannelType* channel);

	/**
	 * \brief Destructor
	 */
	virtual ~NewsListModel();

	/**
	 * \brief Returns the roles qml accessor for the news with the given
	 *        url
	 *
	 * If the news is not in our list, this function returns nullptr
	 * \param newsUrl the url of the news to return
	 * \return the roles qml accessor for the news or nullptr if the news is
	 *         not in our list
	 */
	virtual AbstractRolesQMLAccessor* getAccessorForNewsUrl(QUrl newsUrl);

	/**
	 * \brief Returns the number of rows in the model
	 *
	 * \param parent the parent index (here we do not have hierarchical
	 *               data, so we ignore the parent index)
	 * \return the number of rows in the model (i.e. the number of news)
	 */
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

	/**
	 * \brief Returns the data at the given index
	 *
	 * \param index the index of data to return
	 * \param role the role to return
	 * \return the data at the given index
	 */
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	/**
	 * \brief Returns the data for the header
	 *
	 * \param section the index of the header (whether a row or column index
	 *                depends on the orientation)
	 * \param orientation the orientation for which the header is requested
	 * \param role the role to return
	 * \return the data for the header
	 */
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	/**
	 * \brief Returns names of all possible roles
	 *
	 * \return the names of all possible roles. Key is the role id, value is
	 *         the role name
	 */
	virtual QHash<int, QByteArray> roleNames() const override;

private:
	/**
	 * \brief The slot called when a new news is about to be added
	 *
	 * This calls beginInsertRows() to inform views that data is about to
	 * change
	 * \param index the index where the news will be added
	 */
	void aboutToAddNews(int index) override;

	/**
	 * \brief The slot called when a news has been added
	 *
	 * This calls endInsertRows() to inform views that data has been changed
	 */
	void newsAdded() override;

	/**
	 * \brief The slot calles when some news are about to be deleted
	 *
	 * This is calls beginRemoveRows() to inform views that data is about to
	 * change
	 * \param startIndex the starting index of the news to be deleted
	 * \param endIndex the ending index of the news to be deleted
	 */
	void aboutToDeleteNews(int startIndex, int endIndex) override;

	/**
	 * \brief The slot called when news have been deletes
	 *
	 * This calls endRemoveRows() to inform views that data has been changed
	 */
	void newsDeleted() override;

	/**
	 * \brief The slot called when a news has been changed
	 *
	 * \param index the index of the news that has been updated
	 * \param roles the list of roles that have changed
	 */
	void newsUpdated(int index, const QVector<int>& roles) override;

	/**
	 * \brief The channel to model
	 */
	ChannelType* const m_channel;

	/**
	 * \brief The list of qml accessors for news in the channel
	 */
	QList<RolesQMLAccessor<NewsType>*> m_qmlAccessors;

	/**
	 * \brief The index of the accessor to add
	 */
	int m_accessorToAddIndex;
};

// Implementation of template functions
#include <QDebug>

template <class ChannelType>
NewsListModel<ChannelType>::NewsListModel(ChannelType* channel)
	: AbstractNewsListModel(channel)
	, m_channel(channel)
	, m_qmlAccessors()
	, m_accessorToAddIndex(0)
{
	// Connecting signals from channel
	connect(m_channel, &ChannelType::aboutToAddNews, this, &NewsListModel::aboutToAddNews);
	connect(m_channel, &ChannelType::newsAdded, this, &NewsListModel::newsAdded);
	connect(m_channel, &ChannelType::aboutToDeleteNews, this, &NewsListModel::aboutToDeleteNews);
	connect(m_channel, &ChannelType::newsDeleted, this, &NewsListModel::newsDeleted);
	connect(m_channel, &ChannelType::newsUpdated, this, &NewsListModel::newsUpdated);

	// Creating roles qml accessors for existing news
	for (int i = 0; i < m_channel->numNews(); ++i) {
		m_qmlAccessors.append(new RolesQMLAccessor<NewsType>(&(m_channel->news(i)), this));
	}
}

template <class ChannelType>
NewsListModel<ChannelType>::~NewsListModel()
{
	// Deleting all qml news accessors
	for (auto accessor: m_qmlAccessors) {
		delete accessor;
	}
}

template <class ChannelType>
AbstractRolesQMLAccessor* NewsListModel<ChannelType>::getAccessorForNewsUrl(QUrl newsUrl)
{
	const auto id = m_channel->newsIDForURL(newsUrl);

	// Trying to get the index. If invalid, returning nullptr, otherwise the roles qml accessor
	const auto index = m_channel->newsIndexByID(id);

	if (index == -1) {
		return nullptr;
	} else {
		return m_qmlAccessors[index];
	}
}

template <class ChannelType>
int NewsListModel<ChannelType>::rowCount(const QModelIndex&) const
{
	return m_channel->numNews();
}

template <class ChannelType>
QVariant NewsListModel<ChannelType>::data(const QModelIndex& index, int role) const
{
	// Checking the role is valid
	if (role < Qt::UserRole) {
		return QVariant();
	}

	// Now also checking the index is valid
	if ((!index.isValid()) || (index.row() >= m_channel->numNews())) {
		return QVariant();
	}

	// Returning data
	if (role == Qt::UserRole) {
		// Here we have to return the qml accessors. We cannor directly return it
		// because QVariant has no constructor taking a QObject*. We can however
		// use setValue that stores values of type T in a QMetaType if T is not
		// supported by QVariant directly. QMetaObject has special code to handle
		// QObject, so we just need to static_cast the qml accessor to QObject
		QVariant v;
		v.setValue(static_cast<QObject*>(m_qmlAccessors[index.row()]));
		return v;
	} else {
		// More checks that the role is valid
		const unsigned int myIntRole = role - (Qt::UserRole + 1);
		if (myIntRole > ChannelType::StoredNewsType::numRoles()) {
			return QVariant();
		}

		return m_channel->news(index.row()).data(myIntRole);
	}
}

template <class ChannelType>
QVariant NewsListModel<ChannelType>::headerData(int, Qt::Orientation, int role) const
{
	// Checking role is valid
	if (role < Qt::UserRole) {
		return QVariant();
	}

	// Returning the header
	if (role == Qt::UserRole) {
		return "Roles";
	} else {
		const unsigned int myIntRole = role - (Qt::UserRole + 1);
		if (myIntRole > ChannelType::StoredNewsType::numRoles()) {
			return QVariant();
		}

		return ChannelType::StoredNewsType::getRoleNameFromIndex(myIntRole);
	}
}

template <class ChannelType>
QHash<int, QByteArray> NewsListModel<ChannelType>::roleNames() const
{
	// We don't return the default roles, only ours
	QHash<int, QByteArray> names;

	// We can generate the list of roles for the news automatically (we just add the
	// "roles" role as the first one)
	names[Qt::UserRole] = "roles";
	for (unsigned int i = 0; i < ChannelType::StoredNewsType::numRoles(); ++i) {
		names[i + Qt::UserRole + 1] = ChannelType::StoredNewsType::getRoleNameFromIndex(i);
	}

	return names;
}

template <class ChannelType>
void NewsListModel<ChannelType>::aboutToAddNews(int index)
{
	// Signalling we are starting to insert rows (i.e. one news)
	beginInsertRows(QModelIndex(), index, index);

	// Storing the index of the accessor to add
	m_accessorToAddIndex = index;
}

template <class ChannelType>
void NewsListModel<ChannelType>::newsAdded()
{
	// Signalling rows have been added
	endInsertRows();

	// Adding the new accessor
	m_qmlAccessors.insert(m_accessorToAddIndex, new RolesQMLAccessor<NewsType>(&(m_channel->news(m_accessorToAddIndex)), this));
}

template <class ChannelType>
void NewsListModel<ChannelType>::aboutToDeleteNews(int startIndex, int endIndex)
{
	// Signalling we are starting to remove
	beginRemoveRows(QModelIndex(), startIndex, endIndex);

	// Removing accessors (we do this here instead of newsDeleted() because there
	// news have already been deleted
	for (int i = startIndex; i <= endIndex; ++i) {
		// We always use startIndex because as we remove one accessor all the subsequent
		// ones have their index decreased by one
		delete m_qmlAccessors[startIndex];
		m_qmlAccessors.removeAt(startIndex);
	}
}

template <class ChannelType>
void NewsListModel<ChannelType>::newsDeleted()
{
	// Signalling rows have been deleted
	endRemoveRows();
}

template <class ChannelType>
void NewsListModel<ChannelType>::newsUpdated(int newsIndex, const QVector<int>& roles)
{
	// We have to fix the index of roles
	QVector<int> fixedRoles(roles.size());
	for (int i = 0; i < fixedRoles.size(); ++i) {
		fixedRoles[i] = roles[i] + Qt::UserRole + 1;
	}

	// Emitting the dataChanged signal
	emit dataChanged(index(newsIndex), index(newsIndex), fixedRoles);
}


#endif
