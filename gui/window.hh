/* 

   $Id: window.hh,v 1.82 2008/06/28 09:44:32 peekas Exp $

	Cadabra: an extendable open-source symbolic tensor algebra system.
	Copyright (C) 2001-2006  Kasper Peeters <kasper.peeters@aei.mpg.de>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 2.
	 
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	
*/

#include <gtkmm/main.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/textview.h>
#include <gtkmm/treeview.h>
#include <gtkmm/notebook.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/treestore.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/separator.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/image.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/expander.h>
#include <gtkmm/stock.h>
#include <gtkmm/paned.h>
#include <gtkmm/progressbar.h>
#include <gdkmm.h>
#include <modglue/main.hh>
#include <modglue/pipe.hh>
#include <modglue/ext_process.hh>

#include <stack>

#include "widgets.hh"
#include "help.hh"

class XCadabra;


/// A DataCell contains the information required to display a 
/// cell, but not the actual widget. Therefore, multiple widgets
/// can read from the data stored in this cell; multiple notebook
/// canvasses can therefore display the same notebook data.
///
/// DataCells are stored on the heap and reference counted since there
/// are too many objects with a pointer to them to keep track of 
/// proper de-allocation manually.

class DataCell : public Glib::Object {
	public:
		enum cell_t { c_input, c_output, c_comment, c_tex, c_error };

		DataCell(cell_t, const std::string& str="", bool texhidden=false);

		cell_t                        cell_type;
		Glib::RefPtr<Gtk::TextBuffer> textbuf;
		Glib::RefPtr<TeXBuffer>       texbuf;
		std::string                   cdbbuf;     // c_output only: the output in cadabra input format
		bool                          tex_hidden; // c_tex only
		bool                          sensitive;
		int                           sectioning; // >0 for section header cells
		bool                          running;
};


/// A VisualCell contains pointers to the various cell widgets,
/// which in turn contain pointers to DataCell objects.

class VisualCell {
	public:
		union {
				ExpressionInput *inbox;
				TeXView         *outbox;
				TeXInput        *texbox;
		};
		Glib::RefPtr<DataCell> datacell;
};

/// The Action object is used to pass user action instructions around
/// and store them in the undo/redo stacks. All references to cells is
/// in terms of smart pointers to DataCells. 
///
/// This requires that if we delete a cell, its data cell together
/// with any TextBuffer and TeXBuffer objects should be kept in
/// memory, so that the pointer remains valid.  We keep a RefPtr.

class ActionBase : public Glib::Object {
	public:
		ActionBase(Glib::RefPtr<DataCell>);

		virtual void execute(XCadabra&)=0;
		virtual void revert(XCadabra&)=0;

		Glib::RefPtr<DataCell> cell;
};

/// As a general rule, objects derived from ActionBase are friends of XCadabra, 
/// so they really should be thought of as bits of functionality split off from
/// that large class.

class ActionAddCell : public ActionBase {
	public:
		ActionAddCell(Glib::RefPtr<DataCell>, Glib::RefPtr<DataCell> ref_, bool before_);

		virtual void execute(XCadabra&);
		virtual void revert(XCadabra&);

	private:
		Glib::RefPtr<DataCell> ref; 
		bool                   before;
};

class ActionRemoveCell : public ActionBase {
	public:
		ActionRemoveCell(Glib::RefPtr<DataCell>);
		~ActionRemoveCell();

		virtual void execute(XCadabra&);
		virtual void revert(XCadabra&);

	private:
		Glib::RefPtr<DataCell>               next_cell; 
		std::vector<Glib::RefPtr<DataCell> > associated_cells; // output and comment cells
};

class ActionAddText : public ActionBase {
	public:
		ActionAddText(Glib::RefPtr<DataCell>, int, const std::string&);

		virtual void execute(XCadabra&);
		virtual void revert(XCadabra&);
		
		int         insert_pos;
		std::string text;
};

class ActionRemoveText : public ActionBase {
	public:
		ActionRemoveText(Glib::RefPtr<DataCell>, int, int, const std::string&);

		virtual void execute(XCadabra&);
		virtual void revert(XCadabra&);

		int from_pos, to_pos;
		std::string removed_text;
};

typedef std::stack<Glib::RefPtr<ActionBase> > ActionStack;


/// NotebookCanvas is a view on notebook data. Any number of these
/// may be instantiated, and they all reflect the current status
/// of the document stored in the master XCadabra class.

class NotebookCanvas : public Gtk::VPaned {
	public:
		NotebookCanvas(XCadabra& doc);
		~NotebookCanvas();

//		bool handle_expression_input(std::string str);
//		bool handle_tex_update(const std::string&);
//		bool receive_output(std::string eqno, std::string eq);
//		bool receive_comment(std::string comment);

		bool handle_key_press_event(GdkEventKey*);

		/// Add a VisualCell corresponding to the given DataCell.
		/// The second and third element determine the position relative
		/// to another DataCell (or, by default, relative to the end marker).
		VisualCell* add_cell(Glib::RefPtr<DataCell>, Glib::RefPtr<DataCell> ref, bool before=true);

		/// Remove a VisualCell corresponding to the given DataCell.
		void         remove_cell(Glib::RefPtr<DataCell>);
		/// Make a cell grab focus. This will trigger a run of this cell, with various
		/// other side-effects before it returns.
		void         cell_grab_focus(VisualCell *);
		void         cell_grab_focus(Glib::RefPtr<DataCell>);
		void         select_first_input_cell();
		virtual void show();

//		void         scroll_to(Gtk::Allocation al);
		void         redraw_cells();
		bool         scroll_into_view_callback(VisualCell *);
		bool         scroll_into_view(VisualCell *, bool center=false);
		bool         scroll_into_view(Glib::RefPtr<DataCell>, bool center=false);
		void         scroll_to_start();
		void         scroll_to_end();
		void         scroll_up();
		void         scroll_down();

		XCadabra&                 doc;
		typedef std::list<VisualCell *> VisualCells_t;
		VisualCells_t             visualcells; // managed here
		Gtk::EventBox             ebox;
 		Gtk::ScrolledWindow       scroll;
		Gtk::VBox                 scrollbox;
		Gtk::HSeparator           bottomline;
};


/// Each notebook has one cadabra process associated to it, and one
/// main window to control it. 

class XCadabra : public Gtk::Window {
	public:
		XCadabra(modglue::ext_process&, const std::string& filename, modglue::main *);
		virtual ~XCadabra();

		/// Data coming from cadabra arrives here. This handles
		/// adding boxes to the document and propagating this to the
		/// notebooks.
		bool receive(modglue::ipipe& p);
		bool receive_err(modglue::ipipe& p);
		std::string accumulated_error;

		/// Data from the NotebookCanvas objects arrives here.
		bool handle_editbox_output(std::string str, NotebookCanvas *, VisualCell *);
		void on_my_insert(const Gtk::TextIter& pos, const Glib::ustring& text, int bytes, VisualCell *vis);
		void on_my_erase(const Gtk::TextIter& start, const Gtk::TextIter& end, VisualCell *vis);

		/// Events from the notebook cells arrive here.
		void handle_on_grab_focus(NotebookCanvas *, VisualCell *);
		bool handle_visibility_toggle(GdkEventButton *, NotebookCanvas *, VisualCell *);
		bool handle_outbox_select(GdkEventButton *, NotebookCanvas *, VisualCell *);
		bool handle_tex_update_request(std::string, NotebookCanvas *, VisualCell *);

		/// Toplevel keyboard handling.
		virtual bool on_key_press_event(GdkEventKey *);

		/// All the menu routines.
		void on_file_new();
		void on_file_open();
		void on_file_save();
		void on_file_save_as();
		void on_file_print();
		void on_file_export_text();
		void on_file_quit();
		bool quit_safeguard(bool quit=true);
		void on_edit_copy();
		void on_edit_paste();
		void on_edit_insert_tex_above();
		void on_edit_insert_tex_below();
		void on_edit_insert_input_above();
		void on_edit_insert_input_below();
		void on_edit_insert_section_above();
		void on_edit_remove_cell();
		void on_edit_divide_cell();
		void on_view_split();
		void on_view_close();
		void on_settings_font_size(int);
		void on_tutorial_open(unsigned int);
		void on_help_about();
		void on_help_citing();
		void on_help_properties(const std::string&);
		void on_help_algorithms(const std::string&);
		void on_help_reserved(const std::string&);
		bool on_autocomplete();
		void on_help_context();
		void on_stop();
		void on_kill();
		void on_run();
		void on_run_to();
		void on_run_from();

		bool current_objtype_and_name(CadabraHelp::objtype_t&, std::string&);
		void insert_at_mark(const std::string instxt);

		/// Saving and loading to/from disk. If the return string is
		/// non-empty, it contains an error message.
		void clear();
		std::string save(const std::string&) const;
		std::string load(const std::string&, bool ignore_nonexistence=false); 
		std::string expo(const std::string&) const; 

		/// Handling starting/restarting of the kernel.
		bool on_kernel_exit(modglue::ext_process&);

		/// Clipboard handling
		void on_clipboard_get(Gtk::SelectionData&, guint info);
		void on_clipboard_clear();
		std::string clipboard_txt, clipboard_cdb;

		/// Adding and removing cells from the current document.
		/// These routines also update all NotebookCanvas objects
		/// so that they reflect the current structure of the document.
		/// The DataCell ownership is handled by the XCadabra class once
		/// it has been added here.
		Glib::RefPtr<DataCell> add_cell(Glib::RefPtr<DataCell>, Glib::RefPtr<DataCell> ref, bool before=true);
		void         add_canvas();

		/// Signals from Gtk, such as closing windows or changing the text
		/// of an input cell.
		virtual bool on_delete_event(GdkEventAny*);
		void         input_cell_modified();
		void         tex_cell_modified();

		void         connect_io_signals();
		void         disconnect_io_signals();	

		int          font_step;
	private:
		/// Variables for the undo/redo mechanism.
		ActionStack      undo_stack, redo_stack;
		bool             disable_stacks; // for changes which should not be recorded on the stack
		bool             action_add(Glib::RefPtr<ActionBase>); // takes ownership and fills the stacks 
		bool             action_undo();
		bool             action_redo();

		/// Various assorted other variables related to communicating with the kernel.
		Gdk::Cursor      hglass;
		bool             load_file; // used by main to indicate a load should occur after start
		bool             have_received;
		modglue::main   *cmm;
		std::map<int, sigc::connection> connections;
		std::string      name;
		bool             modified;
		bool             running;
		Glib::RefPtr<DataCell> running_last;
		bool             restarting_kernel;
		bool             callmm(Glib::IOCondition, int fd);
		void             update_title();
		bool             make_backup(const std::string&) const;
		void             remove_noninput_below(Glib::RefPtr<DataCell>);
		void             kernel_idle();
		/// A map to keep track of identifiers of input cells. These are used to associated input
		/// data to the kernel to output data from the kernel, and hence can be used to associate
		/// the output to the right input cell.
		std::map<long, Glib::RefPtr<DataCell> > id_to_datacell;
		long             last_used_id;

		/// Boxes and widgets.
		std::vector<NotebookCanvas *>  canvasses;   // managed by gtk
		CadabraHelp                    help_window;
		NotebookCanvas                *active_canvas;
		VisualCell                    *active_cell;
		Gtk::VBox                      topbox;
		Gtk::HBox                      supermainbox;
		Gtk::VBox                      mainbox;
		Gtk::HBox                      buttonbox;
		Gtk::HBox                      statusbarbox;
		Gtk::VBox                      progressbarvbox;
		Gtk::ProgressBar               progressbar1,progressbar2;
		Glib::RefPtr<Gtk::ActionGroup> actiongroup;
		Glib::RefPtr<Gtk::UIManager>   uimanager;
		Glib::RefPtr<Gtk::RadioAction> font_action0, font_action1, font_action2, font_action3;
		Gtk::HBox                      statusbox;
		Gtk::Label                     b_cdbstatus, b_kernelversion;
		Gtk::Button                    b_kill, b_run, b_run_to, b_run_from, b_help, b_stop;

		/// Storage of document data. This data is not managed by smart
		/// pointers and should thus be deleted by the XCadabra
		/// destructor.
		typedef std::list<Glib::RefPtr<DataCell> > DataCells_t;
		DataCells_t                   datacells;

		/// Data concerning the interaction with the externally started
		/// cadabra process.
		modglue::ext_process&     cdb;
		enum parse_mode_t         { m_status, m_eqno, m_eq, m_property, m_algorithm, m_reserved,
											 m_discard, m_comment, m_error,
		                            m_progress, m_plain };
		std::vector<parse_mode_t> parse_mode;
		std::string               eqno, eq, status, progress, plain, algorithm, property, reserved;
		int                       progress_todo, progress_done, progress_count;

		/// Collection of all known algorithm and property names, as extracted from the kernel.
		void add_property_help(const std::string&);
		void add_algorithm_help(const std::string&);
		void add_reserved_help(const std::string&);
		std::set<std::string> property_set, algorithm_set;
		static const char * const autocomplete_strings[];
		std::string duplicate_underscores(const std::string& str) const;

		/// Cut-n-paste data
		VisualCell  *selected;

		/// Configuration data saving/loading
		std::string save_config() const;
		std::string load_config();

		friend class ActionRemoveCell;
		friend class ActionAddCell;
};
