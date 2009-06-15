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

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <modglue/main.hh>
#include <modglue/ext_process.hh>
#include <modglue/process.hh>
#include <sstream>
#include <stdexcept>

#include <pcrecpp.h>
#ifdef __CYGWIN__
  #include <windows.h>
#endif

#include "window.hh"

std::map<int, sigc::connection> connections;
std::ofstream  debugout;

int main (int argc, char *argv[])
	{
	// Open the debug output file, if requested.
	char hostname[256];
	gethostname(hostname, 255);
	char *pbs_job=getenv("PBS_JOBID");
	std::string logname=std::string("cdb_")+hostname+(pbs_job==0?"":std::string("_")+pbs_job)
                       +std::string(".log");

	char *cdblog=getenv("CDB_LOG");
	if(cdblog==0)
		debugout.open("/dev/null", std::ios::app);
	else switch(atoi(cdblog)) {
		case 1:
			debugout.open(logname.c_str(), std::ios::app);
			break;
		case 2:
			debugout.open("/dev/tty", std::ios::app);
			break;
		}

	// Setup path for tableaux.sty
	char *texinp=getenv("TEXINPUTS");
	std::string oldpath;
	if(texinp!=0) oldpath=std::string(texinp);
	oldpath=std::string(DESTDIR)+std::string("/share/texmf/tex/latex/cadabra:")+oldpath;
	setenv("TEXINPUTS", oldpath.c_str(), 1);

	// Ensure correct installation.
//	if(!verify_breqn_tableaux_presence())
//		return -1;

	// Startup Gtkmm and the GUI
	Gtk::Main     kit(&argc, &argv);
	modglue::main mm(argc, argv);

	// Set the environment for cadabra.
//	setenv("CDB_USE_UTF8", "1", 1);
	unsetenv("CDB_PRINTSTAR");//, "false", 1);

	// Argument parsing
	std::string filename;
	if(argc>1)
		filename=argv[1];

#ifdef __CYGWIN__
	std::string cdbname;
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	cdbname=path;
	cdbname=cdbname.substr(0,cdbname.size()
								  -std::string("xcadabra.exe").size())
		+ "cadabra";
	
	pcrecpp::RE(":\\\\").GlobalReplace("/", &cdbname);
	pcrecpp::RE("\\\\").GlobalReplace("/", &cdbname);
	cdbname="/cygdrive/"+cdbname;
#else
	std::string cdbname="cadabra";
#endif
	modglue::ext_process ls_proc(cdbname);
	ls_proc << "--xcadabra" << "--bare";
	ls_proc.setup_pipes(); // FIXME: need cleaner error messages if this is forgotten
	mm.add(&ls_proc);

	// Setup TeX engine parameters
	tex_engine_help.set_geometry(380);
	tex_engine_help.latex_packages.push_back("cadabra");
	tex_engine_main.latex_packages.push_back("breqn");

	XCadabra theiface(ls_proc, filename, &mm);

	// Setup pipes
	ls_proc.input_pipe("stdout")->receiver.connect(sigc::mem_fun(theiface, &XCadabra::receive));
	ls_proc.input_pipe("stderr")->receiver.connect(sigc::mem_fun(theiface, &XCadabra::receive_err));
	mm.process_died.connect(sigc::mem_fun(theiface, &XCadabra::on_kernel_exit));

	// FIXME: if we run this afterwards, the pipes are not setup yet. Note: this also
	// means that if we want to start a program while in the main loop, we have to
	// do this connecting business again. For the time being, let cadabra run forever
	// and don't worry too much about this.

	ls_proc.fork(); // FIXME: need error message upon failure
	*(ls_proc.output_pipe("stdin")) << "@print_status{true};\n@properties;\n@algorithms;\n@reserved;\n" 
											  << std::flush;

	theiface.connect_io_signals();

	// If done this way, we miss signals if the process exits before
	// the main loop is entered. Well, cadabra doesn't do that.

//	mm.run(0);
	Gtk::Main::run();
	return 0;
	} 
