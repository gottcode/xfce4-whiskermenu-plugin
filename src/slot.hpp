// Copyright (C) 2013 Graeme Gott <graeme@gottcode.org>
//
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this library.  If not, see <http://www.gnu.org/licenses/>.


#ifndef WHISKERMENU_SLOT_HPP
#define WHISKERMENU_SLOT_HPP

extern "C"
{
#include <glib-object.h>
}

namespace WhiskerMenu
{

// Member functions without parameters
template <typename T, typename R>
struct Slot0
{
	T* instance;
	R (T::*member)();
};

template <typename T, typename R>
R invoke_slot0(Slot0<T,R>* slot)
{
	return (slot->instance->*slot->member)();
}

template <typename T, typename R>
void delete_slot0(Slot0<T,R>* slot)
{
	delete slot;
	slot = nullptr;
}

template<typename T, typename R>
gulong g_signal_connect_slot(gpointer signal_obj, const gchar* detailed_signal, R (T::*member)(), T* instance)
{
	return g_signal_connect_closure(signal_obj, detailed_signal,
		g_cclosure_new_swap
		(
			(GCallback)&invoke_slot0<T,R>,
			new Slot0<T,R>{instance, member},
			(GClosureNotify)&delete_slot0<T,R>
		),
		false);
}

// Member functions with parameters
template <typename T, typename R, typename... Args>
struct SlotArgs
{
	T* instance;
	R (T::*member)(Args...);
};

template <typename T, typename R, typename... Args>
R invoke_slot_args(Args... args, SlotArgs<T,R,Args...>* slot)
{
	return (slot->instance->*slot->member)(args...);
}

template <typename T, typename R, typename... Args>
void delete_slot_args(SlotArgs<T,R,Args...>* slot)
{
	delete slot;
	slot = nullptr;
}

template<typename T, typename R, typename... Args>
gulong g_signal_connect_slot(gpointer signal_obj, const gchar* detailed_signal, R (T::*member)(Args...), T* instance)
{
	return g_signal_connect_closure(signal_obj, detailed_signal,
		g_cclosure_new
		(
			(GCallback)&invoke_slot_args<T,R,Args...>,
			new SlotArgs<T,R,Args...>{instance, member},
			(GClosureNotify)&delete_slot_args<T,R,Args...>
		),
		false);
}

}

#endif // WHISKERMENU_SLOT_HPP
