/* 

   $Id: select.cc,v 1.9 2006/06/05 21:54:01 peekas Exp $

	Cadabra: an extendable open-source symbolic tensor algebra system.
	Copyright (C) 2001-2006  Kasper Peeters <kasper.peeters@aei.mpg.de>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 2.
	 
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	
*/


#include "storage.hh"
#include "combinatorics.hh"
#include "select.hh"
#include <sstream>

/*
select::select(exptree& tr, iterator it) 
	: algorithm(tr, it)
	{
	}

void select::description() const
	{
	txtout << "@select( [equation] )( [term], [term], ... )" << std::endl
			 << "Selects the indicated terms of the indicated equation." << std::endl;
	}

bool select::can_apply(sibling_iterator, sibling_iterator)
	{
	return true;
	}

exptree::iterator select::apply(sibling_iterator it, sibling_iterator nd)
	{
	if(tr.number_of_children(it)<2) {
		txtout << "@select needs at least two arguments (the expression number and a list of nodes)." << std::endl;
		tr.erase_expression(cit);
		cit=tr.end();
		}
	else {
		exptree::sibling_iterator args=tr.begin(it);
		unsigned int eqno=atoi((*args->name).c_str());
		exptree::iterator theeq=tr.equation_by_number(eqno);
		if(theeq==tr.end()) {
			txtout << "equation number " << eqno << " does not exist." << std::endl;
			tr.erase_expression(cit);
			return tr.end();
			}
		else {
			exptree::sibling_iterator eit=tr.end(theeq);
			--eit;
			if(eit!=cit) {
				++args;
				while(args!=tr.end(it)) {
					exptree::iterator nid=tr.begin(eit);
					int here=1;
					int nodeno=atoi((*args->name).c_str());
					while(nid!=tr.end(eit)) {
						if(nodeno==0) {
							if(nid->name==args->name) {
								tr.select(nid, 1);
								here=-1;
								}
							}
						else if(here==nodeno) {
							tr.select(nid, 1); // FIXME: allow for mark number!=1.
							here=0;
							break;
							}
						if(here>=0)
							++here;
						++nid;
						}
					if(here>0) {
						if(nodeno==0)
							txtout << "no node \"" << *args->name << "\" ";
						else
							txtout << "no node " << atoi((*args->name).c_str());
						txtout << " in expression (" << eqno << ")." << std::endl;
						}
					++args;
					}	
				}	
			else eit=tr.end(); // FIXME: add error message
			tr.erase_expression(cit);
			cit=eit;
			}
		}
	}


unselect::unselect(exptree& tr, iterator it) 
	: algorithm(tr, it)
	{
	}

exptree::iterator unselect::apply(sibling_iterator st, sibling_iterator nd)
	{
	if(expressions.number_of_children(it)!=1) {
		txtout << "@unselect needs one argument (the expression number)." << std::endl;
		expressions.erase_expression(cit);
		cit=expressions.end();
		}
	else {
		exptree::iterator eit;
		exptree::sibling_iterator eqs=expressions.begin(it);
		unsigned int eqno=atoi((*eqs->name).c_str());
		eit=expressions.equation_by_number(eqno);
		if(eit!=expressions.end() && eit!=cit) {
			expressions.unselect(eit, true);
			expressions.erase_expression(cit);
			cit=eit;
			}
		}
	}
*/

pop::pop(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void pop::description() const
	{
	txtout << "Remove the last step of an expression history." << std::endl;
	}

bool pop::can_apply(iterator st)
	{
	iterator top=tr.named_parent(st, "\\history");
	sibling_iterator sit=tr.begin(top);
	unsigned int num=0;
	while(sit!=tr.end(top)) {
		if(*sit->name=="\\expression")
			++num;
		++sit;
		}
	if(num<=2) // 1 plus the copy made by algorithm.cc
		return false;
	return true;
	}

algorithm::result_t pop::apply(iterator& st)
	{
	iterator top=tr.named_parent(st, "\\history");
	iterator era=tr.active_expression(top);
	tr.erase(era);
	era =tr.active_expression(top);
	tr.erase(era);
	expression_modified=true;

	st=tr.active_expression(top);
	return l_applied;
	}

amnesia::amnesia(exptree&tr, iterator it)
	: algorithm(tr, it)
	{
	}

void amnesia::description() const
	{
	txtout << "Forget all steps leading to the present expression." << std::endl;
	}

bool amnesia::can_apply(iterator)
	{
	return true;
	}

algorithm::result_t amnesia::apply(iterator& it)
	{
	it=tr.keep_only_last(it);
	expression_modified=true;
	return l_applied;
	}
