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

#include <sstream>
#include "substitute.hh"
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
			txtout << er.what() << std::endl;
			throw constructor_error();
			}
		}
	}

void substitute::description() const
	{
	txtout << "Substitute one expression into another." << std::endl;
	}


bool substitute::can_apply(iterator st)
	{
	if(*st->name=="\\expression" || *st->name=="\\asymimplicit") return false;

	tmr.start();
	sibling_iterator subslist=args_begin();
	for(unsigned int i=0; i<tr.arg_size(subslist); ++i) {
		use_rule=i;

		comparator.clear();

		iterator arrow=tr.arg(subslist, i);
		iterator lhs=tr.begin(arrow);
		if(*lhs->name=="\\conditional") {
			lhs=tr.begin(lhs); 
			conditions=lhs;
			conditions.skip_children();
			++conditions;
			}
		else conditions=tr.end();
		
//		std::cerr << *lhs->name << " - " << *st->name << std::endl;

		if(lhs->name!=st->name 
			&& !lhs->is_object_wildcard() && !lhs->is_name_wildcard() && lhs->name->size()>0) 
			continue;

		exptree_comparator::match_t ret;
		comparator.lhs_contains_dummies=lhs_contains_dummies[i];
		if(*lhs->name=="\\prod") ret=comparator.match_subproduct(lhs, tr.begin(lhs), st);
		else                     ret=comparator.equal_subtree(lhs, st);

		if(ret == exptree_comparator::subtree_match) {
			if(conditions==tr.end()) return true;
			std::string error;
			if(comparator.satisfies_conditions(conditions, error))
				return true;
			else txtout << error;
			}
		}
 	return false;
	}

algorithm::result_t substitute::apply(iterator& st)
	{
//	prod_wrap_single_term(st);

   sibling_iterator arrow=tr.arg(args_begin(), use_rule);
   iterator lhs=tr.begin(arrow);
   iterator rhs=lhs;
   rhs.skip_children();
   ++rhs;
   if(*lhs->name=="\\conditional")
      lhs=tr.begin(lhs);

	// We construct a new tree 'repl' which is a copy of the rhs of the
	// replacement rule, and then replace nodes and subtrees in there
	// based on how the pattern matching went.
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
	
	// Replace all patterns on the rhs of the rule with the objects they matched.  
	// Keep track of all indices which _have_ to stay what they are, in ind_forced.
	// Keep track of insertion points of subtrees.
	iterator it=repl.begin();
	exptree_comparator::replacement_map_t::iterator loc;
	exptree_comparator::subtree_replacement_map_t::iterator sloc;
	std::vector<iterator> subtree_insertion_points;
	while(it!=repl.end()) { 
		bool is_stripped=false;
//		tr.print_recursive_treeform(std::cerr, repl.begin());

//		For some reason 'a?' is not found!?! Well, that's presumably because _{a?} does not
//      match ^{a?}. (though this does match when we write 'i' instead of a?. 

		loc=comparator.replacement_map.find(exptree(it));
		if(loc==comparator.replacement_map.end() && it->is_name_wildcard() && tr.number_of_children(it)!=0) {
			 exptree tmp(it);
			 tmp.erase_children(tmp.begin());
			 loc=comparator.replacement_map.find(tmp);
			 is_stripped=true;
			 }

		if(loc!=comparator.replacement_map.end()) { // name wildcards
//			if((*loc).first.begin()->fl.parent_rel==str_node::p_sub)
//				std::cerr << "_";
//			std::cerr << "rule : " << *((*loc).first.begin()->name) << " -> " 
//						 << *((*loc).second.begin()->name) << std::endl;
//			std::cerr << it->fl.parent_rel << " ";
//			std::cerr << "going to replace " << *it->name << " with " << *((*loc).second.begin()->name) << std::endl;

			// When a replacement is made here, and the index is actually
			// a dummy in the replacement, we screw up the ind_dummy
			// map. Then, at the next step, when conflicting dummies are
			// relabelled, things go wrong.  Solution: in this case, the
			// index under consideration should be taken out of ind_dummy.
			// This is easy, because we can just throw out all indices
			// with the original name.

			ind_dummy.erase(exptree(it));

			str_node::bracket_t remember_br=it->fl.bracket;
			if(is_stripped || (it->is_name_wildcard() && !it->is_index()) ) { 
            // a?_{i j k} type patterns should only replace the head
				// TODO: should we replace brackets here too?
				it->name=(*loc).second.begin()->name;
				it->multiplier=(*loc).second.begin()->multiplier;
				it->fl=(*loc).second.begin()->fl;
				}
			else {
				// Careful with the multiplier: the object has been matched to the pattern
				// without taking into account the top-level multiplier. So keep the multiplier
				// of the thing we are replacing.
				multiplier_t mt=*it->multiplier;
				it=tr.replace_index(it, (*loc).second.begin());
				multiply(it->multiplier, mt);
				}
			it->fl.bracket=remember_br;
			if(rhs_contains_dummies[use_rule])
				ind_forced.insert(index_map_t::value_type(exptree(it), it));
			++it;

			}
		else if( (sloc=comparator.subtree_replacement_map.find(it->name)) 
					!=comparator.subtree_replacement_map.end()) { // object wildcards
//			txtout << "srule : " << *it->name << std::endl;
			multiplier_t tmpmult=*it->multiplier; // remember target multiplier
			iterator tmp= tr.insert_subtree(it, (*sloc).second);
			tmp->fl.bracket=it->fl.bracket;
			tmp->fl.parent_rel=it->fl.parent_rel; // ok?
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
//	tr.print_recursive_treeform(std::cerr, repl.begin());

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
			const Indices *dums=properties::get<Indices>(indit->second, true);
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
		for(unsigned int i=1; i<comparator.factor_locations.size(); ++i)
			tr.erase(comparator.factor_locations[i]);
		
		// no need to keep repl
		iterator newtr=tr.move_ontop(iterator(comparator.factor_locations[0]),repl.begin()); 
		multiply(st->multiplier, *newtr->multiplier);
		one(newtr->multiplier);
		if(ind_dummy.size()>0) {
			rename_replacement_dummies(newtr); // do NOW, otherwise the replacement cannot be isolated anymore
			rename_replacement_dummies_called=true;
			}
		if(*rhs->name=="\\prod") {
			tr.flatten(newtr);
			tr.erase(newtr);
			}
		if(tr.number_of_children(st)==1) {
			multiply(tr.begin(st)->multiplier, *st->multiplier);
			tr.flatten(st);
			st=tr.erase(st);
			}
		}
	else {
		multiply(repl.begin()->multiplier, *st->multiplier);
		st=tr.move_ontop(st, repl.begin()); // no need to keep the original repl tree
		}

	if(ind_dummy.size()>0 && !rename_replacement_dummies_called) 
		rename_replacement_dummies(st);

	expression_modified=true;

	// The replacement is done now.  What is left is to take into
	// account any signs caused by moving factors through each other.
	int totsign=1;
	for(unsigned int i=0; i<comparator.factor_moving_signs.size(); ++i)
		totsign*=comparator.factor_moving_signs[i];
	multiply(st->multiplier, totsign);

	// Get rid of numerical '1' factors inside products (this will not clean up
	// '1's from a 'q -> 1' type replacement, since in this case 'st' points to the 'q'
   // node and we are not allowed to touch the tree above the entry point; these
	// things are taken care of by the algorithm class itself).
	if(*st->name=="\\prod") {
//		 debugout << "calling prodcollectnum" << std::endl;
//		 exptree::print_recursive_treeform(debugout, st);
		prodcollectnum pc(tr, tr.end());
		pc.apply(st);
//		 exptree::print_recursive_treeform(debugout, st);
		}

//	tr.print_recursive_treeform(txtout, tr.begin());
//	txtout << "-----" << std::endl;

	// Cleanup nests on all insertion points and on the top node.
	for(unsigned int i=0; i<subtree_insertion_points.size(); ++i) {
		iterator ip=subtree_insertion_points[i];
		if(*ip->name=="\\sum") { // FIXME: is also in algorithm.cc, and should be factored out
			if(*ip->multiplier!=1) {
				sibling_iterator sib=tr.begin(ip);
				while(sib!=tr.end(ip)) {
					multiply(sib->multiplier, *ip->multiplier);
					++sib;
					}
				::one(ip->multiplier);
				}
			}
		cleanup_nests(tr, ip);
		}

//	tr.print_recursive_treeform(txtout, st);
	
//	prod_unwrap_single_term(st);

	cleanup_nests(tr, st);

//	tr.print_recursive_treeform(txtout, tr.begin());
//	prodcollectnum pc(tr, tr.end());
//	pc.apply(st);
	tmr.stop();
//	if(replacer_found) {
//		txtout << "replacement took " << tmr << std::endl;
//		start_reporting_outside=true;
//		}
//	debugout << "leaving with st=" << *st->name << std::endl;
//	tr.print_recursive_treeform(txtout, tr.begin());
//	txtout << "======" << std::endl;

	tmr.reset();
	return l_applied;
	}


/*  bug: cadabra-34.

    Vary should take into account the depth of an object in a more clever way than
	 is currently done. Consider an expression

         A \partial{ A C + B } + D A;

    and A->a, B->b etc. This should vary to

         a  \partial{ A C + B } + A \partial{ a C + A c + b } + d A + a D;

    Right now it produces a total mess for the partial derivative, because it does
	 not understand that the depth counting for factors inside the partial involves
	 knowing about the top-level product.

    So what we should do is introduce a 'factor depth' and a 'sum index', which equal

         A \partial{ A C + B } + D A;
         1           1 1   1     1 1
			1           1 1   1     2 2

    For all 
 */

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
	if(*it->name=="\\sum") return true;
	if(*it->name=="\\pow") return true;
	if(is_single_term(it)) return true;
	if(is_nonprod_factor_in_prod(it)) return true;
	const Derivative *der = properties::get<Derivative>(it);
	if(der) return true;
	der = properties::get<Derivative>(tr.parent(it));
	if(der) return true;
	const Accent *acc = properties::get<Accent>(it);
	if(acc) return true;
	acc = properties::get<Accent>(tr.parent(it));
	if(acc) return true;
	return false;
	}

algorithm::result_t vary::apply(iterator& it)
	{
	const Derivative *der = properties::get<Derivative>(it);
	const Accent     *acc = properties::get<Accent>(it);
	if(der || acc) {
		vary vry(tr, this_command);

		sibling_iterator sib=tr.begin(it);
		while(sib!=tr.end(it)) {
			iterator app=sib;
			++sib;
			if(app->is_index()) continue;
			if(vry.can_apply(app)) {
				if(vry.apply(app)==l_applied)
					expression_modified=true;
				}// not complete: should remove object when zero
			}
		if(expression_modified) return l_applied;
		else                    return l_no_action;
		}

	if(*it->name=="\\prod") {
		exptree result;
		result.set_head(str_node("\\expression"));
		iterator newsum=result.append_child(result.begin(), str_node("\\sum"));
		
		// Iterate over all factors, attempting a substitute. If this
		// succeeds, copy the term to the "result" tree. Then restore the
		// original. We have to do the substitute on the original tree so
		// that index relabelling takes into account the rest of the tree.
		
		exptree prodcopy(it); // keep a copy to restore after each substitute
		
		vary subs(tr, this_command);
		int pos=0;
		for(;;) { 
			sibling_iterator fcit=tr.begin(it);
			fcit+=pos;
			if(fcit==tr.end(it)) break;
			
			iterator fcit2(fcit);
			if(subs.can_apply(fcit2)) {
//				txtout << "in " << *fcit2->name << std::endl;
				algorithm::result_t res = subs.apply(fcit2);
				
				if(fcit2->is_zero()==false && res==algorithm::l_applied) {
					expression_modified=true;
//					txtout << "new term\n";
					iterator newterm=result.append_child(newsum, it);
					}

				// restore original
				it=tr.replace(it, prodcopy.begin());
				}
			++pos;
			}
		if(expression_modified && tr.number_of_children(newsum)>0) {
//			tr.print_recursive_treeform(txtout, newsum.begin());
			it=tr.move_ontop(it, newsum);
			cleanup_nests(tr, it);
			cleanup_expression(tr, it);
			}
		else { // varying any of the factors produces nothing, variation is zero
			zero(it->multiplier);
			expression_modified=true;
			}
		return l_applied;
		}

	if(*it->name=="\\sum") { // call vary on every term
		vary vry(tr, this_command);

		sibling_iterator sib=tr.begin(it);
		while(sib!=tr.end(it)) {
			iterator app=sib;
			++sib;
			if(vry.can_apply(app)) {
				vry.apply(app);
				if(app->is_zero()) {
					tr.erase(app);
					}
				}
			else {
				// remove this term
				tr.erase(app);
				}
			}
		expression_modified=true;
		return l_applied;
		}

	if(*it->name=="\\pow") { 
		// Wrap the power in a \cdb_Derivative and then call @prodrule.
		it=tr.wrap(it, str_node("\\cdb_Derivative"));
		prodrule pr(tr, it);
		pr.can_apply(it);
		pr.apply(it);
		// Find the '\cdb_Derivative node again'.
		sibling_iterator sib=tr.begin(it);
		while(sib!=tr.end()) {
			if(*sib->name=="\\cdb_Derivative") {
				tr.flatten(sib);
				sib=tr.erase(sib);
				vary vry(tr, this_command);
				iterator app=sib;
				if(vry.can_apply(app)) {
					vry.apply(app);
					expression_modified=true;
					}
				break;
				}
			++sib;
			}
//		tr.print_recursive_treeform(txtout, it);
		}
	
	der = properties::get<Derivative>(tr.parent(it));
	acc = properties::get<Accent>(tr.parent(it));

	if(der || acc || is_single_term(it)) { // easy: just vary this term by substitution
		substitute subs(tr, this_command);
//		txtout << "substituting single factor " << *it->name << std::endl;
		if(subs.can_apply(it)) {
			if(subs.apply(it)==l_applied) {
				expression_modified=true;
				return l_applied;
				}
			}
//		txtout << "nothing" << std::endl;
		return l_no_action;
		}
	
	if(is_nonprod_factor_in_prod(it)) {
		substitute subs(tr, this_command);
		if(subs.can_apply(it)) {
			if(subs.apply(it)==l_applied) {
				expression_modified=true;
				return l_applied;
				}
			}
		// else set to zero
		zero(it->multiplier);
		expression_modified=true;
		return l_applied;
		}

	return l_no_action;
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
			const Indices *dums=properties::get<Indices>(dren->first.begin(), true);
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
