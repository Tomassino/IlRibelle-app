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

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QObject>
#include <QQmlEngine>
#include <QtQml>
#include <QUrl>
#include <QTranslator>
#include <QDebug>
#include "include/defaultchannelcompleter.h"
#include "include/channelupdater.h"
#include "include/ilribellechannel.h"
#include "include/ilribellechannelupdater.h"
#include "include/controller.h"
#include "include/newslistmodel.h"
#include "include/remotefileprovider.h"
#include "include/remotefileproviderfactory.h"
#include "include/rolesqmlaccessor.h"
#include "MiscNative/miscnative.h"

int main(int argc, char *argv[])
{
	// Creating the application
	QApplication app(argc, argv);

	// Setting organization, domain and application names to be used by QSettings
	QApplication::setOrganizationName("La Voce del Ribelle");
	QApplication::setOrganizationDomain("ilribelle.com");
	QApplication::setApplicationName("IlRibelle.com");

	// Creating the object that receives the command events
	CommandEventReceiver::createInstance();

	// Creating translation and registering it
	QTranslator translator;
	translator.load("ilribelle_it", ":/");
	app.installTranslator(&translator);

	// Registering the news list model, the remote file provider type, the type to access roles from
	// QML and a singleton to store active RemoteFileProvider objects
	qmlRegisterType<AbstractNewsListModel>();
	qmlRegisterType<RemoteFileProvider>("com.ilribelle", 1, 0, "RemoteFileProvider");
	qmlRegisterType<AbstractRolesQMLAccessor>();
	qmlRegisterSingletonType<RemoteFileProviderFactory>("com.ilribelle", 1, 0, "RemoteFileProviderFactory", remoteFileProviderFactorySingletonProvider);

	// Also registering the singleton for MiscNative
	qmlRegisterSingletonType<MiscNative>("com.ilribelle", 1, 0, "MiscNative", miscNativeSingletonProvider);

	// Creating the main object of the application
	Controller controller(Type2Type<IlRibelleChannel>(), Type2Type<IlRibelleChannelUpdater>(), "www_ilribelle_com", QUrl("http://www.ilribelle.com/la-voce-del-ribelle/rss.xml"), ":/resources/about.html");

	// Creating the view and making controller accessible from QML
	QQmlApplicationEngine qmlApp;
	qmlApp.rootContext()->setContextObject(&controller);
	// We load the main item after having set the context object otherwise it won't be visible to QML
	qmlApp.load(QUrl("qrc:///qml/application.qml"));

	// Connecting the signal to do the final cleanup when application exits
	QObject::connect(QGuiApplication::instance(), &QGuiApplication::aboutToQuit, &controller, &Controller::finalize);

	// Starting the application
	return app.exec();
}
