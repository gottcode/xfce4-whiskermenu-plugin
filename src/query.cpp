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


#include "query.hpp"

#include <sstream>

#include <climits>
#include <cstring>

extern "C"
{
#include <glib.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static inline bool is_start_word(const std::string& string, std::string::size_type pos)
{
	return (pos == 0) || g_unichar_isspace(g_utf8_get_char(g_utf8_prev_char(&string.at(pos))));
}

//-----------------------------------------------------------------------------

Query::Query()
{
}

//-----------------------------------------------------------------------------

Query::Query(const std::string& query)
{
	set(query);
}

//-----------------------------------------------------------------------------

Query::~Query()
{
	clear();
}

//-----------------------------------------------------------------------------

int Query::match(const std::string& haystack) const
{
	// Make sure haystack is longer than query
	if (m_query.empty() || (m_query.length() > haystack.length()))
	{
		return INT_MAX;
	}

	// Check if haystack begins with or is query
	std::string::size_type pos = haystack.find(m_query);
	if (pos == 0)
	{
		return haystack.length() != m_query.length();
	}
	// Check if haystack contains query starting at a word boundary
	else if ((pos != std::string::npos) && is_start_word(haystack, pos))
	{
		return 2;
	}

	if (m_query_words.size() > 1)
	{
		// Check if haystack contains query as words
		std::string::size_type search_pos = 0;
		for (std::vector<std::string>::const_iterator i = m_query_words.begin(), end = m_query_words.end(); i != end; ++i)
		{
			search_pos = haystack.find(*i, search_pos);
			if ((search_pos == std::string::npos) || !is_start_word(haystack, search_pos))
			{
				search_pos = std::string::npos;
				break;
			}
		}
		if (search_pos != std::string::npos)
		{
			return 3;
		}

		// Check if haystack contains query as words in any order
		std::vector<std::string>::size_type found_words = 0;
		for (std::vector<std::string>::const_iterator i = m_query_words.begin(), end = m_query_words.end(); i != end; ++i)
		{
			search_pos = haystack.find(*i);
			if ((search_pos != std::string::npos) && is_start_word(haystack, search_pos))
			{
				++found_words;
			}
			else
			{
				break;
			}
		}
		if (found_words == m_query_words.size())
		{
			return 4;
		}
	}

	// Check if haystack contains query as characters
	bool characters_start_words = true;
	bool start_word = true;
	bool started = false;
	const gchar* query_string = m_query.c_str();
	for (const gchar* pos = haystack.c_str(); *pos; pos = g_utf8_next_char(pos))
	{
		gunichar c = g_utf8_get_char(pos);
		if (c == g_utf8_get_char(query_string))
		{
			if (start_word || started)
			{
				characters_start_words &= start_word;
				query_string = g_utf8_next_char(query_string);
				started = true;
			}
			start_word = false;
		}
		else if (g_unichar_isspace(c))
		{
			start_word = true;
		}
		else
		{
			start_word = false;
		}
	}
	int result = INT_MAX;
	if (*query_string == 0)
	{
		result = characters_start_words ? 5 : 7;
	}

	// Check if haystack contains query
	if ((result > 5) && (pos != std::string::npos))
	{
		result = 6;
	}

	return result;
}

//-----------------------------------------------------------------------------

void Query::clear()
{
	m_query.clear();
	m_query_words.clear();
}

//-----------------------------------------------------------------------------

void Query::set(const std::string& query)
{
	m_query_words.clear();

	m_query = query;
	if (m_query.empty())
	{
		return;
	}

	std::string buffer;
	std::stringstream ss(query);
	while (ss >> buffer)
	{
		m_query_words.push_back(buffer);
	}
}

//-----------------------------------------------------------------------------
