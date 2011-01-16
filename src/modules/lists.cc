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

#include "lists.hh"

length::length(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void length::description() const
	{
	txtout << "Replace the node with its number of arguments." << std::endl;
	}

bool length::can_apply(iterator it) 
	{
	return true;
	}

algorithm::result_t length::apply(iterator& it)
	{
	int ret=tr.number_of_children(it);
	
	node_one(it);
	multiply(it->multiplier, ret);
	expression_modified=true;

	return l_applied;
	}




take::take(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	if(number_of_args()!=1) {
		txtout << "Need a number, a comma-separated list of numbers or a range." << std::endl;
		throw constructor_error();
		}

	nums.clear();
	if(*args_begin()->name=="\\comma") {
		take_type=t_list;
		sibling_iterator sib=tr.begin(args_begin());
		while(sib!=tr.end(args_begin())) {
			if(to_long(*sib->multiplier)<0) {
				txtout << "Elements should be positive or zero." << std::endl;
				throw constructor_error();
				}
			nums.push_back(to_long(*sib->multiplier));
			++sib;
			}
		}
	else if(*args_begin()->name=="\\sequence") {
		take_type=t_sequence;
		sibling_iterator sib=tr.begin(args_begin());
		nums.push_back(to_long(*sib->multiplier));
		++sib;
		if(*sib->name=="\\infty")
			nums.push_back(-1);
		else
			nums.push_back(to_long(*sib->multiplier));
		}
	else {
		take_type=t_single;
		nums.push_back(to_long(*args_begin()->multiplier));
		}
	}

void take::description() const
	{
	txtout << "Isolate selected terms, factors or list elements." << std::endl;
	}

bool take::can_apply(iterator it) 
	{
	if(*it->name=="\\sum" || *it->name=="\\prod" || *it->name=="\\comma") return true;
	return false;
	}

algorithm::result_t take::apply(iterator& it)
	{
	sibling_iterator sib=tr.begin(it);
	long ind=0;
	while(sib!=tr.end(it)) {
		switch(take_type) {
			case t_single:
				if(ind!=nums[0]) {
					sib=tr.erase(sib);
					expression_modified=true;
					}
				else ++sib;
				break;
			case t_list: {
				bool keep=false;
				for(unsigned int i=0; i<nums.size(); ++i) {
					if(ind==nums[i]) {
						keep=true;
						break;
						}
					}
				if(!keep) {
					sib=tr.erase(sib);
					expression_modified=true;
					}
				else ++sib;
				break;
				}
			case t_sequence:
				if(ind<nums[0] || (nums[1]>=0 && ind>nums[1])) {
					sib=tr.erase(sib);
					expression_modified=true;
					}
				else ++sib;
				break;
			}
		++ind;
		}

	cleanup_sums_products(tr, it);

	if(expression_modified) return l_applied;
	else return l_no_action;
	}



list_sum::list_sum(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void list_sum::description() const
	{
	txtout << "Replace a sum of equal-length lists with a list of summed elements." << std::endl;
	}

bool list_sum::can_apply(iterator it) 
	{
	if(*it->name=="\\sum") {
		int len=-1;
		for(sibling_iterator sib=tr.begin(it); sib!=tr.end(it); ++sib) {
			if(*sib->name!="\\comma")
				return false;
			if(len==-1) len=tr.number_of_children(sib);
			else if(len!=static_cast<int>(tr.number_of_children(sib)))
				return false;
			}
		return true;
		}
	return false;
	}

algorithm::result_t list_sum::apply(iterator& it)
	{
	sibling_iterator frstlist=tr.begin(it);

	// Wrap all elements of the first list in a sum node.
	for(sibling_iterator eli=tr.begin(frstlist); eli!=tr.end(frstlist); ) {
		sibling_iterator nxt=eli;
		++nxt;
		tr.wrap(eli, str_node("\\sum"));
		if(*eli->name=="\\sum") { // fix brackets if necessary
			if(tr.begin(eli)->fl.bracket==str_node::b_none) {
				for(sibling_iterator trm=tr.begin(eli); trm!=tr.end(eli); ++trm)
					trm->fl.bracket=str_node::b_round;
				}
			}
		eli=nxt;
		}

	// Do the actual addition.
	sibling_iterator lit=frstlist;
	++lit;
	while(lit!=tr.end(it)) {
		for(sibling_iterator eli=tr.begin(frstlist), eli2=tr.begin(lit); eli!=tr.end(frstlist); ++eli, ++eli2) {
			iterator tmp=tr.append_child(eli, eli2);
			multiply(tmp->multiplier, *lit->multiplier);
			if(*tmp->name=="\\sum") { // fix brackets if necessary
				if(tr.begin(tmp)->fl.bracket==str_node::b_none) {
					for(sibling_iterator trm=tr.begin(tmp); trm!=tr.end(tmp); ++trm)
						trm->fl.bracket=str_node::b_round;
					}
				}
			}
		lit=tr.erase(lit);
		}
	tr.flatten(it);
	it=tr.erase(it);

	expression_modified=true;
	return l_applied;
	}



range::range(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void range::description() const
	{
	txtout << "Replace the two-element list with a list of the range." << std::endl;
	}

bool range::can_apply(iterator it) 
	{
	if(*it->name=="\\comma") {
		sym.clear();
		pat.clear();

		num=tr.number_of_children(it);
		if(num!=2 && num!=3 && num!=4) return false;

		sibling_iterator sib=tr.begin(it);
		if(num==4) { 
			sym=exptree(sib);
			if(sym.number_of_children(sym.begin())!=0) {
				txtout << "Iteration variable must be a single symbol." << std::endl;
				return false;
				}
			++sib;
			}
		from=to_long(*sib->multiplier);
		++sib;
		to=to_long(*sib->multiplier);
		++sib;
		if(from>to) return false;

		if(num==4 || num==3)
			pat=exptree(sib);
		return true;
		}
	return false;
	}

algorithm::result_t range::apply(iterator& it)
	{
	tr.erase_children(it);
	
	for(int i=from; i<=to; ++i) {
		if(num==3 || num==4) {
			if(num==3)
				tr.append_child(it, pat.begin());
			else {
				exptree reppat(pat);
				iterator tmpit=reppat.begin();
				while(tmpit!=reppat.end()) {
					if(tmpit->name==sym.begin()->name) {
						node_one(tmpit);
						multiply(tmpit->multiplier, i);
						}
					++tmpit;
					}
				tr.append_child(it, reppat.begin());
				}
			}
		else {
			iterator nw=tr.append_child(it, str_node("1"));
			multiply(nw->multiplier, i);
			}
		}
	cleanup_expression(tr, it);
	
	expression_modified=true;

	return l_applied;
	}



inner::inner(exptree& tr, iterator it) 
	: algorithm(tr, it)
	{
	}

void inner::description() const
	{
	txtout << "Construct an inner product between two lists appearing in a list." << std::endl;
	}

bool inner::can_apply(iterator it) 
	{
	if(*it->name!="\\comma") return false;
	if(tr.number_of_children(it)!=2) return false;
	sibling_iterator sib=tr.begin(it);
	unsigned int num=tr.number_of_children(sib);
	if(*sib->name!="\\comma") return false;
	++sib;
	if(*sib->name!="\\comma") return false;
	if(tr.number_of_children(sib)!=num) return false;
	return true;
	}

algorithm::result_t inner::apply(iterator& it)
	{
	sibling_iterator com1=tr.begin(it);
	sibling_iterator com2=com1;
	++com2;
	sibling_iterator lst1=tr.begin(com1);
	sibling_iterator lst2=tr.begin(com2);

	while(lst1!=tr.end(com1)) {
		sibling_iterator fr=lst1;
		sibling_iterator to=lst1;
		++to;
		multiplier_t mult=(*lst1->multiplier) * (*lst2->multiplier);
		if(mult!=0) {
			one(lst1->multiplier);
			one(lst2->multiplier);
			iterator prodit=tr.insert(lst1,str_node("\\prod"));
			prodit->multiplier=rat_set.insert(mult).first;
			++lst1;
			tr.reparent(prodit, fr, to);
			tr.append_child(prodit, (iterator)(lst2));
			++lst2;
			}
		else {
			lst1=tr.erase(lst1);
			++lst2;
			}
		}
//	txtout << "reparent done" << std::endl;
	tr.erase(com2);
	tr.flatten(it);
//	txtout << "reparent done" << std::endl;
	it=tr.erase(it);
	it->name=name_set.insert("\\sum").first;
//	tr.print_recursive_treeform(txtout, it);

	cleanup_expression(tr, it);
	cleanup_nests(tr, it);
	expression_modified=true;

	return l_applied;
	}


coefficients::coefficients(exptree& tr, iterator it) 
	: algorithm(tr, it)
	{
	if(number_of_args()!=1) {
		txtout << "Need one argument" << std::endl;
		throw constructor_error();
		}

	obj=exptree(args_begin());
	}

void coefficients::description() const
	{
	txtout << "Given a sum, create a list representing the coefficients of the powers of the indicated object." << std::endl;
	}

bool coefficients::can_apply(iterator it) 
	{
	if(*it->name=="\\sum") return true;
	else return false;
	}

algorithm::result_t coefficients::apply(iterator& it)
	{
	// The replacement list.
	std::map<int, exptree> power_to_coeff;

	// Loop over all terms.
	sibling_iterator sib=tr.begin(it);
	while(sib!=tr.end(it)) {
		if(*sib->name=="\\prod") {
			// Find the power of 
			}
		else {
			
			}
		++sib;
		}

	return l_applied;
	}

/* 

symbolic_inner::symbolic_inner(exptree& tr, iterator it) 
	: algorithm(tr, it)
	{
	}

void symbolic_inner::description() const
	{
	txtout << "Construct a symbolic inner product between two lists appearing in a list." << std::endl;
	}

bool symbolic_inner::can_apply(iterator it) 
	{
	if(*it->name!="\\comma") return false;
	if(tr.number_of_children(it)!=2) return false;
	sibling_iterator sib=tr.begin(it);
	unsigned int num=tr.number_of_children(sib);
	if(*sib->name!="\\comma") return false;
	++sib;
	if(*sib->name!="\\comma") return false;
	if(tr.number_of_children(sib)!=num) return false;
	return true;
	}

algorithm::result_t symbolic_inner::apply(iterator& it)
	{
	sibling_iterator com1=tr.begin(it);
	sibling_iterator com2=com1;
	++com2;
	sibling_iterator lst1=tr.begin(com1);
	sibling_iterator lst2=tr.begin(com2);

	while(lst1!=tr.end(com1)) {
		sibling_iterator fr=lst1;
		sibling_iterator to=lst1;
		++to;
		multiplier_t mult=(*lst1->multiplier) * (*lst2->multiplier);
		if(mult!=0) {
			one(lst1->multiplier);
			one(lst2->multiplier);
			iterator prodit=tr.insert(lst1,str_node("\\prod"));
			prodit->multiplier=rat_set.insert(mult).first;
			++lst1;
			tr.reparent(prodit, fr, to);
			tr.append_child(prodit, (iterator)(lst2));
			++lst2;
			}
		else {
			lst1=tr.erase(lst1);
			++lst2;
			}
		}
//	txtout << "reparent done" << std::endl;
	tr.erase(com2);
	tr.flatten(it);
//	txtout << "reparent done" << std::endl;
	it=tr.erase(it);
	it->name=name_set.insert("\\sum").first;
//	tr.print_recursive_treeform(txtout, it);

	cleanup_expression(tr, it);
	cleanup_nests(tr, it);
	expression_modified=true;

	return l_applied;
	}

*/
