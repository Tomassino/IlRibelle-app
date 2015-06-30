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

#ifndef __IL_RIBELLE_NEWS_COMPLETER_H__
#define __IL_RIBELLE_NEWS_COMPLETER_H__

#include "include/dataavailablenotifee.h"
#include "include/ilribellechannel.h"
#include <QRegularExpression>

/**
 * \brief The class completing a news from www.ilribelle.com
 *
 * This class is the news completer for news from www.ilribelle.com. This takes
 * the url stored in a news, gets the webpage and completes the news. This
 * respects all the requisites of a news completer (see the description of the
 * DefaultNewsCompleter class for the list of requisites). This class downloads
 * only one file at a time
 * \warning This class is not thread-safe nor reentrant
 */
class IlRibelleNewsCompleter : private AllDataArrivedNotifee
{
public:
	/**
	 * \brief Constructor
	 *
	 * \param channel the channel containing the news to complete
	 * \param news the news to complete
	 * \param workFinishedCallback the functor called when this has finished
	 *                             its work
	 */
	IlRibelleNewsCompleter(IlRibelleChannel* channel, IlRibelleNews* news, const std::function<void()>& workFinishedCallback);

	/**
	 * \brief Destructor
	 */
	virtual ~IlRibelleNewsCompleter();

	/**
	 * \brief Starts completing the news
	 *
	 * This function actually starts completing the news. If parsing has
	 * already started this does nothing.
	 */
	void start();

private:
	/**
	 * \brief The function called when all data has been received
	 *
	 * \param id the request ID (that is a news ID)
	 * \param data the data that has just arrived
	 */
	virtual void allDataArrived(int id, const QByteArray& data) override;

	/**
	 * \brief The function called in case of network errors
	 *
	 * \param id the request ID
	 * \param description the description of the error
	 */
	virtual void networkError(int id, const QString& description) override;

	/**
	 * \brief The function called after the request is completed and has
	 *        been deleted
	 *
	 * We use this function to enqueue more requests after one has finished
	 * \param id the ID of the request that has finished
	 */
	virtual void requestCompleted(int id) override;

	/**
	 * \brief Call this function when the news is complete
	 *
	 * This sets the news to the completed status and commits suicide
	 * calling deleteLater()
	 */
	void newsCompleted();

	/**
	 * \brief Parses the data received for the news
	 *
	 * This returns true if another request has been submitted because the
	 * received data is not enough to complete the news
	 * \param data the data that has just arrived
	 * \return trus if a new request has been enqueued, false otherwise
	 */
	bool parseDataForNews(const QByteArray& data);

	/**
	 * \brief Sets the news description after fixing it and extracts the
	 *        images and the link to raz24 (if present)
	 *
	 * This function takes the news description extracted from the webpage
	 * and looks for all the img tags. It then extracts the image url
	 * putting it in the m_imagesUrl list . For every image generates the
	 * name of the file where it will be downloaded and fills the
	 * m_imagesFiles list. Moreover, it adds the width attribute to the
	 * image. Finally, it looks for links to raz24. The modified html is
	 * then set as the news description.
	 * \param newsBody the body of the news extracted from the html page
	 */
	void setNewsDescriptionAndExtractStuffs(const QString& newsBody);

	/**
	 * \brief Extracts the body of the news from the webpage
	 *
	 * \param data the html page with the news
	 * \return the body of the news. Whitespaces are removed from the begin
	 *         and end of the string (calling trimmed())
	 */
	QString extractNewsBody(const QByteArray& data) const;

	/**
	 * \brief Checks if a news body contains a news from Massimo Fini
	 *
	 * News from Massimo Fini are not in the page linked by the rss, we have
	 * to extract the actual page.
	 * \param newsBody the body of the news to check (as returned by the
	 *                 extractNewsBody() function)
	 * \param url if not a null pointer, it will be filled with the url of
	 *            the page that contains the full news
	 * \return true if the page contains a news from Fini, false otherwise
	 */
	bool newsFromFini(const QString& newsBody, QUrl* url = nullptr) const;

	/**
	 * \brief Checks if there are files to download for the news
	 *
	 * This changes the status for the news accordingly (e.g. to
	 * DownloadImages or to DownloadRaz24Page) and enqueues the download of
	 * one file
	 * \return true if there are files to download, false otherwise
	 */
	bool checkFilesToDownloadForNews();

	/**
	 * \brief Extracts the link to the mp3 podcast from a raz24 page
	 *
	 * This adds data to the news as it finds information in the page
	 * \param data the raz24 page from which the link has to be extracted
	 */
	void extractMp3UrlFromRaz24(const QByteArray& data);

	/**
	 * \brief The channel with the news to complete
	 */
	IlRibelleChannel* const m_channel;

	/**
	 * \brief The news that will be completed
	 */
	IlRibelleNews* const m_news;

	/**
	 * \brief The functor called when this has finished its work
	 */
	std::function<void()> m_workFinishedCallback;

	/**
	 * brief The enum with the possible states of news to complete
	 */
	enum class NewsStatus {
		NotStarted, /// The process of completing the news hasn't
			    /// started yet
		DownloadMainPage, /// The page of the news has to be downloaded
		DownloadFiniPage, /// This is a news from Massimo Fini, we have
				  /// to download the page with the actual news
		DownloadImages, /// We are downloading the images of the page
		DownloadRaz24Page /// We are downloading a page from raz24 with
				  /// an audio resource
	};

	/**
	 * \brief The current state of the news
	 */
	NewsStatus m_newsState;

	/**
	 * \brief the next request to do for the news
	 */
	QUrl m_nextRequestForNews;

	/**
	 * \brief The list of URLs of images to download
	 *
	 * The image at url imagesUrls[i] must be saved in file imagesFiles[i]
	 */
	QStringList m_imagesUrls;

	/**
	 * \brief The list of files where images have to be downloaded
	 *
	 * The image at url imagesUrls[i] must be saved in file imagesFiles[i]
	 */
	QStringList m_imagesFiles;

	/**
	 * \brief The URL to the raz24 page with the audio resource
	 *
	 * This is an invalid URL if the news has no link to raz24
	 */
	QUrl m_raz24Page;

	/**
	 * \brief The regular expression to check if a page contains a partial
	 *        article from Massimo Fini
	 * We do a very simple check: it seems all news from fini start with:
	 * <p><a href="http://www.ilribelle.com/archivio-editoriali-fini
	 * so we use a regular expression to check this and extract the url
	 */
	static const QRegularExpression m_checkFiniRE;

	/**
	 * \brief The regular expression to find the img tag
	 */
	static const QRegularExpression m_imgTagRE;

	/**
	 * \brief The regular expression to find the width of the image in an
	 *        img tag
	 */
	static const QRegularExpression m_imgWidthRE;

	/**
	 * \brief The regular expression to find the image url in an img tag
	 */
	static const QRegularExpression m_imgUrlRE;

	/**
	 * \brief The regular expression to find the raz24 link in the page
	 */
	static const QRegularExpression m_raz24UrlRE;

	/**
	 * \brief The regular expression to get information on the stream from
	 *        the raz24 in the page
	 */
	static const QRegularExpression m_audioStreamInfoRE;

	/**
	 * \brief The regular expression to get the url of the audio stream from
	 *        the raz24 in the page
	 */
	static const QRegularExpression m_audioStreamUrlRE;

	/**
	 * \brief The regular expression to get the title of the audio stream
	 *        from the raz24 in the page
	 */
	static const QRegularExpression m_audioStreamTitleRE;

	/**
	 * \brief The regular expression to get the author of the audio stream
	 *        from the raz24 in the page
	 */
	static const QRegularExpression m_audioStreamAuthorRE;

	/**
	 * \brief The regular expression to get the duration of the audio stream
	 *        from the raz24 in the page
	 */
	static const QRegularExpression m_audioStreamDurationRE;

	/**
	 * \brief The regular expression to get a livestream url (if present)
	 */
	static const QRegularExpression m_livestreamUrlRE;

	/**
	 * \brief The regular expression to get an url in an i-frame
	 */
	static const QRegularExpression m_iframeUrlRE;
};

#endif
