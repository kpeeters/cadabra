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

#include "numerical.hh"
#include "manipulator.hh"
#include "algebra.hh"

using namespace numerical;

void numerical::register_properties()
	{
	properties::register_property(&create_property<Integer>);	
	properties::register_property(&create_property<NumericalFlat>);	
	}

std::string NumericalFlat::name() const
	{
	return "NumericalFlat";
	}

std::string Integer::name() const
	{
	return "Integer";
	}

bool Integer::parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t&)
	{
	if(tr.number_of_children(prop)==0)
		return true;

	exptree::iterator seq=tr.child(prop,0);
	if(tr.number_of_children(prop)>1 || *(seq->name)!="\\sequence") {
		txtout << name() << ": only one argument (range) accepted." << std::endl;
		return false;
		}
	
	if(tr.number_of_children(seq)!=2) {
		txtout << name() << ": sequence needs first and last element." << std::endl;
		return false;
		}

	from=exptree(tr.child(seq,0));
	to  =exptree(tr.child(seq,1));
//	tr.subtree(from, tr.child(seq,0), tr.child(seq,1));
//	tr.subtree(to,   tr.child(seq,1), tr.end(seq));
//	from=to_long(*(tr.child(seq,0)->multiplier));
//	to  =to_long(*(tr.child(seq,1)->multiplier));

	exptree::iterator sm=difference.set_head(str_node("\\sum"));
	difference.append_child(sm,to.begin())->fl.bracket=str_node::b_round;
	exptree::iterator term2=difference.append_child(sm,from.begin());
	flip_sign(term2->multiplier);
	term2->fl.bracket=str_node::b_round;
	difference.append_child(sm,str_node("1"))->fl.bracket=str_node::b_round;

	exptree::sibling_iterator sib=difference.begin(sm);
	while(sib!=difference.end(sm)) {
		if(*sib->name=="\\sum") {
			difference.flatten(sib);
			sib=difference.erase(sib);
			}
		else ++sib;
		}

	collect_terms ct(difference, difference.end());
	ct.apply(sm);

	return true;
	}

void Integer::display(std::ostream& str) const
	{
	str << "Integer";
	if(from.begin()!=from.end()) {
		str << "(" << *(from.begin()->multiplier) << ".." 
			 << *(to.begin()->multiplier) << ")";
		}
	}

numerical_flatten::numerical_flatten(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void numerical_flatten::description() const
	{
	}

bool numerical_flatten::can_apply(iterator it) 
	{
	const NumericalFlat *nf=properties::get<NumericalFlat>(it);
	if(nf) return true;
	return false;
	}

algorithm::result_t numerical_flatten::apply(iterator& it)
	{
	multiplier_t fac=*(it->multiplier);

	sibling_iterator sb=tr.begin(it);
	while(sb!=tr.end(it)) {
		 if(!sb->is_index()) {
			  fac*=*(sb->multiplier);
			  one(sb->multiplier);
			  }
		 ++sb;
		 }
	if(fac!=*(it->multiplier)) {
		 expression_modified=true;
		 it->multiplier=rat_set.insert(fac).first;
		 }
	return l_applied;
	}
