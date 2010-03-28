/* 

	Cadabra: a field-theory motivated computer algebra system.
	Copyright (C) 2001-2010  Kasper Peeters <kasper.peeters@aei.mpg.de>

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

#include "display.hh"
#include "manipulator.hh"
#include "props.hh"
#include "modules/algebra.hh"
#include "modules/relativity.hh"
#include "modules/gamma.hh"
#include "modules/output.hh"
#include <stdexcept>
#include <sstream>

#define nbsp   (( parent.utf8_output?(unichar(0x00a0)):" "))
#define zwnbsp (( parent.utf8_output?(unichar(0xfeff)):""))

exptree_output::exptree_output(const exptree& tr_, output_format_t of) 
	: 	tight_star(getenv("CDB_TIGHTSTAR")), tight_plus(getenv("CDB_TIGHTPLUS")), 
 	   tight_brackets(getenv("CDB_TIGHTBRACKETS")),
		print_star(getenv("CDB_PRINTSTAR")), output_format(of),
		xml_structured(false), utf8_output(false), print_expression_number(true),
		tr(tr_), bracket_level(0),
		print_default_(&create<node_printer>)
	{
	setup_handlers();
	}

void exptree_output::setup_handlers(bool infix)
	{
	printers_.clear();
	if(infix) {
		switch(output_format) {
			case out_mathml:
				printers_["\\expression"]   =&create<print_mathml_expression>;
				printers_["\\prod"]    	    =&create<print_mathml_prod>;
				printers_["\\frac"]   	    =&create<print_mathml_frac>;
				printers_["\\sum"]     	    =&create<print_mathml_sum>;
				printers_["\\pow"]          =&create<print_mathml_pow>;
				printers_["\\indexbracket"] =&create<print_mathml_indexbracket>;
				printers_["\\factorial"]    =&create<print_mathml_factorial>;
				printers_["\\equals"]  	    =&create<print_mathml_equals>;
				printers_["\\unequals"]     =&create<print_mathml_unequals>;
//				printers_["\\conditional"]  =&create<print_mathml_conditional>;
				printers_["\\sequence"]     =&create<print_mathml_sequence>;
				printers_["\\comma"]   	    =&create<print_mathml_comma>;
				// FIXME: these two are not yet declared reserved?
				printers_["\\wedge"]        =&create<print_mathml_wedge>;
//			printers_["\\commutator"]   =&create<print_mathml_commutator>;
				break;
			default:
				printers_["\\expression"]   =&create<print_expression>;
				printers_["\\frac"]     	 =&create<print_frac>;
//				printers_["\\wedge"]        =&create<print_wedge>;
				printers_["\\prod"]    	    =&create<print_prod>;
				printers_["\\sum"]     	    =&create<print_sum>;
				printers_["\\equals"]  	    =&create<print_equals>;
				printers_["\\unequals"]     =&create<print_unequals>;
				printers_["\\conditional"]  =&create<print_conditional>;
				printers_["\\regex"]        =&create<print_regex>;
				printers_["\\arrow"]        =&create<print_arrow>;
				printers_["\\cdot"]         =&create<print_dot>;
				printers_["\\comma"]   	    =&create<print_comma>;
				printers_["\\factorial"]    =&create<print_factorial>;
				printers_["\\sequence"]     =&create<print_sequence>;
				printers_["\\commutator"]   =&create<print_commutator>;
				printers_["\\anticommutator"]=&create<print_commutator>;
				printers_["\\indexbracket"] =&create<print_indexbracket>;
				printers_["\\pow"]          =&create<print_pow>;
				printers_["\\sqrt"]         =&create<print_sqrt>;
				if(output_format==out_xcadabra) {
					printers_prop_["Tableau"]       = &create<print_tableau>;
					printers_prop_["FilledTableau"] = &create<print_filled_tableau>;
					printers_prop_["Derivative"]    = &create<print_derivative>;
					printers_prop_["PartialDerivative"]    = &create<print_derivative>;
					}
				break;
			}
		}
	}

void exptree_output::newline(std::ostream& str)
	{
	switch(output_format) {
		case out_xcadabra:
			str << "\\\\" << std::endl;
			break;
		default:
			str << std::endl;
		}
	}

display_error::display_error()
	{
	}

display_interrupted::display_interrupted()
	{
	}

std::auto_ptr<node_base_printer> exptree_output::get_printer(exptree::iterator it) 
	{
	if(interrupted) {
		interrupted=false;
		throw display_interrupted();
		}
	printmap_t::const_iterator prit=printers_.find(*it->name);

	// Match based on node name.
	if(prit!=printers_.end()) return (*prit).second(*this);

	// Match based on property.
	printmap_prop_t::iterator pp=printers_prop_.begin();
	while(pp!=printers_prop_.end()) {
		properties::registered_property_map_t::iterator pit=
			properties::registered_properties.store.find((*pp).first);

		const property_base *aprop=pit->second();
		bool ret=properties::has(aprop, it);
		if(ret) return (*pp).second(*this);

		++pp;
		}

	// Default verbatim output.
	return print_default_(*this);
	}

void exptree_output::print_infix(std::ostream& str, exptree::iterator start)
	{
	setup_handlers(true);
	if(output_format==out_texmacs) str << DATA_BEGIN << "latex:$";
	get_printer(start)->print_infix(str, start);
	if(output_format==out_texmacs) str << "$" << DATA_END << std::flush;
	}

void exptree_output::print_prefix(std::ostream& str, exptree::iterator start)
	{
	setup_handlers(false);
	get_printer(start)->print_infix(str, start);
	}




print_expression::print_expression(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_expression::print_infix(std::ostream& str, exptree::iterator it)
	{
	// We print only the subtree starting at the first sibling node.
	// All other subtrees are supposed to contain other information,
	// like the externally (anti)symmetrised indices, which should
	// be displayed in a different way.
	parent.get_printer(tr.begin(it))->print_infix(str, tr.begin(it));
	}

/* --------------------------------------------------------------------- */

print_wedge::print_wedge(exptree_output& eo)
	: print_productlike(eo)
	{
	}

void print_wedge::print_infix(std::ostream& str, exptree::iterator it)
	{
	doprint(str, it, "^");
	}

print_prod::print_prod(exptree_output& eo)
	: print_productlike(eo)
	{
	}

void print_prod::print_infix(std::ostream& str, exptree::iterator it)
	{
	doprint(str, it, "*");
	}

print_productlike::print_productlike(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_productlike::doprint(std::ostream& str, exptree::iterator it, const std::string& inbetween)
	{
//	bool close_bracket=false;
	if(*it->multiplier!=1) {
		print_multiplier(str, it);
		sibling_iterator st=tr.begin(it);
//		while(st!=tr.end(it)) {
//			if(*st->name=="\\sum") {
//				str << "(";
//				close_bracket=true;
//				break;
//				}
//			++st;
//			}
		}

	// To print \prod{\sum{a}{b}}{\sum{c}{d}} correctly:
	// If there is any sum as child, and if the sum children do not
	// all have the same bracket type (different from b_none or b_no),
	// then print brackets.
	
	str_node::bracket_t previous_bracket_=str_node::b_invalid;
	bool beginning_of_group=true;
	sibling_iterator ch=tr.begin(it);
	while(ch!=tr.end(it)) {
		str_node::bracket_t current_bracket_=(*ch).fl.bracket;
		if(previous_bracket_!=current_bracket_) {
			if(current_bracket_!=str_node::b_none) {
				print_opening_bracket(str, current_bracket_, str_node::p_none);
				beginning_of_group=true;
				}
			}
		parent.get_printer(ch)->print_infix(str, ch);
		++ch;
		if(ch==tr.end(it)) {
			if(current_bracket_!=str_node::b_none) 
				print_closing_bracket(str, current_bracket_, str_node::p_none);
			}

		if(ch!=tr.end(it)) {
			 if(parent.print_star && parent.output_format != exptree_output::out_texmacs) {
				if(parent.tight_star) str << inbetween;
				else if(parent.utf8_output) {
					str << unichar(0x00a0) << inbetween << unichar(0x00a0);
					}
				else str << " " << inbetween << " ";
				}
			else {
				 if(parent.output_format==exptree_output::out_texmacs) str << "\\,";
				 else str << " ";
				 }
			}
		previous_bracket_=current_bracket_;
		}

//	if(close_bracket) str << ")";
	}

print_arrow::print_arrow(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_arrow::print_infix(std::ostream& str, exptree::iterator it)
	{
	sibling_iterator lhs=tr.begin(it), rhs=lhs;
	++rhs;

	parent.get_printer(lhs)->print_infix(str, lhs);
	if(parent.output_format==exptree_output::out_xcadabra || 
		parent.output_format==exptree_output::out_texmacs )
		str << " \\rightarrow ";
	else
		str << " -> ";
	parent.get_printer(rhs)->print_infix(str, rhs);	
	}

print_dot::print_dot(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_dot::print_infix(std::ostream& str, exptree::iterator it)
	{
	sibling_iterator lhs=tr.begin(it), rhs=lhs;
	++rhs;

	if(parent.output_format==exptree_output::out_xcadabra) 
		 str << "\\,";

	parent.get_printer(lhs)->print_infix(str, lhs);
	if(parent.output_format==exptree_output::out_xcadabra || 
		parent.output_format==exptree_output::out_texmacs )
		str << "\\!\\cdot{}\\!";
	else
		str << ".";
	parent.get_printer(rhs)->print_infix(str, rhs);	

	if(parent.output_format==exptree_output::out_xcadabra) 
		 str << "\\,";
	}

print_pow::print_pow(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_pow::print_infix(std::ostream& str, exptree::iterator it)
	{
	if(*it->multiplier!=1)
		print_multiplier(str, it);

	bool close_bracket=false;
	sibling_iterator st=tr.begin(it);

	// The first argument of \pow is to be examined for special
	// cases which require brackets around the entire argument.
	if(*st->name=="\\sum") {
		str << "(";
		close_bracket=true;
		}
	else if(tr.number_of_children(st)!=tr.number_of_indices(st)) {
		str << "(";
		close_bracket=true;
		}
	else if(*st->multiplier!=1) {
		str << "(";
		close_bracket=true;
		}

	sibling_iterator ch=tr.begin(it);
	parent.get_printer(ch)->print_infix(str, ch);
	if(close_bracket) str << ")";

	if(parent.output_format==exptree_output::out_xcadabra) {
		str << "{}^{";
		++ch;
		parent.get_printer(ch)->print_infix(str, ch);
		str << "}";
		}
	else {
		str << "**{";
		++ch;
		parent.get_printer(ch)->print_infix(str, ch);
		str << "}";
		}
	}

print_frac::print_frac(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_frac::print_infix(std::ostream& str, exptree::iterator it)
	{
	sibling_iterator num=tr.begin(it), den=num;
	++den;

	bool close_bracket=false;
	if(*it->multiplier!=1) {
		print_multiplier(str, it);
		str << "(";
		close_bracket=true;
		}
	if(parent.output_format==exptree_output::out_xcadabra) 
		 str << "\\frac{";

	parent.get_printer(num)->print_infix(str, num);

	if(parent.output_format==exptree_output::out_xcadabra) 
		 str << "}{";
	else
		 str << "/";
	parent.get_printer(den)->print_infix(str, den);	
	if(parent.output_format==exptree_output::out_xcadabra) 
		 str << "}";
	if(close_bracket)
		str << ")";
	}

print_sqrt::print_sqrt(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_sqrt::print_infix(std::ostream& str, exptree::iterator it)
	{
	str << "\\sqrt{";
	
	sibling_iterator arg=tr.begin(it);
	parent.get_printer(arg)->print_infix(str, arg);
	str << "}";
	}

print_sum::print_sum(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_sum::print_infix(std::ostream& str, exptree::iterator it)
	{
	std::ostringstream lstr;
//	txtout << "length of sum=" << lstr.str().size() << std::endl;
//	do_actual_print(lstr, it);
	do_actual_print(str, it);
	}

void print_sum::do_actual_print(std::ostream& str, exptree::iterator it)
	{
	bool close_bracket=false;
	if(*it->multiplier!=1) 
		print_multiplier(str, it);

	iterator par=tr.parent(it);
	if(tr.number_of_children(par) - tr.number_of_direct_indices(par)>1) { 
      // for a single argument, the parent already takes care of the brackets
		if(*it->multiplier!=1 || (tr.is_valid(par) && *par->name!="\\expression")) {
			// test whether we need extra brackets
			close_bracket=!children_have_brackets(it);
			if(close_bracket)
				str << "(";
			}
		}

	unsigned int steps=0;

	str_node::bracket_t previous_bracket_=str_node::b_invalid;
	sibling_iterator ch=tr.begin(it);
	bool beginning_of_group=true;
	bool mathematica_postponed_endl=false;
	while(ch!=tr.end(it)) {
		if(++steps==20) {
			if(parent.output_format==exptree_output::out_xcadabra)
				str << "%\n" << std::flush; // prevent LaTeX overflow
			steps=0;
			}
		str_node::bracket_t current_bracket_=(*ch).fl.bracket;
		if(previous_bracket_!=current_bracket_)
			if(current_bracket_!=str_node::b_none) {
				if(ch!=tr.begin(it)) {
					if(parent.tight_plus) str << "+";
					else if(parent.utf8_output) str << " +" << unichar(0x00a0);
					else                        str << " + ";
					}
				print_opening_bracket(str, current_bracket_, str_node::p_none);
				beginning_of_group=true;
				}
		if(beginning_of_group) {
			beginning_of_group=false;
			if(*ch->multiplier<0) {
				if(parent.tight_plus) str << "-";
				else if(parent.utf8_output) str << " -" << unichar(0x00a0);
				else                        str << " - ";
					
				}
			}
		else {
			if(*ch->multiplier<0) {
				if(parent.tight_plus)       str << "-";
				else if(parent.utf8_output) str << " -" << unichar(0x00a0);
				else                        str << " - ";
				}
			else {
				if(parent.tight_plus) str << "+";
				else if(parent.utf8_output) str << " +" << unichar(0x00a0);
				else                        str << " + ";
				}
			}
		if(mathematica_postponed_endl) {
			str << std::endl;
			mathematica_postponed_endl=false;
			}
		if(*ch->name=="1" && (*ch->multiplier==1 || *ch->multiplier==-1)) 
			str << "1"; // special case numerical constant
		else 
			parent.get_printer(ch)->print_infix(str, ch);
		++ch;
		if(ch==tr.end(it)) {
			if(current_bracket_!=str_node::b_none)
				print_closing_bracket(str, current_bracket_, str_node::p_none);
			}
		else {
//			if(current_bracket_!=(*ch).fl.bracket) 
//				throw display_error();
//			assert(current_bracket_==(*ch).fl.bracket);
			if(parent.bracket_level==0) {
				if(parent.output_format==exptree_output::out_mathematica)
					mathematica_postponed_endl=true;
// FIXME: this endl should be re-inserted for nodes having line_per_node true
//				else
//					str << std::endl;
				}
			}
		previous_bracket_=current_bracket_;
		}

	if(close_bracket) str << ")";
	str << std::flush;
	}

print_equals::print_equals(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_equals::print_infix(std::ostream& str, exptree::iterator it)
	{
	sibling_iterator lhs=tr.begin(it), rhs=lhs;
	++rhs;

	parent.get_printer(lhs)->print_infix(str, lhs);
	if(parent.tight_plus) str << "=";
	else                       str << " = ";
	parent.get_printer(rhs)->print_infix(str, rhs);	
	}

print_unequals::print_unequals(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_unequals::print_infix(std::ostream& str, exptree::iterator it)
	{
	sibling_iterator lhs=tr.begin(it), rhs=lhs;
	++rhs;

	parent.get_printer(lhs)->print_infix(str, lhs);
	if(parent.tight_plus) str << " ";
	if(parent.output_format==exptree_output::out_xcadabra || 
		parent.output_format==exptree_output::out_texmacs )
		 str << "\\not=";
	else
		 str << "!=";
	if(parent.tight_plus) str << " ";
	parent.get_printer(rhs)->print_infix(str, rhs);	
	}

print_conditional::print_conditional(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_conditional::print_infix(std::ostream& str, exptree::iterator it)
	{
	sibling_iterator lhs=tr.begin(it), rhs=lhs;
	++rhs;

	parent.get_printer(lhs)->print_infix(str, lhs);
	if(parent.tight_plus) str << " ";
	if(parent.output_format==exptree_output::out_xcadabra || 
		parent.output_format==exptree_output::out_texmacs )
		 str << " \\;\\vert\\; ";
	else
		 str << "|";
	if(parent.tight_plus) str << " ";
	parent.get_printer(rhs)->print_infix(str, rhs);	
	}

print_regex::print_regex(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_regex::print_infix(std::ostream& str, exptree::iterator it)
	{
	if(parent.output_format!=exptree_output::out_xcadabra && 
		parent.output_format!=exptree_output::out_texmacs )
		 return node_printer::print_infix(str, it);
	
	sibling_iterator lhs=tr.begin(it), rhs=lhs;
	++rhs;

	parent.get_printer(lhs)->print_infix(str, lhs);
	if(parent.tight_plus) str << " ";
	str << " =_{\\cal R} ";
	if(parent.tight_plus) str << " ";
	parent.get_printer(rhs)->print_infix(str, rhs);	
	}

print_factorial::print_factorial(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_factorial::print_infix(std::ostream& str, exptree::iterator it)
	{
	sibling_iterator arg=tr.begin(it);

	parent.get_printer(arg)->print_infix(str, arg);
	str << "!";
	}

print_comma::print_comma(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_comma::print_infix(std::ostream& str, exptree::iterator it)
	{
	sibling_iterator ch=tr.begin(it);
	if(! (tr.begin(it)->fl.bracket==str_node::b_none && 
			it->fl.bracket!=str_node::b_none) ) {

		switch(tr.begin(it)->fl.bracket) {
			case str_node::b_none:   str << "\\{";   break;
			case str_node::b_pointy: str << "\\<"; break;
			case str_node::b_curly:  str << "\\{"; break;
			case str_node::b_round:  str << "(";   break;
			case str_node::b_square: str << "[";   break;
			default :	return;
			}
		++(parent.bracket_level);
		}
	while(ch!=tr.end(it)) {
		parent.get_printer(ch)->print_infix(str, ch);
		++ch;
		if(ch!=tr.end(it)) {
			str << ",";
			if(!parent.tight_plus) {
				if(parent.output_format==exptree_output::out_xcadabra)
					str << "\\; ";
				else
					str << " ";
// FIXME: print extra endl depending on line_per_node
//			if(parent.bracket_level==1)
//				str << std::endl;
				}
			}
		}
	if(! (tr.begin(it)->fl.bracket==str_node::b_none && 
			it->fl.bracket!=str_node::b_none) ) {
		switch(tr.begin(it)->fl.bracket) {
			case str_node::b_none:   str << "\\}";   break;
			case str_node::b_pointy: str << "\\>"; break;
			case str_node::b_curly:  str << "\\}"; break;
			case str_node::b_round:  str << ")";   break;
			case str_node::b_square: str << "]";   break;
			default :	return;
			}
		--(parent.bracket_level);
		}
	}

print_tableau::print_tableau(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_tableau::print_infix(std::ostream& str, exptree::iterator it)
	{
	if(*it->multiplier!=1) 
		print_multiplier(str, it);
	str << "\\tableau{";
	sibling_iterator sib=tr.begin(it); 
	bool first=true;
	while(sib!=tr.end(it)) {
		if(!first) str << " ";
		else       first=false;
		str << *sib->multiplier;
		++sib;
		}
	str << "}" << std::endl;
	}

print_filled_tableau::print_filled_tableau(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_filled_tableau::print_infix(std::ostream& str, exptree::iterator it)
	{
	if(*it->multiplier!=1) 
		print_multiplier(str, it);
	str << "\\ftableau{";
	sibling_iterator sib=tr.begin(it); 
	bool first=true;
	while(sib!=tr.end(it)) {
		if(!first) str << ",";
		else       first=false;
		if(*sib->name=="\\comma") {
			sibling_iterator sib2=tr.begin(sib);
			while(sib2!=tr.end(sib)) {
				str << "{";
				parent.get_printer(sib2)->print_infix(str, sib2);
				str << "}";
				++sib2;
				}
			}
		else {
			str << "{";
			parent.get_printer(sib)->print_infix(str, sib);
			str << "}";
			}
		++sib;
		}
	str << "}" << std::endl;
	}


print_derivative::print_derivative(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_derivative::print_infix(std::ostream& str, exptree::iterator it)
	{
	if(*it->multiplier!=1) 
		print_multiplier(str, it);

	const LaTeXForm *lf=properties::get<LaTeXForm>(it);
	bool needs_extra_brackets=false;

	if(parent.output_format==exptree_output::out_xcadabra) {
		const Accent *ac=properties::get<Accent>(it);
		if(!ac) { // accents should never get additional curly brackets, {\bar}{g} does not print.
			sibling_iterator sib=tr.begin(it);
			while(sib!=tr.end(it)) {
				if(sib->is_index()) 
					needs_extra_brackets=true;
				++sib;
				}
			}
		
		if(needs_extra_brackets) str << "{"; // to prevent double sup/sub script errors
		if(lf) str << lf->latex;
		else   str << texify(*it->name);
		if(needs_extra_brackets) str << "}";
		}
	else str << *it->name;

	sibling_iterator idx=tr.begin(it);
	int count=0;
	previous_parent_rel_=str_node::p_none;
	
	while(idx!=tr.end(it)) {
		sibling_iterator nxt=idx;
		++nxt;

		if(idx->is_index()) {
			if(idx->fl.parent_rel!=previous_parent_rel_) {
				print_parent_rel(str, idx->fl.parent_rel, idx==tr.begin(it));
				str << "{";
				previous_parent_rel_=idx->fl.parent_rel;
				parent.get_printer(idx)->print_infix(str, idx);
				}
			else
				parent.get_printer(idx)->print_infix(str, idx);

			if(nxt==tr.end(it) || nxt->fl.parent_rel!=previous_parent_rel_)
				str << "}";
			else 
				str << " ";
			}
		else {
			str << "{";
			if(*idx->name=="\\prod" || *idx->name=="\\sum")
				str << "(";

			parent.get_printer(idx)->print_infix(str, idx);

			if(*idx->name=="\\prod" || *idx->name=="\\sum")
				str << ")";
			str << "}";
			}
		++idx;
		++count;
		}
	if(parent.output_format==exptree_output::out_xcadabra)
		str << "\\, ";
	}

print_sequence::print_sequence(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_sequence::print_infix(std::ostream& str, exptree::iterator it)
	{
	sibling_iterator ch=tr.begin(it);
	parent.get_printer(ch)->print_infix(str, ch);
	str << "..";
	++ch;
	parent.get_printer(ch)->print_infix(str, ch);
	}

print_commutator::print_commutator(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_commutator::print_infix(std::ostream& str, exptree::iterator it)
	{
	bool prettyform;
	if(parent.output_format!=exptree_output::out_xcadabra && 
		parent.output_format!=exptree_output::out_texmacs ) 
		prettyform=false;
	else prettyform=true;
	
	bool anticommutator=true;
	if(*it->name=="\\commutator") anticommutator=false;

	if(*it->multiplier!=1) 
		print_multiplier(str, it);
	sibling_iterator ch=tr.begin(it);
	if(prettyform)	{
		if(anticommutator) str << "\\{";
		else               str << "[";
		}
	else {
		str << *it->name << "{";
		}

	parent.get_printer(ch)->print_infix(str, ch);

	if(prettyform)	str << ",";
	else str << "}{";

	++ch;
	parent.get_printer(ch)->print_infix(str, ch);

	if(prettyform)	{
		if(anticommutator) str << "\\}";
		else               str << "]";
		}
	else str << "}";
	}

print_indexbracket::print_indexbracket(exptree_output& eo)
	: node_printer(eo)
	{
	}

void print_indexbracket::print_infix(std::ostream& str, exptree::iterator it)
	{
	iterator arg=tr.begin(it);
//	if(*arg->name!="\\sum" && *arg->name!="\\prod") {
//		if(children_have_brackets(it))
//			print_opening_bracket(str,arg->fl.bracket);
//		}
//	else {
//		if(!children_have_brackets(tr.begin(it)))
			str << "(";
//		}
	parent.get_printer(tr.begin(it))->print_infix(str, tr.begin(it));
//	if(*arg->name!="\\sum" && *arg->name!="\\prod") {
//		if(children_have_brackets(it))
//			print_closing_bracket(str,arg->fl.bracket);
//		}
//	else {
//		if(!children_have_brackets(tr.begin(it)))
			str << ")";
//		}
	print_children(str, it, 1);
	}

/* --------------------------------------------------------------------- */

print_mathml_expression::print_mathml_expression(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

void print_mathml_expression::print_infix(std::ostream& str, iterator it) 
	{
	str << "<math display=\"block\" xmlns=\"http://www.w3.org/1998/Math/MathML\">" << std::endl;
	parent.get_printer(tr.begin(it))->print_infix(str, tr.begin(it));
	str << "</math>";
	}

print_mathml_productlike::print_mathml_productlike(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

print_mathml_wedge::print_mathml_wedge(exptree_output& eo)
	: print_mathml_productlike(eo)
	{
	}

void print_mathml_wedge::print_infix(std::ostream&, iterator) 
	{
	}

print_mathml_prod::print_mathml_prod(exptree_output& eo)
	: print_mathml_productlike(eo)
	{
	}

void print_mathml_prod::print_infix(std::ostream&, iterator) 
	{
	}

print_mathml_indexbracket::print_mathml_indexbracket(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

void print_mathml_indexbracket::print_infix(std::ostream&, iterator) 
	{
	}

print_mathml_pow::print_mathml_pow(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

void print_mathml_pow::print_infix(std::ostream&, iterator) 
	{
	}

print_mathml_frac::print_mathml_frac(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

void print_mathml_frac::print_infix(std::ostream&, iterator) 
	{
	}

print_mathml_sum::print_mathml_sum(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

void print_mathml_sum::print_infix(std::ostream&, iterator) 
	{
	}

print_mathml_sequence::print_mathml_sequence(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

void print_mathml_sequence::print_infix(std::ostream&, iterator) 
	{
	}

print_mathml_equals::print_mathml_equals(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

void print_mathml_equals::print_infix(std::ostream&, iterator) 
	{
	}

print_mathml_unequals::print_mathml_unequals(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

void print_mathml_unequals::print_infix(std::ostream&, iterator) 
	{
	}

print_mathml_factorial::print_mathml_factorial(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

void print_mathml_factorial::print_infix(std::ostream&, iterator) 
	{
	}

print_mathml_comma::print_mathml_comma(exptree_output& eo)
	: mathml_node_printer(eo)
	{
	}

void print_mathml_comma::print_infix(std::ostream&, iterator) 
	{
	}

/* --------------------------------------------------------------------- */

node_base_printer::node_base_printer(exptree_output& eo)
	: parent(eo), tr(eo.tr)
	{
	}

bool node_base_printer::children_have_brackets(iterator ch) const
	{
	sibling_iterator chlds=tr.begin(ch);
	str_node::bracket_t childbr=chlds->fl.bracket;
	if(childbr==str_node::b_none || childbr==str_node::b_no)
		return false;
	else return true;
	}


/* --------------------------------------------------------------------- */


node_printer::node_printer(exptree_output& eo)
	: node_base_printer(eo), isdelta(false), isweyl(false)
	{
	}

void node_printer::print_infix(std::ostream& str, exptree::iterator it)
	{
	// print multiplier and object name
	if(*it->multiplier!=1)
		print_multiplier(str, it);
	
	if(*it->name=="1") {
		if(*it->multiplier==1) // this would print nothing altogether.
			str << "1";
		return;
		}

	if(parent.output_format==exptree_output::out_xcadabra) {
		const LaTeXForm *lf=properties::get<LaTeXForm>(it);
		bool needs_extra_brackets=false;
		const Accent *ac=properties::get<Accent>(it);
		if(!ac) { // accents should never get additional curly brackets, {\bar}{g} does not print.
			sibling_iterator sib=tr.begin(it);
			while(sib!=tr.end(it)) {
				if(sib->is_index()) 
					needs_extra_brackets=true;
				++sib;
				}
			}
		
		if(needs_extra_brackets) str << "{"; // to prevent double sup/sub script errors
		if(lf) str << lf->latex;
		else   str << texify(*it->name);
		if(needs_extra_brackets) str << "}";
		}
	else str << *it->name;
	
	print_children(str, it);
	}

std::string node_printer::texify(const std::string& str) const
	{
	std::string res;
   for(unsigned int i=0; i<str.size(); ++i) {
		 if(str[i]=='#') res+="\\#";
		 else res+=str[i];
      }
   return res;
	}

void node_printer::print_children(std::ostream& str, exptree::iterator it, int skip) 
	{
	previous_bracket_   =str_node::b_invalid;
	previous_parent_rel_=str_node::p_none;

	int number_of_nonindex_children=0;
	int number_of_index_children=0;
	exptree::sibling_iterator ch=tr.begin(it);
	while(ch!=tr.end(it)) {
		if(ch->is_index()==false) {
			++number_of_nonindex_children;
			if(*ch->name=="\\prod")
				++number_of_nonindex_children;
			}
		else ++number_of_index_children;
		++ch;
		}
	
	ch=tr.begin(it);
	ch+=skip;
	unsigned int chnum=0;
	while(ch!=tr.end(it)) {
		current_bracket_   =(*ch).fl.bracket;
		current_parent_rel_=(*ch).fl.parent_rel;
		const Accent *is_accent=properties::get<Accent>(it);
		
		if(current_bracket_!=str_node::b_none || previous_bracket_!=current_bracket_ || previous_parent_rel_!=current_parent_rel_) {
			if(parent.output_format==exptree_output::out_plain ||
				parent.output_format==exptree_output::out_xcadabra ||
				parent.output_format==exptree_output::out_texmacs )
				print_parent_rel(str, current_parent_rel_, ch==tr.begin(it));
			if(parent.output_format!=exptree_output::out_reduce) {
				if(is_accent==0) 
					print_opening_bracket(str, (number_of_nonindex_children>1 /* &&number_of_index_children>0 */ &&
														 current_parent_rel_!=str_node::p_sub && 
														 current_parent_rel_!=str_node::p_super ? str_node::b_round:current_bracket_), 
												 current_parent_rel_);
				else str << "{";
				}
			else
				str << zwnbsp << "(" << zwnbsp;
			}
		if(parent.output_format==exptree_output::out_mathematica && getenv("CDB_MATH_COMPAC")==0) {
			if(ch!=tr.begin(it)) {
				if(chnum==2 && isweyl)
					str << "}," << nbsp << "{";
				else
					str << "," << nbsp;
				}
			}
		else if(parent.output_format==exptree_output::out_reduce) {
			if(ch!=tr.begin(it))
				str << "," << zwnbsp;
			}
		parent.get_printer(ch)->print_infix(str, ch);
//			if((*it).fl.mark && parent.highlight) str << "\033[1m"; 
		++ch;
		if(ch==tr.end(it) || current_bracket_!=str_node::b_none ||
			current_bracket_!=(*ch).fl.bracket || current_parent_rel_!=(*ch).fl.parent_rel) {
			if(parent.output_format!=exptree_output::out_reduce) {
				if(is_accent==0) 
					print_closing_bracket(str,  (number_of_nonindex_children>1 /* &&number_of_index_children>0 */ && 
														  current_parent_rel_!=str_node::p_sub && 
														  current_parent_rel_!=str_node::p_super ? str_node::b_round:current_bracket_), 
												 current_parent_rel_);
				else str  << "}";
				}
			else
				str << ")";
			}
		else if(parent.output_format==exptree_output::out_plain 
				  || parent.output_format==exptree_output::out_xcadabra
				  || parent.output_format==exptree_output::out_maple
				  || parent.output_format==exptree_output::out_texmacs)
			str << nbsp;
		
		previous_bracket_=current_bracket_;
		previous_parent_rel_=current_parent_rel_;
		++chnum;
		}
	}

void node_printer::print_multiplier(std::ostream& str, exptree::iterator it)
	{
	bool turned_one=false;
	mpz_class denom=it->multiplier->get_den();

	if(*it->multiplier<0) {
		if(*tr.parent(it)->name=="\\sum") { // sum takes care of minus sign
			if(*it->multiplier!=-1) {
				if(denom!=1 && ( parent.output_format==exptree_output::out_texmacs ||
									  parent.output_format==exptree_output::out_xcadabra) ) {
					str << "\\frac{" << -(it->multiplier->get_num()) << "}{" 
						 << it->multiplier->get_den() << "}";
					}
				else {
					str << -(*it->multiplier);
					}
				}
			else                    turned_one=true;
			}
		else	{
			if(denom!=1 && ( parent.output_format==exptree_output::out_texmacs
								  || parent.output_format==exptree_output::out_xcadabra) ) {
				str << "(\\frac{" << it->multiplier->get_num() << "}{" 
					 << it->multiplier->get_den() << "})";
				}
			else {
				str << "(" << *it->multiplier << ")";
				}
			}
		}
	else {
		if(denom!=1 && (parent.output_format==exptree_output::out_texmacs
							 || parent.output_format==exptree_output::out_xcadabra) ) {
			str << "\\frac{" << it->multiplier->get_num() << "}{" 
				 << it->multiplier->get_den() << "}";
			}
		else
			str << *it->multiplier;
		}

	if(!turned_one && !(*it->name=="1")) {
		if(parent.print_star && !(parent.output_format==exptree_output::out_texmacs
								  || parent.output_format==exptree_output::out_xcadabra) ) {
			if(parent.tight_star) str << "*";
			else if(parent.utf8_output)
				str << unichar(0x00a0) << "*" << unichar(0x00a0);
			else
				str << " * ";
			}
		else {
			if(!parent.tight_star) {
				if(parent.output_format==exptree_output::out_texmacs
					|| parent.output_format==exptree_output::out_xcadabra ) str << "\\, ";
				else str << " ";
				}
			} 
		}
	}

void node_printer::print_opening_bracket(std::ostream& str, str_node::bracket_t br, str_node::parent_rel_t pr)
	{
	switch(br) {
		case str_node::b_none: 
			if(parent.output_format==exptree_output::out_xcadabra && pr==str_node::p_none) str << "(";  
			else                                                                           str << "{";
			break;
		case str_node::b_pointy: str << "\\<"; break;
		case str_node::b_curly:  str << "\\{"; break;
		case str_node::b_round:  str << "(";   break;
		case str_node::b_square: str << "[";   break;
		default :	return;
		}
	++(parent.bracket_level);
	}

void node_printer::print_closing_bracket(std::ostream& str, str_node::bracket_t br, str_node::parent_rel_t pr)
	{
	switch(br) {
		case str_node::b_none:   
			if(parent.output_format==exptree_output::out_xcadabra && pr==str_node::p_none) str << ")";  
			else                                                                           str << "}";
			break;
		case str_node::b_pointy: str << "\\>"; break;
		case str_node::b_curly:  str << "\\}"; break;
		case str_node::b_round:  str << ")";   break;
		case str_node::b_square: str << "]";   break;
		default :	return;
		}
	--(parent.bracket_level);
	}

void node_printer::print_parent_rel(std::ostream& str, str_node::parent_rel_t pr, bool first)
	{
	switch(pr) {
		case str_node::p_super:    
			if(!first && (parent.output_format==exptree_output::out_texmacs || 
							  parent.output_format==exptree_output::out_xcadabra) ) str << "\\,";
			str << "^"; break;
		case str_node::p_sub:
			if(!first && (parent.output_format==exptree_output::out_texmacs ||
							  parent.output_format==exptree_output::out_xcadabra)) str << "\\,";
			str << "_"; break;
		case str_node::p_property: str << "$"; break;
		case str_node::p_exponent: str << "**"; break;
		case str_node::p_none: break;
		}
	// Prevent line break after this character.
	str << zwnbsp;
	}


void exptree_output::print_full_standardform(std::ostream& str, exptree::iterator it, bool eqno)
	{
	if(eqno) {
		nset_t::iterator name=tr.equation_label(it);
		if(xml_structured) 
			str << "<eqno>" << std::endl;
		if(name!=name_set.end()) str << *name;
		else                     str << tr.equation_number(it);
		if(!xml_structured) str << ":= ";
		else                str << std::endl << "</eqno>" << std::endl;
		}

	if(output_format==exptree_output::out_xcadabra) { // first output plain
		output_format=exptree_output::out_plain;
		str << "<plain>" << std::endl;
		print_infix(str, tr.active_expression(it));
		str << std::endl << "</plain>" << std::endl;
		output_format=exptree_output::out_xcadabra;
		}

	if(xml_structured) str << "<eq>" << std::endl;
	print_infix(str, tr.active_expression(it));
	str << ";";
	if(output_format==exptree_output::out_plain) str << std::endl;
	if(xml_structured) str << std::endl << "</eq>" << std::endl;
	}

/* ----------------------------------------------------------------------- */

mathml_node_printer::mathml_node_printer(exptree_output& eo)
	: node_base_printer(eo)
	{
	}

void mathml_node_printer::print_infix(std::ostream& str, exptree::iterator it)
	{
//	if((*it).fl.mark && parent.highlight) str << "\033[1m";

	// print multiplier and object name
	if(*it->multiplier!=1)
		print_multiplier(str, it);

	if(*it->name=="1") {
		if(*it->multiplier==1) // this would print nothing altogether.
			str << "1";
		return;
		}

	str << *it->name;

	print_children(str, it);
	}

void mathml_node_printer::print_children(std::ostream& str, exptree::iterator it, int skip) 
	{
	previous_bracket_   =str_node::b_invalid;
	previous_parent_rel_=str_node::p_none;

	int number_of_nonindex_children=0;
	int number_of_index_children=0;
	exptree::sibling_iterator ch=tr.begin(it);
	while(ch!=tr.end(it)) {
		if(ch->is_index()==false) {
			++number_of_nonindex_children;
			if(*ch->name=="\\prod")
				++number_of_nonindex_children;
			}
		else ++number_of_index_children;
		++ch;
		}
	
	ch=tr.begin(it);
	ch+=skip;
	unsigned int chnum=0;
	while(ch!=tr.end(it)) {
		current_bracket_   =(*ch).fl.bracket;
		current_parent_rel_=(*ch).fl.parent_rel;
		
		if(current_bracket_!=str_node::b_none || previous_bracket_!=current_bracket_ || previous_parent_rel_!=current_parent_rel_) {
			if(parent.output_format==exptree_output::out_plain 
				|| parent.output_format==exptree_output::out_xcadabra
				|| parent.output_format==exptree_output::out_texmacs)
				print_parent_rel(str, current_parent_rel_, ch==tr.begin(it));
			if(parent.output_format!=exptree_output::out_reduce)
				print_opening_bracket(str, (number_of_nonindex_children>1 && number_of_index_children>0 &&
													 current_parent_rel_!=str_node::p_sub && 
													 current_parent_rel_!=str_node::p_super ? str_node::b_round:current_bracket_), current_parent_rel_);
			else
				str << zwnbsp << "(" << zwnbsp;
			}
		if(parent.output_format==exptree_output::out_mathematica && getenv("CDB_MATH_COMPAC")==0) {
			if(ch!=tr.begin(it)) {
				str << "," << nbsp;
				}
			}
		else if(parent.output_format==exptree_output::out_reduce) {
			if(ch!=tr.begin(it))
					str << "," << zwnbsp;
			}
		parent.get_printer(ch)->print_infix(str, ch);
//			if((*it).fl.mark && parent.highlight) str << "\033[1m"; 
		++ch;
		if(ch==tr.end(it) || current_bracket_!=str_node::b_none ||
			current_bracket_!=(*ch).fl.bracket || current_parent_rel_!=(*ch).fl.parent_rel) {
			if(parent.output_format!=exptree_output::out_reduce)
				print_closing_bracket(str,  (number_of_nonindex_children>1 && number_of_index_children>0 && 
													  current_parent_rel_!=str_node::p_sub && 
													  current_parent_rel_!=str_node::p_super ? str_node::b_round:current_bracket_), current_parent_rel_);
			else
				str << ")";
			}
		else if(parent.output_format==exptree_output::out_plain 
				  || parent.output_format==exptree_output::out_xcadabra
				  || parent.output_format==exptree_output::out_maple
				  || parent.output_format==exptree_output::out_texmacs)
			str << nbsp;
		
		previous_bracket_=current_bracket_;
		previous_parent_rel_=current_parent_rel_;
		++chnum;
		}
	if(parent.output_format==exptree_output::out_mathematica && 
		it->fl.parent_rel==str_node::p_none && tr.number_of_children(it)>0)
		str << "]";

//	if((*it).fl.mark && parent.highlight) str << "\033[0m";	
	}

void mathml_node_printer::print_multiplier(std::ostream& str, exptree::iterator it)
	{
	bool turned_one=false;
	mpz_class denom=it->multiplier->get_den();

	if(*it->multiplier<0) {
		if(*tr.parent(it)->name=="\\sum") { // sum takes care of minus sign
			if(*it->multiplier!=-1) {
				if(denom!=1 && (parent.output_format==exptree_output::out_texmacs
									 || parent.output_format==exptree_output::out_xcadabra) ) {
					str << "\\frac{" << -(it->multiplier->get_num()) << "}{" 
						 << it->multiplier->get_den() << "}";
					}
				else {
					str << -(*it->multiplier);
					}
				}
			else                    turned_one=true;
			}
		else	{
			if(denom!=1 && (parent.output_format==exptree_output::out_texmacs
								 || parent.output_format==exptree_output::out_xcadabra) ) {
				str << "(\\frac{" << it->multiplier->get_num() << "}{" 
					 << it->multiplier->get_den() << "})";
				}
			else {
				str << "(" << *it->multiplier << ")";
				}
			}
		}
	else {
		if(denom!=1 && (parent.output_format==exptree_output::out_texmacs 
							 || parent.output_format==exptree_output::out_xcadabra) ) {
			str << "\\frac{" << it->multiplier->get_num() << "}{" 
				 << it->multiplier->get_den() << "}";
			}
		else
			str << *it->multiplier;
		}

	if(!turned_one && !(*it->name=="1")) {
		if(parent.print_star && !parent.output_format==exptree_output::out_texmacs &&
			!parent.output_format==exptree_output::out_xcadabra ) {
			if(parent.tight_star) str << "*";
			else if(parent.utf8_output)
				str << unichar(0x00a0) << "*" << unichar(0x00a0);
			else
				str << " * ";
			}
		else {
			if(!parent.tight_star) {
				if(parent.output_format==exptree_output::out_texmacs ||
					parent.output_format==exptree_output::out_xcadabra ) str << "\\, ";
				else str << " ";
				}
			} 
		}
	}

void mathml_node_printer::print_opening_bracket(std::ostream& str, str_node::bracket_t br, str_node::parent_rel_t pr)
	{
	switch(br) {
		case str_node::b_none:   str << "{";   break;
		case str_node::b_pointy: str << "\\<"; break;
		case str_node::b_curly:  str << "\\{"; break;
		case str_node::b_round:  str << "(";   break;
		case str_node::b_square: str << "[";   break;
		default :	return;
		}
	++(parent.bracket_level);
	}

void mathml_node_printer::print_closing_bracket(std::ostream& str, str_node::bracket_t br, str_node::parent_rel_t pr)
	{
	switch(br) {
		case str_node::b_none:   str << "}";   break;
		case str_node::b_pointy: str << "\\>"; break;
		case str_node::b_curly:  str << "\\}"; break;
		case str_node::b_round:  str << ")";   break;
		case str_node::b_square: str << "]";   break;
		default :	return;
		}
	--(parent.bracket_level);
	}

void mathml_node_printer::print_parent_rel(std::ostream& str, str_node::parent_rel_t pr, bool first)
	{
	switch(pr) {
		case str_node::p_super:    
			if(!first && ( parent.output_format==exptree_output::out_texmacs
								|| parent.output_format==exptree_output::out_xcadabra) ) str << "\\,";
			str << "^"; break;
		case str_node::p_sub:
			if(!first && ( parent.output_format==exptree_output::out_texmacs
								|| parent.output_format==exptree_output::out_xcadabra) ) str << "\\,";
			str << "_"; break;
		case str_node::p_property: str << "$"; break;
		case str_node::p_exponent: str << "**"; break;
		case str_node::p_none: break;
		}
	// Prevent line break after this character.
	str << zwnbsp;
	}



/* ----------------------------------------------------------------------- */


// Thanks to Behdad Esfahbod
int k_unichar_to_utf8(kunichar c, char *buf) 
	{
	buf[0]=(c) < 0x00000080 ?   (c)                  :
      (c) < 0x00000800 ?  ((c) >>  6)         | 0xC0 :
      (c) < 0x00010000 ?  ((c) >> 12)         | 0xE0 :
      (c) < 0x00200000 ?  ((c) >> 18)         | 0xF0 :
      (c) < 0x04000000 ?  ((c) >> 24)         | 0xF8 :
		((c) >> 30)         | 0xFC;

	buf[1]=(c) < 0x00000080 ?    0 /* null-terminator */     : 
      (c) < 0x00000800 ?  ((c)        & 0x3F) | 0x80 :         
      (c) < 0x00010000 ? (((c) >>  6) & 0x3F) | 0x80 :         
      (c) < 0x00200000 ? (((c) >> 12) & 0x3F) | 0x80 :         
      (c) < 0x04000000 ? (((c) >> 18) & 0x3F) | 0x80 :         
                            (((c) >> 24) & 0x3F) | 0x80;

	buf[2]=(c) < 0x00000800 ?    0 /* null-terminator */     : 
      (c) < 0x00010000 ?  ((c)        & 0x3F) | 0x80 :         
      (c) < 0x00200000 ? (((c) >>  6) & 0x3F) | 0x80 :         
      (c) < 0x04000000 ? (((c) >> 12) & 0x3F) | 0x80 :         
		(((c) >> 18) & 0x3F) | 0x80;

	buf[3]=(c) < 0x00010000 ?    0 /* null-terminator */     : 
      (c) < 0x00200000 ?  ((c)        & 0x3F) | 0x80 :         
      (c) < 0x04000000 ? (((c) >>  6) & 0x3F) | 0x80 :         
		(((c) >> 12) & 0x3F) | 0x80;

	buf[4]=(c) < 0x00200000 ?    0 /* null-terminator */     : 
      (c) < 0x04000000 ?  ((c)        & 0x3F) | 0x80 :         
		(((c) >>  6) & 0x3F) | 0x80;

	buf[5]=(c) < 0x04000000 ?    0 /* null-terminator */     : 
                             ((c)        & 0x3F) | 0x80;

	buf[6]=0;
	return 6;
	}

const char *unichar(kunichar c)
	{
	static char buffer[7];
	int pos=k_unichar_to_utf8(c, buffer);
	buffer[pos]=0;
	return buffer;
	}

