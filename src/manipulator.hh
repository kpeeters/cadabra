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

#ifndef manipulator_hh_
#define manipulator_hh_

#include "storage.hh"
#include "algorithm.hh"
#include "display.hh"
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <utility>
#include <stack>
#include <sigc++/object.h>
#include <modglue/pipe.hh>

// These are initiated in main.cc
extern std::ostream     *real_txtout;
extern std::ostream     *real_forcedout;
extern modglue::opipe    texout;
extern std::ofstream     debugout;
extern bool              interrupted;
extern bool              loginput;
extern bool              nowarnings;
extern bool              silentfail; 
extern std::vector<std::string> cmdline_arguments;

// All modules write to txtout, which we can point to whatever stream
// we like in order to redirect output to files.
extern std::ostream  *fake_txtout;
extern std::ostream  *fake_forcedout;
#define txtout (*fake_txtout)
#define forcedout (*fake_forcedout)

/// Exception thrown when a stream ends (either from an end-of-file condition
/// or from an explicit \@end statement in the file).
class stream_end_error {
	public:
		stream_end_error();
};

/// Exception thrown when the user requests complete shutdown of the kernel
/// using the \@quit command.
class exit_exception {
	public:
		exit_exception();
};


/// Singleton object which pulls in data from the input streams, handles switching
/// between streams, feeds data to the parser, and calls the algorithm objects
/// to perform action on the expression tree.
class manipulator : public sigc::trackable {
	public:
		manipulator();
		~manipulator();

		/// Open the file with name given by the argument string and add to the top of
		/// the input stream stack.
		void open_stream(const std::string&);

		/// Add the given stream to the top of the input stream stack.
		void open_stream(std::istream *);

		/// Entry point for the class; at least one stream should have been opened
		/// by using the open_stream member before calling this. Will run until
		/// all streams have reached their end or until an explicit stop is requested
		/// through a \@quit command.
		void handle_input();

		/// Callback member to push-receive input from a modglue pipe;
		/// an alternative entry point.  Upon receiving input this will
		/// automatically call handle_input to start parsing.
		bool receive_command(modglue::ipipe&);

		/// Set the prompt to the indicated string. Should be called before the entry points
		/// handle_input or receive_command are called.
		void set_prompt(const std::string&);
		
		/// Display the prompt. Used when a prompt is needed before any input handling takes
		/// place.
		void print_prompt() const;

		exptree_output             eo;

	private:
		typedef exptree::iterator            iterator;
		typedef exptree::post_order_iterator post_order_iterator;
		typedef exptree::sibling_iterator    sibling_iterator;

		std::stack<std::istream *> streamstack;

		void output_comment(const std::string& comment) const; // wrap in xml nodes if necessary
		void output_status() const;
		void read_program_file();

		/// Cleans up the stream at the top of streamstack. If it derives from ifstream,
		/// will call close on it as well. Pops the stream off the stack.
		void cleanup_stream();

		/// Replace \\argv[n] in the input with the n-th command line argument as stored
		/// in the global cmdline_arguments vector.
		void replace_cmdline_args(std::string& oneline);

		/// A modified getline which ensures that the return string is one complete
		/// line of input, and no more than that. Will return false if there is no
		/// or not enough input (yet).
		bool              getline_precut(std::istream&, std::string&);
		std::string       getline_precut_buffer;
		bool              getline_was_eof;

		bool              is_whitespace_(const std::string& str) const;

		/// Activate all '@' nodes. The returned iterator points inside the
		/// expression which has to be printed (though not necessarily to the top node), 
		/// or to expressions.end() if the expression has been removed.
		exptree::iterator handle_active_nodes_(exptree::iterator);  
		nset_t::iterator  collect_labels_(exptree&, exptree::iterator);
		void              cleanup_new_expression_(exptree::iterator);
		exptree::iterator apply_pre_default_rules_(exptree::iterator);
		exptree::iterator apply_post_default_rules_(exptree::iterator);
		void              extract_properties_(exptree::iterator);
		exptree::iterator run_procedure(exptree::iterator, long);
		bool              handle_external_commands_(exptree::iterator&, exptree::iterator, exptree::iterator&);
		std::string       texify(const std::string&) const;

		/// Class to hold information about a dynamically added algorithm class.
		/// Stores a pointer to the member function to create an instance, as well
		/// as global timing information.
		class algo_info {
			public:
				algo_info(std::auto_ptr<algorithm> (*)(exptree&, iterator));

				std::auto_ptr<algorithm> (*create)(exptree&, iterator);
				stopwatch sw;
				long      calls;
		};

		typedef std::map<std::string, algo_info *> algorithm_map_t ;

		algorithm_map_t    algorithms;
		exptree            expressions;
		int                editing_equation;
		bool               keep_result;
		bool               display_result;
		unsigned int       last_used_equation_number;
		bool               utf8_output, status_output;

		std::string        input_buffer;
		std::string        refill_input_buffer; // used in @reset
		std::string        goto_label;
		std::string        bailout_label;
		std::string        prompt_string;
};

#endif
