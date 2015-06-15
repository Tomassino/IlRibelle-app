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

#include "include/iconsgenerator.h"
#include <QMutexLocker>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSvgRenderer>
#include <QPainter>
#include <QImage>

IconsGenerator::IconsGenerator(QString outputPath)
	: QThread()
	, m_outputPath(outputPath)
	, m_iconsQueue()
	, m_stop(false)
	, m_mutex()
	, m_waitCondition()
{
	// Creating the directory where icons will be saved, if it doesn't exist
	QFileInfo dinfo(m_outputPath);
	if (!dinfo.exists()) {
		if (!QDir().mkpath(m_outputPath)) {
			qDebug() << "Cannot create directory" << m_outputPath << "icons generation will probably fail";
		}
	} else if (!dinfo.isDir() || !dinfo.isWritable()) {
		qDebug() << m_outputPath << "is not a directory or is not writable, icons generation will probably fail";
	}
}

IconsGenerator::~IconsGenerator()
{
	// Nothing to do here
}

void IconsGenerator::convertIcon(QString sourceSvg, unsigned int index, int iconDim, SpecifiedDimension s)
{
	// Creating a structure with data for the conversion and enqueuing it
	IconToGenerate i;
	i.sourceSvg = sourceSvg;
	i.index = index;
	i.iconDim = iconDim;
	i.s = s;

	// Enqueuing the new icon
	QMutexLocker locker(&m_mutex);

	m_iconsQueue.append(i);

	// Signalling to unlock the sleeping thread
	m_waitCondition.wakeAll();
}

void IconsGenerator::interruptGeneration()
{
	// Enqueuing the new icon
	QMutexLocker locker(&m_mutex);

	m_stop = true;

	// Signalling to unlock the sleeping thread
	m_waitCondition.wakeAll();
}

void IconsGenerator::run()
{
	QMutexLocker locker(&m_mutex);

	while (!m_stop) {
		// Checking that we have at least one image enqueued, otherwise sleeping
		if (m_iconsQueue.isEmpty()) {
			m_waitCondition.wait(&m_mutex);
		}

		// Checking if we have to stop
		if (m_stop) {
			break;
		}

		// Checking again, just to be sure
		if (m_iconsQueue.isEmpty()) {
			qDebug() << "INTERNAL ERROR: icon conversion thread unlocked but no icons to convert";

			continue;
		}

		// Popping the first icon to convert
		IconToGenerate i = m_iconsQueue.takeFirst();

		// We can unlock the mutex while we do the conversion
		locker.unlock();

		// Doing conversion
		doConvertIcon(i);

		// Re-locking the mutex before continuing
		locker.relock();
	}

	// Resetting m_stop to false
	m_stop = false;
}

void IconsGenerator::doConvertIcon(const IconToGenerate& icon)
{
	// Loading the svg file. Printing a message in case of errors
	QSvgRenderer svgSource(icon.sourceSvg);
	if (!svgSource.isValid()) {
		qDebug() << "Invalid svg image" << icon.sourceSvg;
		emit iconGenerated(icon.index, QString());

		return;
	}

	// Computing the output dimension
	int outputWidth = 0;
	int outputHeight = 0;
	const QSize inputSize = svgSource.defaultSize();
	if (icon.s == SpecifiedDimension::Width) {
		outputWidth = icon.iconDim;
		outputHeight = int((double(inputSize.height()) / double(inputSize.width())) * double(outputWidth));
	} else if (icon.s == SpecifiedDimension::Height) {
		outputHeight = icon.iconDim;
		outputWidth = int((double(inputSize.width()) / double(inputSize.height())) * double(outputHeight));
	} else {
		// We should really never get here...
		qDebug() << "INTERNAL ERROR: invalid specified dimension";
		emit iconGenerated(icon.index, QString());

		return;
	}

	// Generating the output filename
	const QString outputFilename = QString("%1/%2-%3-%4x%5.png")
			.arg(m_outputPath)
			.arg(QFileInfo(icon.sourceSvg).baseName())
			.arg(icon.index)
			.arg(outputWidth)
			.arg(outputHeight);

	// Checking that the icon doesn't already exist
	if (QFileInfo(outputFilename).exists()) {
		// Emitting the signal for the existing image and returning
		emit iconGenerated(icon.index, outputFilename);

		return;
	}

	// Creating the image and the painter to draw the image
	QImage outputImage(outputWidth, outputHeight, QImage::Format_ARGB32);
	outputImage.fill(Qt::transparent);
	QPainter painter(&outputImage);
	painter.setRenderHint(QPainter::Antialiasing, true);

	// Generating the image and saving it. Printing a message in case of errors
	svgSource.render(&painter);
	painter.end();
	if (!outputImage.save(outputFilename, nullptr, 100)) {
		qDebug() << "Could not save png icon" << outputFilename;
		emit iconGenerated(icon.index, QString());

		return;
	}

	// Emitting the signal
	emit iconGenerated(icon.index, outputFilename);
}
