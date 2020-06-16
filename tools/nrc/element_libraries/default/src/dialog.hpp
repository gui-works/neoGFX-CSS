// dialog.hpp
/*
neoGFX Resource Compiler
Copyright(C) 2019 Leigh Johnston

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
#include <neogfx/tools/nrc/ui_element.hpp>
#include "window.hpp"

namespace neogfx::nrc
{
    class dialog : public window
    {
    public:
        dialog(const i_ui_element_parser& aParser) :
            window{ aParser, ui_element_type::Dialog }
        {
        }
        dialog(const i_ui_element_parser& aParser, i_ui_element& aParent) :
            window{ aParser, aParent, ui_element_type::Dialog }
        {
        }
    public:
        const neolib::i_string& header() const override
        {
            static const neolib::string sHeader = "neogfx/gui/dialog/dialog.hpp";
            return sHeader;
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            window::parse(aName, aData);
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            window::parse(aName, aData);
        }
    protected:
        void emit() const override
        {
            window::emit();
        }
        void emit_preamble() const override
        {
            if (has_parent())
                emit("  dialog %1%;\n", id());
            else
                emit("  %1%& %2%;\n", type_name(), id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            window::emit_ctor();
        }
        void emit_body() const override
        {
            ui_element<>::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
    };
}
