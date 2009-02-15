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

#include "window.hh"

std::map<int, sigc::connection> connections;

std::ofstream  debugout;

//bool verify_breqn_tableaux_presence()
//	{
//	char templ[10]="cdbXXXXXX";
//	std::ostringstream total;
//
//	// First test LaTeX without any style files.
//	int fd = mkstemp(templ);
//	total << "\\documentclass{article}\n"
//			<< "\\begin{document}\n"
//			<< "test\n"
//			<< "\\end{document}\n";
//	write(fd, total.str().c_str(), total.str().size());
//	close(fd);
//	std::string nf=std::string(templ)+".tex";
//	rename(templ, nf.c_str());
//
//	std::string result;
//
//	modglue::child_process latex_proc("latex");
//	latex_proc << "--interaction" << "nonstopmode" << nf;
//	try {
//		 latex_proc.call("", result);
//		 unlink(nf.c_str());	
//		 std::string cmd=std::string(templ)+".aux";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ)+".dvi";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ)+".log";
//		 unlink(cmd.c_str());
//		 }
//	catch(std::logic_error& err) {
//		 unlink(nf.c_str());	
//		 std::string cmd=std::string(templ)+".aux";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ)+".dvi";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ)+".log";
//		 unlink(cmd.c_str());
//		 std::cerr << "XCadabra: LaTeX problem, see the debug log for details." << std::endl;
//		 debugout << result << std::endl;
//		 sleep(1);
//		 return false;
//		 }
//
//	// Then test whether breqn can be included.
//	char templ2[10]="cdbXXXXXX";
//	fd = mkstemp(templ2);
//	total.str("");
//	total << "\\documentclass{article}\n"
//			<< "\\usepackage{breqn}\n"
//			<< "\\begin{document}\n"
//			<< "test\n"
//			<< "\\end{document}\n";
//	write(fd, total.str().c_str(), total.str().size());
//	close(fd);
//	nf=std::string(templ2)+".tex";
//	rename(templ2, nf.c_str());
//
//	modglue::child_process latex_proc2("latex");
//	latex_proc2 << "--interaction" << "nonstopmode" << nf;
//	try {
//		 latex_proc2.call("", result);
//		 unlink(nf.c_str());	
//		 std::string cmd=std::string(templ2)+".aux";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ2)+".dvi";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ2)+".log";
//		 unlink(cmd.c_str());
//		 }
//	catch(std::logic_error& err) {
//		 unlink(nf.c_str());	
//		 std::string cmd=std::string(templ2)+".aux";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ2)+".dvi";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ2)+".log";
//		 unlink(cmd.c_str());
//		 std::cerr << "XCadabra: problem finding the breqn.sty style for LaTeX." << std::endl;
//		 std::cerr << "Value of TEXINPUTS = " << getenv("TEXINPUTS") << std::endl;
//		 debugout << result << std::endl;
//		 return false;
//		 }
//	
//	// Then test whether tableaux.sty can be included.
//	char templ3[10]="cdbXXXXXX";
//	fd = mkstemp(templ3);
//	total.str("");
//	total << "\\documentclass{article}\n"
//			<< "\\usepackage{tableaux}\n"
//			<< "\\begin{document}\n"
//			<< "test\n"
//			<< "\\end{document}\n";
//	write(fd, total.str().c_str(), total.str().size());
//	close(fd);
//	nf=std::string(templ3)+".tex";
//	rename(templ3, nf.c_str());
//
//	modglue::child_process latex_proc3("latex");
//	latex_proc3 << "--interaction" << "nonstopmode" << nf;
//	try {
//		 latex_proc3.call("", result);
//		 debugout << result << std::endl;
//		 unlink(nf.c_str());	
//		 std::string cmd=std::string(templ3)+".aux";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ3)+".dvi";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ3)+".log";
//		 unlink(cmd.c_str());
//		 }
//	catch(std::logic_error& err) {
//		 unlink(nf.c_str());	
//		 std::string cmd=std::string(templ3)+".aux";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ3)+".dvi";
//		 unlink(cmd.c_str());
//		 cmd=std::string(templ3)+".log";
//		 unlink(cmd.c_str());
//		 std::cerr << "XCadabra: problem finding the tableaux.sty style for LaTeX." << std::endl;
//		 std::cerr << "Value of TEXINPUTS = " << getenv("TEXINPUTS") << std::endl;
//		 debugout << result << std::endl;
//		 return false;
//		 }
//
//	return true;
//	}


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

//	modglue::ext_process ls_proc(std::string(DESTDIR)+std::string("/bin/cadabra"));
	modglue::ext_process ls_proc("cadabra");
	ls_proc << "--xcadabra" << "--bare";
	ls_proc.setup_pipes(); // FIXME: need cleaner error messages if this is forgotten
	mm.add(&ls_proc);
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
