/*
 * Copyright (C) 2013-2020 Graeme Gott <graeme@gottcode.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WHISKERMENU_SLOT_H
#define WHISKERMENU_SLOT_H

#include <glib-object.h>

namespace WhiskerMenu
{
// Connect flags
enum class Connect
{
	Default = 0,
	After = G_CONNECT_AFTER
};

// Member function with ignored parameters
template<typename... Args, typename T, typename R>
gulong g_signal_connect_slot(gpointer instance, const gchar* detailed_signal, R(T::*member)(), T* obj, Connect flags = Connect::Default)
{
	class Slot
	{
		T* m_instance;
		R (T::*m_member)();

	public:
		Slot(T* instance, R (T::*member)()) :
			m_instance(instance),
			m_member(member)
		{
		}

		static R invoke(Args..., gpointer user_data)
		{
			Slot* slot = static_cast<Slot*>(user_data);
			return (slot->m_instance->*slot->m_member)();
		}

		static void destroy(gpointer data, GClosure*)
		{
			delete static_cast<Slot*>(data);
		}
	};

	return g_signal_connect_data(instance, detailed_signal,
			G_CALLBACK(&Slot::invoke),
			new Slot(obj, member),
			&Slot::destroy,
			GConnectFlags(flags));
}

// Member function with parameters
template<typename T, typename R, typename... Args>
gulong g_signal_connect_slot(gpointer instance, const gchar* detailed_signal, R(T::*member)(Args...), T* obj, Connect flags = Connect::Default)
{
	class Slot
	{
		T* m_instance;
		R (T::*m_member)(Args...);

	public:
		Slot(T* instance, R (T::*member)(Args...)) :
			m_instance(instance),
			m_member(member)
		{
		}

		static R invoke(Args... args, gpointer user_data)
		{
			Slot* slot = static_cast<Slot*>(user_data);
			return (slot->m_instance->*slot->m_member)(args...);
		}

		static void destroy(gpointer data, GClosure*)
		{
			delete static_cast<Slot*>(data);
		}
	};

	return g_signal_connect_data(instance, detailed_signal,
			G_CALLBACK(&Slot::invoke),
			new Slot(obj, member),
			&Slot::destroy,
			GConnectFlags(flags));
}

// Member function with 1 parameter and 1 bound parameter
template<typename T, typename R, typename A1, typename A2>
gulong g_signal_connect_slot(gpointer instance, const gchar* detailed_signal, R(T::*member)(A1,A2), T* obj, A2 bound1, Connect flags = Connect::Default)
{
	class Slot
	{
		T* m_instance;
		R (T::*m_member)(A1,A2);
		A2 m_bound1;

	public:
		Slot(T* instance, R (T::*member)(A1,A2), A2 bound1) :
			m_instance(instance),
			m_member(member),
			m_bound1(bound1)
		{
		}

		static R invoke(A1 arg, gpointer user_data)
		{
			Slot* slot = static_cast<Slot*>(user_data);
			return (slot->m_instance->*slot->m_member)(arg, slot->m_bound1);
		}

		static void destroy(gpointer data, GClosure*)
		{
			delete static_cast<Slot*>(data);
		}
	};

	return g_signal_connect_data(instance, detailed_signal,
			G_CALLBACK(&Slot::invoke),
			new Slot(obj, member, bound1),
			&Slot::destroy,
			GConnectFlags(flags));
}

}

#endif // WHISKERMENU_SLOT_H
