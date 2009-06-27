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

#include "widgets.hh"
#include <cassert>
#include <modglue/pipe.hh>
#include <modglue/process.hh>
#include <fstream>
#include <gtkmm/messagedialog.h>
#include <gtkmm/main.h>
#include <stdexcept>
#include <cairomm/cairomm.h>
#include <gdkmm/general.h>
#include <pcrecpp.h>


// General tool to strip spaces from both ends
std::string trim(const std::string& s) 
	{
	if(s.length() == 0)
		return s;
	int b = s.find_first_not_of(" \t\n");
	int e = s.find_last_not_of(" \t\n");
	if(b == -1) // No non-spaces
		return "";
	return std::string(s, b, e - b + 1);
	}

double TeXEngine::millimeter_per_inch = 25.4;

TeXBuffer::TeXBuffer(Glib::RefPtr<Gtk::TextBuffer> tb, TeXEngine& engine)
	: tex_source(tb), tex_request(0), engine_(engine)
	{
	}

TeXBuffer::~TeXBuffer()
	{
	if(tex_request)
		engine_.checkout(tex_request);
	}

void TeXBuffer::generate(const std::string& startwrap, const std::string& endwrap)
	{		
	tex_request = engine_.checkin(tex_source->get_text(), startwrap, endwrap);
	}

void TeXBuffer::regenerate()
	{
	assert(tex_request);
	engine_.modify(tex_request, tex_source->get_text());
	}

TeXEngine::TeXException::TeXException(const std::string& str)
	: std::logic_error(str)
	{
	}

Glib::RefPtr<Gdk::Pixbuf> TeXEngine::get_pixbuf(TeXEngine::TeXRequest *req)
	{
	if(req==0) // return empty pixbuf
		return Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 1, 1);

	if(req->needs_generating)
		convert_one(req);
	return req->pixbuf;
	}

void TeXEngine::erase_file(const std::string& nm) const
	{
	unlink(nm.c_str());
	}

std::string TeXEngine::handle_latex_errors(const std::string& result) const
	{
	std::string::size_type pos=result.find("! LaTeX Error");
	if(pos != std::string::npos) {
		 return "LaTeX error, probably a missing style file. See the output below.\n\n" +result;
		 }
	
	pos=result.find("! TeX capacity exceeded");
	if(pos != std::string::npos) {
		 return "Output cell too large (TeX capacity exceeded), output suppressed.";
		 }
	
	pos=result.find("! Undefined control sequence");
	if(pos != std::string::npos) {
		 std::string::size_type undefpos=result.find("\n", pos+30);
		 if(undefpos==std::string::npos) 
			  return "Undefined control sequence (failed to parse LaTeX output).";
		 std::string::size_type backslashpos=result.rfind("\\", undefpos);
		 if(backslashpos==std::string::npos || backslashpos < 2) 
			  return "Undefined control sequence (failed to parse LaTeX output).";

		 std::string undefd=result.substr(backslashpos-1,undefpos-pos-30);
		 return "Undefined control sequence:\n\n" +undefd+"\nNote that all symbols which you use in cadabra have to be valid LaTeX expressions. If they are not, you can still use the LaTeXForm property to make them print correctly; see the manual for more information.";
		 }

	return "";
	}

TeXEngine::~TeXEngine()
	{
	// Delete all requests.
	std::set<TeXRequest *>::iterator it=requests.begin();
	while(it!=requests.end()) {
		delete (*it);
		++it;
		}
	}

TeXEngine::TeXEngine()
	: horizontal_pixels_(800), font_size_(12)
	{
	}

void TeXEngine::set_geometry(int horpix)
	{
	if(horizontal_pixels_!=horpix) {
		// flag all requests as requiring an update
		std::set<TeXRequest *>::iterator reqit=requests.begin();
		while(reqit!=requests.end()) {
			(*reqit)->needs_generating=true;
			++reqit;
			}
		}
	horizontal_pixels_=horpix;
	}

void TeXEngine::set_font_size(int fontsize)
	{
	if(font_size_!=fontsize) {
		// flag all requests as requiring an update
		std::set<TeXRequest *>::iterator reqit=requests.begin();
		while(reqit!=requests.end()) {
			(*reqit)->needs_generating=true;
			++reqit;
			}
		}
	font_size_=fontsize;
	}

TeXEngine::TeXRequest::TeXRequest()
	: needs_generating(true)
	{
	}

TeXEngine::TeXRequest *TeXEngine::checkin(const std::string& txt,
																const std::string& startwrap, const std::string& endwrap)
	{
	TeXRequest *req = new TeXRequest;
	req->latex_string=txt;
	req->start_wrap=startwrap;
	req->end_wrap=endwrap;
	req->needs_generating=true;
	requests.insert(req);
	return req;
	}

void TeXEngine::checkout(TeXRequest *req)
	{
	std::set<TeXRequest *>::iterator it=requests.find(req);
	assert(it!=requests.end());
	requests.erase(it);
	}

TeXEngine::TeXRequest *TeXEngine::modify(TeXRequest *req, const std::string& txt)
	{
	req->latex_string=txt;
	req->needs_generating=true;
	return req;
	}

void TeXEngine::convert_all()
	{
	if(requests.size()!=0) 
		convert_set(requests);
	}

void TeXEngine::convert_one(TeXRequest *req)
	{
	std::set<TeXRequest *> reqset;
	reqset.insert(req);
	convert_set(reqset);
	}

void TeXEngine::convert_set(std::set<TeXRequest *>& reqs)
	{
	// We now follow
	// 
	// https://www.securecoding.cert.org/confluence/display/seccode/FI039-C.+Create+temporary+files+securely
	// 
	// for temporary files.

	char olddir[1024];
	if(getcwd(olddir, 1023)==NULL)
		 olddir[0]=0;
	if(chdir("/tmp")==-1)
		throw TeXException("Failed to chdir to /tmp.");

	char templ[]="/tmp/cdbXXXXXX";

	// The size in mm or inches which we use will in the end determine how large
	// the font will come out. 
	//
	// For given horizontal size, we stretch this up to the full window
	// width using horizontal_pixels/(horizontal_size/millimeter_per_inch) dots per inch.
	// The appropriate horizontal size in mm is determined by trial and error, 
	// and of course scales with the number of horizontal pixels.

	const double horizontal_mm=horizontal_pixels_*(12.0/font_size_)/3.94;
//#ifdef DEBUG
//	std::cerr << "tex_it: font_size " << font_size << std::endl
//				 << "        pixels    " << horizontal_pixels_ << std::endl
//				 << "        mm        " << horizontal_mm << std::endl;
//#endif

	//(int)(millimeter_per_inch*horizontal_pixels/100.0); //140;
	const double vertical_mm=10*horizontal_mm;


	// Write each string in the set of requests into a buffer, separating
	// them by a page eject.

	std::ostringstream total;
	int fd = mkstemp(templ);
	if(fd == -1) 
		 throw TeXException("Failed to create temporary file in /tmp.");

	total << "\\documentclass[12pt]{article}\n"
			<< "\\usepackage[dvips,verbose,voffset=0pt,hoffset=0pt,textwidth="
			<< horizontal_mm << "mm,textheight="
			<< vertical_mm << "mm]{geometry}\n"
			<< "\\usepackage{color}\\usepackage{amssymb}\n"
			<< "\\usepackage[parfill]{parskip}\n\\usepackage{tableaux}\n";

	for(size_t i=0; i<latex_packages.size(); ++i)
		total << "\\usepackage{" << latex_packages[i] << "}\n";

	total	<< "\\def\\specialcolon{\\mathrel{\\mathop{:}}\\hspace{-.5em}}\n"
			<< "\\renewcommand{\\bar}[1]{\\overline{#1}}\n"
			<< "\\begin{document}\n\\pagestyle{empty}\n";

	std::set<TeXRequest *>::iterator reqit=reqs.begin();
	while(reqit!=reqs.end()) {
		if((*reqit)->needs_generating) {
			if((*reqit)->latex_string.size()>100000)
				total << "Expression too long, output suppressed.\n";
			else {
				if((*reqit)->start_wrap.size()>0) 
					total << (*reqit)->start_wrap;
				total << (*reqit)->latex_string;
				if((*reqit)->end_wrap.size()>0)
					total << "\n" << (*reqit)->end_wrap;
				else total << "\n";
				}
			total << "\\eject\n";
			}
		++reqit;
		}
	total << "\\end{document}\n";


	// Now write the 'total' buffer to the .tex file

	ssize_t start=0;
	do {
		ssize_t written=write(fd, &(total.str().c_str()[start]), total.str().size()-start);
		if(written>=0)
			start+=written;
		else {
			if(errno != EINTR) {
				close(fd);
				throw TeXException("Failed to write LaTeX temporary file.");
				}
			} 
		} while(start<static_cast<ssize_t>(total.str().size()));
	close(fd);
#ifdef DEBUG
	std::cerr  << templ << std::endl;
	std::cerr << "---\n" << total.str() << "\n---" << std::endl;
#endif

	std::string nf=std::string(templ)+".tex";
	rename(templ, nf.c_str());

#ifdef __CYGWIN__
	// MikTeX does not see /tmp, it needs \cygwin\tmp
	nf="\\cygwin"+nf;
	pcrecpp::RE("/").GlobalReplace("\\\\", &nf);
#endif

	// Run LaTeX on the .tex file.

	modglue::child_process latex_proc("latex");
	latex_proc << "--interaction" << "nonstopmode" << nf;
	std::string result;
	try {
		latex_proc.call("", result);
		erase_file(std::string(templ)+".tex");
		erase_file(std::string(templ)+".aux");
		erase_file(std::string(templ)+".log");
#ifdef DEBUG		
		std::cerr << result << std::endl;
#endif

		std::string err=handle_latex_errors(result);

		if(err.size()>0) {
			reqit=reqs.begin();
			while(reqit!=reqs.end()) 
				(*reqit++)->needs_generating=false;
			 erase_file(std::string(templ)+".dvi");
			 if(chdir(olddir)==-1)
				 throw TeXException(err+" (and cannot chdir back to original "+olddir+")");
			 throw TeXException(err); 
			 }
		}
	catch(std::logic_error& err) {
		erase_file(std::string(templ)+".tex");
		erase_file(std::string(templ)+".dvi");
		erase_file(std::string(templ)+".aux");
		erase_file(std::string(templ)+".log");
		
		std::string err=handle_latex_errors(result);
		reqit=reqs.begin();
		while(reqit!=reqs.end()) 
			(*reqit++)->needs_generating=false;

		if(err.size()>0) {
			 if(chdir(olddir)==-1)
				 throw TeXException(err+" (and cannot chdir back to original "+olddir+")");
			 throw TeXException(err); 
			 }

		// Even if we cannot find an explicit error in the output, we have to terminate
		// since LaTeX has thrown an exception.
		if(chdir(olddir)==-1)
			throw TeXException("Cannot start LaTeX, is it installed? (and cannot chdir back to original)");
		throw TeXException("Cannot start LaTeX, is it installed?");
		}

	// Convert the entire dvi file to png files.
	//
	modglue::child_process dvipng_proc("dvipng");
	std::ostringstream rgbspec, resspec;
	dvipng_proc << "-T" << "tight" << "-bg" << "Transparent"; // << "-fg";
//	rgbspec << "\"rgb "
//			  << foreground_colour.get_red()/65536.0 << " "
//			  << foreground_colour.get_green()/65536.0 << " "
//			  << foreground_colour.get_blue()/65536.0 << "\"";
//	dvipng_proc << rgbspec.str();
	dvipng_proc << "-D";
	resspec << horizontal_pixels_/(1.0*horizontal_mm)*millimeter_per_inch;
	dvipng_proc << resspec.str() << std::string(templ)+".dvi";

	try {
		dvipng_proc.call("", result);
#ifdef DEBUG
	std::cerr << result << std::endl;
#endif
		}
	catch(std::logic_error& ex) {
		// Erase all dvi and png files and put empty pixbufs into the TeXRequests.
		erase_file(std::string(templ)+".dvi");
		reqit=reqs.begin();
		int pagenum=1;
		while(reqit!=reqs.end()) {
			if((*reqit)->needs_generating) {
				std::ostringstream pngname;
				pngname << std::string(templ) << pagenum << ".png";
				erase_file(pngname.str());
				(*reqit)->pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 1, 1);
				(*reqit)->needs_generating=true;
				++pagenum;
				}
			++reqit;
			}
		if(chdir(olddir)==-1)
			throw TeXException(
				std::string("Cannot run dvipng, is it installed? (and cannot chdir back to original)\n\n")+ex.what());
		throw TeXException(std::string("Cannot run dvipng, is it installed?\n\n")+ex.what());
		}

	erase_file(std::string(templ)+".dvi");

	// Conversion completed successfully, now convert all resulting PNG files to Pixbuf images.

	reqit=reqs.begin();
	int pagenum=1;
	while(reqit!=reqs.end()) {
		if((*reqit)->needs_generating) {
			std::ostringstream pngname;
			pngname << std::string(templ) << pagenum << ".png";
			std::ifstream tst(pngname.str().c_str());
			if(tst.good()) {
				(*reqit)->pixbuf = Gdk::Pixbuf::create_from_file(pngname.str());
				(*reqit)->needs_generating=false;
				erase_file(pngname.str());
				}
			++pagenum;
			}
		++reqit;
		}

	if(chdir(olddir)==-1)
		throw TeXException("Failed to chdir back to " +std::string(olddir)+".");
	}

TeXView::TeXView(Glib::RefPtr<TeXBuffer> texb, int hmargin)
	: texbuf(texb), vbox(false, 10), hbox(false, hmargin)
	{
	add(vbox);
	vbox.pack_start(hbox, Gtk::PACK_SHRINK, 0);
	hbox.pack_start(image, Gtk::PACK_SHRINK, hmargin);
//	set_state(Gtk::STATE_PRELIGHT);
	modify_bg(Gtk::STATE_NORMAL, Gdk::Color("white"));
	}

void TeXView::on_show()
	{
	image.set(texbuf->get_pixbuf());
	
	Gtk::EventBox::on_show();
	}

void TeXView::update_image()
	{
	image.set(texbuf->get_pixbuf());
	}


ExpressionInput::exp_input_tv::exp_input_tv(Glib::RefPtr<Gtk::TextBuffer> tb)
	: Gtk::TextView(tb)
	{
//	get_buffer()->signal_insert().connect(sigc::mem_fun(this, &exp_input_tv::on_my_insert), false);
//	get_buffer()->signal_erase().connect(sigc::mem_fun(this, &exp_input_tv::on_my_erase), false);
	}

ExpressionInput::ExpressionInput(Glib::RefPtr<Gtk::TextBuffer> tb, const std::string& fontname, int hmargin)
	: edit(tb)
	{
//	scroll_.set_size_request(-1,200);
//	scroll_.set_border_width(1);
//	scroll_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
	edit.modify_font(Pango::FontDescription(fontname)); 
	edit.set_wrap_mode(Gtk::WRAP_WORD);
	edit.modify_text(Gtk::STATE_NORMAL, Gdk::Color("blue"));
	edit.set_pixels_above_lines(LINE_SPACING);
	edit.set_pixels_below_lines(LINE_SPACING);
	edit.set_pixels_inside_wrap(2*LINE_SPACING);
	edit.set_left_margin(hmargin);
	edit.set_accepts_tab(false);

	edit.signal_button_press_event().connect(sigc::mem_fun(this, 
																				&ExpressionInput::handle_button_press), 
															 false);
//	edit.get_buffer()->signal_changed().connect(sigc::mem_fun(this, &ExpressionInput::handle_changed));


//	add(hbox);
//	hbox.add(vsep);
//	hbox.add(edit);
	add(edit);
//	set_border_width(3);
	}

bool ExpressionInput::exp_input_tv::on_key_press_event(GdkEventKey* event)
	{
//	std::cerr << event->keyval << ", " << event->state << " pressed" << std::endl;
	if(get_editable() && event->keyval==GDK_Return && (event->state&Gdk::SHIFT_MASK)) {// shift-return
		Glib::RefPtr<Gtk::TextBuffer> textbuf=get_buffer();
		std::string tmp(trim(textbuf->get_text(get_buffer()->begin(), get_buffer()->end())));
		// Determine whether this is a valid input cell: should end either on a delimiter or
		// on a delimeter-space-quoted-file-name combination.
		bool is_ok=false;
		if(tmp.size()>0) {
			 if(tmp[0]!='#' && tmp[tmp.size()-1]!=';' && tmp[tmp.size()-1]!=':' && tmp[tmp.size()-1]!='.' 
				 && tmp[tmp.size()-1]!='\"') {
				  is_ok=false;
				  }
			 else is_ok=true;
			 }
		if(!is_ok) {
			 Gtk::MessageDialog md("Input error");
			 md.set_secondary_text("This cell does not end with a delimiter (a \":\", \";\" or \".\")");
			 md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
			 md.run();
			 }
		else {
#ifdef DEBUG
			 std::cerr << "sending: " << tmp << std::endl;
#endif
			 content_changed();
			 emitter(tmp);
			 }
		return true;
		}
	else {
		bool retval=Gtk::TextView::on_key_press_event(event);
		while (gtk_events_pending ())
			gtk_main_iteration ();

		// If this was a real key press (i.e. not just SHIFT or ALT or similar), emit a
		// signal so that the cell can be scrolled into view if necessary.
		// FIXME: I do not know how to do this correctly, check docs.

		if(event->keyval < 65000L)
			 content_changed();
		return retval;
		}
	}

bool ExpressionInput::handle_button_press(GdkEventButton* button)
	{
	if(button->button!=2) return false;

	Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);

	std::vector<Glib::ustring> sah=refClipboard->wait_for_targets();
	bool hastext=false;
	bool hasstring=false;
	Gtk::SelectionData sd;

	// find out _where_ to insert
	Gtk::TextBuffer::iterator insertpos;
	int somenumber;
	edit.get_iter_at_position(insertpos, somenumber, button->x, button->y);
	++insertpos;

	for(unsigned int i=0; i<sah.size(); ++i) {
		 if(sah[i]=="cadabra") {
			  sd=refClipboard->wait_for_contents("cadabra");
			  std::string topaste=sd.get_data_as_string();
			  insertpos=edit.get_buffer()->insert(insertpos, topaste);
			  edit.get_buffer()->place_cursor(insertpos);
			  return true;
			  }
		 else if(sah[i]=="TEXT")
			  hastext=true;
		 else if(sah[i]=="STRING")
			  hasstring=true;
		 }
	
	if(hastext)        sd=refClipboard->wait_for_contents("TEXT");
	else if(hasstring) sd=refClipboard->wait_for_contents("STRING");
	if(hastext || hasstring) {
		 insertpos=edit.get_buffer()->insert(insertpos, sd.get_data_as_string());
		 edit.get_buffer()->place_cursor(insertpos);
		 }

	return true;
	}

bool ExpressionInput::exp_input_tv::on_expose_event(GdkEventExpose *event)
	{
	Glib::RefPtr<Gdk::Window> win = Gtk::TextView::get_window(Gtk::TEXT_WINDOW_TEXT);

	bool ret=Gtk::TextView::on_expose_event(event);

	int w, h, x, y, d;
	win->get_geometry(x,y,w,h,d);
//	std::cout << w << ", " << h << std::endl;

	Cairo::RefPtr<Cairo::Context> cr = win->create_cairo_context();
//	if(event) {
//		// clip to the area that needs to be re-exposed so we don't draw any
//		// more than we need to.
//		std::cout << event->area.width << " x " << event->area.height << std::endl;
//		cr->rectangle(event->area.x, event->area.y,
//						  event->area.width, event->area.height);
//      cr->clip();
//		}

	// paint the background
	cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
	cr->rectangle(5,3,8,h-3);
	cr->fill();

	cr->set_source_rgba(.2, .2, .7, 1.0);
	cr->set_line_width(1.0);
	cr->set_antialias(Cairo::ANTIALIAS_NONE);
	cr->move_to(8,3);
	cr->line_to(5,3);
	cr->line_to(5,h-3); 
	cr->line_to(8,h-3); 
	cr->stroke();
	
	return ret;
	}

//	gdouble page_inc, step_inc, upper, lower, pos;
//
//	GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(
//			GTK_SCROLLED_WINDOW(scrolled_window));
//
//	page_inc = vadj->page_increment;
//	step_inc = vadj->step_increment;
//	lower = vadj->lower;
//
//	/* Otherwise we sometimes scroll down into a page of black. */
//	upper = vadj->upper - page_inc - step_inc;
//
//	/* Center on the widget. */
//	pos = (gdouble)widget->allocation.y - page_inc/2;
//
//	gtk_adjustment_set_value(vadj, CLAMP(pos, lower, upper));
//

TeXInput::exp_input_tv::exp_input_tv(Glib::RefPtr<Gtk::TextBuffer> tb)
	: Gtk::TextView(tb), folded_away(false)
	{
	}

TeXInput::TeXInput(Glib::RefPtr<Gtk::TextBuffer> tb, Glib::RefPtr<TeXBuffer> texb, const std::string& fontname)
	: edit(tb), texview(texb, 10)
	{
//	scroll_.set_size_request(-1,200);
//	scroll_.set_border_width(1);
//	scroll_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
	edit.modify_font(Pango::FontDescription(fontname));
	edit.set_wrap_mode(Gtk::WRAP_WORD);
	edit.modify_text(Gtk::STATE_NORMAL, Gdk::Color("darkgray"));
	edit.set_pixels_above_lines(LINE_SPACING);
	edit.set_pixels_below_lines(LINE_SPACING);
	edit.set_pixels_inside_wrap(2*LINE_SPACING);
	edit.set_left_margin(10);

	set_spacing(10);

//	add(expander);
//	expander.set_label_widget(texview);
//	expander.add(edit);
//	expander.set_expanded();
	pack_start(edit);
	pack_start(texview);
	}

bool TeXInput::is_folded() const
	{
	return edit.folded_away;
	}

void TeXInput::set_folded(bool onoff)
	{
	if(edit.folded_away==onoff) return;

	edit.folded_away=onoff;
	if(edit.folded_away) 
		remove(edit);
	else {
		std::cerr << "here perhaps?" << std::endl;
		pack_start(edit);
		std::cerr << "here perhaps!" << std::endl;
		reorder_child(edit, 0);
		edit.show();
		}
	}

bool TeXInput::exp_input_tv::on_key_press_event(GdkEventKey* event)
	{
	if(get_editable() && event->keyval==GDK_Return && (event->state&Gdk::SHIFT_MASK)) {// shift-return
//		std::cerr << "activate!!" << std::endl;
		Glib::RefPtr<Gtk::TextBuffer> textbuf=get_buffer();
//		std::cerr << textbuf->get_text(textbuf->get_start_iter(), textbuf->get_end_iter()) << std::endl;
		std::string tmp(textbuf->get_text(get_buffer()->begin(), get_buffer()->end()));
#ifdef DEBUG
		std::cerr << "running: " << tmp << std::endl;
#endif
		emitter(tmp);
//		set_editable(false);
//		textbuf->set_text("");
		return true;
		}
	else {
		bool retval=Gtk::TextView::on_key_press_event(event);
		return retval;
		}
	}

Glib::RefPtr<TeXBuffer> TeXBuffer::create(Glib::RefPtr<Gtk::TextBuffer> tb, TeXEngine& engine)
	{
	return Glib::RefPtr<TeXBuffer>(new TeXBuffer(tb, engine) );
	}

Glib::RefPtr<Gdk::Pixbuf> TeXBuffer::get_pixbuf()
	{
	return engine_.get_pixbuf(tex_request);
	}

