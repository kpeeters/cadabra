/* 

	Cadabra: a field-theory motivated computer algebra system.
	Copyright (C) 2001-2011  Kasper Peeters <kasper.peeters@aei.mpg.de>

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

/** \mainpage Cadabra
    \author   Kasper Peeters
    \version  development
    \date     latest
    \see      http://cadabra.phi-sci.com/

	 Cadabra: a field-theory motivated approach to computer algebra.

    The core of the manipulator is stored in the following files

       - main.cc
       - manipulator.hh and manipulator.cc
       - algorithm.hh and algorithm.cc
       - display.hh and display.cc
       - settings.hh and settings.cc
       - preprocessor.hh and preprocessor.cc
       - parser.hh and parser.cc
       - props.hh and props.cc
       - exchange.hh and exchange.cc
       - storage.hh and storage.cc
 
    Further code is in the src/modules directory.
*/


#include "storage.hh"
#include "props.hh"
#include "manipulator.hh"
#include <modglue/main.hh>
#include <modglue/pipe.hh>
#include "stopwatch.hh"
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <iomanip>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>

// prompt does not do non-standard pipe names yet, so therefore the hack:
modglue::ipipe commands("stdin");
modglue::opipe raw_txtout("stdout");
modglue::opipe texout("stderr");
std::ofstream  debugout;
std::ofstream  nullout("/dev/null",std::ios::app);

std::ostream  *real_txtout;
std::ostream  *fake_txtout;
std::ostream  *real_forcedout;
std::ostream  *fake_forcedout;
#define txtout    (*fake_txtout)
#define forcedout (*fake_forcedout)

// global flag to indicate a control-C interrupt.
bool           interrupted=false;
stopwatch      globaltime;
unsigned int   size_x, size_y;
bool           loginput=false;
bool           nowarnings=false;
bool           silentfail=false;

std::vector<std::string> cmdline_arguments;

extern std::string defaults;

void sigc_handler(int num)
	{
	interrupted=true;
   signal(SIGINT,sigc_handler);
	}

void determine_window_size() 
	{
	struct winsize tmp;
	if (ioctl( 0, TIOCGWINSZ, &tmp)==-1) {
//		debugout << "cannot determine window size" << std::endl;
		size_x=80;
		size_y=24;
		}
	else {
		size_x=tmp.ws_col;
		size_y=tmp.ws_row;
		}
//	debugout << "window size now " << size_x << "x" << size_y << std::endl;
	}

void winch_handler(int num)
	{
	debugout << "window size changed" << std::endl;
	determine_window_size();
	}

bool verify_tool_presence()
	{
	return true;
	}

int main(int argc, char **argv)
	{
	std::string inputfile;
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

	signal(SIGINT,sigc_handler);
	signal(SIGWINCH, winch_handler);

	real_txtout=&raw_txtout;
	real_forcedout=&raw_txtout;

	modglue::main mm(argc, argv);
	manipulator mnp;
	bool disable_defaults=false;
	bool disable_dot_cadabra=false;
	bool continue_interactive=false;
	for(int i=1; i<argc; ++i) {
		if(strcmp(argv[i],"--bare")==0)
			disable_dot_cadabra=true;
		else if(strcmp(argv[i],"--nodefaults")==0)
			disable_defaults=true;
		else if(strcmp(argv[i],"-F")==0)
			continue_interactive=true;
		else if(strcmp(argv[i],"--silent")==0) {
			real_txtout=&nullout;
			}
		else if(strcmp(argv[i],"--loginput")==0) {
			loginput=true;
			}
		else if(strcmp(argv[i],"--texmacs")==0) {
			mnp.eo.output_format=exptree_output::out_texmacs;
			}
		else if(strcmp(argv[i],"--xcadabra")==0) {
			mnp.eo.output_format=exptree_output::out_xcadabra;
			}
		else if(strcmp(argv[i],"--mathml")==0) {
			mnp.eo.output_format=exptree_output::out_mathml;
			}
		else if(strcmp(argv[i],"--input")==0) {
			++i;
			inputfile=argv[i];
			}
		else if(strcmp(argv[i],"--prompt")==0) {
			++i;
			mnp.set_prompt(std::string(argv[i]));
			}
		else if(strcmp(argv[i],"--silentfail")==0) {
			silentfail=true;
			}
		else if(strcmp(argv[i],"--nowarnings")==0) {
			nowarnings=true;
			}
		else if(strcmp(argv[i],"--help")==0) {
			std::cout << "Usage: cadabra [options] input\n\n"
						 << "where [options] can be any of \n"
						 << "   --bare             : disable ~/.cadabra\n"
						 << "   --nodefaults       : disable default settings\n"
						 << "   --silent           : disable output\n"
						 << "   --loginput         : repeat input to output\n"
						 << "   --texmacs          : enable texmacs output format\n"
						 << "   --xcadabra         : enable xcadabra output format\n"
						 << "   --mathml           : enable matheml output format (experimental)\n"
						 << "   --input [filename] : read given file as input\n"
						 << "   --prompt [string]  : set the prompt string\n"
						 << "   --silentfail       : do not report errors upon failure\n"
						 << "   --nowarnings       : disable warnings\n"
						 << "   --help             : this help\n" << std::endl;
			return -1;
			}
		else {
			cmdline_arguments.push_back(argv[i]);
//			std::cerr << "argument: " << cmdline_arguments.back() << std::endl;
			}
		}
	fake_txtout=real_txtout;
	fake_forcedout=real_forcedout;

	mm.add(&commands, 0);
	mm.add(&raw_txtout, 1);
	mm.add(&texout, 2);
	commands.receiver.connect(sigc::mem_fun(mnp, &manipulator::receive_command));

	int return_value=0;

	if(mm.check()) {
		determine_window_size();
		globaltime.start();
		if(mnp.eo.output_format==exptree_output::out_texmacs) {
//			txtout << DATA_BEGIN << "scheme:";
//			txtout << "<with|font-family|rm|<with|font-size|2.0|Cadabra> 0.8>" << std::endl;
			txtout << DATA_BEGIN << "latex:";
			txtout << "\\rm {\\Large Cadabra " << RELEASE << "} (";
#ifdef STATICBUILD
			txtout << "static, ";
#endif		 
			txtout << "built on " << HOSTNAME << " " << DATETIME << ")\\\\" << std::endl;
			txtout << "Copyright (c) 2001-2011  Kasper Peeters <kasper.peeters@phi-sci.com>\\\\" << std::endl; 
			txtout << "Available under the terms of the GNU General Public License.\\\\" << std::endl;
			}
		else {
			txtout  << "Cadabra " << RELEASE << " (";
#ifdef STATICBUILD
			txtout << "static, ";
#endif
			txtout  << "built on " << HOSTNAME << " " << DATETIME << ")" << std::endl
					  << "Copyright (C) 2001-2011  Kasper Peeters <kasper.peeters@phi-sci.com>" << std::endl
					  << "Info at http://cadabra.phi-sci.com/" << std::endl
					  << "Available under the terms of the GNU General Public License." << std::endl << std::endl;
			}
		debugout << "-----" << std::endl
					<< "Cadabra (compiled " << DATETIME << " on " << HOSTNAME << ")" << std::endl;

		// Process default startup file (included in the binary).
		if(!disable_defaults) {
			std::istringstream tst(defaults);
			real_txtout=&nullout;
			fake_txtout=real_txtout;
			mnp.open_stream(&tst);
			mnp.handle_input();
			(*real_txtout) << std::flush;
			real_txtout=&raw_txtout;
			fake_txtout=real_txtout;
			}
		// Process user startup file, if any.
		if(!disable_dot_cadabra) {
			std::string defname=getenv("HOME");
			defname+="/.cadabra";
			std::ifstream tst(defname.c_str());
			if(tst.is_open()) {
				tst.close();
				mnp.open_stream(defname);
				mnp.handle_input();
				}
			else debugout << "Default startup file ~/.cadabra not present." << std::endl << std::endl;
			}
		if(mnp.eo.output_format==exptree_output::out_texmacs)
			txtout << DATA_END << std::flush;
		// Process input file, if any.
		if(inputfile.size()>0) {
			int orig_fd=open(inputfile.c_str(), O_RDONLY);
			if(orig_fd!=-1) {
				// make a temporary copy
				char temp_name[]="/tmp/cdbtmp_XXXXXX";
				int temp_fd=mkstemp(temp_name);
				if(temp_fd!=-1) {
					char buffer[8192];
					ssize_t read_len;
					while((read_len=read(orig_fd, buffer, 8192))!=0) {
						ssize_t start=0;
						do {
							ssize_t written=write(temp_fd, &(buffer[start]), read_len);
							if(written>=0) {
								start-=written;
								read_len-=written;
								}
							if(written<0 && errno!=EINTR) {
								close(orig_fd);
								close(temp_fd);
								txtout << "Failure while writing temporary copy of the input file to " << temp_name << std::endl;
								close(orig_fd);
								close(temp_fd);
								return(-1);
								}
							} while(read_len>0);
						}
					close(orig_fd);
					close(temp_fd);
					mnp.open_stream(temp_name);
					unlink(temp_name); // immediately unlink, so that the tmp file goes away in case of crash/abort
					}
				else {
					 txtout << "Failed to make temporary copy of the input file." << std::endl;
					 return -1;
					 }
				}
			else {
				txtout << "Input file " << inputfile << " not found." << std::endl;
				return -1;
				}
			}

//		int oldin;
//		if(isatty(0)==0) {
//			oldin=dup(0);
//			txtout << oldin << std::endl;
//			}

		// Run main loop. 
	mainloop:
		if(inputfile.size()==0)
			mnp.print_prompt();
		bool reading_input_file=false;
		try {
			if(inputfile.size()==0) mm.run(1);
			else { 
				reading_input_file=true;
				inputfile="";
				mnp.handle_input();
				}
			}
		catch(stream_end_error& se) {
			txtout << "Normal stream ended" << std::endl;
			if(continue_interactive) {
				commands.clear();
				assert(reading_input_file==false);
				// We were reading a redirected stdin, we have to reopen the tty.
				if(freopen("/dev/tty","r",stdin)==NULL) {
					txtout << "Failed to reopen tty." << std::endl;
					return_value=-1;
					}
				else goto mainloop;
				}
			else return_value=-2;
			}
		catch(std::exception& ex) {
			txtout << std::endl;
			txtout << "FATAL: " << ex.what() << std::endl;
			if(getenv("CDB_LOG"))
				txtout << "       See the log file \"" << logname << "\" for more details." << std::endl;
			else
				txtout << "       Rerun with the CDB_LOG environment variable set, to generate a log." << std::endl;
			txtout << std::endl;
			// FIXME: output is still screwed up, perhaps we should set fcntl stuff back.
			// Although it seems that this output only arrives when using prompt.
			return_value=-1;
			}
		catch(exit_exception& ex) {
			txtout << "Cadabra exiting";
			if(getenv("CDB_LOG"))
				txtout << "; log written on \"" << logname << "\"." << std::endl;
			else txtout << "." << std::endl;
			return_value=0;
			}
		if(reading_input_file) {
			txtout << "Input file ended" << std::endl;
			reading_input_file=false;
			if(continue_interactive) {
				txtout << "Continuing interactively" << std::endl;
				commands.clear();
				if(freopen("/dev/tty","r",stdin)==NULL)
					txtout << "Failed to re-open stdin.";
				else goto mainloop;
				}
			}
		else if(isatty(0)==0) {
			txtout << "Redirected input ended." << std::endl;
			if(continue_interactive) {
				txtout << "Continuing interactively" << std::endl;
				commands.clear();
				assert(reading_input_file==false);
				// We were reading a redirected stdin, we have to reopen the tty.
				// FIXME: this fails, though it used to work.
				if(freopen("/dev/tty","r",stdin)==NULL) {
					txtout << "Failed to re-open stdin.";
					}
				else goto mainloop;
				}
			}
		debugout << "-----" << std::endl;
		}
	
	// Funny things appear on the output if we do not flush here...
	std::cout << std::flush;
	std::cerr << std::flush;
	raw_txtout << std::flush;
	texout << std::flush;
	return return_value;
	}
