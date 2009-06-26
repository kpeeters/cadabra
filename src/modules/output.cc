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

#include "output.hh"
#include "props.hh"

void output::register_properties()
	{
	properties::register_property(&create_property<LaTeXForm>);
	}

std::string LaTeXForm::name() const
	{
	return "LaTeXForm";
	}

std::string LaTeXForm::unnamed_argument() const
	{
	return "latex";
	}

bool LaTeXForm::parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals)
	{
	keyval_t::const_iterator kv=keyvals.find("latex");
	if(kv!=keyvals.end()) latex=*(kv->second->name);
	// FIXME: handle errors.
	latex=latex.substr(1,latex.size()-2);
	return true;
	}

tree_dump::tree_dump(exptree& thetr, iterator it)
	: algorithm(thetr, it)
	{
	}

void tree_dump::description() const
	{
	txtout << "Dump tree output" << std::endl;
	}

bool tree_dump::can_apply(iterator)
	{
	return true;
	}

algorithm::result_t tree_dump::apply(iterator& st)
	{
	debugout << "tree_dump: start" << std::endl;
	if(st==active_node::tr.end()) {
		debugout << "tree_dump: printing entire tree" << std::endl;
		sibling_iterator eqs=active_node::tr.begin();
		while(eqs!=active_node::tr.end()) {
			if(!(eqs==active_node::tr.named_parent(this_command,"\\history"))) {
				sibling_iterator theexp=eqs;
				if(*eqs->name=="\\history")
					theexp=active_node::tr.active_expression(eqs);
				print_one(txtout, theexp);
				}
			++eqs;
			}
		}
	else {
		sibling_iterator topn=active_node::tr.named_parent(st,"\\history");
		print_one(txtout, topn);
		}
	return l_applied;
	}

void tree_dump::print_one(std::ostream& out, sibling_iterator st) const
	{
	unsigned int num=1;

	bool isnumbered=false;
	unsigned int eqno=active_node::tr.equation_number(st);
	if(eqno!=0) 
		isnumbered=true;

	if(isnumbered)
		out << "(" << eqno << "): ";
	active_node::tr.print_recursive_treeform(out, st, num) << std::endl;
	out << std::flush;
	}


print::print(exptree&tr, iterator it)
	: algorithm(tr, it)
	{
	}

void print::description(void) const
	{
	txtout << "Generate arbitrary output from strings and expressions." << std::endl;
	}

bool print::can_apply(iterator)
	{
	return true;
	}

algorithm::result_t print::apply(iterator& st)
	{
	if(*(st->name)=="\\tie") {
		sibling_iterator si=active_node::tr.begin(st);
		while(si!=active_node::tr.end(st)) {
			if(si->is_quoted_string())
			   forcedout << (*si->name).substr(1,(*si->name).size()-2);
			else {
				if(*si->name=="endl") forcedout << std::endl;
				else                  eo->print_infix(forcedout, si);
				}
			++si;
			}
		}
	else {
		if(st->is_quoted_string())
			forcedout << (*st->name).substr(1,(*st->name).size()-2);
		else {
			if(*st->name=="endl") forcedout << std::endl;
			else                  eo->print_infix(forcedout, st);
			}
		}
	forcedout << std::endl;
	discard_command_node=true;
	return l_applied;
	}

bool print::is_output_module() const
	{
	return true;
	}

indexlist::indexlist(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void indexlist::description() const
	{
	txtout << "Show a list of all free and dummy indices in a given expression." << std::endl;
	}

bool indexlist::can_apply(iterator)
	{
	return true;
	}

algorithm::result_t indexlist::apply(iterator& st)
	{
	exptree::index_iterator ii=active_node::tr.begin_index(st);
	while(ii!=active_node::tr.end_index(st)) {
		txtout << "index: " << *ii->name << std::endl;
		++ii;
		}

	print_classify_indices(st);

	return l_applied;
	}


assert_or_exit::assert_or_exit(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void assert_or_exit::description() const
	{
	txtout << "Assure that the indicated expression is zero. Exit otherwise." << std::endl;
	}

bool assert_or_exit::can_apply(iterator st)
	{
	return true;
	}

algorithm::result_t assert_or_exit::apply(iterator& st)
	{
	if(*st->multiplier!=0)
		throw std::logic_error("Failed to satisfy assert check.");

	txtout << "Assert check passed." << std::endl;
	return l_applied;
	}

number_of_terms::number_of_terms(exptree&tr, iterator it)
	: algorithm(tr, it)
	{
	}

void number_of_terms::description() const
	{
	txtout << "Count the number of terms in a sum." << std::endl;
	}

bool number_of_terms::can_apply(iterator st)
	{
	if(*st->name=="\\sum") return true;
	return false;
	}

algorithm::result_t number_of_terms::apply(iterator& st)
	{
	assert(*st->name=="\\sum");
	txtout << active_node::tr.number_of_children(st) << std::endl;
	return l_applied;
	}

memdump::memdump(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void memdump::description() const
	{
	txtout << "Displays memory usage." << std::endl;
	}

bool memdump::can_apply(iterator)
	{
	return true;
	}

algorithm::result_t memdump::apply(iterator&)
	{
	iterator st=active_node::tr.begin();
	unsigned int noe=0;
	while(st!=active_node::tr.end()) {
		++noe;
		st.skip_children();
		++st;
		}
	unsigned int numnodes=active_node::tr.size();
	float numbytes=numnodes * sizeof(tree_node_<str_node>);
	
	txtout << "# of names      : " << name_set.size() << std::endl
			 << "# of rationals  : " << rat_set.size() << std::endl
			 << "# of nodes      : " << numnodes
			 << " (= ";
	int mult=0;
	while(numbytes>1024) {
		numbytes/=1024;
		++mult;
		}
	txtout << numbytes;
	switch(mult) {
		case 0:
			txtout << " bytes";
			break;
		case 1:
			txtout << " Kb";
			break;
		case 2:
			txtout << " Mb";
			break;
		case 3:
			txtout << " Gb";
			break;
		case 4:
			txtout << " Tb";
			break;
		}
	txtout << ")" << std::endl
			 << "# of expressions: " << noe << std::endl;
	
	return l_applied;
	}


depprint::depprint(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void depprint::description() const
	{
	txtout << "Prints an expression including all property dependencies." << std::endl;
	}

bool depprint::can_apply(iterator)
	{
	return true;
	}

algorithm::result_t depprint::apply(iterator& it)
	{
	iterator end=it;
	end.skip_children();
	++end;
	iterator walk=it;
	std::set<nset_t::iterator, nset_it_less> done;
	while(walk!=end) {
		if(done.count(walk->name)==0) {
			properties::property_map_t::iterator pit=properties::props.begin();
			while(pit!=properties::props.end()) {
				if((*pit).second.first->obj.begin()->name==walk->name) {  // FIXME: match pattern
					eo->print_infix(txtout, (*pit).second.first->obj.begin());
					txtout << "::";
					(*pit).second.second->display(txtout);
					txtout << "." << std::endl;
					}
				++pit;
				}
			done.insert(walk->name);
			}
		++walk;
		}
	txtout << std::endl;
	eo->print_infix(txtout, active_node::tr.active_expression(it));
	txtout << std::endl;
	return l_applied;
	}


eqs::eqs(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void eqs::description() const
	{
	txtout << "Displays all expressions." << std::endl;
	}

bool eqs::can_apply(iterator)
	{
	return true;
	}

algorithm::result_t eqs::apply(iterator&)
	{
	exptree::sibling_iterator eit=active_node::tr.begin();
	int eqno=1;
	while(eit!=active_node::tr.end()) {
		if(eit!=active_node::tr.named_parent(this_command, "\\history")) {
			if(*eit->name=="\\history") {
				eo->print_full_standardform(txtout, eit, eqno);
				txtout << std::endl;
				}
			}
		++eit;
		}
		
	return l_applied;
	}


//   adjmatrix::adjmatrix(exptree& tr, iterator it)
//   	: algorithm(tr,it), exptree_output(tr, txtout)
//   	{
//   	}
//   
//   void adjmatrix::description() const
//   	{
//   	txtout << "Display adjacency matrix form of a product." <<std::endl;
//   	}
//   
//   bool adjmatrix::can_apply(iterator it)
//   	{
//   	if(*it->name=="\\prod") return true;
//   	else return false;
//   	}
//   
//   algorithm::result_t adjmatrix::apply(iterator& it)
//   	{
//   	expgraph mygraph(algorithm::tr, it, args_begin());
//   	mygraph.debug_output(txtout);
//   	return l_applied;
//   	}


proplist::proplist(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void proplist::description() const
	{
	txtout << "Display all properties." <<std::endl;
	}

bool proplist::can_apply(iterator it)
	{
	return true;
	}

algorithm::result_t proplist::apply(iterator& it)
	{
	properties::pattern_map_t::iterator pit=properties::pats.begin();
	
	while(pit!=properties::pats.end()) {
		int num = properties::pats.count(pit->first);
		if(num>1) {
			txtout << "{";
			while(num>0) {
				eo->print_prefix(txtout, pit->second->obj.begin());
				if(num>1) {
					txtout << ", ";
					++pit;
					}
				--num;
				}
			txtout << "}";
			}
		else eo->print_prefix(txtout, pit->second->obj.begin());

		txtout << "::";
		pit->first->display(txtout);
		txtout << ")" << std::endl;
		++pit;
		}

	return l_applied;
	}
