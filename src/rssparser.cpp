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

#include <QObject>
#include "include/rssparser.h"
#include "include/networkmanager.h"
#include "include/utilities.h"
#include "include/news.h"
#include "include/channel.h"
#include "include/channelupdater.h"

RssParser::RssParser(AbstractChannel* channel, AbstractChannelUpdater* channelUpdater)
	: DataAvailableNotifee()
	, m_channel(channel)
	, m_channelUpdater(channelUpdater)
	, m_reader()
	, m_status(States::NotStarted)
	, m_dcNamespacePrefix()
	, m_openedTags()
	, m_channelCategories()
	, m_currentNewsRoles()
	, m_newsCategories()
	, m_guidIsPermalink(false)
{
}

RssParser::~RssParser()
{
	// Nothing to do here
}

void RssParser::fetch()
{
	// Starts fetching data. The id is ignored in all callbacks, putting a random number
	NM::instance().getFile(m_channel->standardRoles().getData<ChannelRoles::siteUrl>(), this, 17);
}

void RssParser::dataAvailable(int id)
{
	// Checking if we have already set the IO device for the reader
	if (m_reader.device() != reply(id)) {
		m_reader.setDevice(reply(id));
	}

	// Reads data that has just arrived and parses it
	while (!m_reader.atEnd()) {
		switch (m_status) {
			case States::NotStarted:
				startReadingDocument();
				break;
			case States::Started:
				readTag("rss", States::RssRead);
				break;
			case States::RssRead:
				// Here we store prefixes for namespaces we know.
				{
					const QXmlStreamNamespaceDeclarations namespaces = m_reader.namespaceDeclarations();
					for (auto it = namespaces.constBegin(); it != namespaces.constEnd(); ++it) {
						if (it->namespaceUri() == "http://purl.org/dc/elements/1.1/") {
							m_dcNamespacePrefix = it->prefix().toString();
						}
					}
				}
				readTag("channel", States::ReadingChannel);
				break;
			case States::ReadingChannel:
				readChannelInfo();
				break;
			case States::ReadingNews:
				readItemInfo();
				break;
			case States::FinishedReadingChannel:
				readClosingRssTag();
				break;
			case States::FinishedReadingRss:
				readEndDocument();
				break;
			case States::DocumentFinished:
				break;
		}
	}

	if ((m_reader.hasError()) && (m_reader.error() != QXmlStreamReader::PrematureEndOfDocumentError)) {
		// An error occurred, ending here
		m_channelUpdater->allNewsReceived(false, QObject::tr("Error parsing rss document: ") + m_reader.errorString());
	}

	// If we have finished reading the document, resetting internal state
	if (m_status == States::DocumentFinished) {
		m_status = States::NotStarted;
		m_openedTags.clear();
		m_channelCategories.clear();
	}
}

void RssParser::allDataAvailable(int)
{
	// Finished reading data, notifying the channel
	if (!m_reader.hasError()) {
		m_channelUpdater->allNewsReceived(true);
	} else {
		// An error occurred, Returning it
		m_channelUpdater->allNewsReceived(false, QObject::tr("Error parsing rss document: ") + m_reader.errorString());
	}
}

void RssParser::networkError(int, const QString& description)
{
	// A network error occurred
	m_channelUpdater->allNewsReceived(false, QObject::tr("Network error: ") + description);
}

void RssParser::startReadingDocument()
{
	while (!m_reader.atEnd()) {
		// We expect the token type to be StartDocument, otherwise an error occurred
		QXmlStreamReader::TokenType type = m_reader.readNext();

		if (type == QXmlStreamReader::StartDocument) {
			// We can change the status and return
			m_status = States::Started;

			return;
		} else if (type != QXmlStreamReader::Comment) {
			// Raising an error
			m_reader.raiseError(QObject::tr("Error waiting for document start"));

			return;
		}
	}
}

void RssParser::readTag(QString tag, States nextState)
{
	while (!m_reader.atEnd()) {
		// We expect the token type to be StartElement, and the element to be equal to tag,
		// otherwise an error occurred
		QXmlStreamReader::TokenType type = m_reader.readNext();

		if ((type == QXmlStreamReader::StartElement) && (m_reader.qualifiedName() == tag)) {
			// Ok tag found, we can change the status and return
			m_status = nextState;

			return;
		} else if ((type != QXmlStreamReader::Comment) && ((type != QXmlStreamReader::Characters) || (!m_reader.isWhitespace()))) {
			// Raising an error
			m_reader.raiseError(QObject::tr("Error waiting for tag") + QString(" (%1)").arg(tag));

			return;
		}
	}
}

void RssParser::readChannelInfo()
{
	while (!m_reader.atEnd()) {
		// Reading the next token
		QXmlStreamReader::TokenType type = m_reader.readNext();

		switch (type) {
			case QXmlStreamReader::StartElement:
				if (readChannelElementAndCheckItem()) {
					m_status = States::ReadingNews;

					// Resetting the news to start adding the new data
					m_currentNewsRoles.reset(true);
					m_newsCategories.clear();

					return;
				}
				break;
			case QXmlStreamReader::EndElement:
				// Checking nesting level: if we are at 0, the closed tag must be channel, so we
				// have finished reading the channel
				if (m_openedTags.size() == 0) {
					// Channel ended, changing status and returning
					m_status = States::FinishedReadingChannel;

					// Here we also set the list of categories
					m_channel->standardRoles().setData<ChannelRoles::categories>(m_channelCategories);

					return;
				} else {
					m_openedTags.removeLast();
				}
				break;
			case QXmlStreamReader::Characters:
				readChannelTagData();
				break;
			case QXmlStreamReader::Comment:
				// We do nothing here, just break to read the next token
				break;
			default:
				// If there is an error and is PrematureEndOfDocumentError, just exiting, otherwise
				// raising an error
				if (m_reader.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
					m_reader.raiseError(QObject::tr("Error reading item information") + QString(" (%1 - %2)").arg(static_cast<int>(type)).arg(m_reader.errorString()));

					return;
				}
		}
	}
}

void RssParser::readItemInfo()
{
	while (!m_reader.atEnd()) {
		// Reading the next token
		QXmlStreamReader::TokenType type = m_reader.readNext();

		switch (type) {
			case QXmlStreamReader::StartElement:
				readItemElement();
				break;
			case QXmlStreamReader::EndElement:
				// Decreasing nesting level and checking: if we go at 0, the closed tag must be item, so we
				// have finished reading the news
				m_openedTags.removeLast();
				if (m_openedTags.size() == 0) {
					// Item ended, changing status and returning
					m_status = States::ReadingChannel;

					// Here we also set the list of categories
					m_currentNewsRoles.setData<NewsRoles::categories>(m_newsCategories);

					// Adding the news
					m_channel->addStandardNews(m_currentNewsRoles);

					return;
				}
				break;
			case QXmlStreamReader::Characters:
				readItemTagData();
				break;
			case QXmlStreamReader::Comment:
				// We do nothing here, just break to read the next token
				break;
			default:
				// If there is an error and is PrematureEndOfDocumentError, just exiting, otherwise
				// raising an error
				if (m_reader.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
					m_reader.raiseError(QObject::tr("Error reading item information") + QString(" (%1 - %2)").arg(static_cast<int>(type)).arg(m_reader.errorString()));

					return;
				}
		}
	}
}

void RssParser::readClosingRssTag()
{
	while (!m_reader.atEnd()) {
		// We expect the token type to be EndElement, and the element to be "rss",
		// otherwise an error occurred
		QXmlStreamReader::TokenType type = m_reader.readNext();

		if ((type == QXmlStreamReader::EndElement) && (m_reader.qualifiedName() == "rss")) {
			// Ok tag found, we can change the status and return
			m_status = States::FinishedReadingRss;

			return;
		} else if (type != QXmlStreamReader::Comment) {
			// Raising an error
			m_reader.raiseError(QObject::tr("Error waiting for closing rss tag"));

			return;
		}
	}
}

void RssParser::readEndDocument()
{
	while (!m_reader.atEnd()) {
		// We expect the token type to be EndDocument, otherwise an error occurred
		QXmlStreamReader::TokenType type = m_reader.readNext();

		if (type == QXmlStreamReader::EndDocument) {
			// Ok tag found, we can change the status and return
			m_status = States::DocumentFinished;

			return;
		} else if (type != QXmlStreamReader::Comment) {
			// Raising an error
			m_reader.raiseError(QObject::tr("Error waiting for end of rss document"));

			return;
		}
	}
}

bool RssParser::readChannelElementAndCheckItem()
{
	// Storing the tag
	const QStringRef elementName = m_reader.qualifiedName();
	m_openedTags.append(elementName.toString());

	// If the tag is item we signal it by returning true
	if (elementName == "item") {
		return true;
	}

	return false;
}

void RssParser::readChannelTagData()
{
	// Here we have to set properties of the channel depending on which tag is open
	if (m_openedTags.size() == 1) {
		if (m_openedTags[0] == "title") {
			m_channel->standardRoles().setData<ChannelRoles::title>(m_reader.text().toString());
		} else if (m_openedTags[0] == "link") {
			m_channel->standardRoles().setData<ChannelRoles::link>(QUrl(m_reader.text().toString()));
		} else if (m_openedTags[0] == "description") {
			m_channel->standardRoles().setData<ChannelRoles::description>(m_reader.text().toString());
		} else if (m_openedTags[0] == "language") {
			m_channel->standardRoles().setData<ChannelRoles::language>(m_reader.text().toString());
		} else if (m_openedTags[0] == "copyright") {
			m_channel->standardRoles().setData<ChannelRoles::copyright>(m_reader.text().toString());
		} else if (m_openedTags[0] == "managingEditor") {
			m_channel->standardRoles().setData<ChannelRoles::managingEditor>(m_reader.text().toString());
		} else if (m_openedTags[0] == "webMaster") {
			m_channel->standardRoles().setData<ChannelRoles::webMaster>(m_reader.text().toString());
		} else if (m_openedTags[0] == "pubDate") {
			m_channel->standardRoles().setData<ChannelRoles::pubDate>(dateTimeFromRssString(m_reader.text().toString()));
		} else if (m_openedTags[0] == "lastBuildDate") {
			m_channel->standardRoles().setData<ChannelRoles::lastBuildDate>(dateTimeFromRssString(m_reader.text().toString()));
		} else if (m_openedTags[0] == "category") {
			m_channelCategories.append(m_reader.text().toString());
		} else if (m_openedTags[0] == "ttl") {
			m_channel->standardRoles().setData<ChannelRoles::ttl>(m_reader.text().toUInt());
		}
	} else if (m_openedTags.size() == 2) {
		if (m_openedTags[0] == "image") {
			if (m_openedTags[1] == "url") {
				m_channel->standardRoles().setData<ChannelRoles::imageUrl>(QUrl(m_reader.text().toString()));
			} else if (m_openedTags[1] == "description") {
				m_channel->standardRoles().setData<ChannelRoles::imageDescription>(m_reader.text().toString());
			}
		}
	}
}

void RssParser::readItemElement()
{
	// Storing the tag
	const QStringRef elementName = m_reader.qualifiedName();
	m_openedTags.append(elementName.toString());

	// For some tags we need to read attributes. We check this here
	if (elementName == "enclosure") {
		m_currentNewsRoles.setData<NewsRoles::enclosureUrl>(QUrl(m_reader.attributes().value(QString(), "url").toString()));
		m_currentNewsRoles.setData<NewsRoles::enclosureLength>(m_reader.attributes().value(QString(), "length").toUInt());
		m_currentNewsRoles.setData<NewsRoles::enclosureType>(m_reader.attributes().value(QString(), "type").toString());
	} else if (elementName == "guid") {
		m_guidIsPermalink = (m_reader.attributes().value(QString(), "isPermaLink").toString().toUpper() == "TRUE") ? true : false;
	}
}

void RssParser::readItemTagData()
{
	// Here we have to set properties of the news depending on which tag is open
	if (m_openedTags.size() == 0) {
		m_reader.raiseError(QObject::tr("Internal error, we should never get here"));
	} else if (m_openedTags.size() == 2) {
		if (m_openedTags[1] == "title") {
			m_currentNewsRoles.setData<NewsRoles::title>(m_reader.text().toString());
		} else if (m_openedTags[1] == "link") {
			m_currentNewsRoles.setData<NewsRoles::link>(QUrl(m_reader.text().toString()));
		} else if (m_openedTags[1] == "description") {
			m_currentNewsRoles.setData<NewsRoles::description>(m_reader.text().toString());
		} else if (m_openedTags[1] == "author") {
			m_currentNewsRoles.setData<NewsRoles::authorEMail>(m_reader.text().toString());
		} else if (m_openedTags[1] == "category") {
			m_newsCategories.append(m_reader.text().toString());
		} else if (m_openedTags[1] == "guid") {
			const QString guid = m_reader.text().toString();

			if (m_guidIsPermalink) {
				m_currentNewsRoles.setData<NewsRoles::permalink>(QUrl(guid));
			}
		} else if (m_openedTags[1] == "pubDate") {
			m_currentNewsRoles.setData<NewsRoles::pubDate>(dateTimeFromRssString(m_reader.text().toString()));
		} else if (m_openedTags[1] == m_dcNamespacePrefix + ":creator") {
			m_currentNewsRoles.setData<NewsRoles::creator>(m_reader.text().toString());
		}
	}
}
