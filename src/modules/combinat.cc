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

#include "combinat.hh"
#include "combinatorics.hh"

permute::permute(exptree& tr, iterator it) 
	: algorithm(tr, it)
	{
	}

void permute::description() const
	{
	txtout << "Generic combinatorial algorithm" << std::endl;
	}

bool permute::can_apply(iterator it) 
	{
	// hack: the order of arguments is 
	//   multiplepick, # elements in result, sublengths, 
	// FIXME: proper argument passing.
	
	if(*it->name!="\\comma") return false;
	if(number_of_args()<2) return false;

	com.clear();
	sibling_iterator arg=args_begin();
	if(*arg->name=="true") com.multiple_pick=true;
	++arg;
	scan_through_range=false;
	if(*arg->name!="\\comma") {
		if(*arg->name=="\\less") {
			scan_through_range=true;
			com.sublengths.push_back(to_long(*tr.begin(arg)->multiplier));
			}
		else com.sublengths.push_back(to_long(*arg->multiplier));
		}
	else {
		sibling_iterator subl=tr.begin(arg);
		while(subl!=tr.end(arg)) {
			com.sublengths.push_back(to_long(*subl->multiplier));
			++subl;
			}
		}
	if(com.multiple_pick) {
		++arg;
		while(arg!=args_end()) {
			combin::weights_t tmp;
			sibling_iterator subl=tr.begin(arg);
			while(subl!=tr.end(arg)) {
				tmp.push_back(to_long(*subl->multiplier));
				++subl;
				}
			com.weights.push_back(tmp);
			++arg;
			assert(arg!=args_end());
			if(*arg->name=="\\less") {
				com.max_weights.push_back(to_long(*tr.begin(arg)->multiplier));
				com.weight_conditions.push_back(combin::combinations<iterator>::weight_less);
				}
			else {
				com.max_weights.push_back(to_long(*arg->multiplier));
				com.weight_conditions.push_back(combin::combinations<iterator>::weight_equals);
				}
			++arg;
			}
		}
	
	return true;
	}

algorithm::result_t permute::apply(iterator& it)
	{
	sibling_iterator obj=tr.begin(it);
	while(obj!=tr.end(it)) {
		com.original.push_back((iterator)(obj));
		++obj;
		}

	exptree rep;
	rep.set_head(str_node("\\comma"));
	unsigned int max_range_len=0;
	if(scan_through_range) {
		txtout << "scan through range" << std::endl;
		assert(com.sublengths.size()==1);
		max_range_len=com.sublengths[0];
		com.sublengths[0]=1;
		}
	
	do {
		com.clear_results();
		com.permute();
		
		for(unsigned int i=0; i<com.size(); ++i) {
			iterator comit=rep.append_child(rep.begin(), str_node("\\comma"));
			for(unsigned int j=0; j<com[i].size(); ++j) {
				rep.append_child(comit, com[i][j]);
				}
			}
		if(scan_through_range)
			com.sublengths[0]+=1;
		} while(scan_through_range && com.sublengths[0]<max_range_len);
	
	if(rep.number_of_children(rep.begin())==0) {
		rep.begin()->name=name_set.insert("1").first;
		zero(rep.begin()->multiplier);
		}
	it=tr.replace(it, rep.begin());
	
	expression_modified=true;

	return l_applied;
	}
