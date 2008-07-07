/* 

   $Id: field_theory.cc,v 1.182 2008/06/28 09:44:33 peekas Exp $

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
#include "algebra.hh"
#include "exchange.hh"
#include "field_theory.hh"
#include "combinatorics.hh"
#include "dummies.hh"
#include "props.hh"
#include "numerical.hh"
#include "linear.hh"
#include "gamma.hh"
#include "relativity.hh"
#include <stdint.h>


void field_theory::register_properties()
	{
	properties::register_property(&create_property<Depends>);
	properties::register_property(&create_property<DependsInherit>);
	properties::register_property(&create_property<Accent>);
//	properties::register_property(&create_property<GrassmannNumber>);
//	properties::register_property(&create_property<GrassmannNumberInherit>);
	properties::register_property(&create_property<Weight>);
	properties::register_property(&create_property<WeightInherit>);
	}

std::string Accent::name() const
	{
	return "Accent";
	}

std::string DependsInherit::name() const
	{
	return "DependsInherit";
	}

std::string Depends::name() const
	{
	return "Depends";
	}

bool Depends::parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t& kv)
	{
	exptree::sibling_iterator frstarg=tr.begin(prop);
	if(*frstarg->name=="\\comma") {
		exptree::sibling_iterator sib=tr.begin(frstarg);
		int num=1;
		while(sib!=tr.end(frstarg)) {
			const Indices    *dum=properties::get<Indices>(sib);
			const Coordinate *crd=properties::get<Coordinate>(sib);
			const Derivative *der=properties::get<Derivative>(sib);
			const Accent     *acc=properties::get<Accent>(sib);
			if(dum==0 && crd==0 && der==0 && acc==0) {
				txtout << "Argument " << *sib->name
				  << " (" << num << ") lacks property Coordinate, Derivative, Accent or Indices." << std::endl;
				return false;
				}
			++num;
			++sib;
			}
		dependencies_=exptree(frstarg);
		}
	else {
		const Indices    *dum=properties::get<Indices>(frstarg);
		const Coordinate *crd=properties::get<Coordinate>(frstarg);
		const Derivative *der=properties::get<Derivative>(frstarg);
		const Accent     *acc=properties::get<Accent>(frstarg);
		if(dum==0 && crd==0 && der==0 && acc==0) {
			txtout << "Argument " << *frstarg->name 
					 << " lacks property Coordinate, Derivative, Accent or Indices." << std::endl;
			return false;
			}
		dependencies_=exptree(frstarg);
		dependencies_.wrap(dependencies_.begin(), str_node("\\comma"));
		}
	return true;
	}

exptree Depends::dependencies(exptree::iterator) const
	{
	return dependencies_;
	}

exptree DependsInherit::dependencies(exptree::iterator it) const
	{
	exptree ret;
	ret.set_head(str_node("\\comma"));
	exptree::sibling_iterator sib=it.begin();
	while(sib!=it.end()) {
		const DependsBase *dep=properties::get_composite<DependsBase>(sib);
		if(dep) {
			exptree::iterator cn=ret.append_child(ret.begin(), dep->dependencies(sib).begin());
			ret.flatten(cn);
			ret.erase(cn);
			}
		++sib;
		}
	return ret;
	}

std::string Weight::name() const 
	{
	return "Weight";
	}

bool Weight::parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t& kv)
	{
	keyval_t::const_iterator kvit=kv.find("value");
	if(kvit!=kv.end()) value_=to_long(*kvit->second->multiplier);
	else               value_=1;

	return true;
	}

int Weight::value(exptree::iterator, const std::string& forcedlabel) const
	{
	if(forcedlabel!=label) return -1;
	return value_;
	}

std::string WeightInherit::name() const 
	{
	return "WeightInherit";
	}

bool WeightInherit::parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t& kv)
	{
	keyval_t::const_iterator tpit=kv.find("type");
	if(tpit!=kv.end()) {
		if(*tpit->second->name=="Multiplicative") combination_type=multiplicative;
		else                                      combination_type=additive;
		}
	else combination_type=multiplicative;
	return true;
	}

int WeightInherit::value(exptree::iterator it, const std::string& forcedlabel) const
	{
	int ret=0;
	if(combination_type==additive) 
		ret=-2;

//	txtout << "calling inherit on " << *it->name << " " << &(*it) << " " << forcedlabel << std::endl;
	exptree::sibling_iterator sib=it.begin();
	while(sib!=it.end()) {
		 if(!sib->is_index()) {
			  if(combination_type==multiplicative) {
					const WeightBase *gnb=properties::get_composite<WeightBase>(sib, forcedlabel);
					if(gnb) {
						 int tmp=gnb->value(sib, forcedlabel);
						 if(tmp<0) {
							  ret=-1;
							  break; // problems
							  }
						 else { 
							  ret+=tmp;
							  }
						 }
					}
			  else {
					int thisone=0;
					const WeightBase *gnb=properties::get_composite<WeightBase>(sib, forcedlabel);
					if(gnb) thisone=gnb->value(sib, forcedlabel);
					else    thisone=0;
					if(ret==-2) {
						 ret=thisone;
						 if(ret<0) break;  // problems
						 }
					else if(ret!=thisone) { // not uniform
						 ret=-1;
						 break;
						 }
					}
			  }
		 ++sib;
		 }
	return ret;
	}

generate_indexbracket::generate_indexbracket(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void generate_indexbracket::description() const
	{
	txtout << "Wrap indices on brackets in indexbracket nodes" << std::endl;
	}

// The things to be wrapped:
//
//   - sub/superscript nodes with a parent with an empty name, e.g. (b)_\mu.
//   - sub/superscript nodes inside a sum or product.

bool generate_indexbracket::can_apply(iterator it)
	{
	if((*it->name).size()==0) return true;
	if((*it->name)=="\\prod" || (*it->name)=="\\sum") {
		sibling_iterator sib=tr.begin(it);
		while(sib!=tr.end(it)) {
			if(sib->fl.parent_rel==str_node::p_super || sib->fl.parent_rel==str_node::p_sub)
				return true;
			++sib;
			}
		}
	return false;
	}

algorithm::result_t generate_indexbracket::apply(iterator& it)
	{
	// (b)_\mu -> \indexbracket(b)_\mu (but leave _{a1} alone)
	if((*it->name).size()==0) {
		sibling_iterator sib=tr.begin(it);
		if(sib->fl.parent_rel!=str_node::p_super && sib->fl.parent_rel!=str_node::p_sub) {
			++sib;
			while(sib!=tr.end(it)) {
				if(sib->fl.parent_rel==str_node::p_super || sib->fl.parent_rel==str_node::p_sub) {
					it->name=name_set.insert("\\indexbracket").first;
					expression_modified=true;
					return l_applied;
					}
				++sib;
				}
			}
		}
	else if(*(it->name)=="\\prod" || *(it->name)=="\\sum") {
		iterator ibrack=tr.insert(it,str_node("\\indexbracket"));
		sibling_iterator nxt=it;
		++nxt;
		tr.reparent(ibrack,sibling_iterator(it),nxt);
		it=tr.begin(ibrack);
		sibling_iterator sib=tr.begin(it);
		while(sib!=tr.end(it)) {
			if(sib->fl.parent_rel==str_node::p_super || sib->fl.parent_rel==str_node::p_sub) {
				tr.append_child(ibrack,*sib);
				sib=tr.erase(sib);
				}
			else ++sib;
			}
		it=ibrack;
		}
	return l_no_action;
	}

unique_indices::unique_indices(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void unique_indices::description() const
	{
	txtout << "Make all open indices of tensors in a list unique." << std::endl;
	}

bool unique_indices::can_apply(iterator it)
	{
	if(*it->name=="\\comma") return true;
	return false;
	}

algorithm::result_t unique_indices::apply(iterator& it)
	{
	index_map_t ind_free, ind_dummy;
	
	sibling_iterator sib=tr.begin(it);
	while(sib!=tr.end(it)) {
		if(tr.number_of_children(sib)>0) {
			index_map_t tmp_ind_free, tmp_ind_dummy, rename_indices, generated_indices;
			classify_indices(sib, tmp_ind_free, tmp_ind_dummy);
			determine_intersection(tmp_ind_free, ind_free, rename_indices, true);
			index_map_t::iterator indit=rename_indices.begin();
			while(indit!=rename_indices.end()) {
				const Indices *dums=properties::get<Indices>(indit->second);
				assert(dums);
				exptree dum=get_dummy(dums, &ind_free, &ind_dummy, &tmp_ind_free, &tmp_ind_dummy,
														 &generated_indices);
				tr.replace_index(indit->second, dum.begin()); //->name=dum;
				generated_indices.insert(index_map_t::value_type(dum, indit->second));
				++indit;
				}
			index_map_t::iterator cpyit=tmp_ind_free.begin();
			while(cpyit!=tmp_ind_free.end()) {
				ind_free.insert(*cpyit);
				++cpyit;
				}
			cpyit=generated_indices.begin();
			while(cpyit!=generated_indices.end()) {
				ind_free.insert(*cpyit);
				++cpyit;
				}
			}
		++sib; 
		}
	return l_applied;
	}

einsteinify::einsteinify(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void einsteinify::description() const
	{
	txtout << "Einsteinify index pairs." << std::endl;
	}

bool einsteinify::can_apply(iterator it)
	{
	if(*it->name=="\\prod") return true;
	return false;
	}

algorithm::result_t einsteinify::apply(iterator& it)
	{
	bool insert_metric=false;
	if(number_of_args()>0)
		insert_metric=true;

	index_map_t ind_free, ind_dummy;
	classify_indices(it, ind_free, ind_dummy);
	index_map_t::iterator dit=ind_free.begin();
	index_map_t::iterator prev=ind_free.end();
	dit=ind_dummy.begin();
	prev=dit;
	++dit;
	while(dit!=ind_dummy.end()) {
		if(tree_exact_equal((*dit).first, (*prev).first)) {
			if(insert_metric) { // put indices down and insert an inverse metric
				(*dit).second->fl.parent_rel=str_node::p_sub;
				(*prev).second->fl.parent_rel=str_node::p_sub;
				iterator invmet=tr.append_child(it,str_node(args_begin()->name));

				// get a new dummy index
				const Indices *dums=properties::get<Indices>(dit->second);
				assert(dums);
				exptree dum=get_dummy(dums, it);

				// relink the indices
				iterator tmpit=tr.append_child(invmet, (*prev).second);
				tmpit->fl.bracket=str_node::b_none;
				tmpit->fl.parent_rel=str_node::p_super;
				tmpit=tr.append_child(invmet, dum.begin());
				tmpit->fl.bracket=str_node::b_none;
				tmpit->fl.parent_rel=str_node::p_super;
				tr.replace_index((*dit).second,dum.begin());
				}
			else { // raise one index
				if((*dit).second->fl.parent_rel==(*prev).second->fl.parent_rel) {
					if((*dit).second->fl.parent_rel==str_node::p_super) 
						(*prev).second->fl.parent_rel=str_node::p_sub;
					else
						(*prev).second->fl.parent_rel=str_node::p_super;
					expression_modified=true;
					}
				}
			}
		prev=dit;
		++dit;
		}
	return l_applied;
	}

combine::combine(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void combine::description() const
	{
	txtout << "Combine indexbrackets." << std::endl;
	}

bool combine::can_apply(iterator it)
	{
	if(*it->name=="\\prod") return true;
	return false;
	}

algorithm::result_t combine::apply(iterator& it)
	{
	sibling_iterator sib=tr.begin(it);
	index_map_t ind_free, ind_dummy;
	while(sib!=tr.end(it)) {
		sibling_iterator ch=tr.begin(sib);
		while(ch!=tr.end(sib)) {
			if(ch->fl.parent_rel==str_node::p_sub || ch->fl.parent_rel==str_node::p_super) {
				classify_add_index(ch, ind_free, ind_dummy);
				}
			++ch;
			}
		++sib;
		}
	if(ind_dummy.size()==0) return l_no_action;

	index_map_t::iterator dums1=ind_dummy.begin(), dums2;
	while(dums1!=ind_dummy.end()) {
//		txtout << "analysing " << std::endl;
//		txtout << *(dums1->second->name) << std::endl;
		dums2=dums1;
		++dums2;

		bool isbrack1=false, isbrack2=false;
		bool ismatorvec1=false, ismatorvec2=false;
		const Matrix *mat1=properties::get<Matrix>(tr.parent(dums1->second));
		if(mat1)
			ismatorvec1=true;
		else if(*(tr.parent(dums1->second)->name)=="\\indexbracket") {
			ismatorvec1=true; isbrack1=true;
			}
		else if(tr.number_of_children(tr.parent(dums1->second))==1) 
			ismatorvec1=true;
		const Matrix *mat2=properties::get<Matrix>(tr.parent(dums2->second));
		if(mat2)
			ismatorvec2=true;
		else if(*(tr.parent(dums2->second)->name)=="\\indexbracket") {
			ismatorvec2=true; isbrack2=true;
			}
		else if(tr.number_of_children(tr.parent(dums2->second))==1) 
			ismatorvec2=true;

		if(ismatorvec1 && ismatorvec2) {
//			txtout << "gluing " << *(dums2->second->name) << std::endl;
			// create new indexbracket with product node
			iterator outerbrack=tr.insert(tr.parent(dums1->second), str_node("\\indexbracket"));
			iterator brackprod=tr.append_child(outerbrack, str_node("\\prod"));
			iterator parn1=tr.parent(dums1->second);
			iterator parn2=tr.parent(dums2->second);
			// remove the dummy index from these two objects, and move
			// the non-dummy indices to the outer indexbracket.
			sibling_iterator ind1=tr.begin(tr.parent(dums1->second));
			sibling_iterator stop1=tr.end(tr.parent(dums1->second));
			if(isbrack1)
				++ind1;
			while(ind1!=stop1) {
				if(ind1!=dums1->second) {
					tr.append_child(outerbrack, iterator(ind1));
					}
				++ind1;
//				ind1=tr.erase(ind1);
				}
			tr.erase(dums1->second);
			sibling_iterator ind2=tr.begin(tr.parent(dums2->second));
			sibling_iterator stop2=tr.end(tr.parent(dums2->second));
			if(isbrack2)
				++ind2;
			while(ind2!=stop2) {
				if(ind2!=dums2->second) {
					tr.append_child(outerbrack, iterator(ind2));
					}
				++ind2;
//				ind2=tr.erase(ind2);
				}
			tr.erase(dums2->second);

			// put both objects inside the indexbracket.
			if(isbrack1) {
				sibling_iterator nxt=tr.begin(parn1);
				++nxt;
				tr.begin(parn1)->fl.bracket=str_node::b_round;
				tr.reparent(brackprod, tr.begin(parn1), nxt);
				multiply(brackprod->multiplier, *parn1->multiplier);
				tr.erase(parn1);
				}
			else {
				sibling_iterator nxt=parn1;
				++nxt;
				parn1->fl.bracket=str_node::b_round;
				tr.reparent(brackprod,parn1,nxt);
				}
			if(isbrack2) {
				sibling_iterator nxt=tr.begin(parn2);
				++nxt;
				tr.begin(parn2)->fl.bracket=str_node::b_round;
				tr.reparent(brackprod, tr.begin(parn2), nxt);
				multiply(brackprod->multiplier, *parn2->multiplier);
				tr.erase(parn2);
				}
			else {
				sibling_iterator nxt=parn2;
				++nxt;
				parn2->fl.bracket=str_node::b_round;
				tr.reparent(brackprod,parn2,nxt);
				}
			}
		++dums1;
		++dums1;
		}
	expression_modified=true;
	prodflatten pf(tr, tr.end());
	pf.apply_recursive(it, false);
	cleanup_expression(tr, it);
	return l_applied;
	}

expand::expand(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void expand::description() const
	{
	txtout << "Expand indexbrackets." << std::endl;
	}

bool expand::can_apply(iterator it)
	{
	if(*it->name=="\\indexbracket") 
		if(*tr.begin(it)->name=="\\prod") {
			// If we have only one external index, determine whether the first
			// or the last object should be the one with only one index. We
			// do this by checking for the 'Matrix' property on the first and
			// last element.
			one_index=(tr.number_of_children(it)==2);
			sibling_iterator prod=tr.begin(it);
			sibling_iterator sib=tr.begin(prod);
			mx_first=tr.end();
			mx_last=tr.end();
			ii_first=tr.end();
			ii_last=tr.end();

			unsigned int index_open=0;

			while(sib!=tr.end(prod)) {
				const ImplicitIndex *impi=properties::get_composite<ImplicitIndex>(sib);
				if(impi) {
					const Matrix *mat=properties::get_composite<Matrix>(sib);
					if(mat) { 
						if(index_open==0) {
							mx_first=sib;
							index_open=2;
							}
						mx_last=sib;
						}
					else {
						if(index_open==0) {
							ii_first=sib;
							mx_first=tr.end();
							index_open=1;
							}
						else {
							ii_last=sib;
							mx_last=tr.end();
							--index_open;
							}
						}
					}
				++sib;
				}
			
			if(index_open+1==tr.number_of_children(it)) return true;
			}
	return false;
	}

algorithm::result_t expand::apply(iterator& it)
	{
	sibling_iterator prod=tr.begin(it); // the first child of the indexbracket is the product

	// Figure out the type of the indices to be inserted.
	sibling_iterator origind=prod;
	++origind;
	const Indices *dums=properties::get<Indices>(origind);		
	if(!dums)
		throw consistency_error("No information about the index types known.");

	// Scan through the factors, adding indexbrackets around any
	// objects which already carry indices, and adding new
	// dummies when necessary.

	sibling_iterator sib=tr.begin(prod);
	exptree dum;
	while(sib!=tr.end(prod)) {
		const ImplicitIndex *impi=properties::get<ImplicitIndex>(sib);
		sib->fl.bracket=str_node::b_none;
		if(impi) {
			const Matrix *mat=properties::get_composite<Matrix>(sib);
			sibling_iterator origobj=sib;

			if(tr.number_of_children(sib)>0)
				sib=tr.wrap(sib, str_node("\\indexbracket"));
			if(dum.size()>0) {
				iterator tmpit=tr.append_child((iterator)(sib), dum.begin());
				tmpit->fl.bracket=str_node::b_none;
				tmpit->fl.parent_rel=str_node::p_sub;
				}

			if(mat) { // two-index object
				if(origobj==mx_first) { // put in an open index
					tr.append_child(sib, origind);
					origind=tr.erase(origind);
					}
				if(origobj==mx_last) {
					tr.append_child(sib, origind);
					origind=tr.erase(origind);
					}
				else {
					dum=get_dummy(dums, sib);
					iterator tmpit=tr.append_child((iterator)(sib), dum.begin());
					tmpit->fl.bracket=str_node::b_none;
					tmpit->fl.parent_rel=str_node::p_sub;
					}
				}
			else { // one-index object
				if(origobj==ii_first) {
					dum=get_dummy(dums, sib);
					iterator tmpit=tr.append_child((iterator)(sib), dum.begin());
					tmpit->fl.bracket=str_node::b_none;
					tmpit->fl.parent_rel=str_node::p_sub;
					}
				else dum.clear(); 
				}
			++sib;
			}
		else ++sib;
		}

	it->name=name_set.insert("\\prod").first;
	cleanup_sums_products(tr, it);

	expression_modified=true;
	return l_applied;
	}

debracket::debracket(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void debracket::description() const
	{
	txtout << "Remove indicated objects outside indexbracket." << std::endl;
	}

bool debracket::can_apply(iterator it)
	{
	if(args_begin()==args_end()) {
		txtout << "Need one argument." << std::endl;
		return false;
		}
	if(*it->name=="\\indexbracket") return true;
	return false;
	}

algorithm::result_t debracket::apply(iterator& it)
	{
	bool changed=false;
	iterator newprod=tr.insert(it, str_node("\\prod"));
	sibling_iterator nxt=it;
	++nxt;
	tr.reparent(newprod,it,nxt);

	iterator ib=tr.begin(newprod);

	sibling_iterator ibarg=tr.begin(ib);
	if(*ibarg->name=="\\prod") {
		sibling_iterator facs=tr.begin(ibarg);
		while(facs!=tr.end(ibarg)) {
			sibling_iterator ar=args_begin();
			sibling_iterator nxtfacs=facs;
			++nxtfacs;
			for(unsigned int i=0; i<tr.arg_size(ar); ++i) {
				if(tr.arg(ar, i)->name==facs->name) {
					facs->fl.bracket=ib->fl.bracket;
					tr.move_before(ib, iterator(facs));
					changed=true;
					break;
					}
				}
			facs=nxtfacs;
			}
		if(changed) {
			if(tr.number_of_children(ibarg)==0) {
				tr.erase(ibarg);
				ib->name=name_set.insert("\\delta").first; // FIXME: use properties
				}
			else if(tr.number_of_children(ibarg)==1) {
				if(ibarg->fl.bracket==str_node::b_none)
					tr.begin(ibarg)->fl.bracket=str_node::b_round;
				else
					tr.begin(ibarg)->fl.bracket=ibarg->fl.bracket;
				tr.flatten(ibarg);
				tr.erase(ibarg);
				}
			}
		}
	else {
		sibling_iterator ar=args_begin();
		for(unsigned int i=0; i<tr.arg_size(ar); ++i) {
			if(tr.arg(ar, i)->name==ibarg->name) {
				ibarg->fl.bracket=ib->fl.bracket;
				sibling_iterator fi=tr.begin(ib);
				sibling_iterator si=fi;
				++fi;
				++si; ++si; ++si;
				tr.reparent(ibarg,fi,si);
				tr.flatten(ib);
				tr.erase(ib);
				changed=true;
				break;
				}
			}
		if(changed) {
			tr.flatten(newprod);
			tr.erase(newprod);
			}
		}

	if(changed) {
		it=newprod;
		cleanup_expression(tr, it);
		cleanup_nests(tr, it);
		expression_modified=true;
		return l_applied;
		}
	else {
		tr.flatten(newprod);
		tr.erase(newprod);
		return l_no_action;
		}
	}

eliminate_kronecker::eliminate_kronecker(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void eliminate_kronecker::description() const
	{
	txtout << "Eliminate Kronecker deltas." << std::endl;
	}

bool eliminate_kronecker::can_apply(iterator st)
	{
	if(*st->name!="\\prod") 
		if(!is_single_term(st))
			return false;

	return true;
	}

algorithm::result_t eliminate_kronecker::apply(iterator& st)
	{
	prod_wrap_single_term(st);
	const nset_t::iterator onept=name_set.insert("1").first;

	int looping=0;

	sibling_iterator it=tr.begin(st);
	while(it!=tr.end(st)) { // Loop over all factors in a product, looking for Kroneckers
		bool replaced=false;
		const KroneckerDelta *kr=properties::get<KroneckerDelta>(it);
		if(kr && tr.number_of_children(it)==2) {
			sibling_iterator ii1=tr.begin(it);
			sibling_iterator ii2=ii1; ++ii2;
			if(subtree_compare(ii1, ii2, 1, false, true)==0) { // a self-contracted Kronecker delta
				 const Integer *itg1=properties::get<Integer>(ii1);
				 const Integer *itg2=properties::get<Integer>(ii2);
				 if(itg1 && itg2 && ii1->is_rational()==false && ii2->is_rational()==false) {
					  if(itg1->from.begin()!=itg1->from.end() && itg2->from.begin()!=itg2->from.end()) {
							if(itg1->difference.begin()->name==onept) {
								 multiply(st->multiplier, *itg1->difference.begin()->multiplier);
								 it=tr.erase(it);
								 }
							else {
								 it=tr.replace(it, itg1->difference.begin());
								 }
							expression_modified=true;
							}
					  else ++it;
					  }
				 else ++it;
				 }
			else {
				 sibling_iterator oi=tr.begin(st);
				 ++looping;
				 // iterate over all factors in the product
				 while(!replaced && oi!=tr.end(st)) {
					  if(oi!=it) { // this is not the delta node
							// compare delta indices with all indices of this object
							exptree::index_iterator ind=tr.begin_index(oi);
							while(ind!=tr.end_index(oi)) {
								 if(ii1->is_rational()==false && subtree_compare(ind, ii1, 1, false, true)==0) {
									  tr.replace_index(ind, ii2)->fl.parent_rel=ii2->fl.parent_rel; 
									  replaced=true;
									  break;
									  }
								 else if(ii2->is_rational()==false && subtree_compare(ind, ii2, 1, false, true)==0) {
									  tr.replace_index(ind, ii1)->fl.parent_rel=ii1->fl.parent_rel;
									  replaced=true;
									  break;
									  }
								 ++ind;
								 }
							}
					  if(!replaced) 
							++oi;
					  }
				 if(replaced) {
					  expression_modified=true;
					  it=tr.erase(it);
					  }
				 else ++it;
				 }
			 }
		else ++it;
		}

	// the product may have reduced to a single term or even just a constant
//	txtout << "exiting eliminate" << std::endl;
//	prod_unwrap_single_term(st);
//	txtout << st.node << " " << tr.parent(st).node << std::endl;
//	txtout << *st->name << " " << *(tr.parent(st)->name) << std::endl;
	sibling_iterator ff=tr.begin(st);
	if(ff==tr.end(st)) {
		st->name=onept;
		}
	else {
		++ff;
		if(ff==tr.end(st)) {
			tr.begin(st)->fl.bracket=st->fl.bracket;
			tr.begin(st)->fl.parent_rel=st->fl.parent_rel;
			tr.begin(st)->multiplier=st->multiplier;
			tr.flatten(st);
			st=tr.erase(st);
			}
		}
	cleanup_sums_products(tr, st);
//	txtout << "looped " << looping << std::endl;
	if(expression_modified) return l_applied;
	else return l_no_action;
	}


reduce_gendelta::reduce_gendelta(exptree&tr, iterator it)
	: algorithm(tr, it)
	{
	}

void reduce_gendelta::description() const
	{
	txtout << "Reduce generalised deltas with contracted indices to simpler form." << std::endl;
	}

bool reduce_gendelta::can_apply(iterator st)
	{
	const KroneckerDelta *kr=properties::get<KroneckerDelta>(st);
	if(kr) {
		if(tr.number_of_children(st)>2)
			return true;
		}
	return false;
	}

algorithm::result_t reduce_gendelta::apply(iterator& st)
	{
	int num=0;
	while(one_step_(st)) {
		++num;
		expression_modified=true;
		if(tr.number_of_children(st)==0) {
			st->name=name_set.insert("1").first;
			break;
			}
		};
//	txtout << "eliminated " << num << " index pairs on \\delta" << std::endl;
//	if(num!=8) 
//		zero(st->multiplier);
	return l_applied;
	}

bool reduce_gendelta::one_step_(sibling_iterator dl)
	{
	sibling_iterator up=tr.begin(dl), dn;
	int flip=999, masterflip=1;
	while(up!=tr.end(dl)) {
		flip=masterflip;
		dn=tr.begin(dl);
		++dn;
		while(dn!=tr.end(dl)) {
			if(up->name==dn->name) 
				goto found;
			++dn; ++dn;
			flip=-flip;
			}
		++up; ++up;
		masterflip=-masterflip;
		}
   return false;
   found:
//	debugout << "reduce_gendelta: eliminating " << *up->name << " contraction." << std::endl;
//	{unsigned int num=1;
//	tr.print_recursive_treeform(debugout, dl, num) << std::endl;}
	// FIXME: use properties for the dimension!
//	txtout << *dl->multiplier << std::endl;
	const Integer *itg=properties::get<Integer>(up);
	int dim;
	if(itg) {
		const nset_t::iterator onept=name_set.insert("1").first;
		if(itg->difference.begin()->name==onept)
			dim=to_long(*itg->difference.begin()->multiplier);
		else
			throw consistency_error("Summation range for index is not an integer.");
		}
	else throw consistency_error("No dimension known for summation index.");

	int mult=flip*(dim-tr.number_of_children(dl)/2+1);
	multiply(dl->multiplier, (multiplier_t)(mult));
	multiply(dl->multiplier, multiplier_t(2)/((multiplier_t)(tr.number_of_children(dl))));
//	txtout << "flip =" << flip << std::endl;

	// remove the indices
	sibling_iterator up2=up; ++up2; ++up2;
	while(up2!=tr.end(dl)) {
		up->name=up2->name;
		++up; ++up;
		++up2; ++up2;
		}
	sibling_iterator dn2=dn; ++dn2; ++dn2;
	while(dn2!=tr.end(dl)) {
		dn->name=dn2->name;
		++dn; ++dn;
		++dn2; ++dn2;
		}
	sibling_iterator lst=tr.end(dl); --lst; --lst;
	lst=tr.erase(lst);
	tr.erase(lst);

//	{unsigned int num=1;
//	tr.print_recursive_treeform(debugout, dl, num) << std::endl;}
	
	return true;
	}


break_gendelta::break_gendelta(exptree&tr, iterator it)
	: algorithm(tr, it)
	{
	}

void break_gendelta::description() const
	{
	txtout << "Convert a generalised KroneckerDelta (with more than two indices) to a product of normal KroneckerDeltas." << std::endl;
	}

bool break_gendelta::can_apply(iterator st)
	{
	const KroneckerDelta *kr=properties::get<KroneckerDelta>(st);
	if(kr)
		if(tr.number_of_children(st)>2)
			return true;
	return false;
	}

algorithm::result_t break_gendelta::apply(iterator& st)
	{
	exptree rep;
	iterator sum=tr.insert(rep.begin(), str_node("\\sum"));
	combin::combinations<str_node> ci;

	std::vector<iterator> remove_these;

	// Determine implicit anti-symmetrisation by looking for anti-symmetric
	// tensors in the product (if any).
	// FIXME: finding these sort of index sets has to be done in a more general way.
	if(*(tr.parent(st)->name)=="\\prod") {
		sibling_iterator oth=tr.begin(tr.parent(st));
		while(oth!=tr.end(tr.parent(st))) {
			if(oth!=st) {
				const TableauBase *tb=properties::get<TableauBase>(oth);
				if(tb) {
					for(unsigned int i=0; i<tb->size(tr, oth); ++i) {
						TableauBase::tab_t tmptab(tb->get_tab(tr, oth, i));
						for(unsigned int asset=0; asset<tmptab.row_size(0); ++asset) {
							if(tmptab.column_size(asset)>1) {
								// CHECK!!! It looks as if I get more than one asym set, and overlapping indices. That cannot be the case, but better check carefully!!!
//							txtout << "asym set: ";
								iterator comma=tr.append_child(this_command, str_node("\\comma", str_node::b_none));
//							sibling_iterator oth2=tr.begin(oth);
								for(unsigned int asel=0; asel<tmptab.column_size(asset); ++asel) {
									int indexnum=tmptab(asel,asset);
									exptree::index_iterator oth2=tr.begin_index(oth);
									oth2+=indexnum;
									tr.append_child(comma, *oth2);
//								txtout << *oth2->name << " ";
									}
//							txtout << std::endl;
								remove_these.push_back(comma);
								}
							}
						}
					}
				}
			++oth;
			}
		}
	args_begin_=tr.end(); // FIXME: this is really ugly...
//	unsigned int thisnum=1;
//	tr.print_recursive_treeform(debugout, this_command, thisnum) << std::endl;

	// First determine the overlap factors.
	multiplier_t divfactor1=1, divfactor2=1;
	sibling_iterator args_it=args_begin();
	while(args_it!=args_end()) {
		if(*args_it->name=="\\comma") {
			sibling_iterator cst=tr.begin(args_it);
			unsigned int overlap1=0, overlap2=0;
			while(cst!=tr.end(args_it)) {
				sibling_iterator ind=tr.begin(st);
				for(unsigned int pairnum=0; pairnum<tr.number_of_children(st); pairnum+=2) {
					if(ind->name==cst->name) ++overlap1;
					++ind;
					if(ind->name==cst->name) ++overlap2;
					++ind;
					}
				++cst;
				}
			divfactor1*=combin::fact(overlap1);
			divfactor2*=combin::fact(overlap2);
			}
		++args_it;
		}
	debugout << "break_gendelta: division factors are " << divfactor1 << ", " << divfactor2 << std::endl;
//	debugout << "for " << std::endl;
//	unsigned int numm=1;
//	tr.print_recursive_treeform(debugout, st, numm);
	bool permute_second_set=(divfactor2>divfactor1);

	// construct the original permutation
	sibling_iterator ind=tr.begin(st);
	for(unsigned int pairnum=0; pairnum<tr.number_of_children(st); pairnum+=2) {
		if(permute_second_set) 
			++ind;
		ci.original.push_back(*ind);
		++ind;
		if(!permute_second_set)
			++ind;
		ci.sublengths.push_back(1);
		}
	// Now construct the output asym ranges.
	sibling_iterator it=args_begin();
	while(it!=args_end()) {
		if(*it->name=="\\comma") {
			sibling_iterator cst=tr.begin(it);
			combin::range_t asymrange1;
			while(cst!=tr.end(it)) {
				for(unsigned int i1=0; i1<ci.original.size(); ++i1) {
					if(ci.original[i1].name==cst->name) {
						asymrange1.push_back(i1);
						break;
						}
					}
				++cst;
				}
			ci.input_asym.push_back(asymrange1);
			}
		++it;
		}
	// do it
	ci.permute();

	// for each permutation, add a product of deltas with two indices
	for(unsigned int permnum=0; permnum<ci.size(); ++permnum) {
		iterator prod=tr.append_child(sum, str_node("\\prod"));
		int sgn=combin::ordersign(ci[permnum].begin(), ci[permnum].end(), 
										  ci.original.begin(), ci.original.end());
		multiplier_t mult=multiplier_t(ci.multiplier(permnum))/combin::fact(ci.original.size())*sgn;
//		debugout << "multipliers: " << mult << " = " << ci.multiplier(permnum) 
//					<< " / " << fact(ci.original.size()) << " * " << sgn << std::endl;
		multiply(prod->multiplier, mult);
		multiply(prod->multiplier, *st->multiplier);
		sibling_iterator ind=tr.begin(st);
		for(unsigned int pairnum=0; pairnum<ci.original.size(); ++pairnum) {
			iterator delta=tr.append_child(prod, str_node(st->name));//"\\delta"));
			if(permute_second_set) 
				tr.append_child(delta, (iterator)(ind));
			tr.append_child(delta, ci[permnum][pairnum]);
			++ind;
			if(!permute_second_set)
				tr.append_child(delta, (iterator)(ind));
			++ind;
			}
		}

	expression_modified=true;
	debugout << "breakgendelta done" << std::endl;

	if(rep.number_of_children(sum)==1) { // flatten \sum node if there was only one term
		rep.flatten(sum);
		rep.erase(sum);
		}
	debugout << "replacing" << std::endl;
	iterator reploc=tr.replace(st, rep.begin());
	if(*reploc->name=="\\sum" && *(tr.parent(reploc)->name)=="\\sum") {
		tr.flatten(reploc);
		reploc=tr.erase(reploc);
		}
	else if(*(tr.parent(reploc)->name)=="\\prod") {
		prodflatten pf(tr, tr.end());
		if(pf.can_apply(tr.parent(reploc))) {
			iterator par=tr.parent(reploc);
			pf.apply(par);
			reploc=par;
			}
		}
	
	debugout << "removing" << std::endl;
	// remove the stuff we added
	for(unsigned int i=0; i<remove_these.size(); ++i)
		tr.erase(remove_these[i]);

	debugout << "removed" << std::endl;

	st=reploc;
	return l_applied;
	}



dualise_tensor::dualise_tensor(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void dualise_tensor::description() const
	{
	txtout << "Use (anti)self-duality of the indicated tensor to dualise it." << std::endl;
	}

bool dualise_tensor::can_apply(iterator st)
	{
	const SelfDual *sd=properties::get<SelfDual>(st);
	if(sd) return true;
	else   return false;
	}

algorithm::result_t dualise_tensor::apply(iterator& st)
	{
	exptree rep;
	iterator top=rep.set_head(str_node("\\prod"));
	multiplier_t mult=1/combin::fact(multiplier_t(tr.number_of_children(st)));
	multiply(top->multiplier, mult);
	iterator eps =rep.append_child(top, str_node("\\epsilon"));
	iterator tens=rep.append_child(top, *st);

	sibling_iterator orig=tr.begin(st);
	while(orig!=tr.end(st)) {
		rep.append_child(eps, *orig);
		++orig;
		}
	const Indices *dums=0;
	if(tr.number_of_children(st)>0)
		dums=properties::get<Indices>(tr.begin(st));
	if(!dums)
		 throw consistency_error("No information about the index type known.");

	index_map_t ind_free, ind_dummy, generated_indices;
	classify_indices(tr.parent(st), ind_free, ind_dummy);

	for(unsigned int i=0; i<tr.number_of_children(st); ++i) {
		assert(dums);
		exptree dummy = get_dummy(dums, &ind_free, &ind_dummy, &generated_indices);
		iterator tmpit=rep.append_child(eps,  dummy.begin());
		generated_indices.insert(index_map_t::value_type(dummy, tmpit));
		tmpit->fl.bracket=str_node::b_none;
		if(dums->position_free)
			 tmpit->fl.parent_rel=str_node::p_sub;
		else
			 tmpit->fl.parent_rel=str_node::p_super;
		tmpit=rep.append_child(tens, dummy.begin());
		tmpit->fl.bracket=str_node::b_none;
		tmpit->fl.parent_rel=str_node::p_sub;
		}
	expression_modified=true;
	multiply(rep.begin()->multiplier, *st->multiplier);

	st=tr.replace(st, rep.begin());
	cleanup_nests(tr, st);
	return l_applied;
	}


epsprod2gendelta::epsprod2gendelta(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void epsprod2gendelta::description() const
	{
	txtout << "Eliminate the product of two epsilon tensors in favour of a generalised kronecker delta." 
			 << std::endl;
	}

bool epsprod2gendelta::can_apply(iterator st)
	{
	if(*st->name!="\\prod")
		return false;

	epsilons.clear();
	// Find the two epsilon tensors in the product.
	sibling_iterator it=tr.begin(st);
	signature=1;
	while(it!=tr.end(st)) {
		const EpsilonTensor *eps=properties::get<EpsilonTensor>(it);
		if(eps) {
			epsilons.push_back(it);
			// FIXME: what if the epsilons are not all the same type?
			if(eps->metric.begin()!=eps->metric.end()) {
				const Metric *met=properties::get<Metric>(eps->metric.begin());
				if(met) 
					signature=met->signature;
				}
			if(eps->krdelta.begin()!=eps->krdelta.end())
				repdelta=eps->krdelta;
			}
		++it;
		}
	if(epsilons.size()<2)
		return false;

	if(repdelta.begin()==repdelta.end())
		return false;
	else repdelta.erase_children(repdelta.begin());

	return true;
	}

algorithm::result_t epsprod2gendelta::apply(iterator& st)
	{
	exptree rep(repdelta);
	iterator delta=rep.begin();

	sibling_iterator eps1=tr.begin(epsilons[0]);
	sibling_iterator eps2=tr.begin(epsilons[1]);
	while(eps1!=tr.end(epsilons[0])) {
		rep.append_child(delta, *eps1);
		rep.append_child(delta, *eps2);
		++eps1;
		++eps2;
		}
	multiply(st->multiplier, *epsilons[0]->multiplier);
	multiply(st->multiplier, *epsilons[1]->multiplier);
	tr.erase(epsilons[0]);
	multiply(st->multiplier, combin::fact(multiplier_t(tr.number_of_children(epsilons[1]))));

	multiply(st->multiplier, signature);

	iterator gend=tr.replace(epsilons[1], rep.begin());

	if(!has_argument("noreduce")) {
		reduce_gendelta rg(tr, tr.end());
		if(rg.can_apply(gend))
			rg.apply(gend);
		if(*gend->multiplier==0) {
			zero(st->multiplier);
			return l_applied;
			}
		}
	
	if(*gend->multiplier!=1) {
		multiply(tr.parent(gend)->multiplier, *gend->multiplier);
		gend->multiplier=rat_set.insert(1).first;
		}

	if(tr.number_of_children(st)==1) {
		multiply(tr.begin(st)->multiplier, *st->multiplier);
		tr.flatten(st);
		st=tr.erase(st);
		}

	expression_modified=true;
	return l_applied;
	}



eliminate_eps::eliminate_eps(exptree& tr, iterator it) 
	: algorithm(tr,it)
	{
	}

void eliminate_eps::description() const
	{
	txtout << "Eliminate epsilon tensors using self-duality." << std::endl;
	}

bool eliminate_eps::can_apply(iterator st)
	{
	if(*st->name!="\\prod") return false;
	if(number_of_args()==0) {
		txtout << "need at least one self-dual tensor as argument." << std::endl;
		return false;
		}

	// Find the epsilon, and find at least one of the indicated self-dual tensors.
	tensors.clear();
	epsilons.clear();
	sibling_iterator it=tr.begin(st);
	while(it!=tr.end(st)) {
		const EpsilonTensor *eps=properties::get<EpsilonTensor>(it);
		if(eps)
			epsilons.push_back(it);
		else {
			sibling_iterator arg=args_begin();
			unsigned int num=0;
			while(arg!=args_end()) {
				if(arg->name==it->name)
					tensors.push_back(it);
				++arg;
				++num;
				}
			}
		++it;
		}
	if(epsilons.size()!=1) {
//		debugout << "found " << epsilons.size() << " epsilons" << std::endl;
		return false;
		}
	if(tensors.size()==0) {
//		debugout << "no self-dual tensors found." << std::endl;
		return false;
		}

	return true;
	}

algorithm::result_t eliminate_eps::apply(iterator& st)
	{
	std::vector<unsigned int> indexmatch(tensors.size(), 0);
	for(unsigned int i=0; i<tensors.size(); ++i) {
		debugout << "eliminate_eps: counting index overlap with tensor " << *(tensors[i]->name) << std::endl;
		sibling_iterator it=tr.begin(tensors[i]);
		while(it!=tr.end(tensors[i])) {
			sibling_iterator fe=epsilons[0].begin();
			while(fe!=epsilons[0].end()) {
				if(fe->name==it->name) {
					++indexmatch[i];
					break;
					}
				++fe;
				}
			++it;
			}
		}
	debugout << "eliminate_eps: determining maximum overlap..." << std::endl;
	unsigned int maxone=0, maxnum=0;
	for(unsigned int i=0; i<indexmatch.size(); ++i) {
		if(indexmatch[i]>maxnum) {
			maxnum=indexmatch[i];
			maxone=i;
			}
		}
	std::vector<iterator> dualise_these;
	for(unsigned int i=0; i<indexmatch.size(); ++i) {
		if(indexmatch[i]==maxnum)
			dualise_these.push_back(tensors[i]);
		}
	debugout << "eliminate_eps: dualising " << dualise_these.size() << " tensors" << std::endl;
	if(dualise_these.size()==1) {
		dualise_tensor dt(tr, tr.end());
		if(dt.can_apply(tensors[maxone]))
			dt.apply(tensors[maxone]);
		}
	else {
		// replace \prod{} with \sum{\prod{}}
		iterator sumnode=st;
		iterator sumcopy=tr.append_child(tr.begin(),str_node("\\sum"));
		dualise_tensor dt(tr, tr.end());
		for(unsigned int i=0; i<dualise_these.size(); ++i) {
			iterator sumnodecopy=tr.append_child(sumcopy, sumnode);
			multiply(sumnodecopy->multiplier, 1/multiplier_t(dualise_these.size()));
			iterator thetensor=sumnodecopy;
			std::advance(thetensor, std::distance(sumnode, dualise_these[i]));
			if(dt.can_apply(thetensor))
				dt.apply(thetensor);
			}
		
		st=tr.replace(st, sumcopy);
		tr.erase(sumcopy);
		if(*(tr.parent(st)->name)=="\\sum") {
			tr.flatten(st);
			st=tr.erase(st);
			}
		}
	prodflatten pf(tr, tr.end());
	iterator par=tr.parent(st);
	pf.apply_recursive(par);
	expression_modified=true;

	return l_applied;
	}

product_shorthand::product_shorthand(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void product_shorthand::description() const
	{
	txtout << "Rewrites the product of two symmetric or anti-symmetric tensors in a compact form by removing the contracting dummy indices." << std::endl;
	}

bool product_shorthand::can_apply(iterator it) 
	{
	if(*it->name=="\\prod")
		if(number_of_args()==2) 
			return true;
	return false;
	}

algorithm::result_t product_shorthand::apply(iterator& it)
	{
	sibling_iterator fac1=args_begin();
	sibling_iterator fac2=fac1; ++fac2;

	index_map_t map1, map2;
	sibling_iterator facs=tr.begin(it);
	while(facs!=tr.end(it)) {
		if(facs->name==fac1->name) {
			fill_map(map1, tr.begin(facs), tr.end(facs));
			}
		else if(facs->name==fac2->name) {
			fill_map(map2, tr.begin(facs), tr.end(facs));
			}
		++facs;
		}
	if(map1.size()==0 || map2.size()==0) return l_no_action;

	index_map_t contr;
	determine_intersection(map1, map2, contr, true);
	if(contr.size()==0)
		return l_no_action;

	int sign=1;
	index_map_t::iterator mi=contr.begin();
//	txtout << contr.size() << std::endl;
	while(mi!=contr.end()) {
		if(properties::get<AntiSymmetric>(fac1))
			sign*=index_parity((*mi).second);
		tr.erase((*mi).second);
		++mi;
		if(properties::get<AntiSymmetric>(fac2))
			sign*=index_parity((*mi).second);
		tr.erase((*mi).second);
		++mi;
		}
	multiply(it->multiplier, sign);
	expression_modified=true;
	return l_applied;
	}


expand_product_shorthand::expand_product_shorthand(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void expand_product_shorthand::description() const
	{
	txtout << "Inverse of product shorthand." << std::endl;
	}

bool expand_product_shorthand::can_apply(iterator it) 
	{
	if(*it->name=="\\prod")
		if(number_of_args()==3) 
			return true;
	return false;
	}

algorithm::result_t expand_product_shorthand::apply(iterator& it)
	{
	sibling_iterator fac1=args_begin();
	sibling_iterator fac2=fac1; ++fac2;
	sibling_iterator fac3=fac2; ++fac3;
	unsigned int fillup_to=static_cast<unsigned int>((*fac3->multiplier).get_d());

	sibling_iterator obj1, obj2;

	sibling_iterator facs=tr.begin(it);
	while(facs!=tr.end(it)) {
		if(facs->name==fac1->name) {
			obj1=facs;
			}
		else if(facs->name==fac2->name) {
			obj2=facs;
			}
		++facs;
		}

	if(tr.number_of_children(obj1)>=fillup_to)
		return l_no_action;

	unsigned int i=tr.number_of_children(obj1);
	const Indices *dums=0;
	if(i>0) dums=properties::get<Indices>(tr.begin(obj1));
	for(; i<fillup_to; ++i) {
		assert(dums);
		exptree dum=get_dummy(dums, obj1);
		iterator tmpit=tr.append_child(iterator(obj1), dum.begin());
		tmpit->fl.bracket=str_node::b_none;
		tmpit->fl.parent_rel=str_node::p_sub;
		tmpit=tr.append_child(iterator(obj2), dum.begin());
		tmpit->fl.bracket=str_node::b_none;
		tmpit->fl.parent_rel=str_node::p_sub;
		}

	expression_modified=true;
	return l_applied;
	}

remove_eoms::remove_eoms(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void remove_eoms::description() const
	{
	// FIXME: this is ugly
	txtout << "Remove DF lowest order equations of motion." << std::endl;
	}

bool remove_eoms::can_apply(iterator it)
	{
	const DAntiSymmetric *da=properties::get<DAntiSymmetric>(it);
	if(da) return true;
	else   return false;
	}

algorithm::result_t remove_eoms::apply(iterator& it) 
	{
	sibling_iterator ch=tr.begin(it);
	nset_t::iterator dindex=ch->name;

	++ch;
	while(ch!=tr.end(it)) {
		if(ch->name==dindex) {
			zero(it->multiplier);
			expression_modified=true;
			break;
			}
		++ch;
		}
	return l_applied;
	}

pintegrate::pintegrate(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void pintegrate::description() const
	{
	txtout << "Perform partial integration." << std::endl;
	}

bool pintegrate::can_apply(iterator it)
	{
	if(number_of_args()==1) {
		 if(*it->name=="\\prod") {
			  if(*(tr.parent(it)->name)=="\\sum") {
					if(*(tr.parent(tr.parent(it))->name)=="\\expression")
						 return true;
					}
			  else if(*(tr.parent(it)->name)=="\\expression")
					return true;
			  }
		 }
	return false;
	}

algorithm::result_t pintegrate::apply(iterator& it)
	{
	nset_t::iterator opname=args_begin()->name;

	sibling_iterator sib=tr.begin(it);
	while(sib!=tr.end(it)) {
		if(sib->name==opname) {
//			if(tr.number_of_children(it)==1) { // total derivative
//				zero(it->multiplier);
//				return l_applied;
//				}
			// If the number of indices of the derivative is zero or odd (implicit),
			// there is an odd number of partial integrations, and we need
			// a sign flip.
			if( (tr.number_of_children(sib)%2) == 0 || tr.number_of_children(sib)==1)
				multiply(it->multiplier, -1);
			sibling_iterator newdiff=tr.insert_subtree(tr.begin(it), sib);
			// FIXME: tree.hh does not remove the existing children of the replaced node
			// Put a product in the new derivative.
			sibling_iterator newdiffarg=tr.begin(newdiff);
			while(newdiffarg->is_index()) ++newdiffarg;
			sibling_iterator prodindiff=tr.replace(newdiffarg, str_node("\\prod"));
			tr.erase_children(prodindiff);
			// Put all nodes up to the derivative inside the new derivative.
			sibling_iterator from=newdiff; ++from;
			sibling_iterator nxt=sib;
			tr.reparent(prodindiff, from, nxt);
			++nxt;
			// and also do so for all nodes after the derivative, if any.
			if(nxt!=tr.end(sib))
				tr.reparent(prodindiff, nxt, tr.end(it));
			// Finally, move the old nodes out of the derivative.
			sibling_iterator oldarg=tr.begin(sib);
			while(oldarg->is_index()) ++oldarg;
			tr.append_child(it, (iterator)(oldarg));
			tr.erase(sib);
			if(tr.number_of_children(prodindiff)==1) {
				tr.flatten(prodindiff);
				tr.erase(prodindiff);
				}
			return l_applied;
			}
		++sib;
		}
	return l_no_action;
	}


unwrap::unwrap(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void unwrap::description() const
	{
	txtout << "" << std::endl;
	}

bool unwrap::can_apply(iterator it)
	{
	const Derivative *der=properties::get<Derivative>(it);
	const Accent     *acc=properties::get<Accent>(it);
	if(der || acc) 
		return true;
	return false;
	}

// \del{a*f*A*b*C*d*e} -> a*f*\del{A*b*C}*d*e
//
// locate first dependent factor
// locate first following independent factor
// ...
//
// Should also work for brackets, like
// 
//    \poisson( A )( B ) -> 0
//
// if either A or B has vanishing poisson bracket, and
//
//    \poisson( N1 D1 )( N2 D2 ) -> N1 N2 \poisson( D1 )( D2 ).
//
// So all "derivatives" 

algorithm::result_t unwrap::apply(iterator& it) 
	{
	prodsort prs(tr, tr.end());
	bool is_accent=properties::get<Accent>(it);

	iterator prodwrap=tr.wrap(it, str_node("\\prod"));

	bool all_arguments_moved_out=true;
	sibling_iterator acton=tr.begin(it);
	while(acton!=tr.end(it)) {
		// Only look at child nodes which are not indices.
		if(acton->is_index()==false) { 
			sibling_iterator derarg=acton;
			++acton; // don't use this anymore this loop

			if(*derarg->name=="\\sum") continue; // FIXME: Don't know how to handle this yet.

//			txtout << "doing " << *derarg->name << std::endl;

			if(*derarg->name!="\\prod")
				derarg=tr.wrap(derarg, str_node("\\prod"));
			
			sibling_iterator factor=tr.begin(derarg);
			while(factor!=tr.end(derarg)) {
				sibling_iterator nxt=factor;
				++nxt;
				bool move_out=true;
				
				// First figure out whether there is implicit dependence on the operator.
				// or on the coordinate.
				const DependsBase *dep=properties::get_composite<DependsBase>(factor);
				if(dep!=0) {
//					txtout << *factor->name << " depends" << std::endl;
					exptree deps=dep->dependencies(it);
					sibling_iterator depobjs=deps.begin(deps.begin());
					while(depobjs!=deps.end(deps.begin())) {
//						txtout << "?" << *it->name << " == " << *depobjs->name << std::endl;
//						if(subtree_exact_equal(it, depobjs)) { WRONG! Depends(\del) should work
// without having any arguments in \del. Otherwise we would need to write this as Depends(\del{#})
						if(it->name == depobjs->name) {
//							txtout << "yep" << std::endl;
							move_out=false;
							break;
							}
						else {
							// compare all indices
							sibling_iterator indit=tr.begin(it);
							while(indit!=tr.end(it)) {
								if(indit->is_index()) {
									if(subtree_exact_equal(indit, depobjs)) {
										move_out=false;
										break;
										}
									}
								++indit;
								}
							if(!move_out) break;
							}
						++depobjs;
						}
					}
				
				// Finally, there may also be explicit dependence.
				if(move_out) {
					// FIXME: This certainly does not handle Y(a,b) correctly
               sibling_iterator chldit=tr.begin(factor);
               while(chldit!=tr.end(factor)) {
                  if(chldit->is_index()==false) {
                     sibling_iterator indit=tr.begin(it);
                     while(indit!=tr.end(indit)) {
                        if(subtree_exact_equal(chldit, indit, 0)) {
                           move_out=false;
                           break;
                           }
                        ++indit;
                        }
                     if(!move_out) break;
                     }
                  ++chldit;
                  }
					}
				
				// If no dependence found, move this child out of the derivative.
				if(move_out) { // FIXME: Does not handle subtree-compare properly, and does not look at the
					// commutativity property of the index wrt. the derivative is taken.
					int sign=1;
					if(factor!=tr.begin(derarg)) 
						sign=prs.can_swap(tr.begin(derarg),factor,2);
					
					expression_modified=true;
					tr.move_before(it, factor);
					multiply(prodwrap->multiplier, sign);
					}
				
				factor=nxt;
				}
			
			// All factors in this argument have been handled now, let's see what's left.
			unsigned int derarg_num_chldr=tr.number_of_children(derarg);
			if(derarg_num_chldr==0) {
				// Empty accents should simply be ignored, but empty derivatives vanish.
				if(!is_accent) {
					zero(prodwrap->multiplier);
					break; // we can stop now, the entire expression is zero.
					}
				}
			else {
				all_arguments_moved_out=false;
				if(derarg_num_chldr==1) {
					 derarg=tr.flatten_and_erase(derarg);
					}
				}
			}
		else ++acton;
		}


	// All non-index arguments have now been handled. 
	if(all_arguments_moved_out && is_accent) 
		it=tr.erase(it);
	else if(*prodwrap->multiplier!=0) {
		if(tr.number_of_children(prodwrap)==1) { // nothing was moved out
			tr.flatten(prodwrap);
			prodwrap=tr.erase(prodwrap);
			}
		else {
			 // Moving factors around has potentially led to a top-level product
			 // which contains children with non-unit multiplier.
			 prodcollectnum pc(tr, prodwrap);
			 if(pc.can_apply(prodwrap))
				  pc.apply(prodwrap);

			 // Unnest products if necessary.
			 if(*prodwrap->name=="\\prod" && *tr.parent(prodwrap)->name=="\\prod") {
				  tr.flatten(prodwrap);
				  prodwrap=tr.erase(prodwrap);
				  prodwrap=tr.parent(prodwrap);
				  }
			 }
		it=prodwrap;
		}
	else it=prodwrap;

//	tr.print_recursive_treeform(txtout, it);
// Adding one of the following lines screws up expressions...
//	cleanup_expression(tr, it);
//	cleanup_nests(tr,it);
	if(expression_modified) return l_applied;
	else return l_no_action;
	}


// remove_vanishing_derivatives::remove_vanishing_derivatives(exptree& tr, iterator it)
// 	: algorithm(tr, it)
// 	{
// 	}
// 
// void remove_vanishing_derivatives::description() const
// 	{
// 	txtout << "Remove terms with identically vanishing derivatives." << std::endl;
// 	}
// 
// bool remove_vanishing_derivatives::can_apply(iterator it)
// 	{
// 	const Derivative *der=properties::get<Derivative>(it);
// 	if(der) 
// 		if(*tr.begin(it)->name!="\\prod")
// 			return true;
// 	return false;
// 	}
// 
// algorithm::result_t remove_vanishing_derivatives::apply(iterator& it) 
// 	{
// 	sibling_iterator op=tr.begin(it);
// 
// 	const DependsBase *dep=properties::get_composite<DependsBase>(op);
// 	if(dep==0) {
// //		txtout << *it->name << " has no deps" << std::endl;
// 		zero(it->multiplier);
// 		expression_modified=true;
// 		return l_applied;
// 		}
// 
// 	exptree deps=dep->dependencies(it);
// 	sibling_iterator depobjs=deps.begin(deps.begin());
// 	while(depobjs!=deps.end(deps.begin())) {
// 		// FIXME: the dependencies tree should contain patterns, not expressions
// 		// right now, we just compare the head node
// 		if(it->name == depobjs->name) 
// 			return l_no_action;
// 		++depobjs;
// 		}
// 	zero(it->multiplier);
// 	expression_modified=true;
// 
// 	return l_applied;
// 	}


impose_bianchi::impose_bianchi(exptree& tr, iterator it)
	: algorithm(tr, it), tb(0)
	{
	}

void impose_bianchi::description() const
	{
	txtout << "Remove this term if a full hook of the TableauSymmetry is contracted with an anti-symmetric set of indices." << std::endl;
	}

bool impose_bianchi::can_apply(iterator it)
	{
	debugout << "trying on " << *it->name << std::endl;
	tb=properties::get<TableauBase>(it);
	if(tb) {
		if(tb->is_simple_symmetry(tr, it)) return false;
		return true;
		}
	else   return false;
	}

algorithm::result_t impose_bianchi::apply(iterator& st) 
	{
	typedef std::set<nset_t::iterator, nset_it_less> nodes_t;
	std::vector<nodes_t> asymsets;
	sibling_iterator thisone(st);

	debugout << "impose_bianchi on " << *st->name << std::endl;
	// Collect all index sets in the same product as the current tensor
	// which are anti-symmetric (if any).
	if(*(tr.parent(st)->name)=="\\prod") {
		sibling_iterator oth=tr.begin(tr.parent(st));
		while(oth!=tr.end(tr.parent(st))) {
			if(oth!=thisone) {
				const TableauBase *tb=properties::get<TableauBase>(oth);
				if(tb) {
					for(unsigned int i=0; i<tb->size(tr, oth); ++i) {
						TableauBase::tab_t tmptab(tb->get_tab(tr, oth, i));
						for(unsigned int asset=0; asset<tmptab.row_size(0); ++asset) {
							if(tmptab.column_size(asset)>1) {
								nodes_t tmp;
								for(unsigned int asel=0; asel<tmptab.column_size(asset); ++asel) {
									int indexnum=tmptab(asel,asset);
									exptree::index_iterator oth2=tr.begin_index(oth);
									oth2+=indexnum;
									tmp.insert(oth2->name);
									}
								asymsets.push_back(tmp);
								}
							}
						}
					}
				}
			++oth;
			}
		}

	// Loop over all tableaux. If we find a Garnir set saturated with indices
	// from one of the anti-symmetric index sets collected above, make this term 
	// zero.
	for(unsigned int tt=0; tt<tb->size(tr, st); ++tt) { // loop over all tableaux
		unsigned int row=0, col=1;
		TableauBase::tab_t tmptab=tb->get_tab(tr, st, tt);
		while(col < tmptab.row_size(row) ) {
			std::vector<int> ids;
			tmptab.Garnir_set(std::back_insert_iterator<std::vector<int> >(ids), row, col);
			nodes_t idsns;
			for(unsigned int i=0; i<ids.size(); ++i) {
				exptree::index_iterator oii=tr.begin_index(st);
				oii+=ids[i];
				idsns.insert(oii->name);
				}
			for(unsigned int i=0; i<asymsets.size(); ++i) {
				nset_it_less comp;
				if(std::includes(asymsets[i].begin(), asymsets[i].end(), idsns.begin(), idsns.end(), comp)) {
					zero(st->multiplier);
					expression_modified=true;
					return l_applied;
					}
				}
			++col;
			}
		}

	return l_applied;
	}


all_contractions::all_contractions(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void all_contractions::description() const
	{
	txtout << "Construct all possible full contractions given mono-term symmetries." << std::endl;
	}

bool all_contractions::can_apply(iterator it)
	{
	if(*(it->name)!="\\prod" && !is_single_term(it)) {
		if(*it->name=="\\comma") {
			sibling_iterator cit=tr.begin(it);
			while(cit!=tr.end(it)) {
				if(*cit->name=="\\comma") return false;
				++cit;
				}
			}
		else return false;
		}

	// The algorithm does not work when sum nodes appear inside the product
	// (it is not worth fixing this)
	sibling_iterator sib=tr.begin(it);
	while(sib!=tr.end(it)) {
		if(*sib->name=="\\sum" || *sib->name=="\\prod" || sib->is_command())
			return false;
		++sib;
		}

	return true;
	}

void all_contractions::create_spinor_contractions(iterator it)
	{
	std::vector<sibling_iterator> spinors;
	std::vector<sibling_iterator> gmatrices;

	if(*it->name=="\\comma") {
		it->name=name_set.insert("\\prod").first;
		sibling_iterator sib=tr.begin(it);
		while(sib!=tr.end(it)) {
			const Spinor *sptmp=properties::get<Spinor>(sib);
			if(sptmp) 
				spinors.push_back(sib);
			else {
				const GammaMatrix *gmtmp=properties::get<GammaMatrix>(sib);
				if(gmtmp)
					gmatrices.push_back(sib);
				}
			++sib;
			}
		if(spinors.size()==0 && gmatrices.size()==0) return;
		if(spinors.size()==2 && gmatrices.size()==1) {
			tr.move_after(spinors[0], gmatrices[0]);
			tr.move_after(gmatrices[0], spinors[1]);
			tr.wrap(spinors[0], str_node("\\bar"));
			return;
			}
		assert(1==0);
		}
	}

all_contractions::iterator_indexgroup_t::iterator_indexgroup_t(const iterator& theit, int theint)
	: it(theit), indexgroup(theint)
	{
	}

algorithm::result_t all_contractions::apply(iterator& it) 
	{
	debugout << "all_contractions on " << *it->name << std::endl;
	if(it->is_command()) return l_no_action; // FIXME: bizarre bug
//	txtout << "single* " << is_single_term(it) << " " << *it->name << std::endl;
	prod_wrap_single_term(it);
	create_spinor_contractions(it);
//	txtout << "single* " << is_single_term(it) << " " << *it->name << std::endl;
	ind_free.clear();
	ind_dummy.clear();
	classify_indices(it, ind_free, ind_dummy);

//	txtout << "asking LiE" << std::endl;
	number_to_find=exchange::possible_singlets(tr, it);
	debugout << "LiE says: " << number_to_find << std::endl;

	if(number_of_args()==1) {
		number_to_find=to_long(*(args_begin()->multiplier));
		debugout << "overriding: only searching " << number_to_find << std::endl;
		}

//	number_to_find=std::min(10U, number_to_find);
//	debugout << "LiE says: " << number_to_find << std::endl;

	if(number_to_find==0) {
		node_zero(it);
		expression_modified=true;
		return l_applied;
		}

	bool nontrivial_symmetries_present=false;
	sibling_iterator factorit=tr.begin(it);
	while(factorit!=tr.end(it)) {
		if(tr.number_of_indices(factorit)>1) {
			const TableauBase *tb = properties::get<TableauBase>(factorit);
			assert(tb);
			if(!tb->is_simple_symmetry(tr, it)) {
				nontrivial_symmetries_present=true;
				break;
				}
			}
		++factorit;
		}

	// Collection of all terms encountered so far in the Young projection.
	std::vector<exptree>                    terms_from_yp;

	// For each term in bigsum (defined below), an expansion in terms of the
	// terms in terms_from_yp. If the known young-projected sums take the 
	// form
	//
	//    t1 = a1 monom1 + a2 monom2 + ...
   //    t2 = b1 monom1 + b2 monom2 + ...
	// 
	// then the matrix looks like
	//
	//      a1   b1   c1   ....             -> coeffs of monom1       
	//      a2   b2   c2   ....             -> coeffs of monom2
	//
	std::vector<std::vector<multiplier_t> > coefficient_matrix;

	// Sum node to collect new terms.
	exptree bigsum;
	bigsum.set_head(str_node("\\comma"));
	exptree discarded_terms; 
	discarded_terms.set_head(str_node("\\comma"));

	// Count number of random-length index ranges
	int random_length_ranges=0;
	index_map_t::iterator indit=ind_free.begin();
	while(indit!=ind_free.end()) {
		if(indit->second->is_range_wildcard()) 
			++random_length_ranges;
		++indit;
		}
	
	long loops_needed=0;

	// The main loop generating all terms.
	do {
		if(interrupted)
			throw algorithm_interrupted("all_contractions");
		++loops_needed;
		report_progress("Constructing basis", number_to_find, tr.number_of_children(bigsum.begin()));
 		exptree workexp(it);    // a new term
		canonicalise        can(workexp, workexp.end());
		remove_gamma_trace  rgt(workexp, workexp.end());
		rename_dummies      ren(workexp, workexp.end());
		index_map_t  workexpind; // and its index map
		parent_map_t index_sets; // collect indices by parent and indexgroup of parent
		iterator     workbegin;

		int random_ranges_seen=0;
		int total_number_of_indices=0;
		index_map_t::iterator indit=ind_free.begin();
		index_map_t generated_indices;

		// Loop over all free indices, expanding range wildcards and also classifying
		// indices in sets within which no contractions should be made.
		while(indit!=ind_free.end()) {
			iterator workpos=workexp.begin();
			int incr=std::distance(it, (*indit).second);
			workpos+=incr;

			iterator thetens=tr.parent(indit->second);
			while(tr.parent(thetens)!=it) 
				thetens=tr.parent(thetens);

			const TableauBase *tb=properties::get<TableauBase>(thetens);
			if(!tb) { // just add this index in a separate group
				workexpind.insert(index_map_t::value_type(indit->first, workpos));
				std::vector<exptree> tmp;
				tmp.push_back(indit->first);
				iterator_indexgroup_t iig(tr.parent(workpos), 0);
				index_sets.insert(parent_map_t::value_type(iig, tmp));
				++total_number_of_indices;
				++indit;
				continue;
				}
			exptree::index_iterator findind=tr.begin_index(thetens);
			unsigned int indexnum=0;
			while(iterator(findind)!=indit->second) {
				++indexnum;
				++findind;
				}
			debugout << "index " << *indit->second->name << " in " 
						<< *thetens->name << " is " << indexnum << std::endl;

			int indexgroupnum=tb->get_indexgroup(tr, thetens, indexnum);
			debugout << *indit->second->name << " indexgroupnum = " << indexgroupnum << std::endl;
			iterator_indexgroup_t iig(thetens, indexgroupnum); 
			parent_map_t::iterator pmit=index_sets.find(iig);
			if(pmit==index_sets.end()) {
				std::vector<exptree> tmp;
				pmit=index_sets.insert(parent_map_t::value_type(iig, tmp)).first;
				}

			// Replace range wildcards with a random-length sequence of indices,
			// such that the total number of indices is even (i.e. can be contracted fully).
			if(workpos->is_range_wildcard()) {
				++random_ranges_seen;
				sibling_iterator seqit=tr.begin(tr.begin(workpos));
				int from = to_long(*seqit->multiplier);
				++seqit;
				int rval=0;
				do {
					int to   = to_long(*seqit->multiplier);
					rval=random()%(to-from+1);
					} while(random_ranges_seen==random_length_ranges && 
							  (total_number_of_indices+rval+from) % 2 != 0); // total even
				total_number_of_indices+=rval+from;

				if(rval+from==0) { // no indices at all, just remove wildcard and index set
					index_sets.erase(pmit);
               //					workexp.erase(workexp.parent(workpos));
					}
				else {
					// insert the actual indices
					const Indices *dums=properties::get<Indices>(indit->second);
					assert(dums);
					for(int i=0; i<rval+from; ++i) {
						// FIXME: once matching to range-wildcard indices is in place, this get
						// can be done directly on the original index node rather than on the name.
//						const Indices *dums=properties::get<Indices>(baseit);
//						assert(dums);
						exptree newindex=get_dummy(dums, &ind_dummy, &ind_free, &generated_indices);
						iterator newindexit=tr.insert_subtree(workpos, newindex.begin());
						newindexit->fl.bracket=indit->second->fl.bracket;
						newindexit->fl.parent_rel=indit->second->fl.parent_rel;
						
						generated_indices.insert(index_map_t::value_type(newindex,newindexit));
						workexpind.insert(index_map_t::value_type(newindex,newindexit));
						pmit->second.push_back(newindex);
						}
					}
				tr.erase(workpos); // erase index wildcard
				}
			else {
				workexpind.insert(index_map_t::value_type(indit->first,workpos));
				++total_number_of_indices;
				pmit->second.push_back(indit->first);
				}

			++indit;
			}

		// Pick a random set of index contractions.
		parent_map_t index_sets_copy(index_sets);
		int index_sets_left=index_sets_copy.size();
		for(int pr=0; pr<total_number_of_indices/2; ++pr) {
			// We choose both indices randomly as this seems to lead to a result
			// more rapidly. No idea why.
			int rnd=random()%index_sets_left;
			parent_map_t::iterator tmpit1=index_sets_copy.begin();
			while(tmpit1->second.size()==0)
				++tmpit1;
			while(rnd--) {
				++tmpit1;
				while(tmpit1->second.size()==0)
					++tmpit1;
				}
			exptree ind1=tmpit1->second.back();
			tmpit1->second.pop_back();
			bool is_asym=!properties::get<AntiSymmetric>(tr.parent(tmpit1->first.it));
			if(is_asym && index_sets_left==1) // the other index would have to come from the same set
				goto endofloop;
			if(tmpit1->second.size()==0) 
				--index_sets_left;

			parent_map_t::iterator tmpit2;
			rnd=random()%index_sets_left;
			tmpit2=index_sets_copy.begin();
			while(tmpit2->second.size()==0)
				++tmpit2;
			while(rnd--) {
				++tmpit2;
				while(tmpit2->second.size()==0)
					++tmpit2;
				}
			
			if(is_asym && tmpit2==tmpit1) { // don't pick both indices from the same anti-symmetric set
				do {
					++tmpit2;
					if(tmpit2==index_sets_copy.end())
						tmpit2=index_sets_copy.begin();
					} while(tmpit2->second.size()==0);
				}
			exptree ind2=tmpit2->second.back();
			tmpit2->second.pop_back();
			if(tmpit2->second.size()==0) 
				--index_sets_left;

         //	debugout << "setting " << *(workexpind.find(ind1)->second->name) 
         //          << " equal to " << *ind2.begin()->name << std::endl;
			workexp.replace_index(workexpind.find(ind1)->second,ind2.begin());
			}
		assert(index_sets_left==0);

		debugout << "attempting contraction:" << std::endl;
		workexp.print_recursive_treeform(debugout, workexp.begin());

		// Remove any GammaTraceless bits (Traceless will be removed by canonicalise)
		workbegin=workexp.begin();
		if(rgt.can_apply(workbegin)) 
				goto endofloop;
		// Now canonicalise using mono-term symmetries
//		txtout << "starting canonicalise" << std::endl;
		can.apply(workbegin);
//		txtout << "canonicalise done" << std::endl;
		if(workbegin->is_zero()) {
//			txtout << "zero " << std::endl;
			}
		else {
			ren.apply(workbegin);
			one(workbegin->multiplier);
			bool found=false;

			// Check whether this term has already been generated; if so,
			// we do not have to continue searching in the Young-projected
			// set.
			sibling_iterator termit=tr.begin(bigsum.begin());
			while(termit!=tr.end(bigsum.begin())) {
				sibling_iterator web=workexp.begin();
				if(tr.equal_subtree(web, termit)) {
//					txtout << "already in tree" << std::endl;
					found=true;
					break;
					}
				++termit;
				}
			if(!found) {
				termit=tr.begin(discarded_terms.begin());
				while(termit!=tr.end(discarded_terms.begin())) {
					sibling_iterator web=workexp.begin();
					if(tr.equal_subtree(web, termit)) {
//						txtout << "already in discarded tree" << std::endl;
						found=true;
						break;
						}
					++termit;
					}
				}
			if(!found && nontrivial_symmetries_present) {
            // Project using Young projector
				exptree toproject;
				toproject.set_head(str_node("\\expression"));
				toproject.append_child(toproject.begin(), workexp.begin());
				
				young_project_tensor ypt(toproject, toproject.begin());
				ypt.modulo_monoterm=true;
				iterator toprojectit=toproject.begin(toproject.begin());
				ypt.apply_recursive(toprojectit, false, 1);
				if(*toprojectit->name=="\\prod") {
					collect_terms ct(toproject, toproject.begin());
					distribute dbt(toproject, toproject.begin());
					dbt.apply(toprojectit);

					// Remove terms which do not have exactly zero or two epsilon terms.
					sibling_iterator tterms=toproject.begin(toprojectit);
					int doubleterms=0, zeroterms=0;
					while(tterms!=toproject.end(toprojectit)) {
						int count=0;
						sibling_iterator facit=toproject.begin(tterms);
						while(facit!=toproject.end(tterms)) {
							const EpsilonTensor *eps=properties::get<EpsilonTensor>(facit);
							if(eps) ++count;
							++facit;
							}
						if(count!=2 && count!=0)
							tterms=tr.erase(tterms);
						else
							++tterms;
						if(count==2) ++doubleterms;
						else if(count==0) ++ zeroterms;
						}
//					txtout << doubleterms << " with two epsilon tensors" << std::endl;
//					txtout << zeroterms << " without epsilon tensors" << std::endl;

					// Take care of epsilon tensors in the Young projector: 
					// work out the double-epsilon terms.

// 					txtout << tr.number_of_children(toprojectit) << " terms left before" << std::endl;
// 					epsprod2gendelta ep2gd(workexp, workexp.begin());
// 					ep2gd.apply_recursive(toprojectit, false);
// 					txtout << tr.number_of_children(toprojectit) << " terms left after" << std::endl;
// 
// 					tterms=toproject.begin(toprojectit);
// 					int ttermscount=0;
// 					while(tterms!=toproject.end(toprojectit)) {
// 						exptree cpytree(tterms);
// 						iterator cpytop=cpytree.begin();
// 						txtout << "epsprod" << std::endl;
// //						cpytree.print_recursive_treeform(debugout, cpytop);
// //						epsprod2gendelta ep2gd(cpytree, cpytree.begin());
// //						ep2gd.apply_recursive(cpytop, false);
// 
// 						txtout << "bgd" << std::endl;
// 						break_gendelta bgd(cpytree, cpytree.begin());
// 						bgd.apply_recursive(cpytop, false);
// 						txtout << "distrib again" << std::endl;
// 						dbt.apply_recursive(cpytop);
// 						txtout << " mid " << ttermscount << " " << cpytree.number_of_children(cpytop) << std::endl;
// 						ct.apply_recursive(cpytop);
// 						txtout << "1" << std::endl;
// 						eliminate_kronecker ekr(cpytree, cpytree.begin());
// 						ekr.apply_recursive(cpytop, false);
// 						txtout << "1b" << std::endl;
// 						ct.apply_recursive(cpytop);
// 						txtout << "2" << std::endl;
// 						indexsort isort(cpytree, cpytree.begin());
// 						isort.apply_recursive(cpytop);
// 						ct.apply_recursive(cpytop);
// 						txtout << "2b " << cpytree.number_of_children(cpytree.begin()) << std::endl;
// 						can.reuse_generating_set=true;
// 						can.apply_recursive(cpytop, false);
// 						txtout << "3" << std::endl;
// 						ren.apply_recursive(cpytop);
// 						txtout << "4" << std::endl;
// 						ct.apply_recursive(cpytop);
// 						txtout << "5" << std::endl;
// 
// 						txtout << " done " << ttermscount << " " << cpytree.number_of_children(cpytop) << std::endl;
// 						++ttermscount;
// 						++tterms;
// 						}
// 					txtout << "through " << std::endl;
// 					can.reuse_generating_set=false; 

//					tr.print_recursive_treeform(txtout, toprojectit);
//					txtout << "ekr" << std::endl;
					eliminate_kronecker ekr(toproject, toproject.begin());
					ekr.apply_recursive(toprojectit, false, 1);
					
					ren.apply_recursive(toprojectit, false);
					ct.apply(toprojectit);
					can.apply_recursive(toprojectit, false); // do not check consistency
//					txtout << *toprojectit->name << std::endl;
					if(toprojectit==tr.end()) debugout << "OHOHO" << std::endl;
					ren.apply_recursive(toprojectit, false);
					if(*toprojectit->name=="\\sum")
						ct.apply(toprojectit);
					}
				// After young projection, we may get identically zero.
				if(toprojectit->is_zero()) {
//					txtout << "zero after projection" << std::endl;
					discarded_terms.append_child(discarded_terms.begin(),workexp.begin());
					goto endofloop;
					}

				// Decompose on the basis forming in terms_from_yp & coefficient_matrix.
				// Set multiplier to one first, otherwise things won't ever match.
				sibling_iterator termit=toproject.begin(toprojectit);
				bool new_monomial_found=false;
				std::vector<multiplier_t> remember_multipliers(coefficient_matrix.size(),0);
				while(termit!=toproject.end(toprojectit)) {
					multiplier_t remember_multiplier=*termit->multiplier;
//					toproject.print_recursive_treeform(txtout, termit);
					one(termit->multiplier);
					unsigned int yp=0;
					for(; yp<terms_from_yp.size(); ++yp) {
						if(toproject.equal_subtree(terms_from_yp[yp].begin(), (iterator)(termit)))
							break;
						}
					if(yp==terms_from_yp.size()) {
						terms_from_yp.push_back(exptree(termit));
//						txtout << "added term to terms_from_yp (now " << terms_from_yp.size() << ")" << std::endl;
						// Add a new column to all rows (new polynomial) if not already done so.
						if(!new_monomial_found) {
							new_monomial_found=true;
							for(unsigned int rows=0; rows<coefficient_matrix.size(); ++rows)
								coefficient_matrix[rows].push_back(remember_multipliers[rows]);
							}
						// Add a new row (new monomial).
						int number_of_terms_found=1;
						if(coefficient_matrix.size()>0) 
							number_of_terms_found=coefficient_matrix[0].size();
						coefficient_matrix.push_back(std::vector<multiplier_t>(number_of_terms_found,0));
						coefficient_matrix.back().back()=remember_multiplier;
						}
					else {
//						txtout << "yp term found at " << yp << std::endl;
						if(!new_monomial_found) remember_multipliers[yp]=remember_multiplier;
						else                    coefficient_matrix[yp].back()=remember_multiplier;
						}
					++termit;
					}

//				txtout << "** coefficient matrix:" << std::endl;
//				for(unsigned int i=0; i<coefficient_matrix.size(); ++i) {
//					for(unsigned int j=0; j<coefficient_matrix[i].size(); ++j)
//						txtout << coefficient_matrix[i][j] << " ";
//					if(new_monomial_found) txtout << std::endl;
//					else txtout << " = " << remember_multipliers[i] << std::endl;
//					}
//				txtout << "**" << std::endl;
				if(!new_monomial_found) { 
               // Can we decompose this polynomial on the existing basis?
					if(linear::gaussian_elimination(coefficient_matrix, remember_multipliers)) {
//						txtout << "is linear combination of known terms" << std::endl;
						discarded_terms.append_child(discarded_terms.begin(),workexp.begin());
						found=true;
						}
					else {
//						txtout << "new polynomial!" << std::endl;
						for(unsigned int i=0; i<remember_multipliers.size(); ++i)
							coefficient_matrix[i].push_back(remember_multipliers[i]);
						}
					}
				}

			if(!found) {
				tr.append_child(bigsum.begin(),workexp.begin());
				debugout << "found a monomial!:" << std::endl;
				tr.print_recursive_treeform(debugout, bigsum.begin());
				}
			}
	   endofloop:;
		} while(tr.number_of_children(bigsum.begin()) < number_to_find);
// 	txtout << "needed " << loops_needed << " loops to find all contractions" << std::endl
// 			 << "with " << discarded_terms.number_of_children(discarded_terms.begin()) << " discarded monomials" 
// 			 << std::endl;

	it=tr.replace(it, bigsum.begin());
	expression_modified=true;

	cleanup_expression(tr, it);
	return l_applied;
	}



/*
For each row in the adjacency matrix, select at random the 
position of the column which receives a '1'. 
	So if we have 10 indices, make all permutations of
		{1,2,...,10}
with the constraint that the ith number has to be >= i.

How do we take into account the sorting order within tensors?
*/
