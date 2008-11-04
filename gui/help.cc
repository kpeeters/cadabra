
#include "help.hh"

#include <fstream>
#include <iostream>

#include <gtkmm/stock.h>
#include <gtkmm/messagedialog.h>

CadabraHelp::CadabraHelp()
	: history_pos(-1), buttonbox(0), back(Gtk::Stock::GO_BACK), forward(Gtk::Stock::GO_FORWARD)
	{
	set_title("XCadabra help");
	try {
		set_icon_from_file(DESTDIR+std::string("/share/pixmaps/cadabra.png"));
		}
	catch(Glib::FileError fe) {
		std::cerr << "cannot find cadabra.png" << std::endl;
		}
	set_gravity(Gdk::GRAVITY_NORTH_EAST);
	set_default_size(std::min(Gdk::Screen::get_default()->get_width()-20,  600),
						  std::min(Gdk::Screen::get_default()->get_height()-20, 800));


	add(topbox);
	topbox.pack_start(navbox, Gtk::PACK_SHRINK);

//	actiongroup=Gtk::ActionGroup::create();
//	actiongroup->add( Gtk::Action::create("MenuFile", "_File") );
//	actiongroup->add( Gtk::Action::create("CloseWindow", Gtk::Stock::CLOSE),
//								  sigc::mem_fun(doc, &XCadabra::on_help_close) );

//	uimanager = Gtk::UIManager::create();
//	uimanager->insert_action_group(actiongroup);
//	add_accel_group(uimanager->get_accel_group());
//	
//	Glib::ustring ui_info =
//		 "<ui>"
//		 "  <menubar name='MenuBar'>"
//		 "    <menu action='MenuFile'>"
//		 "      <menuitem action='CloseWindow'/>"
//		 "    </menu>"
//		 "  </menubar>"
//		 "</ui>";
//	
//	uimanager->add_ui_from_string(ui_info);
//	Gtk::Widget *menubar = uimanager->get_widget("/MenuBar");
//	topbox.pack_start(*menubar, Gtk::PACK_SHRINK);

	navbox.pack_start(back, Gtk::PACK_SHRINK);
	navbox.pack_start(forward, Gtk::PACK_SHRINK);
	back.set_sensitive(false);
	forward.set_sensitive(false);
	back.signal_clicked().connect(sigc::mem_fun(*this, &CadabraHelp::on_back));
	forward.signal_clicked().connect(sigc::mem_fun(*this, &CadabraHelp::on_forward));

	topbox.add(scroll);
	scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
	scroll.set_border_width(1);
	scroll.add(scrollbox);

	// Set-up the TeXView widget
	textbuf=Gtk::TextBuffer::create();
	texbuf=TeXBuffer::create(textbuf);
	texview=new TeXView(texbuf, 12);
	scrollbox.pack_start(*texview, Gtk::PACK_EXPAND_WIDGET, 0);

//	show_all();
	}

void CadabraHelp::display_help(objtype_t objtype, const std::string& objname)
	{
	if(history.size()>0 && history.back()==std::make_pair(objtype, objname)) {
		 show_all();
		 return; // already displaying this one
		 }
	history.resize(history_pos+1);
	history.push_back(std::make_pair(objtype, objname));
	history_pos=history.size()-1;

	display_help();
	}

void CadabraHelp::display_help()
	{
	show_all();

	// Add the buttons for related topics
	if(buttonbox!=0) {
		 topbox.remove(*buttonbox);
		 delete buttonbox;
		 }
	buttonbox = new Gtk::HBox;
	topbox.pack_start(*buttonbox, Gtk::PACK_SHRINK);
	relatedlabel.set_label("See also: ");
	buttonbox->pack_start(relatedlabel, Gtk::PACK_SHRINK);

	back.set_sensitive(history.size()>1);
	forward.set_sensitive(history_pos+1<static_cast<int>(history.size()));

	std::string fname;
	switch(history[history_pos].first) {
		 case t_property:
			  fname=DESTDIR+std::string("/share/doc/cadabra/properties/");
			  break;
		 case t_algorithm:
			  fname=DESTDIR+std::string("/share/doc/cadabra/algorithms/");
			  break;
		 case t_texcommand:
			  fname=DESTDIR+std::string("/share/doc/cadabra/reserved/");
			  break;
		 default:
			  fname=DESTDIR+std::string("/share/doc/cadabra/general");
			  break;
		 }
	fname+=history[history_pos].second+".tex";

	std::string total, line;

	std::ifstream helpfile(fname.c_str());
	if(helpfile.is_open()==false)  {
		 total = "Sorry, no help available for {\\tt "+texify(history[history_pos].second)+"}.\n\n"
			  + "{\\small (Could not open {\\tt "+texify(fname)+"}.)}";
		 helpfile.open((DESTDIR+std::string("/share/doc/cadabra/general.tex")).c_str());
		 if(helpfile.is_open()==false) {
			  textbuf->set_text(total);
			  texbuf->generate("","",560);
			  texview->image.set(texbuf->pixbuf);
			  return;
			  }
		 }

	std::vector<std::string> seeprop, seealgo;
	while(std::getline(helpfile, line)) {
		 total+=line+"\n";
		 if(line.substr(0,11)=="\\cdbseeprop") 
			  seeprop.push_back(line.substr(12,line.size()-13));
		 if(line.substr(0,11)=="\\cdbseealgo") 
			  seealgo.push_back(line.substr(12,line.size()-13));
		 }
//	for(unsigned int i=0; i<seeprop.size(); ++i)
//		 std::cerr << "see also: |::" << seeprop[i] << "|" << std::endl;
//	for(unsigned int i=0; i<seealgo.size(); ++i)
//		 std::cerr << "see also: |@" << seealgo[i] << "|" << std::endl;

	textbuf->set_text(total);
	try {
		 texbuf->generate("","",560,true);
		 }
	catch(std::exception& ex) {
		Gtk::MessageDialog md(ex.what());
		md.set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
		md.run();
		}

	texview->image.set(texbuf->pixbuf);

	
	for(unsigned int i=0; i<seeprop.size(); ++i) {
		 Gtk::Button *tmp = new Gtk::Button("::"+seeprop[i]);
		 tmp->signal_clicked().connect(sigc::bind<objtype_t, std::string>(
													  sigc::mem_fun(*this, &CadabraHelp::on_help_context_link), 
													  t_property,
													  seeprop[i]));
		 buttonbox->pack_start(*tmp, Gtk::PACK_SHRINK); // FIXME: is this leaking?
		 tmp->show_all();
		 }
	for(unsigned int i=0; i<seealgo.size(); ++i) {
		 Gtk::Button *tmp = new Gtk::Button("@"+seealgo[i]);
		 tmp->signal_clicked().connect(sigc::bind<objtype_t, std::string>(
													  sigc::mem_fun(*this, &CadabraHelp::on_help_context_link), 
													  t_algorithm,
													  seealgo[i]));
		 buttonbox->pack_start(*tmp, Gtk::PACK_SHRINK); // FIXME: is this leaking?
		 tmp->show_all();
		 }
	buttonbox->show_all();
	}

bool CadabraHelp::on_delete_event(GdkEventAny*)
	{
	on_help_close();
	return true;
	}

void CadabraHelp::on_back()
	{
	if(history_pos>0) {
		 --history_pos;
		 display_help();
		 }
	}

void CadabraHelp::on_forward()
	{
	if(history_pos+1<static_cast<int>(history.size())) {
		 ++history_pos;
		 display_help();
		 }
	}

void CadabraHelp::on_help_context_link(CadabraHelp::objtype_t objtype, std::string helpname)
	{
	display_help(objtype, helpname);
	}

void CadabraHelp::on_help_close()
	{
	hide_all();
	}

bool CadabraHelp::on_key_press_event(GdkEventKey* event)
	{
//	std::cerr << event->keyval << " " << event->state << std::endl;
	if(event->keyval=='w' && event->state&Gdk::CONTROL_MASK) {
		 hide_all();
		 return true;
		 }
	return Window::on_key_press_event(event);
	}

