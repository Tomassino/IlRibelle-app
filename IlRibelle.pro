TEMPLATE = app

QT += qml quick widgets svg multimedia
android: QT += androidextras

# CONFIG += c++14
QMAKE_CXXFLAGS += -std=c++14 -Wall -Wextra

# This is because we need to define stuffs in JNI_OnLoad
DEFINES += MISC_NATIVE_NO_JNI_ONLOAD_DEFINITION

SOURCES += \
	src/controller.cpp \
	src/dataavailablenotifee.cpp \
	src/main.cpp \
	src/networkmanager.cpp \
	src/rssparser.cpp \
	src/utilities.cpp \
	src/iconsgenerator.cpp \
	src/remotefileprovider.cpp \
	src/remotefileproviderfactory.cpp \
	src/roleshelpers.cpp \
	src/ilribellenewscompleter.cpp \
	MiscNative/miscnative.cpp

android: SOURCES += src/jnionload.cpp \
	MiscNative/miscnative_android.cpp
else: SOURCES += \
	MiscNative/miscnative_desktop.cpp

# Appending QML to sources for lupdate (to generate translations)
lupdate_only {
SOURCES += \
	qml/*.qml
}

RESOURCES += \
	resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

OTHER_FILES += \
	android/AndroidManifest.xml

INCLUDEPATH += QtFacebook

HEADERS += \
	include/channel.h \
	include/controller.h \
	include/dataavailablenotifee.h \
	include/networkmanager.h \
	include/news.h \
	include/newslistmodel.h \
	include/rssparser.h \
	include/utilities.h \
	include/iconsgenerator.h \
	include/roles.h \
	include/ilribellechannel.h \
	include/remotefileprovider.h \
	include/remotefileproviderfactory.h \
	include/allnewscompleter.h \
	include/defaultchannelcompleter.h \
	include/defaultnewscompleter.h \
	include/ilribellenewscompleter.h \
	include/channelupdater.h \
	include/ilribellechannelupdater.h \
	include/standardroles.h \
	include/roleshelpers.h \
	include/rolesqmlaccessor.h \
	MiscNative/miscnative.h

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

TRANSLATIONS = \
	ilribelle_it.ts

DISTFILES += \
	resources/about.html \
	VersioningStrategy.txt \
	android/src/com/ilribelle/IlRibelleActivity.java \
	android/src/com/ilribelle/MiscNativeBinding.java \
	android/res/values/strings.xml \
	android/gradle/wrapper/gradle-wrapper.jar \
	android/gradlew.bat \
	android/res/values/libs.xml \
	android/build.gradle \
	android/gradle/wrapper/gradle-wrapper.properties \
	android/gradlew
