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


#ifndef WHISKERMENU_FILTER_PAGE_HPP
#define WHISKERMENU_FILTER_PAGE_HPP

#include "page.hpp"

namespace WhiskerMenu
{

class FilterPage : public Page
{
public:
	explicit FilterPage(Menu* menu);
	~FilterPage();

	GtkTreeModel* get_model() const;

protected:
	GtkTreePath* convert_child_path_to_path(GtkTreePath* path) const;
	virtual bool on_filter(GtkTreeModel* model, GtkTreeIter* iter)=0;
	void refilter();
	void set_model(GtkTreeModel* model);
	void unset_model();

private:
	static gboolean filter_visible(GtkTreeModel* model, GtkTreeIter* iter, FilterPage* page)
	{
		return page->on_filter(model, iter);
	}

private:
	GtkTreeModelFilter* m_filter_model;
};

}

#endif // WHISKERMENU_FILTER_PAGE_HPP
