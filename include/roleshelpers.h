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

#ifndef __ROLES_HELPERS_H__
#define __ROLES_HELPERS_H__

#include <QVariant>
#include <QHash>
#include <array>
#include <utility>
#include <functional>
#include "include/utilities.h"

class RolesCallbackStorage;
template <class>
class RolesVectors;
template <class>
class Roles;
template <class>
class RolesQMLAccessor;

/**
 * \brief This macro helps in defining a new role
 *
 * By using this you only have to specify the role name and the QVariant
 * function to get the underlying type (e.g. toString, toUrl, toInt...). It is
 * advisable to define related roles inside a namespace or class, to avoid
 * polluting the global namespace (remember that each role is a class). The name
 * of the role (as a string) will be the same as the name of the  class, so you
 * should use a lowercase letter as the first letter of the name
 */
#define DEFINE_ROLE(RoleName, ConversionFunction) \
	class RoleName\
	{\
	public:\
		using RoleType = decltype(std::declval<QVariant>().ConversionFunction());\
		RoleName(int index) : i(index) {}\
		RoleName(const RoleName& other) : i(other.i), v(other.v) {}\
		RoleName(RoleName&& other) : i(other.i), v(std::move(other.v)) {}\
		RoleType toRoleType() const { return v.ConversionFunction(); }\
		void fromRoleType(const RoleType& d) { v = d; }\
		static constexpr const char* s = #RoleName;\
		const int i;\
		QVariant v;\
	};

/**
 * \brief An helper function to call all callbacks with the given role index
 *
 * \param s the object containing the list of callbacks to call
 * \param roleIndex the index to pass to callbacks
 */
void callSetDataCallbacks(const RolesCallbackStorage* s, int roleIndex);

/**
 * \brief An helper class storing callbacks used by RolesLists
 *
 * We need this to create the variable of the list of functors before the list
 * of roles (the list take a reference to the list of functors)
 * \warning You should not use this class directly, use the Roles class instead
 */
class RolesCallbackStorage
{
public:
	/**
	 * \brief Constructor
	 */
	RolesCallbackStorage();

	/**
	 * \brief Adds a callback to call when setData of roles is called
	 *
	 * \param setDataCallback a function to call when setData() of RolesList
	 *                        is called. The function must take an int
	 *                        parameter (the index of the role that has
	 *                        changed) and return void.
	 * \return the id of the callback. You need it in case you want to
	 *         remove the callback
	 */
	int addCallbackForSetData(const std::function<void(int)>& setDataCallback);

	/**
	 * \brief Removes a callback to call when setData of roles is called
	 *
	 * \param id the id of the callback to remove (the one returned when
	 *           addCallbackForSetData was called)
	 * \return false if no callback with the given id exists, true otherwise
	 */
	bool removeCallbackForSetData(int id);

private:
	/**
	 * \brief The function called when the setData function is called
	 *
	 * The parameter is the index of the Role that was changed
	 */
	QHash<int, std::function<void(int)>> m_setDataCallbacks;

	/**
	 * \brief The id to use for the next callback
	 */
	int m_nextCallbackID;

	/**
	 * \brief Roles is friend to call access m_setDataCallback
	 */
	template <class>
	friend class Roles;

	/**
	 * \brief callSetDataCallbacks is friend to access the list of callbacks
	 */
	friend void callSetDataCallbacks(const RolesCallbackStorage* s, int roleIndex);
};

/**
 * \brief The class modelling a list of roles
 *
 * This class and the template specializations below model a list of roles. The
 * template parameters of this class should only be roles (see the template
 * specializations below for other kinds of parameters). You can access a role
 * index, name or value by using the Role name (i.e. the class name) as template
 * parameter of functions below. The index of roles in each list are consecutive
 * starting at startIndex(). You can register a function to be called whenever
 * the setData function is called. Notice that the function is not called if the
 * value of the role is changed via the vector passed in valuesVector.
 * \warning You should not use this class directly, use the Roles class instead
 */
template <typename Role_t, typename... OtherRoles>
class RolesList : protected Role_t, public RolesList<OtherRoles...>
{
public:
	/**
	 * \brief A typedef for the Role_t template parameter
	 */
	using Role = Role_t;

	/**
	 * \brief A typedef for the list with the other roles
	 */
	using InnerList = RolesList<OtherRoles...>;

public:
	/**
	 * \brief Constructor
	 *
	 * \param callbacks the object with all the callbacks
	 * \param valuesVector this is filled with the data of roles (the value
	 *                     of the role with index i will be in the vector at
	 *                     position i)
	 * \param indexOffset the offset to use for indexes of roles. Always
	 *                    set to 0 (this is for internal use only)
	 */
	RolesList(const RolesCallbackStorage* callbacks, QVariant** valuesVector, int indexOffset = 0)
		: Role(sizeof...(OtherRoles) + indexOffset)
		, InnerList(callbacks, valuesVector, indexOffset)
		, m_startIndex(indexOffset)
		, m_callbacks(callbacks)
	{
		if (valuesVector) {
			valuesVector[Role::i] = &(Role::v);
		}
	}

	/**
	 * \brief Returns the index of the given role
	 *
	 * The role is passed as the template parameter
	 * \return the index of the given role
	 */
	template <class R>
	int getIndex() const
	{
		return (static_cast<const R*>(this))->i;
	}

	/**
	 * \brief Returns the name of the given role
	 *
	 * The role is passed as the template parameter
	 * \return the name of the given role
	 */
	template <class R>
	const char* getName() const
	{
		return (static_cast<const R*>(this))->s;
	}

	/**
	 * \brief Returns the value for the given role
	 *
	 * The role is passed as the template parameter
	 * \return the value for the given role
	 */
	template <class R>
	typename R::RoleType getData() const
	{
		return (static_cast<const R*>(this))->toRoleType();
	}

	/**
	 * \brief Sets the value for the given role
	 *
	 * The role is passed as the template parameter
	 * \param d the new value for the given role
	 * \param ignoreCallback if true the callback is not called, otherwise
	 *                       it is called
	 */
	template <class R>
	void setData(const typename R::RoleType& d, bool ignoreCallback = false)
	{
		(static_cast<R*>(this))->fromRoleType(d);
		if (!ignoreCallback) {
			callSetDataCallbacks(m_callbacks, getIndex<R>());
		}
	}

	/**
	 * \brief Returns the number of roles in this list
	 *
	 * \return the number of roles in this list
	 */
	static constexpr int numRoles()
	{
		return sizeof...(OtherRoles) + 1;
	}

	/**
	 * \brief Returns the start index for roles in this list
	 *
	 * \return the start index for roles in this list
	 */
	int startIndex() const
	{
		return m_startIndex;
	}

	/**
	 * \brief Copies the data for all roles from the other list
	 *
	 * \param other the list to copy
	 */
	void copyDataFromOtherRolesList(const RolesList<Role, OtherRoles...>& other)
	{
		Role::v = (static_cast<const Role&>(other)).v;
		InnerList::copyDataFromOtherRolesList(other);
	}

	/**
	 * \brief Resets the values of all roles
	 *
	 * \param ignoreCallback if true the callback is not called, otherwise
	 *                       it is called for each role
	 */
	void reset(bool ignoreCallback)
	{
		Role::v = QVariant();
		if (!ignoreCallback) {
			callSetDataCallbacks(m_callbacks, Role::i);
		}
		InnerList::reset(ignoreCallback);
	}

private:
	/**
	 * \brief The starting index for roles in this list
	 */
	const int m_startIndex;

	/**
	 * \brief The object with all the callbacks
	 */
	const RolesCallbackStorage* m_callbacks;

	/**
	 * \brief The function to fill the array with names of roles
	 *
	 * This function takes an array of const char* and fills it with the
	 * name of roles, putting names at the correct index
	 * \param a the array to fill
	 */
	template <class Array>
	void fillNameArray(Array& a)
	{
		a[Role::i] = Role::s;
		this->InnerList::fillNameArray(a);
	}

	/**
	 * \brief The function to fill the map of role names to indexes
	 *
	 * This function takes a map of strings to integers and fills it with
	 * the name of roles and the corresponding role index
	 * \param a the array to fill
	 */
	void fillNameIndexMap(QHash<QByteArray, int>& h)
	{
		h[Role::s] = Role::i;
		this->InnerList::fillNameIndexMap(h);
	}

	/**
	 * \brief RolesVector is friend to call fillNameArray() and
	 *        fillNameIndexMap()
	 */
	template <class>
	friend class RolesVectors;

	/**
	 * \brief All RolesList template specializations are friend to call
	 *        fillNameArray() and fillNameIndexMap()
	 */
	template <typename, typename...>
	friend class RolesList;
};

/**
 * \brief The class modelling a list of roles
 *
 * This is the template specialization with only one role (also used as the
 * termination of the recursion on template parameters)
 */
template <typename Role_t>
class RolesList<Role_t> : protected Role_t
{
public:
	/**
	 * \brief A typedef for the Role_t template parameter
	 */
	using Role = Role_t;

public:
	/**
	 * \brief Constructor
	 *
	 * \param callbacks the object with all the callbacks
	 * \param valuesVector this is filled with the data of roles (the value
	 *                     of the role with index i will be in the vector at
	 *                     position i)
	 * \param indexOffset the offset to use for indexes of roles. Always
	 *                    set to 0 (this is for intenal use only)
	 */
	RolesList(const RolesCallbackStorage* callbacks, QVariant** valuesVector, int indexOffset = 0)
		: Role(0 + indexOffset)
		, m_startIndex(indexOffset)
		, m_callbacks(callbacks)
	{
		if (valuesVector) {
			valuesVector[Role::i] = &(Role::v);
		}
	}

	/**
	 * \brief Returns the index of the given role
	 *
	 * The role is passed as the template parameter
	 * \return the index of the given role
	 */
	template <class R>
	int getIndex() const
	{
		return (static_cast<const R*>(this))->i;
	}

	/**
	 * \brief Returns the name of the given role
	 *
	 * The role is passed as the template parameter
	 * \return the name of the given role
	 */
	template <class R>
	const char* getName() const
	{
		return (static_cast<const R*>(this))->s;
	}

	/**
	 * \brief Returns the value for the given role
	 *
	 * The role is passed as the template parameter
	 * \return the value for the given role
	 */
	template <class R>
	typename R::RoleType getData() const
	{
		return (static_cast<const R*>(this))->toRoleType();
	}

	/**
	 * \brief Sets the value for the given role
	 *
	 * The role is passed as the template parameter
	 * \param d the new value for the given role
	 * \param ignoreCallback if true the callback is not called, otherwise
	 *                       it is called
	 */
	template <class R>
	void setData(const typename R::RoleType& d, bool ignoreCallback = false)
	{
		(static_cast<R*>(this))->fromRoleType(d);
		if (!ignoreCallback) {
			callSetDataCallbacks(m_callbacks, getIndex<R>());
		}
	}

	/**
	 * \brief Returns the number of roles in this list
	 *
	 * \return the number of roles in this list
	 */
	static constexpr int numRoles()
	{
		return 1;
	}

	/**
	 * \brief Returns the start index for roles in this list
	 *
	 * \return the start index for roles in this list
	 */
	int startIndex() const
	{
		return m_startIndex;
	}

	/**
	 * \brief Copies the data for all roles from the other list
	 *
	 * \param other the list to copy
	 */
	void copyDataFromOtherRolesList(const RolesList<Role>& other)
	{
		Role::v = (static_cast<const Role&>(other)).v;
	}

	/**
	 * \brief Resets the values of all roles
	 *
	 * \param ignoreCallback if true the callback is not called, otherwise
	 *                       it is called for each role
	 */
	void reset(bool ignoreCallback)
	{
		Role::v = QVariant();
		if (!ignoreCallback) {
			callSetDataCallbacks(m_callbacks, Role::i);
		}
	}

private:
	/**
	 * \brief The starting index for roles in this list
	 */
	const int m_startIndex;

	/**
	 * \brief The object with all the callbacks
	 */
	const RolesCallbackStorage* m_callbacks;

	/**
	 * \brief The function to fill the array with names of roles
	 *
	 * This function takes an array of const char* and fills it with the
	 * name of roles, putting names at the correct index
	 * \param a the array to fill
	 */
	template <class Array>
	void fillNameArray(Array& a)
	{
		a[Role::i] = Role::s;
	}

	/**
	 * \brief The function to fill the map of role names to indexes
	 *
	 * This function takes a map of strings to integers and fills it with
	 * the name of roles and the corresponding role index
	 * \param a the array to fill
	 */
	void fillNameIndexMap(QHash<QByteArray, int>& h)
	{
		h[Role::s] = Role::i;
	}

	/**
	 * \brief RolesVector is friend to call fillNameArray() and
	 *        fillNameIndexMap()
	 */
	template <class>
	friend class RolesVectors;

	/**
	 * \brief All RolesList template specializations are friend to call
	 *        fillNameArray() and fillNameIndexMap()
	 */
	template <typename, typename...>
	friend class RolesList;
};

/**
 * \brief The class modelling a list of roles
 *
 * This is the template specialization that can be used to merge two lists. This
 * only works with RolesLists, if you want to add one Role, wrap it in a
 * RolesList.
 */
template <typename... RolesFirstList, typename... RolesSecondList>
class RolesList<RolesList<RolesFirstList...>, RolesList<RolesSecondList...>> : public RolesList<RolesFirstList...>, public RolesList<RolesSecondList...>
{
public:
	/**
	 * \brief A typedef for the first list with roles
	 */
	using FirstList = RolesList<RolesFirstList...>;

	/**
	 * \brief A typedef for the second list with roles
	 */
	using SecondList = RolesList<RolesSecondList...>;

public:
	/**
	 * \brief Constructor
	 *
	 * \param callbacks the object with all the callbacks
	 * \param valuesVector this is filled with the data of roles (the value
	 *                     of the role with index i will be in the vector at
	 *                     position i)
	 * \param indexOffset the offset to use for indexes of roles. Always
	 *                    set to 0 (this is for intenal use only)
	 */
	RolesList(const RolesCallbackStorage* callbacks, QVariant** valuesVector, int indexOffset = 0)
		: FirstList(callbacks, valuesVector, 0 + indexOffset)
		, SecondList(callbacks, valuesVector, FirstList::numRoles() + indexOffset)
		, m_startIndex(indexOffset)
		, m_callbacks(callbacks)
	{
	}

	/**
	 * \brief Returns the index of the given role
	 *
	 * The role is passed as the template parameter
	 * \return the index of the given role
	 */
	template <class R>
	int getIndex() const
	{
		return (static_cast<const R*>(this))->i;
	}

	/**
	 * \brief Returns the name of the given role
	 *
	 * The role is passed as the template parameter
	 * \return the name of the given role
	 */
	template <class R>
	const char* getName() const
	{
		return (static_cast<const R*>(this))->s;
	}

	/**
	 * \brief Returns the value for the given role
	 *
	 * The role is passed as the template parameter
	 * \return the value for the given role
	 */
	template <class R>
	typename R::RoleType getData() const
	{
		return (static_cast<const R*>(this))->toRoleType();
	}

	/**
	 * \brief Sets the value for the given role
	 *
	 * The role is passed as the template parameter
	 * \param d the new value for the given role
	 * \param ignoreCallback if true the callback is not called, otherwise
	 *                       it is called
	 */
	template <class R>
	void setData(const typename R::RoleType& d, bool ignoreCallback = false)
	{
		(static_cast<R*>(this))->fromRoleType(d);
		if (!ignoreCallback) {
			callSetDataCallbacks(m_callbacks, getIndex<R>());
		}
	}

	/**
	 * \brief Returns the number of roles in this list
	 *
	 * \return the number of roles in this list
	 */
	static constexpr int numRoles()
	{
		return FirstList::numRoles() + SecondList::numRoles();
	}

	/**
	 * \brief Returns the start index for roles in this list
	 *
	 * \return the start index for roles in this list
	 */
	int startIndex() const
	{
		return m_startIndex;
	}

	/**
	 * \brief Copies the data for all roles from the other list
	 *
	 * \param other the list to copy
	 */
	void copyDataFromOtherRolesList(const RolesList<RolesList<RolesFirstList...>, RolesList<RolesSecondList...>>& other)
	{
		FirstList::copyDataFromOtherList(other);
		SecondList::copyDataFromOtherList(other);
	}

	/**
	 * \brief Resets the values of all roles
	 *
	 * \param ignoreCallback if true the callback is not called, otherwise
	 *                       it is called for each role
	 */
	void reset(bool ignoreCallback)
	{
		FirstList::reset(ignoreCallback);
		SecondList::reset(ignoreCallback);
	}

private:
	/**
	 * \brief The starting index for roles in this list
	 */
	const int m_startIndex;

	/**
	 * \brief The object with all the callbacks
	 */
	const RolesCallbackStorage* m_callbacks;

	/**
	 * \brief The function to fill the array with names of roles
	 *
	 * This function takes an array of const char* and fills it with the
	 * name of roles, putting names at the correct index
	 * \param a the array to fill
	 */
	template <class Array>
	void fillNameArray(Array& a)
	{
		this->FirstList::fillNameArray(a);
		this->SecondList::fillNameArray(a);
	}

	/**
	 * \brief The function to fill the map of role names to indexes
	 *
	 * This function takes a map of strings to integers and fills it with
	 * the name of roles and the corresponding role index
	 * \param a the array to fill
	 */
	void fillNameIndexMap(QHash<QByteArray, int>& h)
	{
		this->FirstList::fillNameIndexMap(h);
		this->SecondList::fillNameIndexMap(h);
	}

	/**
	 * \brief RolesVector is friend to call fillNameArray() and
	 *        fillNameIndexMap()
	 */
	template <class>
	friend class RolesVectors;

	/**
	 * \brief All RolesList template specializations are friend to call
	 *        fillNameArray() and fillNameIndexMap()
	 */
	template <typename, typename...>
	friend class RolesList;
};

/**
 * \brief A class with additional data structures for faster access to roles
 *
 * This contains data structures for faster access to roles. This should be
 * constructed before a RolesList (this means that if you inherit from both this
 * and RolesList, this should come before RolesList in the list of inherited
 * classes) and its members should be passed to the RolesList constructor. The
 * rolesValue array is initialized when the RolesList is constructed, while the
 * rolesNames array and the rolesNamesToIndex map are const static members.
 * \warning You should not use this class directly, use the Roles class
 */
template <class RoleListType>
class RolesVectors
{
public:
	/**
	 * \brief Constructor
	 *
	 * \param callbacks the object with all the callbacks
	 */
	RolesVectors(const RolesCallbackStorage* callbacks)
		: m_roleValues()
		, m_callbacks(callbacks)
	{
	}

	/**
	 * \brief Returns the name of the role with the given index
	 *
	 * \param index the index of the role
	 * \return the nameof the role
	 */
	static const char* getRoleNameFromIndex(int index)
	{
		return m_roleNames[index];
	}

	/**
	 * \brief Returns the index of the role with the given name
	 *
	 * \param name the name of the role
	 * \return the index of the role or -1 if no role exists with that name
	 */
	static int getRoleIndexFromName(const QByteArray& name)
	{
		return m_roleNamesToIndex.value(name, -1);
	}

protected:
	/**
	 * \brief Returns data as QVariant for the given role
	 *
	 * \param index the index of the role for which to return data
	 * \return a const reference to QVariant data for the role
	 */
	const QVariant& getVariantData(int index) const
	{
		return *(m_roleValues[index]);
	}

	/**
	 * \brief Sets data from a QVariant for the given role
	 *
	 * \param index the index of the role to modify
	 * \param value the value to set
	 * \param ignoreCallback if true the callback is not called, otherwise
	 *                       it is called
	 */
	void setVariantData(int index, const QVariant& value, bool ignoreCallback = false)
	{
		*(m_roleValues[index]) = value;
		if (!ignoreCallback) {
			callSetDataCallbacks(m_callbacks, index);
		}
	}

private:
	/**
	 * \brief Generates the array of roles names
	 *
	 * \return the array of roles names
	 */
	static std::array<const char*, RoleListType::numRoles()> generateRoleNames()
	{
		// The simplest way of filling the vector is to generate a list and call its
		// functions. We can set parameters to nullptr, we are sure not to dereference them
		RoleListType l(nullptr, nullptr);
		std::array<const char*, RoleListType::numRoles()> a;

		l.fillNameArray(a);

		return a;
	}

	/**
	 * \brief Generates the map of roles names to indexes
	 *
	 * \return the map of roles names to indexes
	 */
	static QHash<QByteArray, int> generateRoleNamesToIndex()
	{
		// The simplest way of filling the vector is to generate a list and call its
		// functions. We can set parameters to nullptr, we are sure not to dereference them
		RoleListType l(nullptr, nullptr);
		QHash<QByteArray, int> h;

		l.fillNameIndexMap(h);

		return h;
	}

	/**
	 * \brief The vector of pointers to values of roles
	 */
	std::array<QVariant*, RoleListType::numRoles()> m_roleValues;

	/**
	 * \brief The vector of pointers to names of roles
	 */
	static const std::array<const char*, RoleListType::numRoles()> m_roleNames;

	/**
	 * \brief The dictionary with indexes for role names
	 */
	static const QHash<QByteArray, int> m_roleNamesToIndex;

	/**
	 * \brief The object with all the callbacks
	 *
	 * Callbacks is called when setVariantData is called
	 */
	const RolesCallbackStorage* m_callbacks;

	/**
	 * \brief Roles is friend to call access m_rolesValues
	 */
	template <class>
	friend class Roles;

	/**
	 * \brief RolesToPropertiesWrapper is friend to access vectors
	 */
	template<class>
	friend class RolesQMLAccessor;
};

// Definition of the array with roles names
template <class RoleListType>
const std::array<const char*, RoleListType::numRoles()> RolesVectors<RoleListType>::m_roleNames(generateRoleNames());

// Definition of the map from role names to indexes
template <class RoleListType>
const QHash<QByteArray, int> RolesVectors<RoleListType>::m_roleNamesToIndex(generateRoleNamesToIndex());

#endif
