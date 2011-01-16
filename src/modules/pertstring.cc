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

#include "pertstring.hh"
#include "algebra.hh"
#include "combinatorics.hh"
#include <sstream>

aticksen::aticksen(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void aticksen::description() const
	{
	txtout << "Rewrite a fermionic correlator using Atick/Sen." << std::endl
		 << "Applies to all selected products." << std::endl;
	}

bool aticksen::can_apply(iterator it)
	{
	if(*it->name!="\\corr") return false;
	return true;
	}

algorithm::result_t aticksen::apply(iterator& mit)
	{
	assert(*mit->name=="\\corr");
	// replace all real fermions with complex ones
	exptree::sibling_iterator it=tr.begin(mit);
	while(it!=tr.end(mit)) {
		if(*(*it).name=="Psi" || *(*it).name=="Psibar") {
			exptree rep;
			exptree::iterator sit,mit2;
			sit=rep.insert(rep.begin(), str_node("\\sum", str_node::b_round));
			mit2=rep.append_child(sit,str_node("psi", str_node::b_round));
			rep.append_child(mit2,(*tr.begin(it)));
			mit2=rep.append_child(sit,str_node("psibar", str_node::b_round));
			if(*(*it).name=="Psibar")
				flip_sign(mit2->multiplier);
			rep.append_child(mit2,(*tr.begin(it)));
			it=tr.replace(it, sit);
			}
		++it;
		}
   // distribute the sums
	distribute dis(tr, tr.end());
	mit->name=name_set.insert("\\prod").first;
	iterator sumit=mit;
	dis.apply(sumit);
	// move to canonical order
	// FIXME: This should be done using a generic routine, but for that we need
	// to have object properties. So we do it internally for now.
	debugout << "aticksen: canonically ordering" << std::endl;
	canonical_order(sumit);
	if(*sumit->name!="\\sum") { // distribute on a single factor does not produce a sum
		iterator tmp=sumit; // thetas invalidates sumit iterator.
		debugout << "aticksen: converting single term to thetas" << std::endl;
		thetas(tr, sumit);
		sumit=tmp;
		}
	else {
		it=tr.begin(sumit);
		debugout << "aticksen: converting to thetas" << std::endl;
		while(it!=tr.end(sumit)) {
			if(*it->name=="\\prod") {
				sibling_iterator tmpit=it;
				++tmpit;
				thetas(tr, it);
				it=tmpit;
				}
			else ++it;
			}
		// Even though we started with a sum, the result may have one or even zero
		// terms; repair the sum node. FIXME: does not handle '0' correctly.
		// Better is to have a generic routine somewhere else which fixes the
		// sum nodes: if they have zero childs, remove; if they have one, flatten.
		if(tr.number_of_children(sumit)==1) {
			sumit = tr.flatten(sumit);
			sumit = tr.erase(sumit);
			}
		}
//	std::cerr << "done." << std::endl;
	expression_modified=true;
	mit=sumit;
	return l_applied;
	}

bool aticksen::fermion_order::operator()(const str_node& one, const str_node& two) const
	{
	if(one.name==two.name) return false;
	if(*one.name=="Splus")                       return true;
	if(*one.name=="Sminus" && *two.name!="Splus") return true;
	if(*one.name=="psibar" && *two.name=="psi")   return true;
	return false;
	}

void aticksen::canonical_order(exptree::iterator sumit)
	{
	sibling_iterator it=tr.begin(sumit);
	while(it!=tr.end(sumit)) {
		assert(*it->name!="\\sum");
		if(*it->name=="\\prod") {
			exptree oldsubtree;
			tr.subtree(oldsubtree, tr.begin(it), tr.end(it));
			fermion_order ord;
			tr.sort(tr.begin(it), tr.end(it), ord);
			multiply((*it).multiplier, combin::ordersign(sibling_iterator(oldsubtree.begin()),
															sibling_iterator(oldsubtree.end()),
															tr.begin(it), tr.end(it)));
			}
		++it;
		}
	}

void aticksen::append_single_theta(iterator it, nset_t::iterator one, nset_t::iterator two, 
											  multiplier_t invpow)
	{
	iterator th, sb;
	th=tr.append_child(it, str_node("th1"));
	sb=tr.append_child(th, str_node("\\sum", str_node::b_round));
	tr.append_child(sb, str_node(*one));
	flip_sign(tr.append_child(sb, str_node(*two))->multiplier);
	if(invpow!=1)
		multiply(tr.append_child(th, str_node("1", str_node::b_round, str_node::p_super))->multiplier, 1/invpow);
	}

void aticksen::thetas(exptree& tr, exptree::iterator it)
	{
	assert(*it->name=="\\prod");

	std::vector<nset_t::iterator> splus, sminus, psibar, psi;

	sibling_iterator sit=tr.begin(it);
	while(sit!=tr.end(it)) {
		if(*sit->name=="Splus")       splus.push_back(tr.begin(sit)->name);
		else if(*sit->name=="Sminus") sminus.push_back(tr.begin(sit)->name);
		else if(*sit->name=="psibar") psibar.push_back(tr.begin(sit)->name);
		else if(*sit->name=="psi")    psi.push_back(tr.begin(sit)->name);
		++sit;
		}

	tr.erase_children(it);
	if( ((splus.size()-sminus.size())+2*(psi.size()-psibar.size()))!=0 ) {
		// FIXME: should this be done by inserting a zero and letting algorithm
		// take care of the pushup?
		tr.erase(it);
		return;
		}

	exptree reptree;
	iterator top=reptree.set_head(str_node("\\prod"));

	for(unsigned int i=0; i<splus.size(); ++i)
		for(unsigned int j=0; j<i; ++j) 
			append_single_theta(top, splus[i], splus[j], 4);

	for(unsigned int i=0; i<sminus.size(); ++i)
		for(unsigned int j=0; j<i; ++j)
			append_single_theta(top, sminus[i], sminus[j], 4);

	for(unsigned int i=0; i<psibar.size(); ++i)
		for(unsigned int j=0; j<i; ++j)
			append_single_theta(top, psibar[i], psibar[j], 1);

	for(unsigned int i=0; i<psi.size(); ++i)
		for(unsigned int j=0; j<i; ++j)
			append_single_theta(top, psi[i], psi[j], 1);

	for(unsigned int i=0; i<sminus.size(); ++i) 
		for(unsigned int j=0; j<splus.size(); ++j) 
			append_single_theta(top, sminus[i], splus[j], -4);

	for(unsigned int i=0; i<psi.size(); ++i) 
		for(unsigned int j=0; j<psibar.size(); ++j) 
			append_single_theta(top, psi[i], psibar[j], -1);

	for(unsigned int i=0; i<psi.size(); ++i) 
		for(unsigned int j=0; j<splus.size(); ++j) 
			append_single_theta(top, psi[i], splus[j], 2);

	for(unsigned int i=0; i<psibar.size(); ++i) 
		for(unsigned int j=0; j<sminus.size(); ++j) 
			append_single_theta(top, psibar[i], sminus[j], 2);

	for(unsigned int i=0; i<psibar.size(); ++i) 
		for(unsigned int j=0; j<splus.size(); ++j) 
			append_single_theta(top, psibar[i], splus[j], -2);

	for(unsigned int i=0; i<psi.size(); ++i) 
		for(unsigned int j=0; j<sminus.size(); ++j) 
			append_single_theta(top, psi[i], sminus[j], -2);


	iterator th, sb;
	th=tr.append_child(top, str_node("thnu"));
	sb=tr.append_child(th, str_node("\\sum", str_node::b_round));

	if(splus.size()>0 || sminus.size()>0) {
		for(unsigned int i=0; i<splus.size(); ++i)
			multiply(tr.append_child(sb, str_node(*splus[i]))->multiplier, 1/(multiplier_t(2)));
		for(unsigned int i=0; i<sminus.size(); ++i)
			multiply(tr.append_child(sb, str_node(*sminus[i]))->multiplier, -1/(multiplier_t(2)));
		}
	for(unsigned int i=0; i<psi.size(); ++i)
		tr.append_child(sb, str_node(*psi[i]));
	for(unsigned int i=0; i<psibar.size(); ++i)
		flip_sign(tr.append_child(sb, str_node(*psibar[i]))->multiplier);

	exptree::iterator rit=reptree.begin();
	assert(*rit->name=="\\prod");
	rit->multiplier=it->multiplier;
	it=tr.replace(it,rit);
//	pa.tree.print_recursive_treeform(std::cout, pa.tree.begin());
//	std::cout << std::endl;
	}


riemannid::riemannid(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void riemannid::description() const
	{
	txtout << "Performs the Riemann identity on the four thnu factors in a selected"
		 << "product." << std::endl;
	// FIXME: have to require a sum, and also precisely four (not just first four)
	// but that is all for later once more other routines are in place.
	}

bool riemannid::can_apply(iterator it)
	{
	if(*it->name!="\\prod") return false;
	sibling_iterator facit=tr.begin(it);
	int total=0;
	while(facit!=tr.end(it)) {
		if(*facit->name=="thnu") {
			if((++total)==4)
				return true;
			}
		++facit;
		}
	return false;
	}

algorithm::result_t riemannid::apply(iterator& mit)
	{
	// Collect the arguments of the four thnu factors (remove the thnu
	// nodes at the same time).
	exptree::sibling_iterator facit=tr.begin(mit);
	std::vector<exptree> args;
	while(facit!=tr.end(mit)) {
		if(*facit->name=="thnu") {
			args.push_back(exptree(tr.begin(facit)));
			facit=tr.erase(facit);
			if(args.size()==4)
				break;
			}
		else ++facit;
		}
	if(args.size()<4) {
		txtout << "only " << args.size() << " tnnu factors in the product, need at least 4." << std::endl;
//		cancel_modification();
//		mit->fl.mark=0;
		mit=tr.end();
		return l_error;
		}
//	  for(unsigned int i=0; i<args.size(); ++i) {
//		  args[i].print_recursive_treeform(std::cerr, args[i].begin()) << std::endl;
//		  }

	// Construct the product of th1 nodes. (still need to divide args by two
	// and multiply total by 2).

	sumflatten sf(tr, tr.end());

	{ exptree fac(str_node("th1"));
	exptree::iterator sum=fac.append_child(fac.begin(), str_node("\\sum", str_node::b_round));
	multiply(fac.append_child(sum, args[0].begin())->multiplier, multiplier_t("1/2"));
	multiply(fac.append_child(sum, args[1].begin())->multiplier, multiplier_t("1/2"));
	multiply(fac.append_child(sum, args[2].begin())->multiplier, multiplier_t("1/2"));
	multiply(fac.append_child(sum, args[3].begin())->multiplier, multiplier_t("1/2"));
	sf.apply(sum);
	tr.append_child(iterator(mit), fac.begin()); }

	// Shorter would be something like
	//    sum << args[0] << args[1] << args[2] ,
	// i.e. append children to a node by streaming into an iterator.

	{ exptree fac(str_node("th1"));
	exptree::iterator sum=fac.append_child(fac.begin(), str_node("\\sum", str_node::b_round));
	multiply(fac.append_child(sum, args[0].begin())->multiplier, multiplier_t("1/2"));
	multiply(fac.append_child(sum, args[1].begin())->multiplier, multiplier_t("1/2"));
	multiply(fac.append_child(sum, args[2].begin())->multiplier, multiplier_t("-1/2"));
	multiply(fac.append_child(sum, args[3].begin())->multiplier, multiplier_t("-1/2"));
	sf.apply(sum);
	tr.append_child(iterator(mit), fac.begin()); }

	{ exptree fac(str_node("th1"));
	exptree::iterator sum=fac.append_child(fac.begin(), str_node("\\sum", str_node::b_round));
	multiply(fac.append_child(sum, args[0].begin())->multiplier, multiplier_t("1/2"));
	multiply(fac.append_child(sum, args[1].begin())->multiplier, multiplier_t("-1/2"));
	multiply(fac.append_child(sum, args[2].begin())->multiplier, multiplier_t("-1/2"));
	multiply(fac.append_child(sum, args[3].begin())->multiplier, multiplier_t("1/2"));
	sf.apply(sum);
	tr.append_child(iterator(mit), fac.begin()); }

	{ exptree fac(str_node("th1"));
	exptree::iterator sum=fac.append_child(fac.begin(), str_node("\\sum", str_node::b_round));
	multiply(fac.append_child(sum, args[0].begin())->multiplier, multiplier_t("1/2"));
	multiply(fac.append_child(sum, args[1].begin())->multiplier, multiplier_t("-1/2"));
	multiply(fac.append_child(sum, args[2].begin())->multiplier, multiplier_t("1/2"));
	multiply(fac.append_child(sum, args[3].begin())->multiplier, multiplier_t("-1/2"));
	sf.apply(sum);
	tr.append_child(iterator(mit), fac.begin()); }

//	tr.erase(tr.expression_head(mit));
//	mit->fl.mark=0;
	expression_modified=true;
	return l_applied;
	}
