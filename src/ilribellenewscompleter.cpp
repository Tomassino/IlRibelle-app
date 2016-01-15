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

#include "include/ilribellenewscompleter.h"
#include "include/networkmanager.h"
#include <QUrl>
#include <QDebug>
#include <QBuffer>
#include <QFile>
#include <QStandardPaths>

#warning SEE THIS LIST OF TODOS
// Lista di TODOs
//	- aprire link a news del ribelle non in un browser esterno ma nell'app
//	  (con lo stesso formato delle news "nuove")
//	- aggiungere ricerca nell'archivio delle news (ad esempio per tag,
//	  data...). Forse la pagina principale si può fare con una TabView, un
//	  tab è la lista delle  news, l'altro è la ricerca, per data e tag (con
//	  in alto i campi per la ricerca e sotto le news trovate). La ricerca,
//	  comunque, credo richieda anche supporto lato server...
//	- fare icona app (forse quella che c'è con un rosso più sbiadito e linee
//	  più sottili) e rifare in vettoriale l'immagine dell'header
//	- fare in vettoriale immagine di splash screen (come quella dell'header
//        ma con scritta sotto)
//	- aggiungere drag con un dito delle immagini
//	- fare schermata di about. Prendere dati da footer del sito del ribelle
//	  e mettere che sono io il creatore e link a pagina per donazioni
//	- nel caso di link a Raz24 estrarre l'mp3 e permettere di scaricarlo o
//	  di sentirlo in streaming. Bisogna anche decidere dove scaricare il
//	  file, sarebbe meglio sulla SD (QStandardPaths::DownloadLocation o
//	  QStandardPaths::MusicLocation) perché è grande. Le pagine con le
//	  interviste pare siano in raz24.squarespace.com e raz24.com
//	- pensare anche se spostare le immagini sulla SD
//	  (QStandardPaths::GenericDataLocation o altro sull'external storage).
//        Mettere ella stessa directory (o in una sottodirectory) anche l'audio
//	  delle interviste. Forse ancora meglio se si tiene la configurazione
//	  dove sta adesso e si sposta l'archivio delle news in un file sulla SD
//	- togliere i warnings in giro per il codice
//	- forse aggiungere flag per dire se news è stata già letta o no
//	  (chiedere a Valerio, in realtà la cosa non è semplice e mi sembra
//        anche non utilissima)
//	- nel download dei podcast, aggiungere la possibilità di fare il resume
//	  di un download andato male. Vedere headers Range, ETag e If-Range,
//	  vedere anche queste pagine web:
//	  http://weblog.west-wind.com/posts/2004/Feb/07/Using-HTTP-Ranges-to-resume-resume-aborted-downloads
//	  http://stackoverflow.com/questions/3428102/how-to-resume-an-interrupted-download-part-2
//	- completare il player: mettere la possibilità di fare il seek, forse
//	  continuare il play anche se si esce dalla notizia., controllare se il
//	  play continua anche se il telefono va in standby..
//	- bugs nel player: alle volte anche se il file è scaricato non lo
//	  carica, il tempo totale resta a 0 e nonpermette di scaricarlo (come se
//	  "capisse" che è scaricato ma non riuscisse ad accedere al file).
//	  Inoltre il player scompare se si cambia orientamento del device
//	- ingrandire menu
//	- nel caso di immagini cliccabili, scaricare l'immagine del link invece
//	  di solo quelle mostrate nell'articolo
//	- estrarre dalla pagina web della notizia tutte le informazioni, anche
//	  quelle già presenti nell'rss (e.g. titolo, autore, data...). In questo
//	  modo si può usare lo stesso codice anche per parsare pagine che non
//	  vengono dall'rss (ad esempio quando si aggiungerà la possibilità di
//	  fare una ricerca). Inoltre dividere in due questa classe: una che fa
//	  il parsing e il completer vero e proprio (che usa la classe per il
//	  parsing internamente)
//	- ingrandire tasti per menù e indietro su device con schermo piccolo
//	  (vedi mail Piero)
//	- mettere thumbnail articolo nella lista iniziale (vedi mail Piero).
//	  Aspettare a farlo, forse si deve ripensare la lista iniziale...
//	- controllare la pagina web della news di Fini parsata male
//	- vedere mail Gianluca: loader per avere splash screen e codice per
//	  usare dimensioni in millimetri
//	- estrarre il link alla pagina per ri-ascoltare la trasmissione "Noi nel
//	  Mezzo"
//	- vedere quali sono i metodi deprecati che uso nella parte Java
//	- dovremmo forzare il salvataggio dei dati quando l'app va in pausa
//	  (come in Activity.onPause(), dovrebbe esserci un segnale di
//	  QApplication, applicationStateChanged quando lo stato passa in
//	  Qt:ApplicationSuspended e forse anche Qt::ApplicationHidden e
//	  Qt::ApplicationInactive
//	- per i tablet, si potrebbe mostrare nella stessa schermata l'elenco
//	  delle notizie e la notzia selezionata
//	- vedere se e come chiamare app livestream per Noi Nel Mezzo
//	- per condividere la news: mettere Facebook, Twitter (se non c'è una app
//	  installata fallisce con un errore) e other (usare un generico
//	  ACTION_SEND intent)
//	- per aggiungere l'immagine twitter sembra sia necessario passare l'uri
//	  di un file locale (immagino da scaricare in una directory accessibile
//	  da tutte le applicazioni). Vedere anche http://stackoverflow.com/questions/24436501/share-image-from-url-via-twitter-app-android
//	- forse togliere i segnali da AbstractChannel e Channel ed usare un
//	  observer in modo da evitare che erediti da QObject. In realtà il
//	  NewsListModel usa il channel come QObject parent, inoltre non mi
//	  sembra un grosso problema. Posticipare questa cosa.
//	- controllare le callback per sharing successful e failed in
//	  DisplayIlRibelle.qml (function sharingOk() e function sharingFailed())
//	- Cambiare la gestione della cache delle temporary news in channel. Si
//	  potrebbe fare così: si decidono un soft limits alle news temporanee
//	  che vengono mantenute e un hard limit. Se si prova ad aggiungere una
//	  news oltre l hard limit, qualche news vecchia viene cancellata
//	  (bisogna mettere il meccanismo dei weak_ptr nell accessor per
//	  riscaricare la news, in modo che esso possa capire se la news è stata
//	  cancellata mentre lui è ancora in vita e, se gli vengono richiesti
//	  dati, può chiedere di nuovo la news). Quando viene chiamata la
//	  funzione di channel cleanTemporaryNewsCache() si riportano le news
//	  sotto il soft limit. Il passaggio al soft limit si potrebbe inoltre
//	  fare quando l'app passo allo stato "in pausa" o "stoppata"
//	- Togliere i qDebug() commentati!
//	- Controllare quando l'activity android cambia stato e salvare notizie
//	  quando ad esempio app viene sospesa (e potrebbe venir chiusa in ogni
//	  momento)
//	- Trovare il modo di mettere automaticamente nella pagina di about la
//	  versione dell'applicazione (per ora bisogna cambiarla a mano)
//	- Rimuovere i <p></p> intorno agli iframe dall'html dopo il parsing
//	  (controllare se ci sono e, in caso, rimuoverli)

// A namespace with utility constants
namespace {
	// The url of the main image of news in case the news contains no images
	const QString defaultMainImageForNews = "http://www.ilribelle.com/storage/banner-generali-sito/testata2013OKnuova.png?__SQUARESPACE_CACHEVERSION=1363546281923";

	// The extension of the main image of news in case the news contains no
	// images
	const QString defaultMainImageForNewsExtension = "png";
}

const QRegularExpression IlRibelleNewsCompleter::m_checkFiniRE(R"regexp(^<p><a href="(http://www.ilribelle.com/archivio-editoriali-fini.*?)">)regexp");
const QRegularExpression IlRibelleNewsCompleter::m_imgTagRE(R"regexp(<img .*?/>)regexp");
const QRegularExpression IlRibelleNewsCompleter::m_imgWidthRE(R"regexp(style\s*=\s*".*width:\s*(\d+)px.*")regexp");
const QRegularExpression IlRibelleNewsCompleter::m_imgUrlRE(R"regexp(src\s*=\s*"([^"]+)[.](.*)[?](.*?)")regexp");
const QRegularExpression IlRibelleNewsCompleter::m_raz24UrlRE(R"regexp(<a.*?href="((?:http://raz24\.com|https://raz24\.squarespace\.com)/raz24news/.*?)")regexp");
const QRegularExpression IlRibelleNewsCompleter::m_audioStreamInfoRE(R"regexp(<div\s+class="sqs-audio-embed"([^<]*)>)regexp");
const QRegularExpression IlRibelleNewsCompleter::m_audioStreamUrlRE(R"regexp(data-url="(.*?)")regexp");
const QRegularExpression IlRibelleNewsCompleter::m_audioStreamTitleRE(R"regexp(data-title="(.*?)")regexp");
const QRegularExpression IlRibelleNewsCompleter::m_audioStreamAuthorRE(R"regexp(data-author="(.*?)")regexp");
const QRegularExpression IlRibelleNewsCompleter::m_audioStreamDurationRE(R"regexp(data-duration-in-ms="(.*?)")regexp");
const QRegularExpression IlRibelleNewsCompleter::m_livestreamUrlRE(R"regexp(<iframe.*?src="(http://livestream.com/.*?)".*</iframe>)regexp");
const QRegularExpression IlRibelleNewsCompleter::m_iframeUrlRE(R"regexp(<iframe.*?src="(.*?)".*</iframe>)regexp");

IlRibelleNewsCompleter::IlRibelleNewsCompleter(IlRibelleChannel* channel, IlRibelleNews* news, const std::function<void()>& workFinishedCallback)
	: AllDataArrivedNotifee()
	, m_channel(channel)
	, m_news(news)
	, m_workFinishedCallback(workFinishedCallback)
	, m_newsState(NewsStatus::NotStarted)
	, m_nextRequestForNews()
	, m_imagesUrls()
	, m_imagesFiles()
	, m_raz24Page()
{
}

IlRibelleNewsCompleter::~IlRibelleNewsCompleter()
{
	// Nothing to do here
}

void IlRibelleNewsCompleter::start()
{
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__;

	// If we are already working, do noting
	if (m_newsState != NewsStatus::NotStarted) {
		return;
	}

	// Setting the QML item to show the news
	m_news->setData<NewsRoles::qmlItem>(QUrl("qrc:///qml/DisplayIlRibelle.qml"));

	// Setting the state for the news
	m_newsState = NewsStatus::DownloadMainPage;

	// Getting the page for the news
	const bool ret = NM::instance().getFile(m_news->getData<NewsRoles::link>(), this, m_news->id());

	// We check the unlikely event that ret is false, just for debug purpouse
	if (Q_UNLIKELY(!ret)) {
		qFatal(QString("Internal error, a request with the id already exists, id: %1").arg(m_news->id()).toLatin1().data());
	}
}

void IlRibelleNewsCompleter::allDataArrived(int id, const QByteArray& data)
{
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__;

	// We check the unlikely event that id is not what we expect, just for debug purpouse
	if (Q_UNLIKELY(id != int(m_news->id()))) {
		qFatal(QString("Internal error, wrong id received. Expected %1 got %2").arg(m_news->id()).arg(id).toLatin1().data());
	}

	// Parsing data for the request just arrived
	const bool requestEnqueued = parseDataForNews(data);

	// If the parseData function did not enqueued another request, we mark the news as
	// complete and call the callback
	if (!requestEnqueued) {
		newsCompleted();
	}
}

void IlRibelleNewsCompleter::networkError(int id, const QString& description)
{
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__;

	qDebug() << "Network error while completing news from www.ilribelle.com, id:" << id << "message:" <<  description;

	// Trying to download the next file
	requestCompleted(id);
}

void IlRibelleNewsCompleter::requestCompleted(int id)
{
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__;

	// We check the unlikely event that id is not what we expect, just for debug purpouse
	if (Q_UNLIKELY(id != int(m_news->id()))) {
		qFatal(QString("Internal error, wrong id received. Expected %1 got %2").arg(m_news->id()).arg(id).toLatin1().data());
	}

//qDebug() << ((unsigned long) this) << m_news->id() << "m_nextRequestForNews PRESENT?" << (m_nextRequestForNews.isEmpty() ? "NO" : "YES") << m_nextRequestForNews;

	if (!m_nextRequestForNews.isEmpty()) {
		// Adding the new request
		const bool ret = NM::instance().getFile(m_nextRequestForNews, this, m_news->id());

		// We chack the unlikely event that ret is false, just for debug purpouse
		if (Q_UNLIKELY(!ret)) {
			qFatal(QString("Internal error, wrong id received. Expected %1 got %2").arg(m_news->id()).arg(id).toLatin1().data());
		}

		// Removing the request we have just submitted
		m_nextRequestForNews = QUrl();
	} else {
		// We can get here in case we were called after a network error (in that case
		// allDataArrived is not called and newsCompleted() is not called)
		if (!m_news->getData<NewsRoles::complete>()) {
			newsCompleted();
		}
	}
}

void IlRibelleNewsCompleter::newsCompleted()
{
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__;

	// The news is complete
	m_news->setData<NewsRoles::complete>(true);

	// Calling the callback
	m_workFinishedCallback();
}

bool IlRibelleNewsCompleter::parseDataForNews(const QByteArray& data)
{
//const char* cur_status = nullptr;
//switch (m_newsState) {
//	case NewsStatus::NotStarted:
//		cur_status = "NotStarted";
//		break;
//	case NewsStatus::DownloadMainPage:
//		cur_status = "DownloadMainPage";
//		break;
//	case NewsStatus::DownloadFiniPage:
//		cur_status = "DownloadFiniPage";
//		break;
//	case NewsStatus::DownloadImages:
//		cur_status = "DownloadImages";
//		break;
//	case NewsStatus::DownloadRaz24Page:
//		cur_status = "DownloadRaz24Page";
//		break;
//	default:
//		cur_status = "UNKNOWN";
//		break;
//}
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__ << "STATUS:" << cur_status;

	bool ret = false;

	// Checking the status of the news and acting consequently
	if ((m_newsState == NewsStatus::DownloadMainPage) || (m_newsState == NewsStatus::DownloadFiniPage)) {
		// Extracting the body of the news
		const QString& newsBody = extractNewsBody(data);

		// If we are downloading the main page (i.e. the page from the link in the rss), we have to<
		// check if it contains a news from Massimo Fini and, if so, we have to download the page with
		// the full news
		QUrl finiPageUrl;
		if ((m_newsState == NewsStatus::DownloadMainPage) && (newsFromFini(newsBody, &finiPageUrl))) {
			// Here we discard the page just downloaded and add a request for another page
			m_nextRequestForNews = finiPageUrl;

			// Changing the status and returning true (we will enqueue another request in requestCompleted)
			m_newsState = NewsStatus::DownloadFiniPage;
			ret = true;
		} else {
			// We have to extract the links to images from the page and substitute them with the
			// files where images will be stored
			setNewsDescriptionAndExtractStuffs(newsBody);

			// If there are images to download, changing the status of the news
			ret = checkFilesToDownloadForNews();
		}
	} else if (m_newsState == NewsStatus::DownloadImages) {
		// The image we downloaded is the first in the list. Saving to the first file, then removing
		// both from the lists
		QFile imageFile(m_imagesFiles.first());
		if (imageFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			if (imageFile.write(data) == -1) {
				qDebug() << "Could not save image" << m_imagesUrls.first() << "to file" << m_imagesFiles.first();
			}
		} else {
			qDebug() << "Could open file" << m_imagesFiles.first() << "to write image" << m_imagesUrls.first();
		}

		// Removing the image we have just saved
		m_imagesUrls.removeFirst();
		m_imagesFiles.removeFirst();

		// Checking if there is more to download
		ret = checkFilesToDownloadForNews();
	} else if (m_newsState == NewsStatus::DownloadRaz24Page) {
		// We have to extract the link to the mp3 and other information
		extractMp3UrlFromRaz24(data);

		// Checking if there is more to download
		ret = checkFilesToDownloadForNews();
	}

//switch (m_newsState) {
//	case NewsStatus::NotStarted:
//		cur_status = "NotStarted";
//		break;
//	case NewsStatus::DownloadMainPage:
//		cur_status = "DownloadMainPage";
//		break;
//	case NewsStatus::DownloadFiniPage:
//		cur_status = "DownloadFiniPage";
//		break;
//	case NewsStatus::DownloadImages:
//		cur_status = "DownloadImages";
//		break;
//	case NewsStatus::DownloadRaz24Page:
//		cur_status = "DownloadRaz24Page";
//		break;
//	default:
//		cur_status = "UNKNOWN";
//		break;
//}
//qDebug() << ((unsigned long) this) << m_news->id() << (ret ? "NEW REQUEST" : "NO REQUEST") << "NEW STATUS:" << cur_status;

	return ret;
}

void IlRibelleNewsCompleter::setNewsDescriptionAndExtractStuffs(const QString& newsBody)
{
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__;

	// Here we use simple regular expressions, parsing the news body as XML is not trivial
	// and the support for XSLT in QT is still experimental. Prepending the title to the news body
	QString outputNewsBody = "<center><h1>" + m_news->getData<NewsRoles::title>() + "</h1></center><br/>";

	// Getting the index for the news
	const int newsIndex = m_channel->newsIndexByID(m_news->id());
	if (Q_UNLIKELY(newsIndex == -1)) {
		qFatal(QString("INTERNAL ERROR: Could not find index of news with ID %1").arg(m_news->id()).toLatin1().data());
	}

	// Doing a global match of the img regular expression in the newsBody then
	// using the iterator
	QRegularExpressionMatchIterator it = m_imgTagRE.globalMatch(newsBody);
	int curPos = 0;
	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();

		// First of all copying in the output string everything between the matches
		if (match.capturedStart() > (curPos + 1)) {
			outputNewsBody += newsBody.mid(curPos, match.capturedStart() - curPos);
		}

		// Now extracting the width and url
		QRegularExpressionMatch widthMatch = m_imgWidthRE.match(match.captured());
		QRegularExpressionMatch urlMatch = m_imgUrlRE.match(match.captured());

		// The width of the image
		const qreal imgWidth = widthMatch.captured(1).toDouble();

		// The image url and the file where the image will be saved
		const QString imageExt = urlMatch.captured(2);
		const QString imagePath = (urlMatch.captured(1).startsWith("/") ? "" : "/") + urlMatch.captured(1);
		const QString imageUrl = "http://www.ilribelle.com" + imagePath + "." + imageExt + "?" + urlMatch.captured(3);
		const QString imageFile = m_channel->createFileForNews(newsIndex, imageExt);

//qDebug() << ((unsigned long) this) << m_news->id() << "Found image: " << match.captured() << " - width: " << imgWidth << " - ext: " << imageExt << " - url: " << imageUrl << " - file" << imageFile;

		// If this is the first image for the article, setting it as the main image
		if (m_imagesUrls.isEmpty()) {
			m_news->setData<IlRibelleRoles::mainImageUrl>(QUrl(imageUrl));
			m_news->setData<IlRibelleRoles::mainImageFile>(imageFile);
		}

		// Adding the image url and file to the list for the news
		m_imagesUrls.append(imageUrl);
		m_imagesFiles.append(imageFile);

		// Adding to output the new img tag
		QString imgWidthTag;
		if (imgWidth > 200.0) {
			imgWidthTag = "<$BIGIMAGEWIDTH$>";
		} else {
			imgWidthTag = "<$SMALLIMAGEWIDTH$>";
		}
		outputNewsBody += "<a href=\"image+file://" + imageFile + "\"><img width=\"" + imgWidthTag + "\" src=\"file://" + imageFile + "\" style=\"float:left;\"/></a>";

		// Moving curPos forward
		curPos = match.capturedEnd();
	}

	// If there is no main image, setting a default one
	if (m_imagesUrls.isEmpty()) {
		const QString imageFile = m_channel->createFileForNews(newsIndex, defaultMainImageForNewsExtension);

		m_imagesUrls.append(defaultMainImageForNews);
		m_imagesFiles.append(imageFile);
		m_news->setData<IlRibelleRoles::mainImageUrl>(QUrl(defaultMainImageForNews));
		m_news->setData<IlRibelleRoles::mainImageFile>(imageFile);
	}

	// Copying the remaining part of the news
	outputNewsBody += newsBody.mid(curPos);

	// We also have to look for raz24 links. We don't need to modify the page, so we use outputNewsBody
	it = m_raz24UrlRE.globalMatch(outputNewsBody);
	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();

		// Now extracting the url
		QUrl raz24Url = match.captured(1);

		// Adding the raz24 url for the news. We only support one raz24 link, for the moment,
		// so if there is already one link we keep the old one and print a debug message
		if (!m_raz24Page.isValid() || (m_raz24Page == raz24Url)) {
			m_raz24Page = raz24Url;
		} else {
			qDebug() << "Multiple links to raz24 found in news" << m_news->getData<NewsRoles::title>() << "keeping only the first one (discarded" << raz24Url << ")";
		}
	}

	// If we have found a link to raz24, filling some additional roles of the news
	if (m_raz24Page.isValid()) {
		m_news->setData<IlRibelleRoles::hasAudioResource>(true);
	} else {
		m_news->setData<IlRibelleRoles::hasAudioResource>(false);
	}

	// Extracting livestream urls if present
	QUrl livestreamUrl;
	int livestreamUrlREStart = 0;
	int livestreamUrlRELength = 0;
	it = m_livestreamUrlRE.globalMatch(outputNewsBody);
	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();

		// Now extracting the url
		QUrl curLivestreamUrl = match.captured(1);

		// Adding the livestream url for the news. We only support one livestream link, for the moment,
		// so if there is already one link we keep the old one and print a debug message
		if (!livestreamUrl.isValid() || (livestreamUrl == curLivestreamUrl)) {
			livestreamUrl = curLivestreamUrl;

			livestreamUrlREStart = match.capturedStart();
			livestreamUrlRELength = match.capturedLength();
		} else {
			qDebug() << "Multiple livestream links in news" << m_news->getData<NewsRoles::title>() << "keeping only the first one (discarded" << curLivestreamUrl << ")";
		}
	}

	// If we have found a livestream url, filling some additional roles of the news
	if (livestreamUrl.isValid()) {
		m_news->setData<IlRibelleRoles::hasLivestreamLink>(true);
		m_news->setData<IlRibelleRoles::livestreamUrl>(livestreamUrl);

		// Also removing the i-frame from the news
		outputNewsBody.remove(livestreamUrlREStart, livestreamUrlRELength);
	} else {
		m_news->setData<IlRibelleRoles::hasLivestreamLink>(false);
	}

	// Finally extracting links in iframes
	it = m_iframeUrlRE.globalMatch(outputNewsBody);
	QList<int> iframeUrlsStart;
	QList<int> iframeUrlsLength;
	QStringList iframeUrls;
	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();

		// Now extracting the url and saving the urls we found along with starting position and length
		iframeUrls.append(match.captured(1));
		iframeUrlsStart.append(match.capturedStart());
		iframeUrlsLength.append(match.capturedLength());
	}

	// Removing iframes from output. We start from the last so that we do not have to recompute positions
	// of subsequent matches
	if (iframeUrlsStart.size() != 0) {
		for (int i = (iframeUrlsStart.size() - 1); i >= 0; --i) {
			outputNewsBody.remove(iframeUrlsStart[i], iframeUrlsLength[i]);
		}
	}

	// Now saving all urls of iframe in the news
	m_news->setData<IlRibelleRoles::iframeUrls>(iframeUrls);

	// Setting the body of the news
	m_news->setData<NewsRoles::description>(outputNewsBody);
}

QString IlRibelleNewsCompleter::extractNewsBody(const QByteArray& data) const
{
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__;

	// Unfortunately we cannot use QXmlQuery because webpages from www.ilribelle.com are not valid xml
	// (there are closing tags missing, they are not even valid HTML). We then use some tricks

	// News news body is inside a div with class "body", so look for '<div class="body">' and then move
	// forward by the length of that tag
	const int newsBodyIndex = data.indexOf("<div class=\"body\">") + 18;

	// Now we have to look for the corresponding closing div. We have to look for more opening div so
	// that we can match the right closing tags. For the opening tag we could simply look for "<div" (because
	// there are no other HTML tags whose name starts with "div", but it is better to match "<div " or "<div>",
	// so we use a simple lambda function
	const char* openingDiv = "<div";
	auto findNextOpeningDiv = [&data, &openingDiv](int from)
	{
		const int pos = data.indexOf(openingDiv, from);
		return ((pos != -1) && ((data[pos + 4] == ' ') || (data[pos + 4] == '>'))) ? pos : -1;
	};
	const char* closingDiv = "</div>";
	int nextOpeningDiv = findNextOpeningDiv(newsBodyIndex);
	int nextClosingDiv = data.indexOf(closingDiv, newsBodyIndex);
	int lastClosingDiv = newsBodyIndex;
	int curIndex = newsBodyIndex;
	unsigned int nestingLevel = 1; // We are already inside a div
	while (nestingLevel != 0) {
		// We set the index for the opening and closing div to the length of the buffer if they
		// are -1 (i.e. if they are not found). Moreover, if we cannot find a closing div we can exit
		// from the cycle
		if (nextOpeningDiv == -1) {
			nextOpeningDiv = data.length();
		}
		if (nextClosingDiv == -1) {
			nextClosingDiv = data.length();
			break;
		}

		// Checking what comes before (the opening or closing tag).
		if (nextClosingDiv < nextOpeningDiv) {
			// The next tag is a closing div: decrementing the nesting level and searching for
			// another closing div
			--nestingLevel;
			curIndex = nextClosingDiv + 6;
			lastClosingDiv = nextClosingDiv;
			nextClosingDiv = data.indexOf(closingDiv, curIndex);
		} else {
			// The next tag is an opening div: incrementing the nesting level and searching for
			// another opening div
			++nestingLevel;
			curIndex = nextOpeningDiv + 5;
			nextOpeningDiv = findNextOpeningDiv(curIndex);
		}
	}

	// The news is between newsBodyIndex and lastClosingDiv, setting it as the news description (after checking that
	// there is no error)
	if (lastClosingDiv <= newsBodyIndex) {
		qDebug() << "Cannot find the news in the webpage:" << data;

		return QString();
	}

	return QString(data.mid(newsBodyIndex, lastClosingDiv - newsBodyIndex)).trimmed();
}

bool IlRibelleNewsCompleter::newsFromFini(const QString& newsBody, QUrl* url) const
{
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__;

	// Using the regular expression to check if the page contains a partial article from Fini
	QRegularExpressionMatch match = m_checkFiniRE.match(newsBody.trimmed());
	if (match.hasMatch()) {
		if (url != nullptr) {
			*url = QUrl(match.captured(1));
		}

		return true;
	}

	return false;
}

bool IlRibelleNewsCompleter::checkFilesToDownloadForNews()
{
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__;

	bool ret = false;

	// If there are images to download, changing the status of the news
	if (m_imagesUrls.size() != 0) {
		// Changing status
		m_newsState = NewsStatus::DownloadImages;

		// Requesting the first image
		m_nextRequestForNews = QUrl(m_imagesUrls.first());

		ret = true;
	} else if ((m_newsState != NewsStatus::DownloadRaz24Page) && m_raz24Page.isValid()) {
		// Changing status
		m_newsState = NewsStatus::DownloadRaz24Page;

		// Requesting the Raz24 webpage
		m_nextRequestForNews = m_raz24Page;

		ret = true;
	}

	return ret;
}

void IlRibelleNewsCompleter::extractMp3UrlFromRaz24(const QByteArray& data)
{
//qDebug() << ((unsigned long) this) << m_news->id() << "IlRibelleNewsCompleter" << __func__;

	// Looking for tags with information. We expect only one, but check more than once just to be sure
	QRegularExpressionMatchIterator it = m_audioStreamInfoRE.globalMatch(data);
	bool foundOne = false;
	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();

		if (foundOne) {
			qDebug() << "Found more than one tag with information about the mp3 stream. Keeping only the first one, discarding:" << match.captured();
		} else {
			// Now extracting all information...
			QRegularExpressionMatch urlMatch = m_audioStreamUrlRE.match(match.captured());
			QRegularExpressionMatch titleMatch = m_audioStreamTitleRE.match(match.captured());
			QRegularExpressionMatch authorMatch = m_audioStreamAuthorRE.match(match.captured());
			QRegularExpressionMatch durationMatch = m_audioStreamDurationRE.match(match.captured());

			// ... and setting in the news
			m_news->setData<IlRibelleRoles::audioResourceUrl>(QUrl(urlMatch.captured(1)));
			m_news->setData<IlRibelleRoles::audioResourceTitle>(titleMatch.captured(1));
			m_news->setData<IlRibelleRoles::audioResourceAuthor>(authorMatch.captured(1));
			m_news->setData<IlRibelleRoles::audioResourceDuration>(durationMatch.captured(1).toUInt());

			// For the destination file we get the path of the url, extract the last part and prepend
			// the standard path for downloads. We also create the destination directory
			QString audioDownloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/IlRibelle.com";
			if (!QDir().mkpath(audioDownloadPath)) {
				qDebug() << "Cannot create directory for audio resources download" << audioDownloadPath;
			}
			QString filename = m_news->getData<IlRibelleRoles::audioResourceUrl>().path();
			filename = audioDownloadPath + filename.mid(filename.lastIndexOf("/"));
			m_news->setData<IlRibelleRoles::audioResourcePath>(filename);
		}

		foundOne = true;
	}
}
