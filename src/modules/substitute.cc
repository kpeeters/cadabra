/* 

   $Id: substitute.cc,v 1.85 2008/06/09 10:41:16 peekas Exp $

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

#include <sstream>
#include "substitute.hh"
#include <pcrecpp.h>
#include "algebra.hh"
#include "dummies.hh"

substitute::substitute(exptree& tr, iterator it)
	: algorithm(tr, it), start_reporting_outside(false), prodsort_(tr, it)
	{
	if(number_of_args()==0) {
		txtout << "substitute: need (list of) replacement rules." << std::endl;
		throw constructor_error();
		}
	sibling_iterator subslist=args_begin();
	while(subslist!=args_end() && subslist->fl.bracket!=str_node::b_round) {
		++subslist;
		}
	if(subslist==args_end()) {
		txtout << "substitute: substitution arguments should be in round brackets." << std::endl;
		throw constructor_error();
		}
	for(unsigned int i=0; i<tr.arg_size(subslist); ++i) {
		iterator arrow=tr.arg(subslist, i);
		iterator lhs, rhs=tr.end();
		if(*arrow->name!="\\arrow" && *arrow->name!="\\equals") {
			lhs=arrow;
			txtout << "substitute: argument " << i+1 << " is neither a replacement rule nor an equality." << std::endl;
			throw constructor_error();
			}
		else {
			lhs=tr.begin(arrow);
			rhs=lhs;
			rhs.skip_children();
			++rhs;
			}
		try {
			if(*lhs->multiplier!=1) {
				txtout << "substitute: no numerical pre-factors allowed on lhs of replacement rule." << std::endl;
				throw constructor_error();
				}
			// test validity of lhs and rhs
			iterator lhsit=lhs, stopit=lhs;
			stopit.skip_children();
			++stopit;
			while(lhsit!=stopit) {
				if(lhsit->is_object_wildcard()) {
					if(tr.number_of_children(lhsit)>0) {
						txtout << "substitute: object wildcards cannot have child nodes." << std::endl;
						throw constructor_error();
						}
					}
				++lhsit;
				}
			lhsit=rhs;
			stopit=rhs;
			stopit.skip_children();
			++stopit;
			while(lhsit!=stopit) {
				if(lhsit->is_object_wildcard()) {
					if(tr.number_of_children(lhsit)>0) {
						txtout << "substitute: object wildcards cannot have child nodes." << std::endl;
						throw constructor_error();
						}
					}
				++lhsit;
				}

			// check whether there are dummies.
			index_map_t ind_free, ind_dummy;
			classify_indices(lhs, ind_free, ind_dummy);
			if(ind_dummy.size()>0)
				lhs_contains_dummies.push_back(true);
			else
				lhs_contains_dummies.push_back(false);
			ind_free.clear(); ind_dummy.clear();
			if(rhs!=tr.end()) {
				classify_indices(rhs, ind_free, ind_dummy);
				if(ind_dummy.size()>0)
					rhs_contains_dummies.push_back(true);
				else
					rhs_contains_dummies.push_back(false);
				}
			}
		catch(std::exception& er) {
			txtout << "substitute: index error in replacement rule " << i+1 << "." << std::endl;
			throw constructor_error();
			}
		}
	}

void substitute::description() const
	{
	txtout << "Substitute one expression into another." << std::endl;
	}


// Find a subproduct in a product. The 'lhs' iterator points to the product which
// we want to find, the 'tofind' iterator to the current factor which we are looking
// for. The product in which to search is pointed to by 'st'.
//
// Once 'tofind' is found, this routine calls itself to find the next factor in
// 'lhs'. If the next factor cannot be found, we backtrack and try to find the
// previous factor again (it may have appeared multiple times).
//
bool substitute::match_subproduct(sibling_iterator lhs, sibling_iterator tofind, sibling_iterator st)
	{
	replacement_map_t         backup_replacements(replacement_map);
	subtree_replacement_map_t backup_subtree_replacements(subtree_replacement_map);

	sibling_iterator start=tr.begin(st);
	while(start!=tr.end(st)) {
		if(std::find(factor_locations.begin(), factor_locations.end(), start)==factor_locations.end()) {  
//			txtout << tofind.node << "number = " << backup_replacements.size() << std::endl;
//			txtout << *tofind->name << " vs " << *start->name << std::endl;
			if(equal_subtree(tofind, start)) { // found factor
				// If a previous factor was found, verify that the factor found now can be
				// moved next to the previous factor (nontrivial if factors do not commute).
				int sign=1;
				if(factor_locations.size()>0) {
					sign=prodsort_.can_move_adjacent(st, factor_locations.back(), start);
					}
				if(sign==0) { // object found, but we cannot move it in the right order
					replacement_map=backup_replacements;
					subtree_replacement_map=backup_subtree_replacements;
					}
				else {
					factor_locations.push_back(start);
					factor_moving_signs.push_back(sign);
					
					sibling_iterator nxt=tofind; 
					++nxt;
					if(nxt!=tr.end(lhs)) {
						bool res=match_subproduct(lhs, nxt, st);
						if(res) return true;
						else {
//						txtout << tofind.node << "found factor useless " << start.node << std::endl;
							factor_locations.pop_back();
							factor_moving_signs.pop_back();
							replacement_map=backup_replacements;
							subtree_replacement_map=backup_subtree_replacements;
							}
						}
					else return true;
					}
				}
			else {
//				txtout << tofind.node << "does not match" << std::endl;
				replacement_map=backup_replacements;
				subtree_replacement_map=backup_subtree_replacements;
				}
			}
		++start;
		}
	return false;
	}

bool substitute::can_apply(iterator st)
	{
	tmr.start();
	sibling_iterator subslist=args_begin();
	for(unsigned int i=0; i<tr.arg_size(subslist); ++i) {
		use_rule=i;
		replacement_map.clear();
		subtree_replacement_map.clear();
		factor_locations.clear();
		factor_moving_signs.clear();

		iterator arrow=tr.arg(subslist, i);
		iterator lhs=tr.begin(arrow);
		if(*lhs->name=="\\conditional") {
			lhs=tr.begin(lhs); 
			conditions=lhs;
			conditions.skip_children();
			++conditions;
			}
		else conditions=tr.end();
		
		if(lhs->name!=st->name && lhs->name->size()>0) 
			continue;

		bool ret=false;
		if(*lhs->name=="\\prod") ret=match_subproduct(lhs, tr.begin(lhs), st);
		else                     ret=equal_subtree(lhs, st);

		if(ret) 
			if(satisfies_conditions())
				return true;
		}
 	return false;
	}

substitute::match_t substitute::compare(exptree::iterator& one, exptree::iterator& two, bool nobrackets) 
	{
	// nobrackets also implies 'no multiplier', i.e. 'toplevel'.

	// one is the substitute pattern, two the expression under consideration
	if(one->fl.bracket==two->fl.bracket || nobrackets)
		if(one->fl.parent_rel==two->fl.parent_rel) {
			bool pattern=false;
			bool objectpattern=false;
			bool implicit_pattern=false;
			bool is_index=false;
			if(one->fl.bracket==str_node::b_none && one->is_index() )
				is_index=true;
			if(one->is_name_wildcard())
				pattern=true;
			else if(one->is_object_wildcard())
				objectpattern=true;
			else if(is_index && one->is_integer()==false) 
				implicit_pattern=true;

			if(pattern || (implicit_pattern && two->is_integer()==false)) { // never match integers to implicit patterns!
				if(lhs_contains_dummies[use_rule]) {
					replacement_map_t::iterator loc=replacement_map.find(one);
					if(loc!=replacement_map.end()) {
						// If this is an index, try to match the whole index.
						// We want to make sure that a pattern k1_a k2_a does not match an expression k1_c k2_d.
						if(is_index) {
							if(subtree_exact_equal((*loc).second.begin(), two, 0)) return subtree_match;
							else                                                   return no_match;
							}
						else {
							if((*loc).second.begin()->name == two->name)        return node_match;
							else                                                return no_match;
							}
						}
					else {
						// check that the index types agree (if known, otherwise assume they match)
						const Indices *t1=properties::get<Indices>(one);
						const Indices *t2=properties::get<Indices>(two);
						if( (t1 || t2) && implicit_pattern ) {
							if(t1 && t2) {
								if((*t1).set_name != (*t2).set_name)
									return no_match;
								}
							else return no_match;
							 }
						replacement_map[one]=two;
						// if this is a pattern and the pattern has a non-zero number of children,
						// also add the pattern without the children
						if(tr.number_of_children(one)!=0) {
							 exptree tmp1(one), tmp2(two);
							 tmp1.erase_children(tmp1.begin());
							 tmp2.erase_children(tmp2.begin());
							 replacement_map[tmp1]=tmp2;
							 }
						}
					}
				else {
					const Indices *t1=properties::get<Indices>(one);
					const Indices *t2=properties::get<Indices>(two);
					if( (t1 || t2) && implicit_pattern ) {
						if(t1 && t2) {
							if((*t1).set_name != (*t2).set_name)
								return no_match;
							}
						else return no_match;
						}
					replacement_map[one]=two;
					// if this is a pattern and the pattern has a non-zero number of children,
					// also add the pattern without the children
					if(tr.number_of_children(one)!=0) {
						 exptree tmp1(one), tmp2(two);
						 tmp1.erase_children(tmp1.begin());
						 tmp2.erase_children(tmp2.begin());
						 replacement_map[tmp1]=tmp2;
						 }
					}
				if(is_index) return subtree_match;
				else         return node_match;
				}
			else if(objectpattern) {
				subtree_replacement_map_t::iterator loc=subtree_replacement_map.find(one->name);
				if(loc!=subtree_replacement_map.end()) {
					if(tr.equal_subtree((*loc).second,two))
						return subtree_match;
					else return no_match;
					}
				else subtree_replacement_map[one->name]=two;

				return subtree_match;
				}
			else { // object is not dummy
				 if(one->is_rational() && two->is_rational() && one->multiplier!=two->multiplier) return no_match;
				 if(one->name==two->name) {
					  if(nobrackets || (one->multiplier == two->multiplier) )
							return node_match;
					  }
				}
			}
	return no_match;
	}

bool substitute::equal_subtree(exptree::iterator i1, exptree::iterator i2)
	{
	sibling_iterator i1end(i1);
	sibling_iterator i2end(i2);
	++i1end;
	++i2end;

	bool first_call=true;
	while(i1!=i1end && i2!=i2end) {
		match_t mm=compare(i1,i2,first_call);
		first_call=false;
		switch(mm) {
			case no_match:
				return false;
			case node_match:
				if(tr.number_of_children(i1)!=tr.number_of_children(i2)) 
					return false;
				break;
			case subtree_match:
				i1.skip_children();
				i2.skip_children();
				break;
			}
		++i1;
		++i2;
		}
	return true;
	}

bool substitute::satisfies_conditions() 
	{
	if(conditions==tr.end()) return true;
	for(unsigned int i=0; i<tr.arg_size(conditions); ++i) {
		iterator cond=tr.arg(conditions, i);
		if(*cond->name=="\\unequals") {
			sibling_iterator lhs=tr.begin(cond);
			sibling_iterator rhs=lhs;
			++rhs;
			// If we have a match, all indices have replacement rules.
			if(tree_exact_equal(replacement_map[exptree(lhs)], replacement_map[exptree(rhs)])) {
//				txtout << *lhs->name  << " = " << *rhs->name << std::endl;
				return false;
				}
			}
		else if(*cond->name=="\\indexpairs") {
			int countpairs=0;
			replacement_map_t::const_iterator it=replacement_map.begin(),it2;
			while(it!=replacement_map.end()) {
				it2=it;
				++it2;
				while(it2!=replacement_map.end()) {
					if(tree_exact_equal(it->second, it2->second)) {
						++countpairs;
						break;
						}
					++it2;
					}
				++it;
				}
//			txtout << countpairs << " pairs" << std::endl;
			if(countpairs!=*(tr.begin(cond)->multiplier))
				return false;
			}
		else if(*cond->name=="\\regex") {
//			txtout << "regex matching..." << std::endl;
			sibling_iterator lhs=tr.begin(cond);
			sibling_iterator rhs=lhs;
			++rhs;
			// If we have a match, all indices have replacement rules.
			std::string pat=(*rhs->name).substr(1,(*rhs->name).size()-2);
//			txtout << "matching " << *comp.replacement_map[lhs->name]
//					 << " with pattern " << pat << std::endl;
			pcrecpp::RE reg(pat);
			if(reg.FullMatch(*(replacement_map[exptree(lhs)].begin()->name))==false)
				return false;
			}
		else if(*cond->name=="\\hasprop") {
			sibling_iterator lhs=tr.begin(cond);
			sibling_iterator rhs=lhs;
			++rhs;
			properties::registered_property_map_t::iterator pit=
				properties::registered_properties.find(*rhs->name);
			if(pit==properties::registered_properties.end()) {
				txtout << "Property \"" << *rhs->name << "\" not registered." << std::endl;
				return false;
				}
			const property_base *aprop=pit->second();
			bool ret=properties::has(aprop, subtree_replacement_map[lhs->name]);
			delete aprop;
			return ret;
			}
		else {
			txtout << "substitute: condition involving " << *cond->name << " not understood." << std::endl;
			}
		}
	return true;
	}

algorithm::result_t substitute::apply(iterator& st)
	{
   sibling_iterator arrow=tr.arg(args_begin(), use_rule);
   iterator lhs=tr.begin(arrow);
   iterator rhs=lhs;
   rhs.skip_children();
   ++rhs;
   if(*lhs->name=="\\conditional")
      lhs=tr.begin(lhs);

	// We construct a new tree which is a copy of the rhs of the replacement rule, 
	// and then replace nodes and subtrees in there based on how the pattern
	// matching went.
   exptree repl(rhs);

	repl.wrap(repl.begin(), str_node("\\expression"));
	// First activate the inert '@(...)' commands present on the rhs.
	// FIXME: this is a hack, it should be much easier to activate inert commands
	// inside algorithm modules.
	bool replacer_found=false;
	iterator rit=repl.begin();
	while(rit!=repl.end()) {
		if(*rit->name=="@@") {
			replacer_found=true;
			eqn replacer(tr, tr.end());
			iterator num=repl.begin(rit);
			replacer.apply(num);
			iterator newrit=rit;
			newrit.skip_children();
			++newrit;
			repl.flatten(rit);
			repl.erase(rit);
			rit=newrit;
			}
		else ++rit;
		}
   index_map_t ind_free, ind_dummy, ind_forced;

	if(rhs_contains_dummies[use_rule])
		classify_indices(repl.begin(), ind_free, ind_dummy);
	
	// Replace all patterns with the objects they matched.  
	// Keep track of all indices which _have_ to stay what they are, in ind_forced.
	// Keep track of insertion points of subtrees.
	iterator it=repl.begin();
	replacement_map_t::iterator loc;
	subtree_replacement_map_t::iterator sloc;
	std::vector<iterator> subtree_insertion_points;
	while(it!=repl.end()) { 
//		debugout << "attempting to find " << *it->name << std::endl;
		bool is_stripped=false;
		loc=replacement_map.find(exptree(it));
		if(loc==replacement_map.end() && it->is_name_wildcard() && tr.number_of_children(it)!=0) {
			 exptree tmp(it);
			 tmp.erase_children(tmp.begin());
			 loc=replacement_map.find(tmp);
			 is_stripped=true;
			 }

		if(loc!=replacement_map.end()) { // name wildcards
//			debugout << "rule : " << *((*loc).first.begin()->name) << " -> " << std::endl;
//			debugout << "going to replace " << *it->name << " with " << *((*loc).second.begin()->name) << std::endl;

			// When a replacement is made here, and the index is actually
			// a dummy in the replacement, we screw up the ind_dummy
			// map. Then, at the next step, when conflicting dummies are
			// relabelled, things go wrong.  Solution: in this case, the
			// index under consideration should be taken out of ind_dummy.
			// This is easy, because we can just throw out all indices
			// with the original name.

			ind_dummy.erase(exptree(it));

			str_node::bracket_t remember_br=it->fl.bracket;
			if(is_stripped)
				 it->name=(*loc).second.begin()->name;
			else
				 it=tr.replace_index(it, (*loc).second.begin());
			it->fl.bracket=remember_br;
			if(rhs_contains_dummies[use_rule])
				ind_forced.insert(index_map_t::value_type(exptree(it), it));
			++it;

			}
		else if( (sloc=subtree_replacement_map.find(it->name))!=subtree_replacement_map.end()) { // object wildcards
			multiplier_t tmpmult=*it->multiplier; // remember target multiplier
			iterator tmp= tr.insert_subtree(it, (*sloc).second);
			tmp->fl.bracket=it->fl.bracket;
			it=tr.erase(it);
			multiply(tmp->multiplier, tmpmult);
			subtree_insertion_points.push_back(tmp);
			index_map_t ind_subtree_free, ind_subtree_dummy;
			// FIXME: as in the name wildcard case above, we only need these
			// next three lines if there are wildcards in the rhs.
			classify_indices(tmp, ind_subtree_free, ind_subtree_dummy);
			ind_forced.insert(ind_subtree_free.begin(), ind_subtree_free.end());
			ind_forced.insert(ind_subtree_dummy.begin(), ind_subtree_dummy.end());
			}
		else ++it;
		}

	// If the replacement contains dummies, avoid clashes introduced when
	// free indices in the replacement (induced from the original expression)
   // take values already used for the dummies.
	// 
	// Note: the dummies which clash with other factors in a product are
	// not replaced here, but rather in the next step.
	if(ind_dummy.size()>0) {
		index_map_t must_be_empty;
		determine_intersection(ind_forced, ind_dummy, must_be_empty);
		index_map_t::iterator indit=must_be_empty.begin();
		index_map_t added_dummies;
//		txtout << must_be_empty.size() << " dummies have to be relabelled" << std::endl;
		while(indit!=must_be_empty.end()) {
			exptree the_key=indit->first;
			const Indices *dums=properties::get<Indices>(indit->second);
			if(dums==0)
				throw consistency_error("Need to know an index set for " + *indit->second->name +".");
			exptree relabel=get_dummy(dums, &ind_dummy, &ind_forced, &added_dummies);
			added_dummies.insert(index_map_t::value_type(relabel,(*indit).second));
			do {
//				txtout << "replace index " << *(indit->second->name) << " with " << *(relabel.begin()->name) << std::endl;
				tr.replace_index(indit->second,relabel.begin());
				++indit;
//				txtout << *(indit->first.begin()->name) << " vs " << *(the_key.begin()->name) << std::endl;
				} while(indit!=must_be_empty.end() && tree_exact_equal(indit->first,the_key,-1));
			}
		}

	// Remove the wrapping "\expression" node, not needed anymore.
	repl.flatten(repl.begin());
	repl.erase(repl.begin());

	repl.begin()->fl.bracket=st->fl.bracket;
	bool rename_replacement_dummies_called=false;
	// Now we do the actual replacement, putting the "repl" in the tree.
	// If the to-be-replaced object sits in a product, we have to relabel all
	// dummy indices in the replacement which clash with indices in other factors
	// in the product.
	if(*lhs->name=="\\prod") {
		for(unsigned int i=1; i<factor_locations.size(); ++i)
			tr.erase(factor_locations[i]);
//		if(*rhs->name=="\\prod") {
			iterator newtr=tr.move_ontop(iterator(factor_locations[0]),repl.begin()); // no need to keep repl
			multiply(st->multiplier, *newtr->multiplier);
			one(newtr->multiplier);
			pushup_multiplier(st);
			if(ind_dummy.size()>0) {
				rename_replacement_dummies(newtr); // do NOW, otherwise the replacement cannot be isolated anymore
				rename_replacement_dummies_called=true;
				}
			if(*rhs->name=="\\prod") {
				 tr.flatten(newtr);
				 tr.erase(newtr);
				 }
//			}
//		else {
//			if(*st->name=="\\prod") {
//				multiply(st->multiplier, *(repl.begin()->multiplier));
//				one(repl.begin()->multiplier);
//				}
//			else {
//				assert(factor_locations[0]==st);
//				multiply(repl.begin()->multiplier, *st->multiplier);
//				}
//			
//			tr.move_ontop(iterator(factor_locations[0]),repl.begin()); // no need to keep repl
//			}

		if(tr.number_of_children(st)==1) {
			multiply(tr.begin(st)->multiplier, *st->multiplier);
			tr.flatten(st);
			st=tr.erase(st);
			}
		}
	else {
		multiply(repl.begin()->multiplier, *st->multiplier);
		st=tr.move_ontop(st, repl.begin()); // no need to keep the original repl tree
		pushup_multiplier(st);
		}
	if(ind_dummy.size()>0 && !rename_replacement_dummies_called) {
		rename_replacement_dummies(st);
		}
	expression_modified=true;

	// The replacement is done now.  What is left is to take into
	// account any signs caused by moving factors through each other.
	int totsign=1;
	for(unsigned int i=0; i<factor_moving_signs.size(); ++i)
		totsign*=factor_moving_signs[i];
	multiply(st->multiplier, totsign);

	// Get rid of numerical '1' factors inside products (this will not clean up
	// '1's from a 'q -> 1' type replacement, since in this case 'st' points to the 'q'
   // node and we are not allowed to touch the tree above the entry point; these
	// things are taken care of by the algorithm class itself).
	if(*st->name=="\\prod") {
		prodcollectnum pc(tr, tr.end());
		pc.apply(st);
		}

	// Cleanup nests on all insertion points and on the top node.
	for(unsigned int i=0; i<subtree_insertion_points.size(); ++i) {
		iterator ip=subtree_insertion_points[i];
		cleanup_nests(tr, ip);
		}
	cleanup_nests(tr, st);
//	prodcollectnum pc(tr, tr.end());
//	pc.apply(st);
	tmr.stop();
//	if(replacer_found) {
//		txtout << "replacement took " << tmr << std::endl;
//		start_reporting_outside=true;
//		}
	tmr.reset();
	return l_applied;
	}


vary::vary(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void vary::description() const
	{
	txtout << "Vary one-by-one each factor in a product, according to the substitution rules." << std::endl;
	}

bool vary::can_apply(iterator it) 
	{
	if(*it->name=="\\prod") return true;
	return false;
	}

algorithm::result_t vary::apply(iterator& it)
	{
	exptree result;
	result.set_head(str_node("\\expression"));
	iterator newsum=result.append_child(result.begin(), str_node("\\sum"));

	// Iterate over all factors, attempting a substitute. If this
	// succeeds, copy the term to the "result" tree. Then restore the
	// original. We have to do the substitute on the original tree so
	// that index relabelling takes into account the rest of the tree.

	exptree prodcopy(it); // keep a copy to restore after each substitute

	substitute subs(tr, this_command);
	int pos=0;
	for(;;) { 
		sibling_iterator fcit=tr.begin(it);
		fcit+=pos;
		if(fcit==tr.end(it)) break;

		iterator fcit2(fcit);
		if(subs.apply_recursive(fcit2, false)) {
			expression_modified=true;
			result.append_child(newsum, it);
			// restore original
			it=tr.replace(it, prodcopy.begin());
			}
		++pos;
		}
	if(expression_modified) {
		it=tr.move_ontop(it, newsum);
		cleanup_nests(tr, it);
		cleanup_expression(tr, it);
		}
	else { // substitute didn't act anywhere, variation is zero
		zero(it->multiplier);
		expression_modified=true;
		}
	return l_applied;
	}



take_match::take_match(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void take_match::description() const
	{
	txtout << "Select terms or elements in a list which match the pattern." << std::endl;
	}

bool take_match::can_apply(iterator it) 
	{
	if(*it->name=="\\sum" || *it->name=="\\comma") return true;
	return false;
	}

algorithm::result_t take_match::apply(iterator& it)
	{
	tr.wrap(args_begin(), str_node("\\arrow", str_node::b_round));
	substitute subs(tr, this_command);

	sibling_iterator sib=tr.begin(it);
//	int i=0;
	while(sib!=tr.end(it)) {
		if(subs.can_apply(sib)==false) {
			sib=tr.erase(sib);
			expression_modified=true;
			}
		else {
			++sib;
			}
		}
	if(expression_modified)
		cleanup_sums_products(tr, it);

	if(expression_modified) return l_applied;
	else return l_no_action;
	}


replace_match::replace_match(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	if(*args_begin()->name!="\\arrow") 
		throw constructor_error();
	}

void replace_match::description() const
	{
	txtout << "Replace terms or elements in a list which match the pattern." << std::endl;
	}

bool replace_match::can_apply(iterator it) 
	{
	if(*it->name=="\\sum" || *it->name=="\\comma") return true;
	return false;
	}

algorithm::result_t replace_match::apply(iterator& it)
	{
//	tr.wrap(args_begin(), str_node("\\arrow", str_node::b_round));
	substitute subs(tr, this_command);

	sibling_iterator sib=tr.begin(it);
//	int i=0;
	bool replaced=false;
	while(sib!=tr.end(it)) {
		if(subs.can_apply(sib)) {
			sib=tr.erase(sib);
			if(!replaced) {
				replaced=true;
				iterator lhs=tr.begin(args_begin());
				iterator rhs=lhs;
				rhs.skip_children();
				++rhs;
				
				tr.insert_subtree(sib, rhs);
				expression_modified=true;
				}
			}
		else ++sib;
		}
	if(expression_modified)
		cleanup_sums_products(tr, it);

	if(expression_modified) return l_applied;
	else return l_no_action;
	}



simple_rename::simple_rename(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	if(number_of_args()==2) {
		from_=args_begin();
		to_=from_;
		++to_;
		if((*from_->name).size()<2 || (*to_->name).size()<2 ) {
			txtout << "Need quoted names." << std::endl;
			throw constructor_error();
			}
		}
	}

void simple_rename::description() const
	{
	txtout << "Simple renaming of symbols or sets of symbols." << std::endl;
	}

bool simple_rename::can_apply(iterator st)
	{
	return true;
	}

// void simple_rename::rename_existing_dummies(iterator& st, nset_t::iterator to_name) const
// 	{
// 	index_map_t ind_free, ind_dummy, ind_freedown, ind_dummydown, new_dummy;
// 	classify_indices_up(tr.parent(st),ind_free,ind_dummy);
// 	classify_indices(tr.parent(st),ind_freedown,ind_dummydown);
// 	determine_intersection(ind_freedown, ind_free, new_dummy, true);
// 	ind_dummy.insert(new_dummy.begin(), new_dummy.end());
// 	ind_dummy.insert(ind_dummydown.begin(), ind_dummydown.end());
// 
// 	index_map_t::iterator imit=ind_dummy.find(to_name);
// 	if(imit!=ind_dummy.end()) {
// 		const Indices *dums=properties::get<Indices>(to_name);
// 		assert(dums);
// 		nset_t::iterator relabel=get_dummy(dums, &ind_dummy, &ind_free, &ind_freedown);
// 		do {
// 			imit->second->name=relabel;
// 			++imit;
// 			} while(imit->first==to_name);
// 		}
// 	}

algorithm::result_t simple_rename::apply(iterator& st)
	{
	std::string fromstr=*from_->name, tostr=*to_->name;
	fromstr=fromstr.substr(1,fromstr.size()-2);
	tostr=tostr.substr(1,tostr.size()-2);

	if(*st->name==fromstr) {
//		rename_existing_dummies(st, to_);
		st->name=name_set.insert(tostr).first;
		expression_modified=true;
		}
//	else { // rename numbered objects, e.g. arguments {m}{n} lead to m1 -> n1 .
//		if(st->name->size()>from_->name->size())
//			if((*st->name).substr(0,from_->name->size())==*from_->name) {
//				unsigned int i=from_->name->size();
//				while(i<st->name->size()) {
//					if(!isdigit((*st->name)[i]))
//						return l_applied;
//					++i;
//					}
//				std::string nm=*to_->name + st->name->substr(from_->name->size());
//				nset_t::iterator to_it=name_set.insert(nm).first;
//				rename_existing_dummies(st, to_it);
//				st->name=to_it;
//				expression_modified=true;
//				}
//		}
	return l_applied;
	}

index_rename::index_rename(exptree& tr, iterator it)
	: algorithm(tr, it), relabel_numbered_indices(false)
	{
	if(number_of_args()!=2 && number_of_args()!=3) 
		throw constructor_error();

	from_=args_begin();
	to_=from_;
	++to_;
	if(number_of_args()==3)
		relabel_numbered_indices=true;
	}

void index_rename::description() const
	{
	txtout << "Rename indices and relabel dummies" << std::endl;
	}

bool index_rename::can_apply(iterator it)
	{
	// act on a single term in a sum, or on an isolated expression at the top node.
	if(*(it->name)!="\\sum") 
		if(*(tr.parent(it)->name)=="\\sum" || 
			( *(tr.parent(it)->name)=="\\expression" && !(*(it->name)=="\\asymimplicit"))) return true;

	return false;
	}

algorithm::result_t index_rename::apply(iterator& it)
	{
	// Determine all indices in this factor.
	index_map_t ind_free, ind_dummy, ind_freedown, ind_dummydown, new_dummy;
	classify_indices_up(tr.parent(it),ind_free,ind_dummy);
	classify_indices(tr.parent(it),ind_freedown,ind_dummydown);
	determine_intersection(ind_freedown, ind_free, new_dummy, true);
	ind_dummy.insert(new_dummy.begin(), new_dummy.end());
	ind_dummy.insert(ind_dummydown.begin(), ind_dummydown.end());
	ind_free.insert(ind_freedown.begin(), ind_freedown.end());

	// Go through all free indices and determine the ones we want to rename.
	index_map_t to_rename; // Contains the _new_ names, not the old ones.
	if(relabel_numbered_indices) {
		index_map_t::iterator fnd=ind_free.begin();
		while(fnd!=ind_free.end()) {
			if(fnd->first.begin()->name->substr(0,from_->name->size())==*from_->name) {
				unsigned int i=from_->name->size();
				while(i<fnd->first.begin()->name->size()) {
					if(!isdigit((*fnd->first.begin()->name)[i]))
						return l_applied;
					++i;
					}
				std::string nm=*to_->name + fnd->first.begin()->name->substr(from_->name->size());
				nset_t::iterator to_it=name_set.insert(nm).first;
				to_rename.insert(std::make_pair(to_it, fnd->second));
				}
			++fnd;
			}
		}
	else {
		std::pair<index_map_t::iterator, index_map_t::iterator> eq=ind_free.equal_range(exptree(from_));
		index_map_t::iterator fnd=eq.first;
		while(fnd!=eq.second) {
			to_rename.insert(std::make_pair(to_->name, fnd->second));
			++fnd;
			}
		}

	// Rename all dummy pairs which clash with the renaming which is about to take place.
	index_map_t::iterator toren=to_rename.begin();
	index_map_t dummies_added;
	while(toren!=to_rename.end()) {
		std::pair<index_map_t::iterator, index_map_t::iterator> eq=ind_dummy.equal_range(toren->first);
		index_map_t::iterator dren=eq.first;
		while(dren!=eq.second) {
			const Indices *dums=properties::get<Indices>(dren->first.begin());
			if(dums==0)
				throw consistency_error("Need to know an index set for " + *dren->first.begin()->name +".");
			exptree relabel=get_dummy(dums, &ind_dummy, &ind_free, &dummies_added);
			dummies_added.insert(std::make_pair(relabel, tr.end()));
			do {
				tr.replace_index(dren->second,relabel.begin());
				++dren;
				} while(tree_exact_equal(dren->first,toren->first,true) && dren!=eq.second);
			}

		// Skip to the next index name.
		exptree justdone=toren->first;
		do {
			++toren;
			} while(toren!=to_rename.end() && tree_exact_equal(toren->first,justdone,true));
		}

   // Now do the actual rename.
	index_map_t::iterator doit=to_rename.begin();
	while(doit!=to_rename.end()) {
		tr.replace_index(doit->second,doit->first.begin());
		++doit;
		}
	
	return l_applied;
	}
