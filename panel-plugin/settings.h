/*
 * Copyright (C) 2013-2023 Graeme Gott <graeme@gottcode.org>
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

#ifndef WHISKERMENU_SETTINGS_H
#define WHISKERMENU_SETTINGS_H

#include "icon-size.h"

namespace WhiskerMenu
{

class Command;
class Plugin;
class SearchAction;


// Boolean setting
class Boolean
{
public:
	explicit Boolean(const gchar* property, bool data);

	operator bool() const
	{
		return m_data;
	}

	Boolean& operator=(bool data)
	{
		set(data);
		return *this;
	}

	void load(XfceRc* rc);
	void save(XfceRc* rc);

private:
	void set(bool data);

private:
	const gchar* const m_property;
	bool m_data;
};


// Integer setting
class Integer
{
public:
	explicit Integer(const gchar* property, int data, int min, int max);

	operator int() const
	{
		return m_data;
	}

	Integer& operator=(int data)
	{
		set(data);
		return *this;
	}

	void load(XfceRc* rc);
	void save(XfceRc* rc);

private:
	void set(int data);

private:
	const gchar* const m_property;
	const int m_min;
	const int m_max;
	int m_data;
};


// String setting
class String
{
public:
	explicit String(const gchar* property, const std::string& data = std::string());

	bool empty() const
	{
		return m_data.empty();
	}

	operator const char*() const
	{
		return m_data.c_str();
	}

	bool operator==(const char* data) const
	{
		return data ? (m_data == data) : m_data.empty();
	}

	bool operator!=(const std::string& data) const
	{
		return m_data != data;
	}

	String& operator=(const std::string& data)
	{
		set(data);
		return *this;
	}

	String& operator=(const char* data)
	{
		return *this = std::string(data ? data : "");
	}

	void load(XfceRc* rc);
	void save(XfceRc* rc);

private:
	void set(const std::string& data);

private:
	const gchar* const m_property;
	std::string m_data;
};


// String list setting
class StringList
{
public:
	explicit StringList(const gchar* property, std::initializer_list<std::string> data);

	bool empty() const
	{
		return m_data.empty();
	}

	std::vector<std::string>::const_iterator begin() const
	{
		return m_data.cbegin();
	}

	std::vector<std::string>::const_iterator end() const
	{
		return m_data.cend();
	}

	const std::string& operator[](int pos) const
	{
		return m_data[pos];
	}

	int size() const
	{
		return m_data.size();
	}

	void clear();
	void erase(int pos);
	void insert(int pos, const std::string& value);
	void push_back(const std::string& value);
	void resize(int count);
	void set(int pos, const std::string& value);

	void load(XfceRc* rc);
	void save(XfceRc* rc);

private:
	const gchar* const m_property;
	std::vector<std::string> m_data;
};


// SearchAction list setting
class SearchActionList
{
public:
	explicit SearchActionList(std::initializer_list<SearchAction*> data);
	~SearchActionList();

	bool empty() const
	{
		return m_data.empty();
	}

	std::vector<SearchAction*>::const_iterator begin() const
	{
		return m_data.cbegin();
	}

	std::vector<SearchAction*>::const_iterator end() const
	{
		return m_data.cend();
	}

	SearchAction* operator[](int pos) const
	{
		return m_data[pos];
	}

	int size() const
	{
		return m_data.size();
	}

	void erase(SearchAction* value);
	void push_back(SearchAction* value);

	void load(XfceRc* rc);
	void save(XfceRc* rc);

private:
	std::vector<SearchAction*> m_data;
};


// Settings class
class Settings
{
	Settings();
	~Settings();

	Settings(const Settings&) = delete;
	Settings(Settings&&) = delete;
	Settings& operator=(const Settings&) = delete;
	Settings& operator=(Settings&&) = delete;

	void load(gchar* file);
	void save(gchar* file);

	std::string m_button_title_default;
	bool m_modified;

public:
	bool get_modified() const
	{
		return m_modified;
	}

	void set_modified()
	{
		m_modified = true;
	}

public:
	StringList favorites;
	StringList recent;

	String custom_menu_file;

	String button_title;
	String button_icon_name;
	Boolean button_title_visible;
	Boolean button_icon_visible;
	Boolean button_single_row;

	Boolean launcher_show_name;
	Boolean launcher_show_description;
	Boolean launcher_show_tooltip;
	IconSize launcher_icon_size;

	Boolean category_hover_activate;
	Boolean category_show_name;
	Boolean sort_categories;
	IconSize category_icon_size;

	enum ViewMode
	{
		ViewAsIcons = 0,
		ViewAsList,
		ViewAsTree
	};
	Integer view_mode;

	enum DefaultCategory
	{
		CategoryFavorites = 0,
		CategoryRecent,
		CategoryAll
	};
	Integer default_category;

	Integer recent_items_max;
	Boolean favorites_in_recent;

	Boolean position_search_alternate;
	Boolean position_commands_alternate;
	Boolean position_categories_alternate;
	Boolean position_categories_horizontal;
	Boolean stay_on_focus_out;

	enum ProfileShape
	{
		ProfileRound = 0,
		ProfileSquare,
		ProfileHidden
	};
	Integer profile_shape;

	enum Commands
	{
		CommandSettings = 0,
		CommandLockScreen,
		CommandSwitchUser,
		CommandLogOutUser,
		CommandRestart,
		CommandShutDown,
		CommandSuspend,
		CommandHibernate,
		CommandLogOut,
		CommandMenuEditor,
		CommandProfile,
		CountCommands
	};
	Command* command[CountCommands];
	Boolean confirm_session_command;

	SearchActionList search_actions;

	Integer menu_width;
	Integer menu_height;
	Integer menu_opacity;

	friend class Plugin;
};

extern Settings* wm_settings;

}

#endif // WHISKERMENU_SETTINGS_H
