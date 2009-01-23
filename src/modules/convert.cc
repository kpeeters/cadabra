/* 

   $Id: convert.cc,v 1.16 2007/12/07 16:50:56 peekas Exp $

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

#include "convert.hh"
#include "display.hh"
#include "parser.hh"
#include <modglue/process.hh>
#include <sstream>

frommath::frommath(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void frommath::description() const
	{
	txtout << "Convert a Mathematica expression in InputForm." << std::endl;
	}

bool frommath::can_apply(sibling_iterator, sibling_iterator)
	{
	// FIXME: crude check: if there are still comma separated lists, there
	// is work to be done.
	return true;
	}

algorithm::result_t frommath::apply(sibling_iterator& st, sibling_iterator& nd)
	{
	// hack: insert something at beginning and end, to be removed later.
	st=tr.insert(st, str_node("\\dummy"));
	nd=tr.insert(nd, str_node("\\dummy"));
	iterator it=st;
	while(it!=nd) {
		if(*it->name=="Delta" || *it->name=="delta") {
			if(tr.number_of_children(it)!=1) {
				it.skip_children();
				++it;
				continue;
				}
			iterator clist=tr.begin(it);
			if(! (*clist->name=="\\comma" && tr.number_of_children(clist)==2 )) {
				it.skip_children();
				++it;
				continue;
				}
			exptree rep;
			iterator delta=rep.set_head(str_node("\\delta"));
			
			sibling_iterator lst1=tr.begin(clist);
			if(*lst1->name=="\\comma" || (*lst1->name).size()==0) {
				sibling_iterator lst2=lst1; ++lst2;
				sibling_iterator ind1=tr.begin(lst1);
				sibling_iterator ind2=tr.begin(lst2);
				while(ind1!=tr.end(lst1)) {
					std::ostringstream str1, str2;
					str1 << *ind1->name;
					if(tr.number_of_children(ind1)!=0) 
						str1 << *(tr.child(ind1,0)->multiplier);
					str2 << *ind2->name;
					if(tr.number_of_children(ind2)!=0) 
						str2 << *(tr.child(ind2,0)->multiplier);
					rep.append_child(delta, str_node(str1.str(), str_node::b_none, str_node::p_sub));
					rep.append_child(delta, str_node(str2.str(), str_node::b_none, str_node::p_sub));
					++ind1;
					++ind2;
					}
				}
			else {
				// FIXME: this index rewriting occurs in various places
				std::ostringstream str1;
				str1 << *lst1->name;
				if(tr.number_of_children(lst1)!=0) 
					str1 << *(tr.child(lst1,0)->multiplier);
				rep.append_child(delta, str_node(str1.str(), str_node::b_none, str_node::p_sub));
				++lst1;
				str1.str("");
				str1 << *lst1->name;
				if(tr.number_of_children(lst1)!=0) 
					str1 << *(tr.child(lst1,0)->multiplier);
				rep.append_child(delta, str_node(str1.str(), str_node::b_none, str_node::p_sub));
				}

			debugout << "frommath: replacing Delta object" << std::endl;
			rep.begin()->fl.bracket=it->fl.bracket;
			it=tr.replace(it,rep.begin());
			debugout << "frommath: replace done" << std::endl;
			it.skip_children();
			expression_modified=true;
			}
		else if(*it->name=="GammaProd") {
			exptree  rep;
			iterator rephead=rep.set_head(str_node("\\prod"));

			int numch=0;
			sibling_iterator prodch=tr.begin(it);
			if(*prodch->name=="\\comma") {
				sibling_iterator searchcomma=tr.begin(prodch);
				while(searchcomma!=tr.end(prodch)) {
					if(*searchcomma->name=="\\comma") {
						tr.flatten(prodch);
						tr.erase(prodch);
						prodch=tr.begin(it);
						break;
						}
					++searchcomma;
					}
				}
			while(prodch!=tr.end(it)) {
				++numch;
				iterator gam=tr.append_child(rephead, str_node("\\Gamma"));
				if(*prodch->name=="\\comma") tr.reparent(gam, tr.begin(prodch), tr.end(prodch));
				else                         tr.append_child(gam, *prodch);
				sibling_iterator ind=tr.begin(gam);
				while(ind!=tr.end(gam)) {
					ind->fl.parent_rel=str_node::p_sub;
					++ind;
					}
				++prodch;
				}
			if(numch>1)
				it=tr.replace(it, rep.begin());
			else
				it=tr.replace(it, rep.begin(rep.begin()));
			expression_modified=true;
			it.skip_children();
			}
		else if(*it->name=="Weyl") {
			if(tr.number_of_children(it)==1) {
				iterator clist=tr.begin(it);
				tr.flatten(clist);
				clist=tr.erase(clist);
				if(tr.number_of_children(it)==2) {
					clist=tr.begin(it);
					iterator nxt=clist; nxt.skip_children(); ++nxt;
					tr.flatten(clist);
					tr.flatten(nxt);
					tr.erase(clist);
					tr.erase(nxt);
					clist=tr.begin(it);
					while(clist!=tr.end(it)) {
						clist->fl.parent_rel=str_node::p_sub;
						++clist;
						}
					it->name=name_set.insert("W").first;
					expression_modified=true;
					}
				}
			}
		else if(*it->name=="Tensor") {  // Tensor[F, {a1,a2,a3}] or Tensor[F, {a1}]
			if(tr.number_of_children(it)!=1 || 
				( tr.number_of_children(it)==1 && *(tr.begin(it)->name)!="\\comma")) {
				it.skip_children();
				++it;
				continue;
				}
			tr.flatten(tr.begin(it));
			tr.erase(tr.begin(it));    // Tensor[F][{a1,a2,a3}] or Tensor[F]{a1}
			iterator clist=tr.begin(it); ++clist;
			assert(clist!=tr.end(it));
			assert(*clist->name=="\\comma" || (*clist->name).size()==0);

			sibling_iterator ind=tr.begin(clist);
			while(ind!=tr.end(clist)) {
				ind->fl.parent_rel=str_node::p_sub;
				++ind;
				}
			tr.reparent(tr.begin(it), tr.begin(clist), tr.end(clist)); 

//			else { // zero or one indices
//				assert((*clist->name).size()==0); 
//				clist->fl.parent_rel=str_node::p_sub;
//				tr.append_child(tr.begin(it), *clist);
//				}
			tr.erase(clist);  // Tensor[F{a1 a2 a3}]   or Tensor[F{a1}]
			tr.begin(it)->fl.bracket=it->fl.bracket;
			tr.flatten(it);   // Tensor F{a1 a2 a3}  or Tensor F{a1}
			it=tr.erase(it);
			it.skip_children();
			expression_modified=true;
			}
		++it;
		}
	st=tr.erase(st);
	nd=tr.erase(nd);

//	tr.print_recursive_treeform(debugout, st) << std::endl;

	return l_applied;
	}


frommaple::frommaple(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void frommaple::description() const
	{
	txtout << "Convert a Maplee expression in prettyprint=0 form." << std::endl;
	}

bool frommaple::can_apply(iterator)
	{
	// FIXME: crude check: if there are still comma separated lists, there
	// is work to be done.
	return true;
	}

// R[d1,d2,d3,d4]*R[-d1,d5,-d3,d6]*R[-d2,d7,-d4,d8]*R[-d5,-d8,-d6,-d7];

algorithm::result_t frommaple::apply(iterator& st)
	{
	// FIXME: log the entering of this routine, it's truly weird...
	iterator it=st;
	iterator end=it;
	end.skip_children();
	++end;
	while(it!=end) {
		if(*it->name=="R" && tr.number_of_children(it)==1 &&
			*tr.begin(it)->name=="\\comma" && tr.number_of_children(tr.begin(it))==4) {
			tr.flatten(tr.begin(it));
			tr.erase(tr.begin(it));
			sibling_iterator args=tr.begin(it);
			while(args!=tr.end(it)) {
				args->fl.parent_rel=str_node::p_sub;
				args->fl.bracket=str_node::b_none;
				one(args->multiplier);
				expression_modified=true;
				++args;
				}
			it.skip_children();
			++it;
			}
		else ++it;
		}
	return l_applied;
	}



run::run(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void run::description() const
	{
	txtout << "Run an external program on an expression." << std::endl;
	}

bool run::can_apply(iterator it)
	{
	if(number_of_args()<1) {
		txtout << "run needs at least one arguments: a program name" << std::endl;
		return false;
		}
	return true;
	}

algorithm::result_t run::apply(iterator& it)
	{
	sibling_iterator progit=args_begin();
	std::string progname=*progit->name;

	bool domaple=false;
	if(number_of_args()==2) {
		sibling_iterator nxt=progit;
		++nxt;
		if(*nxt->name=="maple")
			domaple=true;
		}
	if(progit->is_quoted_string())
		progname=progname.substr(1,progname.size()-2);

	return apply(it, progname, domaple);
	}

algorithm::result_t run::apply(iterator& it, std::string program_name, bool mapleout)
	{
	std::ostringstream argstr;
	exptree_output eo(tr, argstr);
	if(mapleout)
		eo.output_format=exptree_output::out_maple;
	
	if(*(it->name)=="\\tie") {
		sibling_iterator si=tr.begin(it);
		while(si!=tr.end(it)) {
			if(si->is_quoted_string())
			   argstr << (*si->name).substr(1,(*si->name).size()-2);
			else {
				if(*si->name=="endl") argstr << std::endl;
				else                  eo.print_infix(si);
				}
			++si;
			}
		}
	else {
		if(it->is_quoted_string())
			argstr << (*it->name).substr(1,(*it->name).size()-2);
		else {
			if(*it->name=="endl") argstr << std::endl;
			else                  eo.print_infix(it);
			}
		}

	modglue::child_process theproc(program_name);
	theproc << argstr.str() + ";";
	std::string result;
	theproc.call("",result);
	std::string::size_type pos=result.find(";",0);
//	txtout << "output: |" << result << "|" << std::endl; 
	if(pos==std::string::npos) {
		txtout << "program " << program_name << " did not produce semicolon deliminated output."
				 << std::endl;
		return l_error;
		}
	result.erase(pos);
	while((pos=result.find("\n",0))!=std::string::npos) {
		result.erase(pos,1);
		}
//	txtout << "parsing |" << result << "|" << std::endl;

	// parse the output
	std::stringstream str(result);
	parser pa(true);
	try {
		str >> pa;
		}
	catch(std::exception& ex) {
		txtout << ex.what() << std::endl;
		return l_error;
		}
	it=tr.replace(it,pa.tree.begin().begin());
	cleanup_expression(tr,it);
	expression_modified=true;
	
	return l_applied;
	}

// @run[3c+d]{"./testfeed"};
// sin(x+@run[c-3]{"./testfeed"});

/*

R_{a b c d}::WeylTensor.
one: R_{d1 d2 d3 d4} * R_{d1 d2 d5 d6} * R_{d3 d7 d5 d8} * R_{d4 d8 d6 d7};
@einsteinify(%);
@from_maple[@run[@(one)]{"./testfeed"}{maple}];

R_{a b c d}::WeylTensor.
@dummies{vector{d1,d2,d3,d4,d5,d6,d7,d8}}.
16/315 * R_{d1 d2 d3 d4} * R_{d1 d2 d3 d4} * R_{d5 d6 d7 d8} * R_{d5 d6 d7 d8} + 32/2835 * R_{d1 d2 d3 d4} * R_{d1 d2 d5 d6} * R_{d3 d4 d7 d8} * R_{d5 d6 d7 d8} + 128/2835 * R_{d1 d2 d3 d4} * R_{d1 d2 d3 d5} * R_{d4 d6 d7 d8} * R_{d5 d6 d7 d8} + 512/2835 * R_{d1 d2 d3 d4} * R_{d1 d2 d5 d6} * R_{d3 d7 d5 d8} * R_{d4 d8 d6 d7} + 256/2835 * R_{d1 d2 d3 d4} * R_{d1 d5 d3 d6} * R_{d2 d7 d5 d8} * R_{d4 d7 d6 d8};
@einsteinify!(%);
@run[@(%)]{"./testfeed"}{maple};
@from_maple!(%);
@expand_power!(%);
@prodflatten!(%);

R_{a b c d}::WeylTensor.
@dummies{vector{d1,d2,d3,d4,d5,d6,d7,d8}}.
16/315 * R_{d1 d2 d3 d4} * R_{d1 d2 d3 d4} * R_{d5 d6 d7 d8} * R_{d5 d6 d7 d8} + 32/2835 * R_{d1 d2 d3 d4} * R_{d1 d2 d5 d6} * R_{d3 d4 d7 d8} * R_{d5 d6 d7 d8} + 128/2835 * R_{d1 d2 d3 d4} * R_{d1 d2 d3 d5} * R_{d4 d6 d7 d8} * R_{d5 d6 d7 d8} + 512/2835 * R_{d1 d2 d3 d4} * R_{d1 d2 d5 d6} * R_{d3 d7 d5 d8} * R_{d4 d8 d6 d7} + 256/2835 * R_{d1 d2 d3 d4} * R_{d1 d5 d3 d6} * R_{d2 d7 d5 d8} * R_{d4 d7 d6 d8};
@canonicalise!(%){maple};


*/
