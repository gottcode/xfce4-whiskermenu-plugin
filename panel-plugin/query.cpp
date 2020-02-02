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

#include "query.h"

#include <sstream>

#include <climits>
#include <cstring>

#include <glib.h>

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static inline bool is_start_word(const std::string& string, std::string::size_type pos)
{
	return (pos == 0) || g_unichar_isspace(g_utf8_get_char(g_utf8_prev_char(&string.at(pos))));
}

//-----------------------------------------------------------------------------

Query::Query(const std::string& query)
{
	set(query);
}

//-----------------------------------------------------------------------------

unsigned int Query::match(const std::string& haystack) const
{
	// Make sure haystack is longer than query
	if (m_query.empty() || (m_query.length() > haystack.length()))
	{
		return UINT_MAX;
	}

	// Check if haystack begins with or is query
	std::string::size_type pos = haystack.find(m_query);
	if (pos == 0)
	{
		return (haystack.length() == m_query.length()) ? 0x4 : 0x8;
	}
	// Check if haystack contains query starting at a word boundary
	else if ((pos != std::string::npos) && is_start_word(haystack, pos))
	{
		return 0x10;
	}

	if (m_query_words.size() > 1)
	{
		// Check if haystack contains query as words
		std::string::size_type search_pos = 0;
		for (const auto& word : m_query_words)
		{
			search_pos = haystack.find(word, search_pos);
			if ((search_pos == std::string::npos) || !is_start_word(haystack, search_pos))
			{
				search_pos = std::string::npos;
				break;
			}
		}
		if (search_pos != std::string::npos)
		{
			return 0x20;
		}

		// Check if haystack contains query as words in any order
		decltype(m_query_words.size()) found_words = 0;
		for (const auto& word : m_query_words)
		{
			search_pos = haystack.find(word);
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
			return 0x40;
		}
	}

	// Check if haystack contains query
	if (pos != std::string::npos)
	{
		return 0x80;
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
	unsigned int result = UINT_MAX;
	if (*query_string == 0)
	{
		result = characters_start_words ? 0x100 : 0x200;
	}

	return result;
}

//-----------------------------------------------------------------------------

void Query::clear()
{
	m_raw_query.clear();
	m_query.clear();
	m_query_words.clear();
}

//-----------------------------------------------------------------------------

void Query::set(const std::string& query)
{
	m_query.clear();
	m_query_words.clear();

	m_raw_query = query;
	if (m_raw_query.empty())
	{
		return;
	}

	gchar* normalized = g_utf8_normalize(m_raw_query.c_str(), -1, G_NORMALIZE_DEFAULT);
	gchar* utf8 = g_utf8_casefold(normalized, -1);
	m_query = utf8;
	g_free(utf8);
	g_free(normalized);

	std::string buffer;
	std::stringstream ss(m_query);
	while (ss >> buffer)
	{
		m_query_words.push_back(buffer);
	}
}

//-----------------------------------------------------------------------------
