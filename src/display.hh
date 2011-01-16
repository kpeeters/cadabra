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

#ifndef display_hh_
#define display_hh_

#include "storage.hh"
//#include "modules/properties.hh"
#include <map>

typedef uint32_t kunichar;

// TeXmacs markers
#define DATA_BEGIN   ((char) 2)
#define DATA_END     ((char) 5)
#define DATA_COMMAND ((char) 16)
#define DATA_ESCAPE  ((char) 27)

class exptree_output;

class display_error {
	public:
		display_error();
};

class display_interrupted {
	public:
		display_interrupted();
};

/// The base class for displaying/printing nodes in the expression
/// tree. 
class node_base_printer {
	public:
		typedef exptree::iterator         iterator;
		typedef exptree::sibling_iterator sibling_iterator;

		node_base_printer(exptree_output&);
		virtual ~node_base_printer() {};
		virtual void print_infix(std::ostream&, iterator) = 0;

	protected:
		exptree_output& parent;
		const exptree&  tr;

		str_node::parent_rel_t previous_parent_rel_, current_parent_rel_;
		str_node::bracket_t    previous_bracket_, current_bracket_;

		// Determine whether the children of the indicated node all have
		// the same, non-empty bracket type (i.e. determines whether, when 
		// the indicated node is a \\sum, the children will automatically
		// be wrapped in a bracket).
		bool children_have_brackets(iterator) const;
};


/// The default printing class for standard text output to the console.
/// Also contains logic for printing in Mathematica and Maple format,
/// to be split off in a separate class hierarchy later.
class node_printer : public node_base_printer {
	public:
		node_printer(exptree_output&);
		virtual ~node_printer() {};
		virtual void print_infix(std::ostream&, iterator);

	protected:
		void print_multiplier(std::ostream&, exptree::iterator);
		void print_opening_bracket(std::ostream&, str_node::bracket_t, str_node::parent_rel_t);
		void print_closing_bracket(std::ostream&, str_node::bracket_t, str_node::parent_rel_t);
		void print_parent_rel(std::ostream&, str_node::parent_rel_t, bool first);
		void print_children(std::ostream&, exptree::iterator, int skip=0);

		std::string texify(const std::string&) const;

		// some random junk variables
		bool isdelta;
		bool isweyl;
};

/// The default node printing class for MathML output.
class mathml_node_printer : public node_base_printer {
	public:
		mathml_node_printer(exptree_output&);
		virtual ~mathml_node_printer() {};
		virtual void print_infix(std::ostream&, iterator);

	protected:
		void print_multiplier(std::ostream&, exptree::iterator);
		void print_opening_bracket(std::ostream&, str_node::bracket_t, str_node::parent_rel_t);
		void print_closing_bracket(std::ostream&, str_node::bracket_t, str_node::parent_rel_t);
		void print_parent_rel(std::ostream&, str_node::parent_rel_t, bool first);
		void print_children(std::ostream&, exptree::iterator, int skip=0);
};


/* ------------------------------------------------------------------------------- */

class print_expression : public node_printer {
	public:
		print_expression(exptree_output&);
		virtual ~print_expression() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_productlike : public node_printer {
	public:
		print_productlike(exptree_output&);
		virtual ~print_productlike() {};
	protected:
		void doprint(std::ostream&, exptree::iterator, const std::string&);
};

class print_wedge : public print_productlike {
	public:
		print_wedge(exptree_output&);
		virtual ~print_wedge() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_prod : public print_productlike {
	public:
		print_prod(exptree_output&);
		virtual ~print_prod() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_dot : public node_printer {
	public:
		print_dot(exptree_output&);
		virtual ~print_dot() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_sqrt : public node_printer {
	public:
		print_sqrt(exptree_output&);
		virtual ~print_sqrt() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_arrow : public node_printer {
	public:
		print_arrow(exptree_output&);
		virtual ~print_arrow() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_commutator : public node_printer {
	public:
		print_commutator(exptree_output&);
		virtual ~print_commutator() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_indexbracket : public node_printer {
	public:
		print_indexbracket(exptree_output&);
		virtual ~print_indexbracket() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_pow : public node_printer {
	public:
		print_pow(exptree_output&);
		virtual ~print_pow() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_frac : public node_printer {
	public:
		print_frac(exptree_output&);
		virtual ~print_frac() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_sum : public node_printer {
	public:
		print_sum(exptree_output&);
		virtual ~print_sum() {};
		virtual void print_infix(std::ostream&, iterator );
	private:
		void do_actual_print(std::ostream&, iterator);
};

class print_sequence : public node_printer {
	public:
		print_sequence(exptree_output&);
		virtual ~print_sequence() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_equals : public node_printer {
	public:
		print_equals(exptree_output&);
		virtual ~print_equals() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_unequals : public node_printer {
	public:
		print_unequals(exptree_output&);
		virtual ~print_unequals() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_conditional : public node_printer {
	public:
		print_conditional(exptree_output&);
		virtual ~print_conditional() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_regex : public node_printer {
	public:
		print_regex(exptree_output&);
		virtual ~print_regex() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_factorial : public node_printer {
	public:
		print_factorial(exptree_output&);
		virtual ~print_factorial() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_comma : public node_printer {
	public:
		print_comma(exptree_output&);
		virtual ~print_comma() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_tableau : public node_printer {
	public:
		print_tableau(exptree_output&);
		virtual ~print_tableau() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_filled_tableau : public node_printer {
	public:
		print_filled_tableau(exptree_output&);
		virtual ~print_filled_tableau() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_derivative : public node_printer {
	public:
		print_derivative(exptree_output&);
		virtual ~print_derivative() {};
		virtual void print_infix(std::ostream&, iterator );
};


/* ------------------------------------------------------------------------------- */

class print_mathml_expression : public mathml_node_printer {
	public:
		print_mathml_expression(exptree_output&);
		virtual ~print_mathml_expression() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_mathml_productlike : public mathml_node_printer {
	public:
		print_mathml_productlike(exptree_output&);
		virtual ~print_mathml_productlike() {};
	protected:
		void doprint(std::ostream&, exptree::iterator, const std::string&);
};

class print_mathml_wedge : public print_mathml_productlike {
	public:
		print_mathml_wedge(exptree_output&);
		virtual ~print_mathml_wedge() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_mathml_prod : public print_mathml_productlike {
	public:
		print_mathml_prod(exptree_output&);
		virtual ~print_mathml_prod() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_mathml_indexbracket : public mathml_node_printer {
	public:
		print_mathml_indexbracket(exptree_output&);
		virtual ~print_mathml_indexbracket() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_mathml_pow : public mathml_node_printer {
	public:
		print_mathml_pow(exptree_output&);
		virtual ~print_mathml_pow() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_mathml_frac : public mathml_node_printer {
	public:
		print_mathml_frac(exptree_output&);
		virtual ~print_mathml_frac() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_mathml_sum : public mathml_node_printer {
	public:
		print_mathml_sum(exptree_output&);
		virtual ~print_mathml_sum() {};
		virtual void print_infix(std::ostream&, iterator );
	private:
		void do_actual_print(std::ostream&, iterator);
};

class print_mathml_sequence : public mathml_node_printer {
	public:
		print_mathml_sequence(exptree_output&);
		virtual ~print_mathml_sequence() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_mathml_equals : public mathml_node_printer {
	public:
		print_mathml_equals(exptree_output&);
		virtual ~print_mathml_equals() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_mathml_unequals : public mathml_node_printer {
	public:
		print_mathml_unequals(exptree_output&);
		virtual ~print_mathml_unequals() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_mathml_factorial : public mathml_node_printer {
	public:
		print_mathml_factorial(exptree_output&);
		virtual ~print_mathml_factorial() {};
		virtual void print_infix(std::ostream&, iterator );
};

class print_mathml_comma : public mathml_node_printer {
	public:
		print_mathml_comma(exptree_output&);
		virtual ~print_mathml_comma() {};
		virtual void print_infix(std::ostream&, iterator );
};



/* ------------------------------------------------------------------------------- */

class exptree_output {
	public:
		enum output_format_t { out_plain, out_mathematica, out_reduce, out_maple, 
									  out_texmacs, out_mathml, out_xcadabra };

		exptree_output(const exptree&, output_format_t of=out_plain);

		bool            highlight;
		const bool      tight_star;
		const bool      tight_plus;
		const bool      tight_brackets;
		bool            print_star;
		output_format_t output_format;
		bool            xml_structured;
		bool            utf8_output;
		bool            print_expression_number;
		const exptree&  tr;

		void print_full_standardform(std::ostream&, exptree::iterator, bool eqno);
		void print_infix(std::ostream&, exptree::iterator);
		void print_prefix(std::ostream&, exptree::iterator);
		void setup_handlers(bool infix=true);
		void newline(std::ostream&);

		std::auto_ptr<node_base_printer> get_printer(exptree::iterator);		
		
		unsigned int bracket_level; // FIXME: perhaps a stack?
	private:
		typedef std::map<std::string, std::auto_ptr<node_base_printer> (*)(exptree_output&)> printmap_t;
		typedef std::map<std::string, std::auto_ptr<node_base_printer> (*)(exptree_output&)> printmap_prop_t;

		printmap_t        printers_;
		printmap_prop_t   printers_prop_;

		std::auto_ptr<node_base_printer> (*print_default_)(exptree_output&);
};

template<class T>
std::auto_ptr<node_base_printer> create(exptree_output& eo)
	{
	return std::auto_ptr<node_base_printer>(new T(eo));
	}

const char *unichar(kunichar c);

#endif

