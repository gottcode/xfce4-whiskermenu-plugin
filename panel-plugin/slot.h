/*
 * Copyright (C) 2013 Graeme Gott <graeme@gottcode.org>
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

// Member function without parameters
template<typename T, typename R>
gulong g_signal_connect_slot(gpointer instance, const gchar* detailed_signal, R(T::*member)(), T* obj, bool after = false)
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

		static R invoke(Slot* slot)
		{
			return (slot->m_instance->*slot->m_member)();
		}

		static void destroy(Slot* slot)
		{
			delete slot;
		}
	};
	R (*invoke_slot)(Slot*) = &Slot::invoke;
	void (*destroy_slot)(Slot*) = &Slot::destroy;

	return g_signal_connect_data(instance, detailed_signal,
			reinterpret_cast<GCallback>(invoke_slot),
			new Slot(obj, member),
			reinterpret_cast<GClosureNotify>(destroy_slot),
			after ? GConnectFlags(G_CONNECT_AFTER | G_CONNECT_SWAPPED) : G_CONNECT_SWAPPED);
}

// Member function with 1 parameter
template<typename T, typename R, typename A1>
gulong g_signal_connect_slot(gpointer instance, const gchar* detailed_signal, R(T::*member)(A1), T* obj, bool after = false)
{
	class Slot
	{
		T* m_instance;
		R (T::*m_member)(A1);

	public:
		Slot(T* instance, R (T::*member)(A1)) :
			m_instance(instance),
			m_member(member)
		{
		}

		static R invoke(A1 a1, Slot* slot)
		{
			return (slot->m_instance->*slot->m_member)(a1);
		}

		static void destroy(Slot* slot)
		{
			delete slot;
		}
	};
	R (*invoke_slot)(A1,Slot*) = &Slot::invoke;
	void (*destroy_slot)(Slot*) = &Slot::destroy;

	return g_signal_connect_data(instance, detailed_signal,
			reinterpret_cast<GCallback>(invoke_slot),
			new Slot(obj, member),
			reinterpret_cast<GClosureNotify>(destroy_slot),
			after ? G_CONNECT_AFTER : GConnectFlags(0));
}

// Member function with 2 parameters
template<typename T, typename R, typename A1, typename A2>
gulong g_signal_connect_slot(gpointer instance, const gchar* detailed_signal, R(T::*member)(A1,A2), T* obj, bool after = false)
{
	class Slot
	{
		T* m_instance;
		R (T::*m_member)(A1,A2);

	public:
		Slot(T* instance, R (T::*member)(A1,A2)) :
			m_instance(instance),
			m_member(member)
		{
		}

		static R invoke(A1 a1, A2 a2, Slot* slot)
		{
			return (slot->m_instance->*slot->m_member)(a1, a2);
		}

		static void destroy(Slot* slot)
		{
			delete slot;
		}
	};
	R (*invoke_slot)(A1,A2,Slot*) = &Slot::invoke;
	void (*destroy_slot)(Slot*) = &Slot::destroy;

	return g_signal_connect_data(instance, detailed_signal,
			reinterpret_cast<GCallback>(invoke_slot),
			new Slot(obj, member),
			reinterpret_cast<GClosureNotify>(destroy_slot),
			after ? G_CONNECT_AFTER : GConnectFlags(0));
}

// Member function with 3 parameters
template<typename T, typename R, typename A1, typename A2, typename A3>
gulong g_signal_connect_slot(gpointer instance, const gchar* detailed_signal, R(T::*member)(A1,A2,A3), T* obj, bool after = false)
{
	class Slot
	{
		T* m_instance;
		R (T::*m_member)(A1,A2,A3);

	public:
		Slot(T* instance, R (T::*member)(A1,A2,A3)) :
			m_instance(instance),
			m_member(member)
		{
		}

		static R invoke(A1 a1, A2 a2, A3 a3, Slot* slot)
		{
			return (slot->m_instance->*slot->m_member)(a1, a2, a3);
		}

		static void destroy(Slot* slot)
		{
			delete slot;
		}
	};
	R (*invoke_slot)(A1,A2,A3,Slot*) = &Slot::invoke;
	void (*destroy_slot)(Slot*) = &Slot::destroy;

	return g_signal_connect_data(instance, detailed_signal,
			reinterpret_cast<GCallback>(invoke_slot),
			new Slot(obj, member),
			reinterpret_cast<GClosureNotify>(destroy_slot),
			after ? G_CONNECT_AFTER : GConnectFlags(0));
}

}

#endif // WHISKERMENU_SLOT_H
