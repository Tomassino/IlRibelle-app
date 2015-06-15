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

#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <QList>
#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QEvent>
#include <QObject>
#include <functional>
#include <memory>
#include <type_traits>

/**
 * \brief The class to implement singletons
 *
 * This class implements a singleton which can be explicitly created and
 * deleted. If not explicitly created, memory is allocated on the first call to
 * instance(); if it is not explicitly deleted, memory is freed automatically
 * when static function variables are deleted
 */
template <class T>
class Singleton
{
protected:
	/**
	 * \brief Constructor
	 *
	 * This is here so that it is possible to inherit from Singleton
	 */
	Singleton()
	{
	}

public:
	/**
	 * \brief Copy constructor is disabled
	 */
	Singleton(const Singleton&) = delete;

	/**
	 * \brief Move constructor is disabled
	 */
	Singleton(Singleton&&) = delete;

	/**
	 * \brief Copy operator is disabled
	 */
	Singleton& operator=(const Singleton&) = delete;

	/**
	 * \brief Move operator is disabled
	 */
	Singleton& operator=(Singleton&&) = delete;

	/**
	 * \brief The function to get the unique instance of the class
	 *
	 * If the instance hasn't been created already, this creates the
	 * instance
	 * \return the unique instance of T
	 */
	static T& instance()
	{
		T* ptr = getPointer();

		if (ptr == nullptr) {
			ptr = getPointer(true, new T());
		}

		return *ptr;
	}

	/**
	 * \brief Creates the instance if it doesn't exists, otherwise does
	 *        nothing
	 */
	static void createInstance()
	{
		instance();
	}

	/**
	 * \brief Deletes the instance if it exists
	 */
	static void deleteInstance()
	{
		getPointer(true, nullptr);
	}

private:
	/**
	 * \brief Returns the value of a static pointer
	 *
	 * This function returns the value of a static pointer. It is
	 * initialized to nullptr and then, if overwrite is true, replaced by
	 * the ptr pointer (the memory for the old pointer is released)
	 * \param overwrite whether to overwrite the pointer or not
	 * \param ptr the new value of the static pointer
	 * \return the value of the internal static pointer
	 */
	static T* getPointer(bool overwrite = false, T* ptr = nullptr)
	{
		static std::unique_ptr<T> internalPtr;

		if (overwrite) {
			internalPtr.reset(ptr);
		}

		return internalPtr.get();
	}
};

/**
 * \brief Converts an enum class value to its underlying type
 *
 * \param v the value to convert
 * \return the value in the undelrying type
 */
template <class EnumClass>
constexpr typename std::underlying_type<EnumClass>::type toUnderlying(EnumClass v)
{
	static_assert(std::is_enum<EnumClass>::value, "toUnderlying requires an enum type");

	return static_cast<typename std::underlying_type<EnumClass>::type>(v);
}

/**
 * \brief Converts an integral type to an enum class value
 *
 * \param v the value to convert
 * \return the enum class value
 */
template <class EnumClass>
constexpr EnumClass fromUnderlying(typename std::underlying_type<EnumClass>::type v)
{
	static_assert(std::is_enum<EnumClass>::value, "toUnderlying requires an enum type");

	return static_cast<EnumClass>(v);
}

/**
 * \brief Resizes the given QList
 *
 * This function resizes the list to the given size, similarly to what the
 * resize function of QVectors does
 * \param list the list to resize
 * \param size the new size of the list
 */
template <class T>
void resizeList(QList<T>& list, int size)
{
	const int initListSize = list.size();
	if (initListSize < size) {
		for (int i = initListSize; i < size; ++i) {
			list.append(T());
		}
	} else if (initListSize > size) {
		for (int i = size; i < initListSize; ++i) {
			list.removeLast();
		}
	}
}

/**
 * \brief Converts a date from string in rss format to QDateTime
 *
 * \param str the string to convert
 * \return the date represented by str or an invalid data if conversion fails
 */
QDateTime dateTimeFromRssString(const QString& str);

/**
 * \brief A lightweigth type representation
 */
template <class T>
class Type2Type
{
public:
	/**
	 * \brief The represented type
	 */
	using Type = T;
};

/**
 * \brief A small function that converts the first letter of the string to
 *        lowercase
 *
 * \param s the string to convers
 * \return the string with the first letter lowercase
 */
inline QByteArray firstToLowercase(const QByteArray& s)
{
	return s.left(1).toLower() + s.mid(1);
}

/**
 * \brief An event delivering a custom command to execute
 *
 * This is an event that carries a std::function<void()> object. The function
 * object can be executed calling the executeCommand() function. The QObjects
 * that want to accept this kind of event should re-implement the
 * QObject::customEvent() function, call executeCommand() and accept() the
 * event.
 */
class CommandEvent : public QEvent
{
public:
	/**
	 * \brief The id of this type of events
	 */
	static const QEvent::Type eventType;

public:
	/**
	 * \brief Constructor
	 *
	 * \param command the command to execute
	 */
	CommandEvent(const std::function<void()>& command);

	/**
	 * \brief Destructor
	 */
	virtual ~CommandEvent() = default;

	/**
	 * \brief Executes the command carried by this object
	 */
	void executeCommand();

private:
	/**
	 * \brief The command carried by this object
	 */
	std::function<void()> m_command;
};

/**
 * \brief The class receiving and handling command events
 *
 * You can send command events to this class so that it can handle it. You can
 * use the CommandEventReceiver singleton.
 */
class CommandEventReceiverClass : public QObject
{
	Q_OBJECT

public:
	/**
	 * \brief Constructor
	 *
	 * This is private to only have one instace of this class
	 */
	CommandEventReceiverClass();

	/**
	 * \brief Destructor
	 *
	 * This is private to only have one instance of this class
	 */
	virtual ~CommandEventReceiverClass();

	/**
	 * \brief The function called when we receive an event
	 *
	 * This function is overridden here to handle CommandEvents
	 * \param e the event we received
	 * \return true if we handled the event, false otherwise
	 */
	virtual bool event(QEvent* e) override;
};

/**
 * \brief The singleton for the CommandEventReceiverClass
 *
 * You should create this class in the main thread and only use it from the main
 * thread
 */
using CommandEventReceiver = Singleton<CommandEventReceiverClass>;

#endif
