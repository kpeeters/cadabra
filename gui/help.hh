/* 

	Cadabra: a field-theory motivated computer algebra system.
	Copyright (C) 2001-2009  Kasper Peeters <kasper.peeters@aei.mpg.de>

   This program is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
*/

#ifndef help_hh__
#define help_hh__

#include <string>
#include <vector>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/uimanager.h>

#include "widgets.hh"

/// The help browser window is separate from the main window.

class CadabraHelp : public Gtk::Window {
	public:
		CadabraHelp();

		enum objtype_t { t_none=0, t_property=1, t_algorithm=2, t_texcommand=3 };
		
		void on_help_context_link(CadabraHelp::objtype_t, std::string);
		void on_help_close();
		virtual bool on_configure_event(GdkEventConfigure *);
		void display_help(objtype_t, const std::string& obj);
		void display_help();
		std::string texify(const std::string& str) const;

		virtual bool on_key_press_event(GdkEventKey *);
		
		std::vector<std::pair<objtype_t, std::string> > history;
		int                                             history_pos;
	private:
		virtual bool on_delete_event(GdkEventAny*);
		void on_back();
		void on_forward();

		Gtk::VBox topbox;
		Gtk::HBox *buttonbox;
		Gtk::Label relatedlabel;
		Gtk::HBox  navbox;
		Gtk::Button back, forward;

		Glib::RefPtr<Gtk::ActionGroup> actiongroup;
		Glib::RefPtr<Gtk::UIManager>   uimanager;

 		Gtk::ScrolledWindow           scroll;
		Gtk::VBox                     scrollbox;
		Glib::RefPtr<Gtk::TextBuffer> textbuf;
		Glib::RefPtr<TeXBuffer>       texbuf;
		TeXView                      *texview;
}; 


#endif
