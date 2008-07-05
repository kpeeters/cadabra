/* 

   $Id: window.cc,v 1.126 2008/06/28 09:44:32 peekas Exp $

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

#include <modglue/pipe.hh>
#include <modglue/process.hh>
#include <gdk/gdkkeysyms.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/aboutdialog.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>
#include <signal.h>
#include <stdexcept>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../src/config.h"

#include "window.hh"


#define THEFONT "cmtt12"


const char * const XCadabra::autocomplete_strings[] = { 
	 "\\alpha",
	 "\\beta",
	 "\\gamma",
	 "\\delta",
	 "\\epsilon",
	 "\\zeta",
	 "\\eta",
	 "\\theta",
	 "\\iota",
	 "\\kappa",
	 "\\lambda",
	 "\\mu",
	 "\\nu",
	 "\\xi",
	 "\\omicron",
	 "\\pi",
	 "\\rho",
	 "\\sigma",
	 "\\tau",
	 "\\upsilon",
	 "\\phi",
	 "\\chi",
	 "\\psi",
	 "\\omega",
	 "\\Alpha",
	 "\\Beta",
	 "\\Gamma",
	 "\\Delta",
	 "\\Epsilon",
	 "\\Zeta",
	 "\\Eta",
	 "\\Theta",
	 "\\Iota",
	 "\\Kappa",
	 "\\Lambda",
	 "\\Mu",
	 "\\Nu",
	 "\\Xi",
	 "\\Omicron",
	 "\\Pi",
	 "\\Rho",
	 "\\Sigma",
	 "\\Tau",
	 "\\Upsilon",
	 "\\Phi",
	 "\\Chi",
	 "\\Psi",
	 "\\Omega",
	 "\\partial",
	 "\\infty",
	 "\\dot",
	 "\\commutator",
	 "\\anticommutator"
};


//PropertyList::PropertyList()
//	{
//	set_size_request(200,900);
//	pack_start(scroll, true, true);
// 	scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
// 	scroll.set_border_width(1);
//	tv.set_editable(false);
// 	tv.set_wrap_mode(Gtk::WRAP_WORD);
//	textbuf=tv.get_buffer();
//	scroll.add(tv);
//	show_all();
//	}

DataCell::DataCell(cell_t ct, const std::string& str, bool texhidden)
	: cell_type(ct), tex_hidden(texhidden), sensitive(true), sectioning(0), running(false)
	{
	textbuf=Gtk::TextBuffer::create();
	textbuf->set_text(trim(str));
	switch(cell_type) {
		case c_error:
		case c_output:
		case c_comment:
		case c_tex:
			texbuf=TeXBuffer::create(textbuf);
			break;
		case c_input:
			break;
		}
	}

NotebookCanvas::NotebookCanvas(XCadabra& doc_)
	: doc(doc_)
	{
	pack1(scroll, true, true);
 	scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
 	scroll.set_border_width(1);
	scroll.add(ebox);
	ebox.add(scrollbox);
	ebox.modify_bg(Gtk::STATE_NORMAL, Gdk::Color("white"));
//	scrollbox.pack_start(bottomline, Gtk::PACK_SHRINK);
	}

NotebookCanvas::~NotebookCanvas()
	{
	VisualCells_t::iterator it=visualcells.begin();
	while(it!=visualcells.end()) {
		delete (*it);
		++it;
		}
	}

void NotebookCanvas::redraw_cells()
	{
	VisualCells_t::iterator it=visualcells.begin();
	std::ostringstream fstr;
	fstr << THEFONT << " " << 12+(doc.font_step*2); 
	while(it!=visualcells.end()) {
		switch((*it)->datacell->cell_type) {
			case DataCell::c_input:
				(*it)->inbox->edit.modify_font(Pango::FontDescription(fstr.str()));
				break;
			case DataCell::c_comment:
			case DataCell::c_error:
			case DataCell::c_output:
				(*it)->outbox->image.set((*it)->datacell->texbuf->pixbuf);
				break;
			case DataCell::c_tex:
				(*it)->texbox->edit.modify_font(Pango::FontDescription(fstr.str()));
				(*it)->texbox->texview.image.set((*it)->datacell->texbuf->pixbuf);
				break;
			}
		++it;
		}
	}

VisualCell *NotebookCanvas::add_cell(DataCell *dc, DataCell *ref, bool before)
	{
	VisualCell *newcell=new VisualCell;
	newcell->datacell=dc;

	// Temporarily remove bottom line marker.
//	scrollbox.remove(bottomline);

	// Find the correct place to insert the new cell in visualcells, and keep
	// track of the number so we can insert the widget at the right spot in the Gtk
	// container later.
	int cellnum=0;
	if(ref==0)
		visualcells.push_back(newcell);
	else {
		VisualCells_t::iterator it=visualcells.begin();
		while(it!=visualcells.end()) {
			if((*it)->datacell==ref) {
				if(!before) { // skip to the next input cell; NOTE: this should skip in the same way
          					  // as the XCadabra::add_cell method!
					do {
						++it;
						++cellnum;
						} while(it!=visualcells.end() && 
								  ( /* (*it)->datacell->cell_type==DataCell::c_comment || */
									 (*it)->datacell->cell_type==DataCell::c_output ||
									 (*it)->datacell->cell_type==DataCell::c_error) );
					}
				visualcells.insert(it, newcell);
				break;
				}
			++it;
			++cellnum;
			}
		}
	
#ifdef DEBUG
	std::cerr << "gtk logic" << std::endl;
#endif
	Gtk::VBox::BoxList bl=scrollbox.children();
	Gtk::VBox::BoxList::iterator gtkit;
	if(ref==0) gtkit=bl.end();
	else {
		gtkit=bl.begin();
		while(cellnum!=0) {
			++gtkit;
			--cellnum;
			}
		}

	// Insert the widget in the Gtk container and connect signals.
#ifdef DEBUG
	std::cerr << "inserting" << std::endl;
#endif
	switch(dc->cell_type) {
		case DataCell::c_input: {
#ifdef DEBUG
			std::cerr << "incell" << std::endl;
#endif
			std::ostringstream fstr;
			fstr << THEFONT << " " << 12+(doc.font_step*2); 
			newcell->inbox=manage( new ExpressionInput(dc->textbuf, fstr.str()) );
			Gtk::VBox::BoxList::iterator newit=bl.insert(gtkit, *newcell->inbox);

			gtk_box_set_child_packing(((Gtk::Box *)(&scrollbox))->gobj(), 
											  ((Gtk::Widget *)(newcell->inbox))->gobj(),
											  false, false, 0, GTK_PACK_START);

//			(*newit).set_options(Gtk::PACK_SHRINK);

			newcell->inbox->edit.emitter.connect(
				sigc::bind<NotebookCanvas *, VisualCell *>(
					sigc::mem_fun(doc, &XCadabra::handle_editbox_output), this, newcell));
			newcell->inbox->edit.signal_grab_focus().connect(
				sigc::bind<NotebookCanvas *, VisualCell *>(
					sigc::mem_fun(doc, &XCadabra::handle_on_grab_focus),
					this, newcell));
			newcell->inbox->edit.content_changed.connect(
				 sigc::bind<VisualCell *, bool>(
					 sigc::mem_fun(this, &NotebookCanvas::scroll_into_view), newcell, false));
	
			newcell->inbox->show_all();
			break;
			}
		case DataCell::c_error:
		case DataCell::c_output:
		case DataCell::c_comment: {
			newcell->outbox=manage( new TeXView(dc->texbuf) );
			Gtk::VBox::BoxList::iterator newit=bl.insert(gtkit, *newcell->outbox);
// REPORT BUG: this sometimes segfaults
//			(*newit).set_options(Gtk::PACK_SHRINK);
			gtk_box_set_child_packing(((Gtk::Box *)(&scrollbox))->gobj(), 
											  ((Gtk::Widget *)(newcell->outbox))->gobj(),
											  false, false, 0, GTK_PACK_START);
			newcell->outbox->show_all();
			
			newcell->outbox->signal_button_release_event().connect( 
				sigc::bind<NotebookCanvas *, VisualCell *>(
					sigc::mem_fun(doc, &XCadabra::handle_outbox_select), this, newcell));
			break;
			}
		case DataCell::c_tex: {
			std::ostringstream fstr;
			fstr << THEFONT << " " << 12+(doc.font_step*2); 
			newcell->texbox=manage( new TeXInput(dc->textbuf, dc->texbuf, fstr.str()) );
			newcell->texbox->texview.signal_button_release_event().connect( 
				sigc::bind<NotebookCanvas *, VisualCell *>(
					sigc::mem_fun(doc, &XCadabra::handle_visibility_toggle), this, newcell));
			newcell->texbox->edit.emitter.connect(
				sigc::bind(
					sigc::mem_fun(doc, &XCadabra::handle_tex_update_request), this, newcell));
			Gtk::VBox::BoxList::iterator newit=bl.insert(gtkit, *newcell->texbox);

			gtk_box_set_child_packing(((Gtk::Box *)(&scrollbox))->gobj(), 
											  ((Gtk::Widget *)(newcell->texbox))->gobj(),
											  false, false, 0, GTK_PACK_START);

//			(*newit).set_options(Gtk::PACK_SHRINK);
			newcell->texbox->edit.signal_grab_focus().connect(
				sigc::bind<NotebookCanvas *, VisualCell *>(
					sigc::mem_fun(doc, &XCadabra::handle_on_grab_focus),
					this, newcell));
			// Hide source depending on setting in the datacell.

			newcell->texbox->texview.show_all();
			if(newcell->datacell->tex_hidden) newcell->texbox->edit.hide_all();
			else                              newcell->texbox->edit.show_all();
			break;
			}
		}

	// Restore bottom line marker 
//	scrollbox.pack_start(bottomline, Gtk::PACK_SHRINK);
//	bottomline.show();

	return newcell; //visualcells.back();
	}

void NotebookCanvas::remove_cell(DataCell *dc)
	{
	int cellnum=0;

	VisualCells_t::iterator it=visualcells.begin();
	while(it!=visualcells.end()) {
		if((*it)->datacell==dc) 
			break;
		++it;
		++cellnum;
		}
	assert(it!=visualcells.end());
	
	switch((*it)->datacell->cell_type) {
		case DataCell::c_input:
			scrollbox.remove(*((*it)->inbox));
			break;
		case DataCell::c_error:
		case DataCell::c_comment:
		case DataCell::c_output:
			scrollbox.remove(*((*it)->outbox));
			break;
		case DataCell::c_tex:
			scrollbox.remove(*((*it)->texbox));
			break;
		}
	visualcells.erase(it);
	}

void NotebookCanvas::cell_grab_focus(DataCell *dc)
	{
	VisualCells_t::iterator it=visualcells.begin();
	while(it!=visualcells.end()) {
		if((*it)->datacell==dc) {
#ifdef DEBUG
			std::cerr << "grabbing " << dc << " " << *it << std::endl;
#endif
			cell_grab_focus(*it);
			break;
			}
		++it;
		}
	}

// void NotebookCanvas::adjust_scroll(Gtk::Allocation& al)
// 	{
// 	Gtk::Adjustment *va=scroll.get_vadjustment();
// #ifdef DEBUG
// 	std::cerr << va->get_value() << " vs " << al.get_y() << " + " << al.get_height() << std::endl;
// 	std::cerr << va->get_lower() << " - " << va->get_upper() << " - " << va->get_page_size() << std::endl;
// #endif
// 	if(al.get_y()+al.get_height() < va->get_value() || 
// 		al.get_y()+al.get_height() > va->get_value() + va->get_page_size()) {
// #ifdef DEBUG
// 		std::cerr << "adjusting scrollbar to";
// //		va->set_value(std::min((double)(al.get_y()), va->get_upper()-va->get_page_size()));		
// 		std::cerr << al.get_y()-va->get_page_size()+al.get_height() << std::endl;
// #endif
// 		va->set_value(std::max(0.0,
// 									  std::min((double)(al.get_y()-va->get_page_size()+al.get_height()), 
// 												  va->get_upper()-va->get_page_size())));		
// 		}
// 	adjust_scroll_connection.disconnect();
// 	}

void NotebookCanvas::cell_grab_focus(VisualCell *vis)
	{
	VisualCells_t::iterator it=std::find(visualcells.begin(), visualcells.end(), vis);
	if(it!=visualcells.end()) {
		switch(vis->datacell->cell_type) {
			case DataCell::c_input: {
				vis->inbox->edit.grab_focus();

				// make sure the display is updated
				while (gtk_events_pending ())
					gtk_main_iteration ();
				
				// and scroll to the right location
				//scroll_to(vis->inbox->get_allocation());
				scroll_into_view(vis);
				break;
				}
			case DataCell::c_error:
			case DataCell::c_output:
			case DataCell::c_comment:
				break;
			case DataCell::c_tex:
				break;
			}
		}
	}

//void NotebookCanvas::scroll_to(Gtk::Allocation al)
//	{
//	Gtk::Adjustment *va=scroll.get_vadjustment();
//	if(al.get_y()+al.get_height() < va->get_value() || 
//		al.get_y()+al.get_height() > va->get_value() + va->get_page_size()) {
//		va->set_value(std::max(0.0,
//									  std::min((double)(al.get_y()-va->get_page_size()+al.get_height()+40), 
//												  va->get_upper()-va->get_page_size())));		
//		}
//	}

bool NotebookCanvas::scroll_into_view(VisualCell *vc, bool center)
	{
	Gdk::Rectangle rect;
	vc->inbox->edit.get_iter_location(vc->inbox->edit.get_buffer()->get_iter_at_mark(
													 vc->inbox->edit.get_buffer()->get_insert()), rect);

	Gtk::Allocation  al=vc->inbox->get_allocation();
	Gtk::Adjustment *va=scroll.get_vadjustment();

	double upper_visible=va->get_value();
	double lower_visible=va->get_value()+va->get_page_size();

	if(center) {
		 double aim=std::max(0.0, (double)(al.get_y() + rect.get_y() 
													  - 0.5*(va->get_page_size() - rect.get_height())));
		 aim=std::min(aim, va->get_upper()-va->get_page_size());
		 va->set_value(aim);
		 }
	else {
		 if(al.get_y() + rect.get_y() < upper_visible) 
			  va->set_value(std::max(0.0, (double)(al.get_y() + rect.get_y() - LINE_SPACING)));
		 else if(al.get_y() + rect.get_y() + rect.get_height() > lower_visible)
			  va->set_value(al.get_y() + rect.get_y() + rect.get_height() - va->get_page_size());
		 }

	return false;
	}

void NotebookCanvas::scroll_to_start()
	{
	Gtk::Adjustment *va=scroll.get_vadjustment();
	va->set_value(va->get_lower());
	}

void NotebookCanvas::scroll_to_end()
	{
	Gtk::Adjustment *va=scroll.get_vadjustment();
	va->set_value(va->get_upper()-va->get_page_size());
	}

void NotebookCanvas::scroll_up()
	{
	Gtk::Adjustment *va=scroll.get_vadjustment();
	va->set_value(std::max(va->get_lower(), va->get_value()-va->get_page_size()));
	}

void NotebookCanvas::scroll_down()
	{
	Gtk::Adjustment *va=scroll.get_vadjustment();
	va->set_value(std::min(va->get_upper()-va->get_page_size(), va->get_value()+va->get_page_size()));
	}


void NotebookCanvas::select_first_input_cell()
	{
#ifdef DEBUG
	std::cerr << "select first input" << std::endl;
#endif
	VisualCells_t::iterator it=visualcells.begin();
	while(it!=visualcells.end()) {
#ifdef DEBUG
		std::cerr << "inspect" << std::endl;
#endif
		if((*it)->datacell->cell_type==DataCell::c_input)
			break;
		++it;
		}
	if(it!=visualcells.end()) {
#ifdef DEBUG
		std::cerr << "notebook going to grab focus" << std::endl;
#endif
		cell_grab_focus(*it);
		}
	}

void NotebookCanvas::show() 
	{
	ebox.show();
	scroll.show();
	scrollbox.show();
//	bottomline.show();
	VPaned::show();
	}


XCadabra::XCadabra(modglue::ext_process& cdbproc, const std::string& filename, modglue::main *mm)
	: font_step(0), hglass(Gdk::WATCH),
	  load_file(false), have_received(false), cmm(mm), name(filename), modified(false), running(false),
	  running_last(0), restarting_kernel(false),
	  b_kernelversion("Kernel version: not running"),
	  b_help(Gtk::Stock::HELP), b_stop(Gtk::Stock::STOP),
     cdb(cdbproc), selected(0)
	{
	std::string res=load_config();
	if(res.size()>0) 
		 std::cerr << res << std::endl;

	b_stop.set_sensitive(false);
	b_run.set_label("Run all");
	b_run_to.set_label("Run to cursor");
	b_run_from.set_label("Run from cursor");
	b_kill.set_label("Restart kernel");
	parse_mode.push_back(m_discard);

#if (GTKMM_VER == 212)
	b_help.set_tooltip_text("Show context-sensitive help. Your cursor needs to be over an algorithm (anything starting with '@') or a property (anything starting with '::'). For other types of help, see the help menu.");
	b_stop.set_tooltip_text("Interrupt the kernel when it is running.");
	b_run.set_tooltip_text("Evaluate all input cells of the notebook in turn.");
	b_run_to.set_tooltip_text("Evaluate all input cells from the start of the notebook until and including the cell before the one in which the cursor is currently located.");
	b_run_from.set_tooltip_text("Evaluate all input cells starting from the one in which the cursor is currently located, until the end of the notebook.");
	b_kill.set_tooltip_text("Restart the cadabra kernel. This brings you back to the state in which none of the cells in the notebook have been evaluated.");
#endif

	b_cdbstatus.set_alignment( 0.0, 0.5 );
	b_kernelversion.set_alignment( 0.0, 0.5 );
	b_cdbstatus.set_size_request(200,-1);

	if(filename.size()>0) load_file=true;

	update_title();
	set_default_size(std::min(Gdk::Screen::get_default()->get_width()-20,  800),
						  std::min(Gdk::Screen::get_default()->get_height()-20, 900));
	add(topbox);
	
	actiongroup=Gtk::ActionGroup::create();
	actiongroup->add( Gtk::Action::create("MenuFile", "_File") );
	actiongroup->add( Gtk::Action::create("MenuEdit", "_Edit") );
	actiongroup->add( Gtk::Action::create("MenuView", "_View") );
	actiongroup->add( Gtk::Action::create("MenuSettings", "_Settings") );
	actiongroup->add( Gtk::Action::create("MenuTutorial", "_Tutorial") );
	actiongroup->add( Gtk::Action::create("MenuFontSize", "Font size") );
	actiongroup->add( Gtk::Action::create("MenuHelp", "_Help") );

	actiongroup->add( Gtk::Action::create("New", Gtk::Stock::NEW),
								  sigc::mem_fun(*this, &XCadabra::on_file_new) );
	actiongroup->add( Gtk::Action::create("Open", "_Open"), Gtk::AccelKey("<control>O"),
								  sigc::mem_fun(*this, &XCadabra::on_file_open) );
	actiongroup->add( Gtk::Action::create("Save", Gtk::Stock::SAVE), Gtk::AccelKey("<control>S"),
								  sigc::mem_fun(*this, &XCadabra::on_file_save) );
	actiongroup->add( Gtk::Action::create("SaveAs", Gtk::Stock::SAVE_AS),
								  sigc::mem_fun(*this, &XCadabra::on_file_save_as) );
	actiongroup->add( Gtk::Action::create("Print", Gtk::Stock::PRINT),
								  sigc::mem_fun(*this, &XCadabra::on_file_print) );
	actiongroup->add( Gtk::Action::create("ExportTxt", "Export as text..."),
								  sigc::mem_fun(*this, &XCadabra::on_file_export_text) );
	actiongroup->add( Gtk::Action::create("Quit", Gtk::Stock::QUIT),
								  sigc::mem_fun(*this, &XCadabra::on_file_quit) );
	actiongroup->add( Gtk::Action::create("InsertTeXAbove", "Insert TeX cell above"),
							Gtk::AccelKey("<alt><shift>Up"),
								  sigc::mem_fun(*this, &XCadabra::on_edit_insert_tex_above) );   
	actiongroup->add( Gtk::Action::create("InsertTeXBelow", "Insert TeX cell below"),
							Gtk::AccelKey("<alt><shift>Down"),
								  sigc::mem_fun(*this, &XCadabra::on_edit_insert_tex_below));   
	actiongroup->add( Gtk::Action::create("InsertInputAbove", "Insert input cell above"), 
							Gtk::AccelKey("<alt>Up"),
								  sigc::mem_fun(*this, &XCadabra::on_edit_insert_input_above) );   
	actiongroup->add( Gtk::Action::create("InsertInputBelow", "Insert input cell below"), 
							Gtk::AccelKey("<alt>Down"),
								  sigc::mem_fun(*this, &XCadabra::on_edit_insert_input_below) );   
	actiongroup->add( Gtk::Action::create("RemoveActive", "Remove active cell"),
							Gtk::AccelKey("<alt>Delete"),
								  sigc::mem_fun(*this, &XCadabra::on_edit_remove_cell) );
	actiongroup->add( Gtk::Action::create("DivideCell", "Divide active cell"),
							Gtk::AccelKey("<shift><ctrl>D"),
								  sigc::mem_fun(*this, &XCadabra::on_edit_divide_cell) );
	actiongroup->add( Gtk::Action::create("SplitView", "Split view"),
								  sigc::mem_fun(*this, &XCadabra::on_view_split) );   
	actiongroup->add( Gtk::Action::create("CloseView", "Close view"),
								  sigc::mem_fun(*this, &XCadabra::on_view_close) );   

	Gtk::RadioAction::Group group_font_size;

	font_action0=Gtk::RadioAction::create(group_font_size, "FontSmall", "Small");
	font_action0->property_value()=-1;
	actiongroup->add( font_action0, sigc::bind(sigc::mem_fun(*this, &XCadabra::on_settings_font_size),-1 ));
	if(font_step==-1) font_action0->set_active();

	font_action1=Gtk::RadioAction::create(group_font_size, "FontMedium", "Medium (default)");
	font_action1->property_value()= 0;
	actiongroup->add( font_action1, sigc::bind(sigc::mem_fun(*this, &XCadabra::on_settings_font_size), 0));
	if(font_step==0) font_action1->set_active();

	font_action2=Gtk::RadioAction::create(group_font_size, "FontLarge", "Large");
	font_action2->property_value()= 2;
	actiongroup->add( font_action2, sigc::bind(sigc::mem_fun(*this, &XCadabra::on_settings_font_size), 2));
	if(font_step==2) font_action2->set_active();

	font_action3=Gtk::RadioAction::create(group_font_size, "FontExtraLarge", "Extra large");
	font_action3->property_value()= 4;
	actiongroup->add( font_action3, sigc::bind(sigc::mem_fun(*this, &XCadabra::on_settings_font_size), 4));
	if(font_step==4) font_action3->set_active();

	actiongroup->add( Gtk::Action::create("Basics", "Basics"),
								  sigc::bind(sigc::mem_fun(*this, &XCadabra::on_tutorial_open),0) );   
	actiongroup->add( Gtk::Action::create("Derivatives", "Derivatives"),
								  sigc::bind(sigc::mem_fun(*this, &XCadabra::on_tutorial_open),1) );   
	actiongroup->add( Gtk::Action::create("Relativity", "Relativity"),
								  sigc::bind(sigc::mem_fun(*this, &XCadabra::on_tutorial_open),2) );   
	actiongroup->add( Gtk::Action::create("Spinors", "Spinors"),
								  sigc::bind(sigc::mem_fun(*this, &XCadabra::on_tutorial_open),3) );   
	actiongroup->add( Gtk::Action::create("About", Gtk::Stock::ABOUT),
								  sigc::mem_fun(*this, &XCadabra::on_help_about) );   
	actiongroup->add( Gtk::Action::create("CitingInfo", "Citing cadabra"),
								  sigc::mem_fun(*this, &XCadabra::on_help_citing) );   
	actiongroup->add( Gtk::Action::create("AllProperties", "Properties"));
	actiongroup->add( Gtk::Action::create("AllAlgorithms", "Algorithms"));
	actiongroup->add( Gtk::Action::create("AllReserved", "Reserved node names"));
	actiongroup->add( Gtk::Action::create("ContextHelp", "Current object"),
							Gtk::AccelKey("F1"),
								  sigc::mem_fun(*this, &XCadabra::on_help_context) );   
//	actiongroup->add( Gtk::Action::create("AutoComplete", "Auto-complete"),
//							Gtk::AccelKey("<tab>"),
//								  sigc::mem_fun(*this, &XCadabra::on_autocomplete) );   
	
	uimanager = Gtk::UIManager::create();
	uimanager->insert_action_group(actiongroup);
	add_accel_group(uimanager->get_accel_group());

	Glib::ustring ui_info =
		"<ui>"
		"  <menubar name='MenuBar'>"
		"    <menu action='MenuFile'>"
		"      <menuitem action='New'/>"
		"      <menuitem action='Open'/>"
		"      <menuitem action='Save'/>"
		"      <menuitem action='SaveAs'/>"
		"      <menuitem action='Print'/>"
		"      <menuitem action='ExportTxt'/>"
		"      <separator/>"
		"      <menuitem action='Quit'/>"
		"    </menu>"
		"    <menu action='MenuEdit'>"
		"      <menuitem action='DivideCell'/>"
		"      <separator/>"
		"      <menuitem action='InsertTeXAbove'/>"
		"      <menuitem action='InsertTeXBelow'/>"
		"      <menuitem action='InsertInputAbove'/>"
		"      <menuitem action='InsertInputBelow'/>"
		"      <separator/>"
		"      <menuitem action='RemoveActive'/>"
		"    </menu>"
		"    <menu action='MenuView'>"
		"      <menuitem action='SplitView'/>"
		"      <menuitem action='CloseView'/>"
		"    </menu>"
		"    <menu action='MenuSettings'>"
		"      <menu action='MenuFontSize'>"
		"         <menuitem action='FontSmall'/>"
		"         <menuitem action='FontMedium'/>"
		"         <menuitem action='FontLarge'/>"
		"         <menuitem action='FontExtraLarge'/>"
      "      </menu>"
		"    </menu>"
		"    <menu action='MenuHelp'>"
		"      <menuitem action='About'/>"
		"      <menuitem action='CitingInfo'/>"
		"      <menuitem action='ContextHelp'/>"
		"    </menu>"
		"  </menubar>"
		"  <toolbar  name='ToolBar'>"
		"    <toolitem action='Open'/>"
		"    <toolitem action='Quit'/>"
		"  </toolbar>"
		"</ui>";


//		"    <menu action='MenuTutorial'>"
//		"      <menuitem action='Basics'/>"
//		"      <menuitem action='Derivatives'/>"
//		"      <menuitem action='Relativity'/>"
//		"      <menuitem action='Spinors'/>"
//		"    </menu>"

    uimanager->add_ui_from_string(ui_info);
	 Gtk::Widget *menubar = uimanager->get_widget("/MenuBar");
	 topbox.pack_start(*menubar, Gtk::PACK_SHRINK);

//  	// add the lot to the window
//  	topbox.pack_start(menubar, Gtk::PACK_SHRINK);
	topbox.pack_start(supermainbox, true, true);
	topbox.pack_start(statusbarbox, false, false);
	supermainbox.pack_start(mainbox, true, true);

	// The three main widgets
	mainbox.pack_start(buttonbox, Gtk::PACK_SHRINK, 0);
	buttonbox.pack_start(statusbox, Gtk::PACK_EXPAND_WIDGET, 0);
	b_cdbstatus.set_justify(Gtk::JUSTIFY_LEFT);
	statusbarbox.pack_start(b_cdbstatus);
	b_kernelversion.set_justify(Gtk::JUSTIFY_LEFT);
	statusbarbox.pack_start(b_kernelversion);
//	statusbarbox.pack_start(progressbarvbox);
	statusbarbox.pack_start(progressbar1);
	statusbarbox.pack_start(progressbar2);
	progressbar1.set_size_request(200,-1);
	progressbar2.set_size_request(200,-1);
	buttonbox.pack_start(b_help, Gtk::PACK_SHRINK);
	buttonbox.pack_start(b_run, Gtk::PACK_SHRINK);
	buttonbox.pack_start(b_run_to, Gtk::PACK_SHRINK);
	buttonbox.pack_start(b_run_from, Gtk::PACK_SHRINK);
	buttonbox.pack_start(b_stop, Gtk::PACK_SHRINK);
	buttonbox.pack_start(b_kill, Gtk::PACK_SHRINK);
	b_help.signal_clicked().connect(sigc::mem_fun(*this, &XCadabra::on_help_context));
	b_stop.signal_clicked().connect(sigc::mem_fun(*this, &XCadabra::on_stop));
	b_kill.signal_clicked().connect(sigc::mem_fun(*this, &XCadabra::on_kill));
	b_run.signal_clicked().connect(sigc::mem_fun(*this, &XCadabra::on_run));
	b_run_to.signal_clicked().connect(sigc::mem_fun(*this, &XCadabra::on_run_to));
	b_run_from.signal_clicked().connect(sigc::mem_fun(*this, &XCadabra::on_run_from));

	// We always have at least one canvas: this one
	canvasses.push_back(manage( new NotebookCanvas(*this) ));
	mainbox.pack_start(*canvasses.back(), Gtk::PACK_EXPAND_WIDGET, 0);

//	canvasses[0]->to_cdb.connect(sigc::mem_fun(*this, &XCadabra::handle_editbox_output));
	active_canvas=canvasses[0];
	show_all();

	// Setup an empty notebook and add a single empty input cell.
	DataCell *newcell=new DataCell(DataCell::c_input);
	add_cell(newcell);
	active_canvas->cell_grab_focus(newcell);
	modified=false;
	kernel_idle();
	update_title();
	}

bool XCadabra::on_key_press_event(GdkEventKey* event)
	{
	switch(event->keyval) {
		 case GDK_Home:
			  active_canvas->scroll_to_start();
			  return true; // if we don't return immediately, selections will go away
		 case GDK_End:
			  active_canvas->scroll_to_end();
			  return true; // if we don't return immediately, selections will go away
		 case GDK_Page_Up:
			  active_canvas->scroll_up();
			  return true; // if we don't return immediately, selections will go away
		 case GDK_Page_Down:
			  active_canvas->scroll_down();
			  return true; // if we don't return immediately, selections will go away
		 case 108:
			  if( (event->state&Gdk::CONTROL_MASK) ) { // Ctrl-L: center display
					active_canvas->scroll_into_view(active_cell, true);
					return true; // if we don't return immediately, selections will go away
					}
			  break;
		 case GDK_Tab:
			  if(on_autocomplete())
					return true; // prevent normal Tab action
		 }

	return Window::on_key_press_event(event);
	}

bool XCadabra::callmm(Glib::IOCondition, int fd)
	{
	if(!(cmm->select_callback(fd))) {
		connections[fd].disconnect();
		connections.erase(fd);
		}
	return true;
	}

void XCadabra::add_canvas()
	{
	canvasses.push_back(manage( new NotebookCanvas(*this) ));
	canvasses[canvasses.size()-2]->pack2(*canvasses.back());
//	canvasses.back()->to_cdb.connect(sigc::mem_fun(*this, &XCadabra::handle_editbox_output));

	// Make it display all DataCells
	DataCells_t::iterator it=datacells.begin();
	while(it!=datacells.end()) {
		canvasses.back()->add_cell(*it);
		++it;
		}
	canvasses.back()->show();
	
	// Set up selection mechanisms. For Maple, it seems that we need LENGTH.
	// Also, we will put normal, non-utf8-dressed text in TEXT, instead of the
	// \x{...} stuff that the TextView widget normally spits out.

   //	selection_add_target(GDK_SELECTION_PRIMARY, Gtk::TargetEntry("LENGTH", 1));
   //	selection_add_target(GDK_SELECTION_PRIMARY, Gtk::TargetEntry("TEXT", 2));
	}

void XCadabra::on_stop()
	{
	kill(cdb.get_pid(), SIGINT);
	}

void XCadabra::on_kill()
	{
	restarting_kernel=true;
	*(cdb.output_pipe("stdin")) << "@quit;\n" << std::flush;
	b_cdbstatus.set_text(" Status: Restarting kernel...");
	b_kernelversion.set_text("Kernel: not running");

	// We now wait for the process to terminate properly and
	// modglue to send us a signal about that.

//	cdb.terminate();
//	cdb.fork();
//	*(cdb.output_pipe("stdin")) << "@print_status{true};\n" << std::flush;
//	b_cdbstatus.set_text("Status: Kernel restarted, idle.");
	}

void XCadabra::on_run()
	{
	get_window()->set_cursor(hglass);

	running=true;
	running_last=0;
	b_cdbstatus.set_text(" Status: Executing notebook.");
	b_stop.set_sensitive(true);
	active_canvas->select_first_input_cell();
	}

void XCadabra::on_run_to()
	{
	get_window()->set_cursor(hglass);

	running=true;
	running_last=active_cell->datacell;
	b_stop.set_sensitive(true);
	b_cdbstatus.set_text(" Status: Executing until cursor.");
	active_canvas->select_first_input_cell();
	}

void XCadabra::on_run_from()
	{
	if(active_cell!=0) {
		get_window()->set_cursor(hglass);

		running=true;
		running_last=0;
		b_cdbstatus.set_text(" Status: Executing from cursor.");
		b_stop.set_sensitive(true);
		active_canvas->cell_grab_focus(active_cell);
		}
	}

void XCadabra::on_help_about()
	{
	Gtk::AboutDialog md;
	md.set_name("XCadabra");
	md.set_website("http://www.aei.mpg.de/~peekas/cadabra/");
	md.set_website_label("cadabra website");
	md.set_version(RELEASE);
	md.set_copyright("\xC2\xA9 2006-2008 Kasper Peeters");
	md.set_comments("Graphical user interface for the cadabra symbolic computer algebra system.");
	md.set_license("XCadabra and Cadabra are available under the Gnu General Public License version 2.\n\nIf you use Cadabra or even just play with it, I would like to hear about it. Please send me an email so that I can get an idea of who is interested in this program.\n\nIf you use Cadabra in your own work, please cite both\n\nKasper Peeters\n\"A field-theory motivated approach to computer algebra\"\ncs.sc/0608005\nComput. Phys. Commun 176 (2007) 550\n\nKasper Peeters\n\"Introducing Cadabra: a symbolic computer algebra system for field theory problems\"\nhep-th/0701238\n\nThank you!");
#if (GTKMM_VER == 28 || GTKMM_VER == 212)
	md.set_wrap_license(true);
#endif

	std::vector<std::string> authors;
	authors.push_back("Kasper Peeters <kasper.peeters@aei.mpg.de>");
	md.set_authors(authors);

	md.run();
	}

void XCadabra::on_help_algorithms(const std::string& algorithm)
	{
	help_window.on_help_context_link(CadabraHelp::t_algorithm, algorithm.substr(1));
	}

void XCadabra::on_help_properties(const std::string& property)
	{
	help_window.on_help_context_link(CadabraHelp::t_property, property.substr(2));
	}

void XCadabra::on_help_reserved(const std::string& reserved)
	{
	help_window.on_help_context_link(CadabraHelp::t_texcommand, reserved.substr(1));
	}

void XCadabra::on_help_citing()
	{
	std::ostringstream str;
	str << "Cadabra citing information";
	Gtk::MessageDialog md(str.str());
   md.set_secondary_text("If you use Cadabra to write a paper, please cite both\n\nKasper Peeters\n\"A field-theory motivated approach to computer algebra\"\ncs.sc/0608005\nComput. Phys. Commun. 176 (2007) 550\n\nKasper Peeters\n\"Introducing Cadabra: a symbolic computer algebra system for field theory problems\"\nhep-th/0701238\n\nThank you!");
	md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
//	md.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
	md.run();
	}

void XCadabra::insert_at_mark(const std::string instxt)
	{
	// Find the Gtk::TextBuffer corresponding to the current cell.
	Glib::RefPtr<Gtk::TextBuffer> buf;
	switch(active_cell->datacell->cell_type) {
		 case DataCell::c_input:
			  buf=active_cell->datacell->textbuf;
			  break;
		 case DataCell::c_tex:
			  buf=active_cell->datacell->texbuf->tex_source;
			  break;
		 default:
			  return;
		 }

//	Gtk::TextBuffer::iterator it=buf->get_iter_at_mark(buf->get_insert());
//	--it;
	buf->insert_at_cursor(instxt);
//	for(int i=instxt.size(); i>0; --i)
//		 ++it;
//	buf->place_cursor(buf->end());
//	active_canvas->cell_grab_focus(active_cell);
	}

bool XCadabra::current_objtype_and_name(CadabraHelp::objtype_t& objtype, std::string& helpname)
	{
	// Find the Gtk::TextBuffer corresponding to the current cell.
	Glib::RefPtr<Gtk::TextBuffer> buf;
	switch(active_cell->datacell->cell_type) {
		 case DataCell::c_input:
			  buf=active_cell->datacell->textbuf;
			  break;
		 case DataCell::c_tex:
			  buf=active_cell->datacell->texbuf->tex_source;
			  break;
		 default:
			  return false;
		 }

	Gtk::TextBuffer::iterator it=buf->get_iter_at_mark(buf->get_insert());
	std::string before=buf->get_slice(buf->begin(), it);
	std::string after =buf->get_slice(it, buf->end());


	if(! (before.size()==0 && after.size()==0) ) {
		 
		 // We provide help for properties, algorithms and reserved node names. 
       // Properties are delimited
		 // to the left by '::' and to the right by anything non-alnum. Algorithms
		 // are delimited to the left by '@' and to the right by anything non-alnum or
		 // non '_'. Reserved node names are TeX symbols, starting with '\'.
		 // 
		 // So scan the 'before' string for a left-delimiter and the 'after' string
		 // for a right-delimiter.
		 
		 
		 size_t lpos=before.size()-1;
		 bool accepted_underscore=false;
		 while(lpos>=0) {
			  if(before[lpos]==':' && lpos>0 && before[lpos-1]==':') {
					objtype=CadabraHelp::t_property;
					break;
					}
			  if(before[lpos]=='\\') {
					objtype=CadabraHelp::t_texcommand;
					break;
					}
			  if(before[lpos]=='@') {
					objtype=CadabraHelp::t_algorithm;
					break;
					}
			  if(before[lpos]=='_')            accepted_underscore=true;
			  else if(isalnum(before[lpos])==0) return false;
			  --lpos;
			  }
		 if(objtype==CadabraHelp::t_none) return false;
		 if(accepted_underscore && objtype==CadabraHelp::t_property) return false;
		 ++lpos;
		 
		 size_t rpos=0;
		 while(rpos<after.size()) {
			  if(objtype==CadabraHelp::t_property) {
					if(isalnum(after[rpos])==0)
						 break;
					}
			  else if(objtype==CadabraHelp::t_algorithm) {
					if(isalnum(after[rpos])==0 && after[rpos]!='_')
						 break;
					}
			  else if(objtype==CadabraHelp::t_texcommand) {
					if(isalnum(after[rpos])==0 && after[rpos]!='_')
						 break;
					}
			  ++rpos;
			  }
		 helpname=before.substr(lpos)+after.substr(0,rpos);
		 }

	return true;
	}

bool XCadabra::on_autocomplete()
	{
	if(active_cell==0) // no cell active
		 return false;

	CadabraHelp::objtype_t objtype=CadabraHelp::t_none;
	std::string helpname;

	if(current_objtype_and_name(objtype, helpname)) {
		 if(helpname.size()==0) return false;
		 std::set<std::string>::iterator it, itend;
		 std::string candidate;
		 bool options_exist=false;
		 bool alg_or_prop=true;

		 if(objtype==CadabraHelp::t_algorithm) {
			  it=algorithm_set.begin();
			  itend=algorithm_set.end();
			  }
		 else if(objtype==CadabraHelp::t_property) {
			  it=property_set.begin();
			  itend=property_set.end();
			  }
		 else {
			  alg_or_prop=false;
			  helpname="\\"+helpname;
			  for(unsigned int i=0; i<sizeof(autocomplete_strings)/sizeof(*autocomplete_strings); ++i) {
					if(helpname.size()<strlen(autocomplete_strings[i])) {
						 if(strncmp(autocomplete_strings[i], helpname.c_str(), helpname.size())==0) {
							  options_exist=true;
							  if(candidate.size()==0) candidate=autocomplete_strings[i];
							  else {
									unsigned int j=0;
									for(; j<std::min(candidate.size(), strlen(autocomplete_strings[i])); ++j) 
										 if(candidate[j]!=(autocomplete_strings[i])[j])
											  break;
									candidate=candidate.substr(0,j);
									}
							  }
						 }
					}
			  }
		 if(alg_or_prop) {
			  while(it!=itend) {
					if(helpname.size()<(*it).size()) {
						 if((*it).substr(0,helpname.size())==helpname) {
							  options_exist=true;
							  if(candidate.size()==0) candidate=(*it);
							  else {
									unsigned int i=0;
									for(; i<std::min(candidate.size(), (*it).size()); ++i) 
										 if(candidate[i]!=(*it)[i])
											  break;
									candidate=candidate.substr(0,i);
									}
							  }
						 }
					++it;
					}
			  }
		 if(candidate.size()!=0)
			  insert_at_mark(candidate.substr(helpname.size()));

		 return options_exist;
		 }
	else return false;
	}

void XCadabra::on_help_context()
	{
	if(active_cell==0) // no cell active
		 return;

	CadabraHelp::objtype_t objtype=CadabraHelp::t_none;
	std::string helpname;

	if(current_objtype_and_name(objtype, helpname)) 
		 help_window.on_help_context_link(objtype, helpname);
	}

bool XCadabra::on_kernel_exit(modglue::ext_process& pr)
	{
#ifdef DEBUG
	std::cerr << pr.name() << " has ended, restarting..." << std::endl;
#endif
	disconnect_io_signals();

	if(!restarting_kernel) {
		kernel_idle();
		Gtk::MessageDialog md("The cadabra kernel has disconnected unexpectedly.");
		md.set_secondary_text("If you can reproduce this, please file a bug report, quoting the text below.\n\n"+accumulated_error);
		md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
//		md.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
		md.run();
		}
	
	cdb.fork();
	connect_io_signals();

	*(cdb.output_pipe("stdin")) << "@print_status{true};\n" << std::flush;
	b_cdbstatus.set_text(" Status: Kernel restarted, idle.");


	// return 'false' to keep the main loop running.
	return false;
	}

void XCadabra::disconnect_io_signals()
	{
	std::map<int, sigc::connection>::iterator it=connections.begin();
	while(it!=connections.end()) {
		it->second.disconnect();
		++it;
		}
	connections.clear();
	}

void XCadabra::connect_io_signals()
	{
	assert(connections.size()==0);

	std::vector<int> fds;
	cmm->fds_to_watch(fds);
	for(unsigned int i=0; i<fds.size(); ++i) {
//		std::cerr << "watching " << fds[i] << std::endl;
		connections[fds[i]]=
			Glib::signal_io().connect(sigc::bind(sigc::mem_fun(this, &XCadabra::callmm),
															 fds[i]), fds[i], Glib::IO_IN);
		}
	}

DataCell *XCadabra::add_cell(DataCell *newcell, DataCell *ref, bool before)
	{
	modified=true;
	update_title();

	// First the data object, of which there is only one per cell.
	if(ref==0)
		datacells.push_back(newcell);
	else {
		DataCells_t::iterator fnd=find(datacells.begin(), datacells.end(), ref); 
		if(fnd==datacells.end()) {
			datacells.push_back(newcell);
			}
		else {
			 if(!before) {
				  do {
						++fnd;
						} while( fnd!=datacells.end() &&
									( (*fnd)->cell_type==DataCell::c_output ||
									  (*fnd)->cell_type==DataCell::c_error ) );
				  }
			datacells.insert(fnd, newcell);
			}
		}

	Gtk::Allocation al=get_allocation();
	const int horizontal_pixels=al.get_width()-20-35;

	try {
		switch(newcell->cell_type) {
			case DataCell::c_output:
				newcell->texbuf->font_size=12+(font_step*2);
				newcell->texbuf->generate("\\begin{dmath*}[compact,spread=2pt]\n","\\end{dmath*}\n",
												  horizontal_pixels);
				break;
			case DataCell::c_comment:
				newcell->texbuf->font_size=12+(font_step*2);
				newcell->texbuf->generate("\\begin{verbatim}\n","\\end{verbatim}\n", horizontal_pixels);
				break;
			case DataCell::c_input:
				newcell->textbuf->signal_changed().connect(sigc::mem_fun(this, &XCadabra::input_cell_modified));
				break;
			case DataCell::c_error:
				newcell->texbuf->font_size=12+(font_step*2);
				newcell->texbuf->generate("{\\color[named]{Red}", "}", horizontal_pixels);
				break;
			case DataCell::c_tex:
				newcell->texbuf->font_size=12+(font_step*2);
				newcell->texbuf->generate("","", horizontal_pixels);
				newcell->textbuf->signal_changed().connect(sigc::mem_fun(this, &XCadabra::tex_cell_modified));
				break;
			}
		}
	catch(std::exception& ex) {
		kernel_idle();
		Gtk::MessageDialog md(ex.what());
		md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
		md.run();
		}

	// Now we have to tell all NotebookCanvas objects to create a
	// view on the just created DataCell. 

	for(unsigned int i=0; i<canvasses.size(); ++i) {
		VisualCell *vis=canvasses[i]->add_cell(newcell, ref, before);
		if(canvasses[i]==active_canvas) {
#ifdef DEBUG
			std::cerr << "found active canvas" << std::endl;
#endif
//			active_canvas->cell_grab_focus(vis);
			// make this new cell the active cell if it is an input or tex cell.
			if(newcell->cell_type==DataCell::c_input || newcell->cell_type==DataCell::c_tex)
				 active_cell=vis;
			}
		}

	return newcell;
	}

bool XCadabra::handle_editbox_output(std::string str, NotebookCanvas *can, VisualCell *vis)
	{
	// Disable this cell until output has been received, so we
	// don't get it twice.
	if(vis->datacell->running==false) {
		vis->datacell->running=true;
		
		// Remove any old error boxes until the next 
		// Now run the input through cadabra.
#ifdef DEBUG
		std::cerr << "sending to cdb " << str << std::endl;
#endif
		b_cdbstatus.set_text(" Status: Kernel busy.");
		get_window()->set_cursor(hglass);
		*(cdb.output_pipe("stdin")) << "#cellstart " << vis->datacell << "\n"
											 << str << "\n"
											 << "#cellend\n" << std::flush;
		}
	return true;
	}

void XCadabra::handle_on_grab_focus(NotebookCanvas *can, VisualCell *vis) 
	{
#ifdef DEBUG
	std::cerr << "focus grabbed" << std::endl;
#endif
	active_canvas=can;
	active_cell=vis;
	if(selected) {
		selected->outbox->set_state(Gtk::STATE_NORMAL);
		selected=0;
		}

	if(running && vis) {
		if(running_last==active_cell->datacell) {
#ifdef DEBUG
			std::cerr << "end cell reached" << std::endl;
#endif
			kernel_idle();
			}
		else {
			Glib::RefPtr<Gtk::TextBuffer> textbuf=active_cell->datacell->textbuf;
			std::string tmp(trim(textbuf->get_text(textbuf->begin(), textbuf->end())));
			if(tmp[0]!='#' && tmp[tmp.size()-1]!=';' && tmp[tmp.size()-1]!=':' && tmp[tmp.size()-1]!='.' ) {
#ifdef DEBUG
				std::cerr << "cell does not end with delimiter" << std::endl;
#endif
				kernel_idle();
				}
			else {
#ifdef DEBUG
				std::cerr << "executing cell\n" << tmp << std::endl;
#endif
				*(cdb.output_pipe("stdin")) << "#cellstart " << active_cell->datacell << "\n"
													 << tmp << "\n"
												 << "#cellend\n" << std::flush;
				}
			}
		}
	}

bool XCadabra::handle_visibility_toggle(GdkEventButton *, NotebookCanvas *can, VisualCell *vis) 
	{
	vis->datacell->tex_hidden = !vis->datacell->tex_hidden;
	for(unsigned int i=0; i<canvasses.size(); ++i) {
		NotebookCanvas::VisualCells_t::iterator it=canvasses[i]->visualcells.begin();
		while(it!=canvasses[i]->visualcells.end()) {
			if((*it)->datacell==vis->datacell) {
				if(vis->datacell->tex_hidden) (*it)->texbox->edit.hide_all();
				else                          (*it)->texbox->edit.show_all();
				break;
				}
			++it;
			}
		}
	return true;
	}

bool XCadabra::handle_outbox_select(GdkEventButton *, NotebookCanvas *can, VisualCell *vis) 
	{
	Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);

	if(selected) {
		selected->outbox->set_state(Gtk::STATE_NORMAL);
		if(selected==vis) {
			refClipboard->set_text("");
			selected=0;
			return true;
			}
		}
	selected=vis;
	vis->outbox->set_state(Gtk::STATE_PRELIGHT);

	std::string cpystring=vis->datacell->texbuf->tex_source->get_text();
	size_t pos=cpystring.find("\\specialcolon{}");
	if(pos!=std::string::npos) 
		cpystring.replace(pos, 15, " :");
	
	// Setup clipboard handling
	clipboard_txt = cpystring;
	clipboard_cdb = vis->datacell->cdbbuf;

	std::list<Gtk::TargetEntry> listTargets;
	if(clipboard_cdb.size()>0) 
		listTargets.push_back( Gtk::TargetEntry("cadabra") ); 
	listTargets.push_back( Gtk::TargetEntry("UTF8_STRING") ); 
	listTargets.push_back( Gtk::TargetEntry("TEXT") ); 
	refClipboard->set( listTargets, 
							 sigc::mem_fun(this, &XCadabra::on_clipboard_get), 
							 sigc::mem_fun(this, &XCadabra::on_clipboard_clear) );

	return true;
	}

bool XCadabra::handle_tex_update_request(std::string, NotebookCanvas *can, VisualCell *vis)
	{
	// First re-generate the image.
	Gtk::Allocation al=get_allocation();
	const int horizontal_pixels=al.get_width()-20-35;
	try {
		 vis->texbox->texview.texbuf->generate("","",horizontal_pixels);
		 }
	catch(std::exception& ex) {
		 kernel_idle();
		 Gtk::MessageDialog md(ex.what());
		 md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
		 md.run();
		 }		

	// Now walk through all TeXInput cells and update the Image widget.
	for(unsigned int i=0; i<canvasses.size(); ++i) {
		NotebookCanvas::VisualCells_t::iterator it=canvasses[i]->visualcells.begin();
		while(it!=canvasses[i]->visualcells.end()) {
			if((*it)->datacell==vis->datacell) {
				(*it)->texbox->texview.image.set(vis->texbox->texview.texbuf->pixbuf);
				break;
				}
			++it;
			}
		}
	return true;
	}


XCadabra::~XCadabra()
	{
	DataCells_t::iterator it=datacells.begin();
	while(it!=datacells.end()) {
		delete *it;
		++it;
		}
	}

bool XCadabra::on_delete_event(GdkEventAny* event)
	{
	if(quit_safeguard()) {
		cdb.terminate();
		Gtk::Window::on_delete_event(event);
		Gtk::Main::quit();
		return false;
		}
	else return true;
	}

void XCadabra::input_cell_modified()
	{
	if(!modified) {
		modified=true;
		update_title();
		}
	
	}

void XCadabra::tex_cell_modified()
	{
	if(!modified) {
		modified=true;
		update_title();
		}
	}

void XCadabra::remove_noninput_below(DataCell *dc)
	{
	if(selected) {
		selected->outbox->set_state(Gtk::STATE_NORMAL);
		selected=0;
		}
	DataCells_t::iterator it=datacells.begin();
	while(it!=datacells.end()) {
		if((*it)==dc) {
			++it;
			while(it!=datacells.end() && (
						(*it)->cell_type==DataCell::c_output ||
						(*it)->cell_type==DataCell::c_comment ||
						(*it)->cell_type==DataCell::c_error) ) {
				 for(unsigned int i=0; i<canvasses.size(); ++i) {
					  canvasses[i]->remove_cell(*it);
					  }
				it=datacells.erase(it);
				}
			return;
			}
		++it;
		}
	}

void XCadabra::kernel_idle()
	{
	running=false;
	b_cdbstatus.set_text(" Status: Kernel idle.");
	get_window()->set_cursor();
	b_stop.set_sensitive(false);
	}

bool XCadabra::receive(modglue::ipipe& p)
	{
	static std::string str;
	static std::string comment;
	static std::string error;
	static bool error_occurred=false;
	static bool last_was_prompt=true; // avoid repeated empty cells
	static bool in_cell=false; // prompts only get honored outside cells
	static DataCell *cp=0, *origcell=0;

	have_received=true;

	while(std::getline(p,str)) {
#ifdef DEBUG
		std::cerr << "rec: " << str << std::endl;
#endif
		if(str.substr(0,10)=="#cellstart") {
			std::istringstream ss(str.substr(11));
			int help;
			ss >> std::hex >> help;
			cp=reinterpret_cast<DataCell *>(help);
			remove_noninput_below(cp);
			origcell=cp;
			in_cell=true;
			comment="";
			continue;
			}
		else if(str=="#cellend") {
			in_cell=false;
			last_was_prompt=false;
			if(trim(comment).size()!=0 && trim(comment)!=">") {
				DataCell *newcell=new DataCell(DataCell::c_comment, trim(comment));
				cp=add_cell(newcell, cp, false);
				}
			comment="";
			continue;
			}
		else if(str.substr(0,7)=="Cadabra") {
			b_kernelversion.set_label("Kernel: "+str.substr(8,5)+".");
			}
		else if(str=="<comment>") {
			parse_mode.push_back(m_comment);
			last_was_prompt=false;
			continue;
			}
		else if(str=="</comment>") {
			parse_mode.pop_back();
			last_was_prompt=false;
			continue;
			}
		else if(str=="<status>") {
			parse_mode.push_back(m_status);
			last_was_prompt=false;
			continue;
			}
		else if(str=="</status>") {
			parse_mode.pop_back();
			continue;
			}
		else if(str=="<plain>") {
			parse_mode.push_back(m_plain);
			plain="";
			last_was_prompt=false;
			continue;
			}
		else if(str=="</plain>") {
			parse_mode.pop_back();
			last_was_prompt=false;			
			continue;
			}
		else if(str=="<progress>") {
			progress="";
			progress_todo=-1;
			progress_done=-1;
			progress_count=-1;
			parse_mode.push_back(m_progress);
			continue;
			}
		else if(str=="</progress>") {
			std::ostringstream ss;
			if(progress_todo>0) 
				ss << progress << "..." << " (" << progress_done << " of " << progress_todo << ")";
			else
				ss << progress << "...";
			if(progress_count==1) {
				progressbar1.set_text(ss.str());
				if(progress_todo==0) progressbar1.set_fraction(0);
				else                 progressbar1.set_fraction(std::min(1.0,progress_done/(1.0*progress_todo)));
				}
			else {
				progressbar2.set_text(ss.str());
				if(progress_todo==0) progressbar2.set_fraction(0);
				else                 progressbar2.set_fraction(std::min(1.0,progress_done/(1.0*progress_todo)));
				}
			parse_mode.pop_back();
			continue;
			}
		else if(str=="<error>") {
			parse_mode.push_back(m_error);
			last_was_prompt=false;
			error_occurred=true;
			continue;
			}
		else if(str=="</error>") {
			parse_mode.pop_back();
			if(trim(error).size()!=0) {
				DataCell *newcell=new DataCell(DataCell::c_error, trim(error));
				kernel_idle();
				cp=add_cell(newcell, cp, false);
//				// make previous input cell active
//				DataCells_t::iterator dit=datacells.begin();
//				while(dit!=datacells.end()) {
//					 if((*dit)==newcell) {
//						  while(dit!=datacells.begin()) {
//								
//								}
//						  }
//					 prevcell=(*dit);
//					 ++dit;
//					 }
				}
			error="";
			last_was_prompt=false;
			continue;
			}
		else if(str=="<eqno>") {
			parse_mode.push_back(m_eqno);
			last_was_prompt=false;
			continue;
			}
		else if(str=="</eqno>") {
			parse_mode.pop_back();
			continue;
			}
		else if(str=="<eq>") {
			parse_mode.push_back(m_eq);
			last_was_prompt=false;
			continue;
			}
		else if(str=="</eq>") {
			parse_mode.pop_back();
			if(eq.size()!=0) {
				DataCell *newcell=new DataCell(DataCell::c_output, eqno+"\\specialcolon{}= "+eq);
				newcell->cdbbuf=plain;
				cp=add_cell(newcell, cp, false);
				}
			eq="";
			eqno="";
			plain="";
			last_was_prompt=false;
			continue;
			}
		else if(str=="<property>") {
			parse_mode.push_back(m_property);
			last_was_prompt=false;
			continue;
			}
		else if(str=="</property>") {
			 add_property_help(property);
			 property="";
			parse_mode.pop_back();
			continue;
			}
		else if(str=="<algorithm>") {
			parse_mode.push_back(m_algorithm);
			last_was_prompt=false;
			continue;
			}
		else if(str=="</algorithm>") {
			 add_algorithm_help(algorithm);
			 algorithm="";
			parse_mode.pop_back();
			continue;
			}
		else if(str=="<reserved>") {
			parse_mode.push_back(m_reserved);
			last_was_prompt=false;
			continue;
			}
		else if(str=="</reserved>") {
			 add_reserved_help(reserved);
			 reserved="";
			parse_mode.pop_back();
			continue;
			}
		else if(trim(str).substr(0,1)==">") {
#ifdef DEBUG
			 std::cerr << "received empty prompt" << std::endl;
#endif
			progressbar2.set_fraction(0);
			progressbar1.set_fraction(0);
			progressbar2.set_text(" ");
			progressbar1.set_text(" ");
			if(!last_was_prompt && !in_cell) {
				if(!running) {
					b_cdbstatus.set_text(" Status: Kernel idle.");
					get_window()->set_cursor();
					}
				last_was_prompt=true;
				if(error_occurred) {
					error_occurred=false;
					// re-enable original cell
					if(origcell!=0) {
						origcell->running=false;
						origcell=0;
						}
					}
				else { // everything hunky dorey
					if(datacells.back()==cp || cp==0 ) { // we are at the last cell of the notebook
						if(datacells.size()>0 && datacells.back()->cell_type==DataCell::c_input &&
							trim(datacells.back()->textbuf->get_text()).size()==0 ) {
#ifdef DEBUG
							std::cerr << "no cell needed" << std::endl;
#endif
							if(restarting_kernel) {
								restarting_kernel=false;
								active_canvas->cell_grab_focus(active_cell);
								}
							else
								active_canvas->cell_grab_focus(datacells.back());
							}
						else { // this last cell is not an input cell
#ifdef DEBUG
							std::cerr << "adding empty input cell" << std::endl;
#endif
							DataCell *newcell=new DataCell(DataCell::c_input, "");
							cp=add_cell(newcell, cp, false);
							active_canvas->cell_grab_focus(cp);
							}
						if(restarting_kernel)
							 restarting_kernel=false;
						// re-enable original cell
						if(origcell!=0) {
							origcell->running=false;
							origcell=0;
							}
						}
					else {
						if(restarting_kernel) {
							restarting_kernel=false;
							}
						else {
							DataCells_t::iterator it=datacells.begin();
							while(it!=datacells.end()) {
								if(*it==cp) {
									++it;
									while(it!=datacells.end() && (*it)->cell_type!=DataCell::c_input)
										++it;
									if(it==datacells.end()) {
										DataCell *newcell=new DataCell(DataCell::c_input, "");
										cp=add_cell(newcell, cp, false);
										active_canvas->cell_grab_focus(cp);
										}
									else active_canvas->cell_grab_focus(*it);
									// re-enable original cell
									if(origcell!=0) {
										origcell->running=false;
										origcell=0;
										}
									break;
									}
								++it;
								}
							}
						}
					}
				}
			else str="\n";
			}
		switch(parse_mode.back()) {
			case m_eq:
				eq+=str;
				if(str[str.size()-1]=='%')
					eq+="\n";
				break;
			case m_eqno:
				eqno+=str;
				break;
			case m_status:
			case m_discard:
				break;
			case m_property:
				property+=str;
				break;
			case m_algorithm:
				algorithm+=str; 
				break;
			case m_reserved:
				reserved+=str; 
				break;
			case m_comment:
				if(trim(str).size()>0)
					comment+=str+"\n";
				break;
			case m_error:
				error+=str+"\n";
				break;
			case m_plain:
				plain+=str;
				break;
			case m_progress:
				if(progress=="")            progress=str;
				else if(progress_todo==-1)  progress_todo=atoi(str.c_str());
				else if(progress_done==-1)  progress_done=atoi(str.c_str());
				else                        progress_count=atoi(str.c_str());
				break;
			}
		}
	p.clear();

	if(load_file) {
		load_file=false;
#ifdef DEBUG
		std::cerr << "Loading file..." << std::endl;
#endif		
		std::string tmp=name;
		std::string res=load(tmp, true);
		if(res.size()>0) {
			 Gtk::MessageDialog md("Error loading document "+tmp);
			 md.set_secondary_text(res);
			 md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
			 md.run();
			 name="";
			 }
		else name=tmp;
		modified=false;
		update_title();
		}

	return true;
	}

bool XCadabra::receive_err(modglue::ipipe& p)
	{
	std::string str;

	while(std::getline(p,str)) {
		 accumulated_error+=str;
		 }

	p.clear();
	return true;
	}

void XCadabra::on_file_new()
	{
	if(quit_safeguard(false)) {
		clear();
		DataCell *newcell=new DataCell(DataCell::c_input);
		add_cell(newcell);
		active_canvas->cell_grab_focus(newcell);
		modified=false;
		update_title();
		on_kill();
		}
	}

void XCadabra::on_file_open()
	{
	if(quit_safeguard(false)) {
		Gtk::FileChooserDialog fd("Open notebook...", Gtk::FILE_CHOOSER_ACTION_OPEN);
		fd.add_button(Gtk::Stock::OPEN,1);
		fd.add_button(Gtk::Stock::CANCEL,2);
		fd.set_default_response(1);
		int action=fd.run();
		if(action==1) {
			fd.hide_all();
			std::string res=load(fd.get_filename());
			if(res.size()>0) {
				Gtk::MessageDialog md("Error loading document "+fd.get_filename());
				md.set_secondary_text(res);
				md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
//				md.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
				md.run();
				}
			else {
				name=fd.get_filename();
				DataCell *newcell=new DataCell(DataCell::c_input);
				add_cell(newcell); 
				active_canvas->select_first_input_cell();
				modified=false;
				update_title();
				}
			}
		}
	}

void XCadabra::on_file_save()
	{
	// check if name known, otherwise call save_as
	if(name.size()>0) {
		std::string res=save(name);
		if(res.size()>0) {
			Gtk::MessageDialog md("Error saving document "+name);
			md.set_secondary_text(res);
			md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
//			md.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
			md.run();
			}
		else {
			modified=false;
			update_title();
			}
		}
	else on_file_save_as();
	}

void XCadabra::on_file_export_text()
	{
	Gtk::FileChooserDialog fd("Export to text...", Gtk::FILE_CHOOSER_ACTION_SAVE);
	fd.add_button(Gtk::Stock::SAVE,1);
	fd.add_button(Gtk::Stock::CANCEL,2);
	int action=fd.run();
	if(action==1) {
		fd.hide_all();
#ifdef DEBUG
		std::cerr << "going to export as " << fd.get_filename() << std::endl;
#endif
		std::string res=expo(fd.get_filename());
		if(res.size()>0) {
			Gtk::MessageDialog md("Error exporting document "+fd.get_filename());
			md.set_secondary_text(res);
			md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
//			md.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
			md.run();
			}
		}
	}

void XCadabra::update_title()
	{
	if(name.size()>0) {
		if(modified)
			set_title("XCadabra: "+name+"*");
		else
			set_title("XCadabra: "+name);
		}
	else {
		if(modified) 
			set_title("XCadabra*");
		else
			set_title("XCadabra");
		}
	}

void XCadabra::on_file_save_as()
	{
	Gtk::FileChooserDialog fd("Save notebook as...", Gtk::FILE_CHOOSER_ACTION_SAVE);
	fd.add_button(Gtk::Stock::SAVE,1);
	fd.add_button(Gtk::Stock::CANCEL,2);
	fd.set_default_response(1);
	int action=fd.run();
	if(action==1) {
		fd.hide_all();
#ifdef DEBUG
		std::cerr << "going to save as " << fd.get_filename() << std::endl;
#endif
		std::string res=save(fd.get_filename());
		if(res.size()>0) {
			Gtk::MessageDialog md("Error saving document "+fd.get_filename());
			md.set_secondary_text(res);
			md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
//			md.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
			md.run();
			}
		else {
			name=fd.get_filename();
			modified=false;
			update_title();
			}
		}
	}

void XCadabra::on_file_print()
	{
	Gtk::MessageDialog md("Printing information");
	md.set_secondary_text("In order to print a notebook file, simply save it and then run LaTeX on it.\n\nCadabra notebook files are at the same time also valid LaTeX files, ready to be processed in any way you would normally process LaTeX files.");
	md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
//	md.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
	md.run();
	}


void XCadabra::clear()
	{
	// Remove all NotebookCanvas objects by removing the 
	// outer-most one.
	mainbox.remove(*canvasses[0]);
	canvasses.clear();

	// Remove all DataCells.
	DataCells_t::iterator it=datacells.begin();
	while(it!=datacells.end()) {
		delete *it;
		++it;
		}
	datacells.clear();

	// Add in a new NotebookCanvas
	canvasses.push_back(manage( new NotebookCanvas(*this) ));
	mainbox.pack_start(*canvasses.back(), Gtk::PACK_EXPAND_WIDGET, 0);
	active_canvas=canvasses.back();
	active_cell=0;
	selected=0;
//	canvasses.back()->to_cdb.connect(sigc::mem_fun(*this, &XCadabra::handle_editbox_output));
	show_all();

	name="";
	modified=false;
	update_title();
	}

// Returns false if backup could not be made.
// Returns true if original file not present or backup ok.
bool XCadabra::make_backup(const std::string& nm) const
	{
	std::ifstream old(nm.c_str());
	std::ofstream temp(std::string(nm+"~").c_str());

	if(!old) return true;

	if(temp) {
		std::string ln;
		while(std::getline(old, ln)) {
			temp << ln << "\n";
			if(!temp) return false;
			}
		return true;
		}
	return false;
	}

std::string XCadabra::save(const std::string& fn) const
	{
	if(!make_backup(fn)) 
		return "Cannot create backup file.";

	std::ofstream str(fn.c_str());

	if(!str) {
		return "Cannot open file for writing.";
		}
	else {
		str << "% Cadabra notebook version 1.1\n"
			 << "\\documentclass[11pt]{article}\n"
			 << "\\usepackage[textwidth=460pt, textheight=660pt]{geometry}\n"
			 << "\\usepackage[usenames]{color}\n"
			 << "\\usepackage{amssymb}\n"
			 << "\\usepackage[parfill]{parskip}\n"
			 << "\\usepackage{breqn}\n"
			 << "\\usepackage{tableaux}\n"
			 << "\\def\\specialcolon{\\mathrel{\\mathop{:}}\\hspace{-.5em}}\n"
			 << "\\renewcommand{\\bar}[1]{\\overline{#1}}\n"
			 << "\\begin{document}\n";
		DataCells_t::const_iterator it=datacells.begin();
		while(it!=datacells.end()) {
			if((*it)->textbuf->size()>0) {
				switch((*it)->cell_type) {
					case DataCell::c_input:
						str << "{\\color[named]{Blue}\\begin{verbatim}\n"
							 << trim( (*it)->textbuf->get_text() )
							 << "\n\\end{verbatim}}\n";
						break;
					case DataCell::c_output:
						str << "% orig\n";
						str << "% " << (*it)->cdbbuf << "\n";
						str << "% end_orig\n";
						str << "\\begin{dmath*}[compact, spread=2pt]\n"
							 << trim( (*it)->textbuf->get_text()  )
							 << "\n\\end{dmath*}\n";
						break;
					case DataCell::c_comment:
						str << "\\begin{verbatim}\n"
							 << trim( (*it)->textbuf->get_text() )
							 << "\n\\end{verbatim}\n";
						break;
					case DataCell::c_tex:
						str << "% Begin TeX cell "
							 << ( (*it)->tex_hidden==true ? "closed\n":"open\n" );
						if((*it)->sectioning>0) 
							str << "\\section{" << trim( (*it)->textbuf->get_text() ) << "}";
						else str << trim( (*it)->textbuf->get_text() );
						str << "\n% End TeX cell\n";
						break;
					case DataCell::c_error:
						str << "{\\color[named]{Red}%\n"
							 << trim( (*it)->textbuf->get_text() ) << "%\n"
							 << "} % error\n";
						break;
					}
				}
			++it;
			}
		str << "\\end{document}\n";
		}
	return "";
	}

std::string XCadabra::expo(const std::string& fn) const
	{
	if(!make_backup(fn)) 
		return "Cannot create backup file.";

	std::ofstream str(fn.c_str());

	if(!str) {
		return "Cannot open file for writing.";
		}
	else {
		str << "# Exported Cadabra notebook\n";
		DataCells_t::const_iterator it=datacells.begin();
		while(it!=datacells.end()) {
			if((*it)->textbuf->size()>0) {
				switch((*it)->cell_type) {
					case DataCell::c_input:
						str << trim( (*it)->textbuf->get_text() ) << "\n\n";
						break;
					case DataCell::c_tex:
//						str << "% Begin TeX cell "
//							 << ( (*it)->tex_hidden==true ? "closed\n":"open\n" );
//						if((*it)->sectioning>0) 
//							str << "\\section{" << trim( (*it)->textbuf->get_text() ) << "}";
//						else str << trim( (*it)->textbuf->get_text() );
//						str << "\n% End TeX cell\n";
						break;
					case DataCell::c_error:
					case DataCell::c_output:
					case DataCell::c_comment:
						break;
						break;
					}
				}
			++it;
			}
		str << "\n";
		}
	return "";
	}

std::string XCadabra::load(const std::string& fn, bool ignore_nonexistence)
	{
	struct stat buf;
	int statres=lstat(fn.c_str(), &buf);
	if(statres==-1) {
		 switch(errno) {
			  case EACCES:
					return "Search permission denied.";
			  case ELOOP:
					return "Too many symbolic links.";
			  case ENOENT:
					if(ignore_nonexistence) return "";
					else return "File does not exist.";
			  default:
					return "Error stat'ing file.";
			  }
		 }
	
	std::ifstream str(fn.c_str());
	std::ostringstream err;

	if(str.is_open()==false) {
		return "Read permission denied.";
		}
	else {
		std::string ln;
		std::getline(str,ln);
		if(ln!="% Cadabra notebook version 1.0") {
			 if(ln!="% Cadabra notebook version 1.1") {
				  return "Not in Cadabra notebook version <= 1.1 format.";
				  }
			}

		clear();

		enum state_t { s_top, s_input, s_output, s_comment, s_tex, s_error, s_output_as_cdb };
		state_t curstat=s_top;
		std::string buffer, cdb_buffer;
		int line_num=2;
		bool tex_hidden=false;

		b_cdbstatus.set_text(" Status: Loading notebook...");
		while(std::getline(str,ln)) {
#ifdef DEBUG
			std::cerr << "read: " << ln << std::endl;
#endif
			if(ln=="{\\color[named]{Blue}\\begin{verbatim}") {
				if(curstat!=s_top) {
					err << "Illegal location of input cell at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
				curstat=s_input;
				buffer="";
				}
			else if(ln=="\\end{verbatim}}") {
				if(curstat!=s_input) {
					err << "Unmatched input cell closing at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
				DataCell *newcell=new DataCell(DataCell::c_input, buffer);
				add_cell(newcell);
				curstat=s_top;
				}
			else if(ln=="{\\color[named]{Red}%") {
				if(curstat!=s_top) {
					err << "Illegal location of error cell at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
				curstat=s_error;
				buffer="";
				}
			else if(ln=="} % error") {
				if(curstat!=s_error) {
					err << "Unmatched error cell closing at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
				DataCell *newcell=new DataCell(DataCell::c_error, buffer);
				add_cell(newcell);
				curstat=s_top;
				}
			else if(ln.substr(0,6)=="% orig") {
				if(curstat!=s_top) {
					err << "Illegal location of output cell in Cadabra input format at line " 
						 << line_num << ".";
					kernel_idle();
					return err.str();
					}
				curstat=s_output_as_cdb;
				buffer="";
				cdb_buffer="";
				}
			else if(ln.substr(0,10)=="% end_orig") {
				if(curstat!=s_output_as_cdb) {
					err << "Unmatched output cell in Cadabra input format closing at line " 
						 << line_num << ".";
					kernel_idle();
					return err.str();
					}
				curstat=s_top;
				cdb_buffer=buffer;
				buffer="";
				}
			else if(ln.substr(0,14)=="\\begin{dmath*}") {
				if(curstat!=s_top) {
					err << "Illegal location of output cell at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
				curstat=s_output;
				buffer="";
				}
			else if(ln=="\\end{dmath*}") {
				if(curstat!=s_output) {
					err << "Unmatched output cell closing at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
#ifdef DEBUG
				std::cerr << buffer << std::endl;
#endif
				DataCell *newcell=new DataCell(DataCell::c_output, buffer);
				if(cdb_buffer.size()>2)
					newcell->cdbbuf=cdb_buffer.substr(2);
				cdb_buffer="";
				add_cell(newcell);
				curstat=s_top;
				}
			else if(ln=="\\begin{verbatim}") {
				if(curstat!=s_top) {
					err << "Illegal location of comment cell at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
				curstat=s_comment;
				buffer="";
				}
			else if(ln=="\\end{verbatim}") {
				if(curstat!=s_comment) {
					err << "Unmatched comment cell closing at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
				DataCell *newcell=new DataCell(DataCell::c_comment, buffer);
				add_cell(newcell);
				curstat=s_top;
				}
			else if(ln=="% Begin TeX cell open") {
				if(curstat!=s_top) {
					err << "Illegal location of TeX cell at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
				curstat=s_tex;
				tex_hidden=false;
				buffer="";
				}
			else if(ln=="% Begin TeX cell closed") {
				if(curstat!=s_top) {
					err << "Illegal location of TeX cell at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
				curstat=s_tex;
				tex_hidden=true;
				buffer="";
				}
			else if(ln=="% End TeX cell") {
				if(curstat!=s_tex) {
					err << "Unmatched TeX cell closing at line " << line_num << ".";
					kernel_idle();
					return err.str();
					}
				DataCell *newcell=new DataCell(DataCell::c_tex, trim(buffer));
				newcell->tex_hidden=tex_hidden;
				add_cell(newcell);
				curstat=s_top;
				}
			else buffer+=ln+"\n";

			++line_num;
			
			while (gtk_events_pending ())
				gtk_main_iteration ();
			}
		}

//	show_all();
	kernel_idle();
	return "";
	}

void XCadabra::on_file_quit()
	{
	if(quit_safeguard()) {
		cdb.terminate();
		Gtk::Main::quit();
		}
	}

bool XCadabra::quit_safeguard(bool quit)
	{
	if(modified) {
		std::string mes;
		if(quit) {
			if(name.size()>0) mes="Save changes to "+name+" before closing?";
			else              mes="Save changes before closing?";
			}
		else {
			if(name.size()>0) mes="Save changes to "+name+" before continuing?";
			else              mes="Save changes before continuing?";
			}
		Gtk::MessageDialog md(mes, false, Gtk::MESSAGE_WARNING, 
									 Gtk::BUTTONS_NONE, true);
		md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
//		md.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
		md.add_button(Gtk::Stock::SAVE,1);
		md.add_button(Gtk::Stock::CANCEL,2);
		if(quit)
			md.add_button(Gtk::Stock::QUIT,3);
		else 
			md.add_button(Gtk::Stock::NO, 3);
		int action=md.run();
		switch(action) {
			case 1: 
				on_file_save();
				return true;
			case 2:
				break;
			case 3:
				return true;
			}
		}
	else return true;

	return false;
	}

void XCadabra::on_edit_copy()
	{
	}

void XCadabra::on_edit_paste()
	{
	}

void XCadabra::on_edit_insert_tex_above()
	{
	DataCell *newcell=new DataCell(DataCell::c_tex, "[empty TeX cell]");
	active_canvas->cell_grab_focus(
		add_cell(newcell, active_cell->datacell) );
	}

void XCadabra::on_edit_insert_tex_below()
	{
	DataCell *newcell=new DataCell(DataCell::c_tex, "[empty TeX cell]");
	active_canvas->cell_grab_focus(
		add_cell(newcell, active_cell->datacell, false) );
	}

void XCadabra::on_edit_insert_input_above()
	{
	DataCell *newcell=new DataCell(DataCell::c_input, "");
	active_canvas->cell_grab_focus(
		add_cell(newcell, active_cell->datacell) );
	}

void XCadabra::on_edit_insert_input_below()
	{
	DataCell *newcell=new DataCell(DataCell::c_input, "");
	active_canvas->cell_grab_focus(
		add_cell(newcell, active_cell->datacell, false) );
	}

void XCadabra::on_edit_insert_section_above()
	{
	DataCell *newcell=new DataCell(DataCell::c_tex, "[insert section header]");
	newcell->sectioning=1;
	active_canvas->cell_grab_focus(
		add_cell(newcell, active_cell->datacell) );
	}

void XCadabra::on_edit_remove_cell()
	{
	DataCell *dc=active_cell->datacell;
	if(dc->cell_type!=DataCell::c_input && dc->cell_type!=DataCell::c_tex) return;

	selected=0;
	DataCells_t::iterator fnd=std::find(datacells.begin(), datacells.end(), dc);
	if(fnd==datacells.end()) return;

	// Remove cells until an input or tex cell is found.
	while(fnd!=datacells.end()) {
		for(unsigned int i=0; i<canvasses.size(); ++i)  
			canvasses[i]->remove_cell(*fnd);
		fnd=datacells.erase(fnd);
		if(fnd==datacells.end() || (*fnd)->cell_type==DataCell::c_input || (*fnd)->cell_type==DataCell::c_tex)
			 break;
		}
#ifdef DEBUG
	std::cerr << "all widgets removed" << std::endl;
#endif
	if(fnd!=datacells.end()) {
#ifdef DEBUG
		std::cerr << "grabbing focus" << std::endl;
#endif
		// We have to put the cursor in the next input cell or in the next open TeX cell.
		// So walk down the cells until we meet this condition (or end).
		while(fnd!=datacells.end() 
				&& (*fnd)->cell_type!=DataCell::c_input 
				&& !( (*fnd)->cell_type==DataCell::c_comment && (*fnd)->tex_hidden==false) )
			 ++fnd;

		if(fnd!=datacells.end())
			 active_canvas->cell_grab_focus(*fnd);
		}
	else {
#ifdef DEBUG
		std::cerr << "adding new cell" << std::endl;
#endif
		DataCell *newcell=new DataCell(DataCell::c_input);
		active_canvas->cell_grab_focus(
			add_cell(newcell, active_cell->datacell) );
		}
	}

void XCadabra::on_edit_divide_cell()
	{
	DataCell *dc=active_cell->datacell;
	if(dc->cell_type!=DataCell::c_input) return;

	selected=0;
	DataCells_t::iterator fnd=std::find(datacells.begin(), datacells.end(), dc);
	if(fnd==datacells.end()) return;

#ifdef DEBUG
	std::cerr << "dividing cell" << std::endl;
#endif

	// Find the position of the cursor.
	std::string segment1=
		trim(
			dc->textbuf->get_slice(dc->textbuf->begin(), 
										  dc->textbuf->get_iter_at_mark(dc->textbuf->get_insert())));
	std::string segment2=
		trim(
			dc->textbuf->get_slice(dc->textbuf->get_iter_at_mark(dc->textbuf->get_insert()), 
										  dc->textbuf->end()));
	
	if(segment1.size()==0 || segment2.size()==0) return;

	remove_noninput_below(dc);
	dc->textbuf->erase(dc->textbuf->get_iter_at_mark(dc->textbuf->get_insert()), 
										  dc->textbuf->end());

	DataCell *newcell=new DataCell(DataCell::c_input, segment2);
	add_cell(newcell, dc, false);
	}

void XCadabra::on_view_split()
	{
	add_canvas();
	}

void XCadabra::on_view_close()
	{
	if(canvasses.size()>1) {
		canvasses[canvasses.size()-2]->remove(*canvasses.back());
		canvasses.pop_back();
		}
	}

void XCadabra::on_settings_font_size(int num)
	{
	if(font_step==num) return;

	font_step=num;

	std::string res=save_config();
	if(res.size()>0) {
		 Gtk::MessageDialog md("Error");
		 md.set_secondary_text(res);
		 md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
		 md.run();
		 }

	// Update all TeXBuffers, i.e. rerun tex & dvipng on them.
	DataCells_t::iterator it=datacells.begin();
	while(it!=datacells.end()) {
		if((*it)->cell_type==DataCell::c_output || (*it)->cell_type==DataCell::c_tex 
			|| (*it)->cell_type==DataCell::c_error || (*it)->cell_type==DataCell::c_comment) {
			(*it)->texbuf->font_size=12+(num*2);
			(*it)->texbuf->regenerate();
			}
		++it;
		}

	// Update all VisualCells.
	for(unsigned int i=0; i<canvasses.size(); ++i) 
		canvasses[i]->redraw_cells();
	}

void XCadabra::on_tutorial_open(unsigned int num)
	{
	Gtk::MessageDialog md("Tutorial information");
	md.set_secondary_text("Tutorials are not yet available, sorry.");
	md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
	md.run();
	}

void XCadabra::on_clipboard_get(Gtk::SelectionData& selection_data, guint info) 
	{ 
	const Glib::ustring target = selection_data.get_target(); 
	
	if(target == "cadabra")
		selection_data.set("cadabra", clipboard_cdb);
	else if(target == "UTF8_STRING" || target=="TEXT") {
		selection_data.set_text(clipboard_txt);
		}
	}

void XCadabra::on_clipboard_clear()
	{
	}

std::string XCadabra::save_config() const
	{
	std::string defname=getenv("HOME");
	defname+="/.xcadabra";
	std::ofstream conf(defname.c_str(), std::ofstream::out);
	if(conf.fail()) 
		 return "Cannot open ~/.xcadabra for writing.";

	conf << "# XCadabra configuration file version 1.0" << std::endl;
	conf << "font_step:=" << font_step << std::endl;
	conf.close();
	return "";
	}

std::string XCadabra::load_config()
	{
	std::string defname=getenv("HOME");
	defname+="/.xcadabra";
	std::ifstream conf(defname.c_str());
	std::ostringstream str;
	std::string rl;
	int line=1;
	if(conf.is_open()) {
		 while(getline(conf, rl)) {
			  if(rl.size()>0 && rl[0]=='#')
					continue;

			  unsigned int pos=rl.find(":=");
			  if(pos==std::string::npos) {
					str << "Error parsing ~/.xcadabra on line " << line << std::endl;
					return str.str();
					}
			  if(rl.substr(0,pos)=="font_step") {
					int tmp=atoi(rl.substr(pos+2).c_str());
					if(tmp<-2 || tmp>10) {
						 str << "Out-of-bounds value for " << rl.substr(0,pos) 
							  << " in ~/.xcadabra on line " << line << std::endl;
						 return str.str();
						 }
					font_step=tmp;
					}
			  else {
					str << "Unknown identifier " << rl.substr(0,pos) 
						 << " in ~/.xcadabra line " << line << std::endl;
					return str.str();
					}
			  ++line;
			  }
		 }
	return "";
	}


std::string CadabraHelp::texify(const std::string& str) const
	{
	std::string res;
	for(unsigned int i=0; i<str.size(); ++i) {
		if(str[i]=='_') res+='\\';
		res+=str[i];
		}
	return res;
	}

std::string XCadabra::duplicate_underscores(const std::string& str) const
	{
	std::string res;
	for(unsigned int i=0; i<str.size(); ++i) {
		if(str[i]=='_') res+='_';
		res+=str[i];
		}
	return res;
	}

void XCadabra::add_property_help(const std::string& property)
	{
	actiongroup->add( Gtk::Action::create(property, duplicate_underscores(property)),
							sigc::bind<std::string>(
								 sigc::mem_fun(*this, &XCadabra::on_help_properties), property) );
	uimanager->add_ui_from_string("<ui><menubar name='MenuBar'><menu action='MenuHelp'><menu action='AllProperties'><menuitem action='"+property+"'/></menu></menu></menubar></ui>");
	property_set.insert(property.substr(2));
	}

void XCadabra::add_algorithm_help(const std::string& algorithm)
	{
	actiongroup->add( Gtk::Action::create(algorithm, duplicate_underscores(algorithm)),
							sigc::bind<std::string>(
								 sigc::mem_fun(*this, &XCadabra::on_help_algorithms), algorithm) );   
	uimanager->add_ui_from_string("<ui><menubar name='MenuBar'><menu action='MenuHelp'><menu action='AllAlgorithms'><menuitem action='"+algorithm+"'/></menu></menu></menubar></ui>");
	algorithm_set.insert(algorithm.substr(1));
	}

void XCadabra::add_reserved_help(const std::string& reserved)
	{
	actiongroup->add( Gtk::Action::create(reserved, duplicate_underscores(reserved)),
							sigc::bind<std::string>(
								 sigc::mem_fun(*this, &XCadabra::on_help_reserved), reserved) );   
	uimanager->add_ui_from_string("<ui><menubar name='MenuBar'><menu action='MenuHelp'><menu action='AllReserved'><menuitem action='"+reserved+"'/></menu></menu></menubar></ui>");
	}
