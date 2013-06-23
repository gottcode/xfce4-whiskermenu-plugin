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

#define SLOT_CALLBACK(klassmember) G_CALLBACK(klassmember ## _slot)

#define SLOT_0(R, klass, member) \
	static R member ## _slot(klass* obj) \
		{ return obj->member(); } \
	R member()

#define SLOT_1(R, klass, member, T1) \
	static R member ## _slot(T1 arg1, klass* obj) \
		{ return obj->member(arg1); } \
	R member(T1)

#define SLOT_2(R, klass, member, T1, T2) \
	static R member ## _slot(T1 arg1, T2 arg2, klass* obj) \
		{ return obj->member(arg1, arg2); } \
	R member(T1, T2)

#define SLOT_3(R, klass, member, T1, T2, T3) \
	static R member ## _slot(T1 arg1, T2 arg2, T3 arg3, klass* obj) \
		{ return obj->member(arg1, arg2, arg3); } \
	R member(T1, T2, T3)

#define SLOT_4(R, klass, member, T1, T2, T3, T4) \
	static R member ## _slot(T1 arg1, T2 arg2, T3 arg3, T4 arg4, klass* obj) \
		{ return obj->member(arg1, arg2, arg3, arg4); } \
	R member(T1, T2, T3, T4)

#define SLOT_5(R, klass, member, T1, T2, T3, T4, T5) \
	static R member ## _slot(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, klass* obj) \
		{ return obj->member(arg1, arg2, arg3, arg4, arg5); } \
	R member(T1, T2, T3, T4, T5)

#endif // WHISKERMENU_SLOT_HPP
