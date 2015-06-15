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

#ifndef __ICONS_GENERATOR_H__
#define __ICONS_GENERATOR_H__

#include <QThread>
#include <QString>
#include <QList>
#include <QMutex>
#include <QWaitCondition>

/**
 * \brief The class responsible of generating the icons with the correct
 *        resolutions
 *
 * Icons of the program are resources in svg (vector) format. They are converted
 * to png when the application is first loaded and stored in the data directory.
 * The are re-generated in subsequent executions if missing or having the wrong
 * dimension (the dimension is part of the filename to avoid having to load the
 * icon to check the size). This class runs conversion in a separate thread.
 * You can enqueue conversions using the convertIcon() function. The
 * iconGenerated() signal is emitted when an icon is generated
 */
class IconsGenerator : public QThread
{
	Q_OBJECT

public:
	/**
	 * \brief The enum to specify what is the dimension specified in the
	 *        convertIcon function
	 */
	enum class SpecifiedDimension {
		Width, /// Specified dimension is the width
		Height /// Specified dimension is the height
	};

private:
	/**
	 * \brief The structure with data regarding an icon to generate
	 */
	struct IconToGenerate {
		/**
		 * \brief The path of the svg icon to convert to png
		 */
		QString sourceSvg;

		/**
		 * \brief The index of the icon to convert
		 *
		 * This is used in output filename generation and in the
		 * iconGenerated() signal
		 */
		unsigned int index;

		/**
		 * brief The dimension of the icon to generate
		 *
		 * Whether this is interpreted as the width or height of the
		 * output icon depends on the s parameter. The proportion of the
		 * source width and height is kept
		 */
		int iconDim;

		/**
		 * brief This specifies whether iconDim is the width or height
		 *       of the output icon
		 */
		SpecifiedDimension s;
	};

public:
	/**
	 * \brief Constructor
	 *
	 * \param outputPath the path of the directory where icons will be
	 *                   saved. The directory will be created if it doesn't
	 *                   exists
	 */
	IconsGenerator(QString outputPath);

	/**
	 * \brief Destructor
	 *
	 * You must stop the thread externally before calling this
	 */
	virtual ~IconsGenerator();

	/**
	 * \brief Enqueues the conversion of an icon
	 *
	 * This enqueues the conversion of an icon in a separate thread. When
	 * the conversion is finished the iconGenerated() signal is emitted. The
	 * name of the file of the generated icon is returned by the
	 * iconGenerated() signal and has the following format:
	 *	<sourceSvg basename>-<index>-<width>x<height>.png
	 * This function is thread-safe
	 * \param sourceSvg the path of the svg icon to convert to png
	 * \param index the index of the icon to convert (used in output
	 *              filename generation and in the iconGenerated() signal)
	 * \param iconDim the dimension of the icon to generate. Whether this
	 *                is interpreted as the width or height of the output
	 *                icon depends on the s parameter (the proportion is
	 *                kept)
	 * \param s specifies whether iconDim is the width or height of the
	 *          output icon
	 */
	void convertIcon(QString sourceSvg, unsigned int index, int iconDim, SpecifiedDimension s);

	/**
	 * \brief Forcibly stops generation of icons
	 *
	 * This als stops the thread
	 */
	void interruptGeneration();

signals:
	/**
	 * \brief The signal emitted when an icons has been generated
	 *
	 * If there was an error generating the image, the signal is emitted but
	 * the filename is the empty string
	 * \param index the index of the icon that has been generated
	 * \param filename the full path of the file with the generated icon
	 */
	void iconGenerated(unsigned int index, QString filename);

private:
	/**
	 * \brief The function doing the actual work
	 */
	virtual void run();

	/**
	 * \brief The function performing the actual conversion
	 *
	 * This emits the iconGenerated() signal when done
	 * \param icon the structure with information about the icon to convert
	 */
	void doConvertIcon(const IconToGenerate& icon);

	/**
	 * \brief The path of the directory where icons will be saved
	 */
	const QString m_outputPath;

	/**
	 * \brief The queue of icons to generate
	 */
	QList<IconToGenerate> m_iconsQueue;

	/**
	 * \brief This is set to true when the thread must stop
	 */
	bool m_stop;

	/**
	 * \brief The mutex protecting access to member across threads
	 */
	QMutex m_mutex;

	/**
	 * \brief The wait condition on which the worker thread waits for icons
	 *        to convert
	 */
	QWaitCondition m_waitCondition;
};

#endif
