// grid_layout.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <neogfx/neogfx.hpp>
#include <map>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/lifetime.hpp>
#include "layout.hpp"
#include "vertical_layout.hpp"
#include "horizontal_layout.hpp"

namespace neogfx
{
	class grid_layout : private neolib::lifetime, public layout
	{
	public:
		struct cell_unoccupied : std::logic_error { cell_unoccupied() : std::logic_error("neogfx::grid_layout::cell_unoccupied") {} };
	public:
		typedef uint32_t cell_coordinate;
		typedef basic_point<cell_coordinate> cell_coordinates;
		typedef basic_size<cell_coordinate> cell_dimensions;
	private:
		struct row_major;
		struct column_major;
		typedef std::map<cell_coordinates, item_list::iterator, std::less<cell_coordinates>, boost::pool_allocator<std::pair<cell_coordinates, item_list::iterator>>> cell_list;
		typedef std::vector<std::pair<cell_coordinates, cell_coordinates>> span_list;
	public:
		grid_layout(neogfx::alignment aAlignment = neogfx::alignment::Centre | neogfx::alignment::VCentre);
		grid_layout(cell_coordinate aRows, cell_coordinate aColumns, neogfx::alignment aAlignment = neogfx::alignment::Centre | neogfx::alignment::VCentre);
		grid_layout(i_widget& aParent, neogfx::alignment aAlignment = neogfx::alignment::Centre | neogfx::alignment::VCentre);
		grid_layout(i_layout& aParent, neogfx::alignment aAlignment = neogfx::alignment::Centre | neogfx::alignment::VCentre);
		grid_layout(i_widget& aParent, cell_coordinate aRows, cell_coordinate aColumns, neogfx::alignment aAlignment = neogfx::alignment::Centre | neogfx::alignment::VCentre);
		grid_layout(i_layout& aParent, cell_coordinate aRows, cell_coordinate aColumns, neogfx::alignment aAlignment = neogfx::alignment::Centre | neogfx::alignment::VCentre);
		~grid_layout();
	public:
		cell_coordinate rows() const;
		cell_coordinate columns() const;
		cell_coordinates dimensions() const;
		void set_dimensions(cell_coordinate aRows, cell_coordinate aColumns);
		bool is_item_at_position(cell_coordinate aRow, cell_coordinate aColumn) const;
		i_layout_item& add(i_layout_item& aItem) override;
		i_layout_item& add(std::shared_ptr<i_layout_item> aItem) override;
		virtual i_layout_item& add_item_at_position(cell_coordinate aRow, cell_coordinate aColumn, i_layout_item& aItem);
		virtual i_layout_item& add_item_at_position(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_layout_item> aItem);
		using layout::add_at;
		i_spacer& add_spacer() override;
		i_spacer& add_spacer_at(item_index aPosition) override;
		virtual i_spacer& add_spacer_at_position(cell_coordinate aRow, cell_coordinate aColumn);
		void remove_at(item_index aIndex) override;
		bool remove(i_layout_item& aItem) override;
		virtual void remove_item_at_position(cell_coordinate aRow, cell_coordinate aColumn);
		void remove_all() override;
		i_layout_item& item_at_position(cell_coordinate aRow, cell_coordinate aColumn);
		i_widget& widget_at_position(cell_coordinate aRow, cell_coordinate aColumn);
		i_layout& layout_at_position(cell_coordinate aRow, cell_coordinate aColumn);
	public:
		void invalidate() override;
	public:
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
	public:
		void set_spacing(const optional_size& aSpacing, bool aUpdateLayout = true) override;
		void add_span(cell_coordinate aRowFrom, cell_coordinate aColumnFrom, uint32_t aRows, uint32_t aColumns);
		void add_span(const cell_coordinates& aFrom, const cell_coordinates& aTo);
		void set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout = true) override;
	public:
		void layout_items(const point& aPosition, const size& aSize) override;
	private:
		uint32_t visible_rows() const;
		bool is_row_visible(uint32_t aRow) const;
		uint32_t visible_columns() const;
		bool is_column_visible(uint32_t aColumn) const;
		size::dimension_type row_minimum_size(cell_coordinate aRow, const optional_size& aAvailableSpace = optional_size()) const;
		size::dimension_type column_minimum_size(cell_coordinate aColumn, const optional_size& aAvailableSpace = optional_size()) const;
		size::dimension_type row_maximum_size(cell_coordinate aRow, const optional_size& aAvailableSpace = optional_size()) const;
		size::dimension_type column_maximum_size(cell_coordinate aColumn, const optional_size& aAvailableSpace = optional_size()) const;
		void increment_cursor();
		horizontal_layout& row_layout(cell_coordinate aRow);
		span_list::const_iterator find_span(const cell_coordinates& aCell) const;
		void init();
		// helpers
	public:
		template <typename WidgetT>
		WidgetT& widget_at_position(cell_coordinate aRow, cell_coordinate aColumn)
		{
			return static_cast<WidgetT&>(widget_at_position(aRow, aColumn));
		}
	private:
		vertical_layout iRowLayout;
		std::vector<std::shared_ptr<horizontal_layout>> iRows;
		cell_list iCells;
		cell_dimensions iDimensions;
		cell_coordinates iCursor;
		span_list iSpans;
	};
}