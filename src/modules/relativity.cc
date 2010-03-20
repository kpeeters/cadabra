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

#include "relativity.hh"
#include "properties.hh"
#include "algebra.hh"

void relativity::register_properties()
	{
	properties::register_property(&create_property<Metric>);
	properties::register_property(&create_property<InverseMetric>);
	properties::register_property(&create_property<Vielbein>);
	properties::register_property(&create_property<InverseVielbein>);
	properties::register_property(&create_property<WeylTensor>);
	properties::register_property(&create_property<RiemannTensor>);
	}

std::string Vielbein::name() const
	{
	return "Vielbein";
	}

std::string InverseVielbein::name() const
	{
	return "InverseVielbein";
	}

WeylTensor::WeylTensor()
	{
	tab_t tab;
	tab.add_box(0,0);
	tab.add_box(0,2);
	tab.add_box(1,1);
	tab.add_box(1,3);
	tabs.push_back(tab);
	}

std::string WeylTensor::name() const
	{
	return "WeylTensor";
	}

bool WeylTensor::parse(exptree& tr, exptree::iterator pat, exptree::iterator arg, keyval_t&)
	{
	if(tr.number_of_children(pat)!=4) {
		txtout << name() << ": needs exactly 4 indices." << std::endl;
		return false;
		}
	return true;
	}

void WeylTensor::display(std::ostream& str) const
	{
	TableauSymmetry::display(str);
	Traceless::display(str);
	}

Metric::Metric()
	{
	tab_t tab;
	tab.add_box(0,0);
	tab.add_box(0,1);
	tabs.push_back(tab);
	}

std::string Metric::name() const
	{
	return "Metric";
	}

bool Metric::parse(exptree& tr, exptree::iterator pat, exptree::iterator arg, keyval_t& keyvals)
	{
	if(tr.number_of_children(pat)!=2) {
		txtout << name() << ": needs exactly 2 indices." << std::endl;
		return false;
		}

	keyval_t::const_iterator kv=keyvals.find("signature");
	signature=1;
	if(kv!=keyvals.end())
		signature=to_long(*(kv->second->multiplier));

	return true;
	}

InverseMetric::InverseMetric()
	{
	tab_t tab;
	tab.add_box(0,0);
	tab.add_box(0,1);
	tabs.push_back(tab);
	}

std::string InverseMetric::name() const
	{
	return "InverseMetric";
	}

bool InverseMetric::parse(exptree& tr, exptree::iterator pat, exptree::iterator arg, keyval_t&)
	{
	if(tr.number_of_children(pat)!=2) {
		txtout << name() << ": needs exactly 2 indices." << std::endl;
		return false;
		}
	return true;
	}

RiemannTensor::RiemannTensor()
	{
	tab_t tab;
	tab.add_box(0,0);
	tab.add_box(0,2);
	tab.add_box(1,1);
	tab.add_box(1,3);
	tabs.push_back(tab);
	}

std::string RiemannTensor::name() const
	{
	return "RiemannTensor";
	}

bool RiemannTensor::parse(exptree& tr, exptree::iterator pat, exptree::iterator arg, keyval_t&)
	{
	if(tr.number_of_indices(pat)!=4) {
		txtout << name() << ": needs exactly 4 indices." << std::endl;
		return false;
		}
	return true;
	}

eliminate_vielbein::eliminate_vielbein(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void eliminate_vielbein::description() const
	{
	txtout << "Bla" << std::endl;
	}

bool eliminate_vielbein::can_apply(iterator it)
	{
	if(*it->name=="\\prod") return true;
	return false;
	}

algorithm::result_t eliminate_vielbein::apply(iterator& it)
	{
	// Put arguments in canonical form.

	sibling_iterator objs=args_begin();
//	sibling_iterator vielb=objs;
//	++vielb;
	if(*objs->name!="\\comma")
		objs=tr.wrap(objs, str_node("\\comma"));

	index_map_t ind_dummy, ind_free;
	classify_indices(it, ind_free, ind_dummy);

	// Run over all factors, find vielbeine, figure out whether they can
	// be used to turn the indices on the other tensors to preferred type.

	sibling_iterator fit=tr.begin(it);
	while(fit!=tr.end(it)) {
		const Vielbein         *vb=properties::get<Vielbein>(fit);
		const InverseVielbein *ivb=properties::get<InverseVielbein>(fit);
		bool replaced=false;
		if(vb || ivb) {
	      sibling_iterator ind1=tr.begin(fit), ind2=ind1; // the 1st and 2nd index of the vielbein
			++ind2;
			// The contracted index can be either ind1 or ind2. Hence two similar loops:

			// 1st index to 2nd index conversion?
			std::pair<index_map_t::const_iterator,index_map_t::const_iterator> 
				 locs= ind_dummy.equal_range(exptree(ind1));
			while(locs.first!=locs.second) {
				 if(locs.first->second!=(iterator)ind1) {
					  // Does this index sit on an object in the "preferred form" list?
					  iterator par=tr.parent(locs.first->second);
					  sibling_iterator prefit=tr.begin(objs);
					  while(prefit!=tr.end(objs)) {
							if(subtree_equal(prefit, par, -2, false)) {
								 tr.replace_index(locs.first->second, ind2);
								 fit=tr.erase(fit);
								 expression_modified=true;
								 replaced=true;
								 break;
								 }
							++prefit;
							}
					  }
				 if(replaced) break;
				 }
			if(replaced) break;
			
         // 2nd index to 1st index conversion?
			locs=ind_dummy.equal_range(exptree(ind2));
			while(locs.first!=locs.second) {
				 if(locs.first->second!=(iterator)ind2) {
					  // Does this index sit on an object in the "preferred form" list?
					  iterator par=tr.parent(locs.first->second);
					  sibling_iterator prefit=tr.begin(objs);
					  while(prefit!=tr.end(objs)) {
							if(subtree_equal(prefit, par, -2, false)) {
								 tr.replace_index(locs.first->second, ind1);
								 fit=tr.erase(fit);
								 expression_modified=true;
								 replaced=true;
								 break;
								 }
							++prefit;
							}
					  }
				 if(replaced) break;
				 ++locs.first;
				 }
			 }
		if(!replaced)
			 ++fit;
		}
	if(expression_modified) {
		cleanup_sums_products(tr, it);
		return l_applied;
		}
	else return l_no_action;
	}

eliminate_metric::eliminate_metric(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void eliminate_metric::description() const
	{
	txtout << "Eliminate metrics." << std::endl;
	}

bool eliminate_metric::can_apply(iterator it)
	{
	if(*it->name=="\\prod") return true;
	return false;
	}

algorithm::result_t eliminate_metric::apply(iterator& it)
	{
	// Put arguments in canonical form.

	sibling_iterator objs=args_begin();
//	sibling_iterator vielb=objs;
//	++vielb;
	if(objs!=args_end())
		if(*objs->name!="\\comma")
			objs=tr.wrap(objs, str_node("\\comma"));

	index_map_t ind_dummy, ind_free;
	classify_indices(it, ind_free, ind_dummy);

	// Run over all factors, find vielbeine, figure out whether they can
	// be used to turn the indices on the other tensors to preferred type.

	sibling_iterator fit=tr.begin(it);
	while(fit!=tr.end(it)) {
		const Metric        *vb=properties::get<Metric>(fit);
		const InverseMetric *ivb=properties::get<InverseMetric>(fit);

		bool replaced=false;
		if(vb || ivb) {
			sibling_iterator ind1=tr.begin(fit), ind2=ind1;
			++ind2;
			
         // 1st index to 2nd index conversion?
			std::pair<index_map_t::const_iterator,index_map_t::const_iterator> locs=
				ind_dummy.equal_range(exptree(ind1));
			if(std::distance(locs.first, locs.second)==2) {
//				txtout << "index " << *ind1->name << std::endl;
				while(locs.first!=locs.second) {
					if(locs.first->second!=(iterator)ind1) {
						// Does this index sit on an object in the "preferred form" list?
						// (if there is no preferred form, always eliminate)
						if(objs==args_end()) {
							tr.move_ontop(locs.first->second, iterator(ind2))->fl.parent_rel=ind2->fl.parent_rel;;
							fit=tr.erase(fit);
//							exptree::print_recursive_treeform(txtout, it);
							expression_modified=true;
							replaced=true;
							break;
							}
						else {
							iterator par=tr.parent(locs.first->second);
							sibling_iterator prefit=tr.begin(objs);
							while(prefit!=tr.end(objs)) {
								if(subtree_equal(prefit, par, -1, false)) {
									tr.move_ontop(locs.first->second, iterator(ind2))->fl.parent_rel=ind2->fl.parent_rel;;
									fit=tr.erase(fit);
									expression_modified=true;
									replaced=true;
									break;
									}
								++prefit;
								}
							}
						}
					if(replaced) break;
					++locs.first;
					}
				}

			// 2nd index to 1st index conversion?
			locs=ind_dummy.equal_range(exptree(ind2));
			if(std::distance(locs.first, locs.second)==2) {
				while(locs.first!=locs.second) {
					if(!replaced && locs.first->second!=(iterator)ind2) {
						// Does this index sit on an object in the "preferred form" list?
//						txtout << "testing second index" << std::endl;
						// (if there is no preferred form, always eliminate)
						if(objs==args_end()) {
							tr.move_ontop(locs.first->second, iterator(ind1))->fl.parent_rel=ind1->fl.parent_rel;
							fit=tr.erase(fit);
//							exptree::print_recursive_treeform(txtout, it);
							expression_modified=true;
							replaced=true;
							break;
							}
						else {
							iterator par=tr.parent(locs.first->second);
							sibling_iterator prefit=tr.begin(objs);
							while(prefit!=tr.end(objs)) {
//							txtout << "testing " << *prefit->name << " against " << *par->name << std::endl;
								if(subtree_equal(prefit, par, -1, false)) {
//								txtout << "yes" << std::endl;
									tr.move_ontop(locs.first->second, iterator(ind1))->fl.parent_rel=ind1->fl.parent_rel;
									fit=tr.erase(fit);
									expression_modified=true;
									replaced=true;
									break;
									}
								++prefit;
								}
							}
						}
					if(replaced) break;
					++locs.first;
					}
				}
			}
		if(!replaced)
			++fit;
		}
//	exptree::print_recursive_treeform(txtout, it);
	if(expression_modified) {
		cleanup_sums_products(tr, it);
		return l_applied;
		}
	else return l_no_action;
	}


rewrite_indices::rewrite_indices(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	if(number_of_args()!=2) {
		txtout << "Need two arguments." << std::endl;
		throw constructor_error();
		}
	}

void rewrite_indices::description() const
	{
	txtout << "Rewrite indices in preferred form using the indicated vielbein object." << std::endl;
	}

bool rewrite_indices::can_apply(iterator it) 
	{
	if(*it->name=="\\prod" || is_single_term(it))
		return true;

	return false;
	}

algorithm::result_t rewrite_indices::apply(iterator& it) 
	{
	prod_wrap_single_term(it);

	index_map_t ind_free, ind_dummy;
	classify_indices(it, ind_free, ind_dummy);

	// Put arguments in canonical form.

	sibling_iterator objs=args_begin();
	sibling_iterator vielb=objs;
	++vielb;
	if(*objs->name!="\\comma") {
		args_begin_=tr.end(); // force recompute FIXME: this is a hack
		objs=tr.wrap(objs, str_node("\\comma"));
		}

	// Determine which conversion types are possible.
	// itype1 and itype2 are the index types of the 1st and 2nd index of the 
	// converter (i.e. vielbein or metric).
	
	sibling_iterator vbind=tr.begin(vielb);
	const Indices *itype1=properties::get<Indices>(vbind);
	str_node::parent_rel_t pr1=vbind->fl.parent_rel;
	++vbind;
	const Indices *itype2=properties::get<Indices>(vbind);
	str_node::parent_rel_t pr2=vbind->fl.parent_rel;	

	// Since this algorithm works both on dummy indices and on free
	// ones, we merge the two.

	ind_dummy.insert(ind_free.begin(), ind_free.end());

	// Go through all indices, determine on which object they sit,
	// and see if that object appears in the list of preferred-form
	// objects. If so, take appropriate action.

	// 'dit' is the index under consideration for a rewrite.
	index_map_t::const_iterator dit=ind_dummy.begin();
	while(dit!=ind_dummy.end()) {
		sibling_iterator par=tr.parent(dit->second);
//		txtout << "handling " << *dit->second->name << std::endl;
		for(sibling_iterator prefit=tr.begin(objs); prefit!=tr.end(objs); ++prefit) {
//			txtout << "one " << *par->name << ", " << *prefit->name << std::endl;
			if(subtree_equal(par, prefit, 1, false)) {
//				txtout << "found " << *par->name << std::endl;
				// Determine whether the indices are of preferred type or not.
				int num=std::distance(tr.begin(par), (sibling_iterator)dit->second);
				const Indices *origtype=properties::get<Indices>(dit->second);
				if(!origtype) {
					txtout << "Need to know about the index type of index " << *dit->second->name << std::endl;
					return l_error;
					}
//				txtout << "index " << *dit->second->name << "(" << num << ") has type " 
//						 << origtype->set_name << std::endl;

				// 'walk' is the index on the preferred form of the tensor, corresponding
				// to the index on the original tensor which is currently under consideration 
				// for change.
				sibling_iterator walk=tr.begin_index(prefit);
				while(num-- > 0)
					++walk;

				const Indices *newtype=properties::get<Indices>(walk);
				if(!newtype) {
					txtout << "Need to know about the index type of index " << *walk->name << std::endl;
					return l_error;
					}
//				txtout << "prefi " << *walk->name << "(" << num << ") has type " 
//						 << newtype->set_name << std::endl;

				if(newtype->set_name == origtype->set_name) {
//					txtout << "index already has same type" << std::endl;
					if(origtype->position_type==Indices::free || walk->fl.parent_rel==dit->second->fl.parent_rel) {
//						txtout << "and position is also already fine" << std::endl;
						continue; // already fine
						}
//					txtout << "need to raise/lower" << std::endl;
					}

				exptree repvb(vielb);
				sibling_iterator vbi1=repvb.begin(repvb.begin());
				sibling_iterator vbi2=vbi1; ++vbi2;

				if(origtype->set_name == itype1->set_name && newtype->set_name == itype2->set_name) {
//					txtout << "hit 1" << std::endl;
					if( itype1->position_type==Indices::free || dit->second->fl.parent_rel == pr1 ) {
						if( itype2->position_type==Indices::free || walk->fl.parent_rel != pr2 ) {
//							txtout << "activate" << std::endl;
							tr.replace_index(vbi1, dit->second);
							exptree nd=get_dummy(itype2, par);
							tr.replace_index(vbi2, nd.begin());
							tr.replace_index(dit->second, nd.begin())->fl.parent_rel=walk->fl.parent_rel;
							}
						else continue;
						}
					else continue;
					}
				else if(origtype->set_name == itype2->set_name && newtype->set_name == itype1->set_name) {
//					txtout << "hit 2" << std::endl;
					if( itype2->position_type==Indices::free || dit->second->fl.parent_rel == pr2 ) {
						if( itype1->position_type==Indices::free || walk->fl.parent_rel != pr1 ) {
							tr.replace_index(vbi2, dit->second);
							exptree nd=get_dummy(itype1, par);
							tr.replace_index(vbi1,nd.begin());
							tr.replace_index(dit->second,nd.begin())->fl.parent_rel=walk->fl.parent_rel;
							}
						else continue;
						}
					else continue;
					}
				else continue; // next index

				// Insert the conversion object.
				iterator vbit;
				if(*tr.parent(par)->name=="\\sum") { // need to wrap inside a product
					iterator prod=tr.wrap(par, str_node("\\prod"));
					prod->fl.bracket=par->fl.bracket;
					par->fl.bracket=str_node::b_none;
					vbit=tr.append_child(prod, repvb.begin());
					expression_modified=true;
					}
				else {
					assert(*tr.parent(par)->name=="\\prod");
					vbit=tr.append_child((iterator)tr.parent(par), repvb.begin());
					vbit->fl.bracket=par->fl.bracket;
					expression_modified=true;
					}
				}
			}
		++dit;
		}

	prod_unwrap_single_term(it);
	if(expression_modified) return l_applied;
	else return l_no_action;
	}

ricci_identity::ricci_identity(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void ricci_identity::description() const
	{
	txtout << "Remove Riemann or Weyl tensors with three anti-symmetrised indices." << std::endl;
	}

bool ricci_identity::can_apply(iterator st) 
	{
	const RiemannTensor *rt=properties::get<RiemannTensor>(st);
	const WeylTensor    *wt=properties::get<WeylTensor>(st);
	if(rt || wt) {
		if(*(tr.parent(st)->name)=="\\prod")  // FIXME: This only catches explicit 
			                                   // contractions with asym tensors
			return true;
		}
	return false;
	}

algorithm::result_t ricci_identity::apply(iterator& st)
	{
	iterator prod=tr.parent(st);
	sibling_iterator factor=tr.begin(prod);
	while(factor!=tr.end(prod)) {
		const AntiSymmetric  *as=properties::get<AntiSymmetric>(factor);
		const DAntiSymmetric *da=properties::get<DAntiSymmetric>(factor);
		if(as) {
			if(intersection_number(tr.begin(factor), tr.end(factor),
										  tr.begin(st), tr.end(st), 
										  str_node::compare_names_only)>=3) {
				zero(st->multiplier);
				expression_modified=true;
				break;
				}
			}
		else if(da) {
			sibling_iterator fbegin=tr.begin(factor);
			++fbegin;
			if(intersection_number(fbegin, tr.end(factor),
										  tr.begin(st), tr.end(st), 
										  str_node::compare_names_only)>=3) {
				zero(st->multiplier);
				expression_modified=true;
				break;
				}
			}
		++factor;
		}
	return l_applied;
	}


/*

F${antisymmetric}.
R${riemann}.

F^{r1 r2 r3 r4 r5} R_{r2 r3 r6 r7};
@ricci_identity!(%);

F^{r1 r2 r3 r4 r5} R_{r7 r2 r3 r4};
@ricci_identity!(%);

 */

weyl_index_order::weyl_index_order(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void weyl_index_order::description() const
	{
	txtout << "Canonically order the indices on a Weyl tensor, first within pairs, and then the pairs." 
			 << std::endl;
	}

bool weyl_index_order::can_apply(iterator st)
	{
	const RiemannTensor *rt=properties::get<RiemannTensor>(st);
	const WeylTensor    *wt=properties::get<WeylTensor>(st);
	if(rt || wt) return true;
	else         return false;
	}

algorithm::result_t weyl_index_order::apply(iterator& st)
	{
	range_vector_t ranges;
	find_argument_lists(ranges, false);

	iterator ind[4];
	sibling_iterator sit=tr.begin(st);
	for(unsigned int i=0; i<4; ++i) {
		ind[i]=sit;
		++sit;
		}

	int sign=1;
	sibling_iterator tmp;
	// sort within pairs
	if(*ind[1]<*ind[0]) {
		tr.swap(ind[0]);
		tmp=ind[0]; ind[0]=ind[1]; ind[1]=tmp;
		sign=-sign;
		expression_modified=true;
		}
	if(*ind[3]<*ind[2]) {
		tr.swap(ind[2]);
		tmp=ind[2]; ind[2]=ind[3]; ind[3]=tmp;
		sign=-sign;
		expression_modified=true;
		}
	// exchange pairs
	int count1=0, count2=0;
	if(ranges.size()>0) {
		if(find_arg_superset(ranges, ind[0])!=ranges.end()) ++count1;
		if(find_arg_superset(ranges, ind[1])!=ranges.end()) ++count1;
		if(find_arg_superset(ranges, ind[2])!=ranges.end()) ++count2;
		if(find_arg_superset(ranges, ind[3])!=ranges.end()) ++count2;
		}
	if( (*ind[2]<*ind[0] && count2>=count1) || count2>count1 ) {
		if(count2>=count1) {
			tr.swap(ind[1]);
			tr.swap(ind[1]);
			tr.swap(ind[0]);
			tr.swap(ind[0]);
			}
		expression_modified=true;
		}
	if(*(tr.parent(st)->name)=="\\prod")
		multiply(tr.parent(st)->multiplier, sign);
	else
		multiply(st->multiplier, sign);

	return l_applied;
	}

riemann_index_regroup::riemann_index_regroup(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void riemann_index_regroup::description() const
	{
	txtout << "Given sets of anti-symmetric indices, re-order the indices on the Riemann tensor when two (but not three) of such a set occur, one in the first pair, one in the second." << std::endl;
	}

bool riemann_index_regroup::can_apply(iterator st)
	{
	const RiemannTensor *rt=properties::get<RiemannTensor>(st);
	const WeylTensor    *wt=properties::get<WeylTensor>(st);
	if(rt || wt) return true;
	else         return false;
	}


algorithm::result_t riemann_index_regroup::apply(iterator& st)
	{
	sibling_iterator ind[4];
	sibling_iterator sit=tr.begin(st);
	for(unsigned int i=0; i<4; ++i) {
		ind[i]=sit;
		++sit;
		}

	range_vector_t ranges;
	find_argument_lists(ranges);
	if(*(tr.parent(st)->name)=="\\prod") { // search for anti-symmetric tensors
		iterator par=tr.parent(st);
		sibling_iterator sib=tr.begin(par);
		while(sib!=tr.end(par)) {
			if(sib!=st) {
				const AntiSymmetric  *as=properties::get<AntiSymmetric>(sib);
				const DAntiSymmetric *da=properties::get<DAntiSymmetric>(sib);
				const WeylTensor     *wt=properties::get<WeylTensor>(sib);
				const RiemannTensor  *rt=properties::get<RiemannTensor>(sib);
				if(as)
					ranges.push_back(range_t(tr.begin(sib), tr.end(sib)));
				else if(da) {
					sibling_iterator tmp=tr.begin(sib);
					++tmp;
					ranges.push_back(range_t(tmp, tr.end(sib)));
					}
				else if(wt || rt) {
					sibling_iterator one=tr.begin(sib);
					sibling_iterator two=one;
					++two; ++two;
					ranges.push_back(range_t(one, two));
					++one; ++one;
					++two; ++two;
					ranges.push_back(range_t(one, two));
					}
				}
			++sib;
			}
		}

	// a[b|c|d] -> ac bd
	range_vector_t::iterator doublet1=find_arg_superset(ranges, ind[1]);
	range_vector_t::iterator doublet2=find_arg_superset(ranges, ind[3]);
	range_vector_t::iterator singlet =find_arg_superset(ranges, ind[2]);
	nset_t::iterator tmp;

	if(doublet1==doublet2 && doublet1!=ranges.end()) {
		if(doublet1!=singlet) {
			tmp=ind[1]->name;
			ind[1]->name=ind[2]->name;
			ind[2]->name=tmp;
			half(st->multiplier);
			pushup_multiplier(st);
			expression_modified=true;
			return l_applied;
			}
		}

	// a[bc]d   -> ad cb
	doublet1=find_arg_superset(ranges, ind[1]);
	doublet2=find_arg_superset(ranges, ind[2]);
	singlet =find_arg_superset(ranges, ind[3]);

	if(doublet1==doublet2 && doublet1!=ranges.end()) {
		if(doublet1!=singlet) {
			tmp=ind[1]->name;
			ind[1]->name=ind[3]->name;
			ind[3]->name=tmp;
			half(st->multiplier);
			pushup_multiplier(st);
			expression_modified=true;
			return l_applied;
			}
		}

	// [a|b|c]d -> ac bd
	doublet1=find_arg_superset(ranges, ind[0]);
	doublet2=find_arg_superset(ranges, ind[2]);
	singlet =find_arg_superset(ranges, ind[1]);

	if(doublet1==doublet2 && doublet1!=ranges.end()) {
		if(doublet1!=singlet) {
			tmp=ind[1]->name;
			ind[1]->name=ind[2]->name;
			ind[2]->name=tmp;
			half(st->multiplier);
			pushup_multiplier(st);
			expression_modified=true;
			return l_applied;
			}
		}

	// [a|bc|d] ->  ad cb
	doublet1=find_arg_superset(ranges, ind[0]);
	doublet2=find_arg_superset(ranges, ind[3]);
	singlet =find_arg_superset(ranges, ind[1]);

	if(doublet1==doublet2 && doublet1!=ranges.end()) {
		if(doublet1!=singlet) {
			tmp=ind[1]->name;
			ind[1]->name=ind[3]->name;
			ind[3]->name=tmp;
			half(st->multiplier);
			pushup_multiplier(st);
			expression_modified=true;
			return l_applied;
			}
		}
	
	return l_applied;
	}

remove_weyl_traces::remove_weyl_traces(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void remove_weyl_traces::description() const
	{
	txtout << "Remove Weyl tensors which are traced over." << std::endl;
	}

bool remove_weyl_traces::can_apply(iterator st)
	{
	const WeylTensor    *wt=properties::get<WeylTensor>(st);
	if(wt) return true;
	else   return false;
	}

algorithm::result_t remove_weyl_traces::apply(iterator& st)
	{
	sibling_iterator it=tr.begin(st), nd=tr.end(st);
	// FIXME: need a member 'elements_unique' to test for this sort of thing.
	while(it!=nd) {
		sibling_iterator nx(it);
		++nx;
		while(nx!=nd) {
			if(nx->name==it->name) {
				zero(st->multiplier);
				expression_modified=true;
				return l_applied;
				}
			++nx;
			}
		++it;
		}
	return l_applied;
	}

split_index::split_index(exptree& tr, iterator it)
	: algorithm(tr, it), part1_is_number(false), part2_is_number(false)
	{
	if(number_of_args()==1)
		if(*(args_begin()->name)=="\\comma") 
			if(tr.number_of_children(args_begin())==3) {
				iterator trip=args_begin();
				sibling_iterator iname=tr.begin(trip);
				full_class=properties::get<Indices>(iname);
				++iname;
				if(iname->is_integer()) {
					part1_is_number=true;
					num1=to_long(*(iname->multiplier));
					}
				else part1_class=properties::get<Indices>(iname);
				++iname;
				if(iname->is_integer()) {
					part2_is_number=true;
					num2=to_long(*(iname->multiplier));
					}
				else part2_class=properties::get<Indices>(iname);
				if(full_class && (part1_is_number || part1_class) && (part2_is_number || part2_class) )
					return;
				txtout << "The index types of (some of) these indices are not known." << std::endl;
				}
	throw algorithm::constructor_error();
	}

void split_index::description() const
	{
	txtout << "Split a dummy index into two." << std::endl;
	}

bool split_index::can_apply(iterator it)
	{
	// act on a single term in a sum, or on an isolated expression at the top node.
	if(*(it->name)!="\\sum") 
		if(*(tr.parent(it)->name)=="\\sum" || 
			( *(tr.parent(it)->name)=="\\expression" && !(*(it->name)=="\\asymimplicit"))) return true;

	return false;
	}

algorithm::result_t split_index::apply(iterator& it)
	{
	exptree rep;
	rep.set_head(str_node("\\sum"));
	exptree workcopy(it); // so we can make changes without spoiling the big tree
//	assert(*it->multiplier==1); // see if this made a difference

//	txtout << "split index acting at " << *(it->name) << std::endl;

	// we only replace summed indices, so first find them.
	index_map_t ind_free, ind_dummy;
	classify_indices(workcopy.begin(), ind_free, ind_dummy);
//	txtout << "indices classified" << std::endl;

	index_map_t::iterator prs=ind_dummy.begin();
	while(prs!=ind_dummy.end()) {
		const Indices *tcl=properties::get<Indices>((*prs).second);
		if(tcl) {
			if((*tcl).set_name==(*full_class).set_name) {
				exptree dum1,dum2;
				if(!part1_is_number)
					dum1=get_dummy(part1_class, it);
				index_map_t::iterator current=prs;
				while(current!=ind_dummy.end() && tree_exact_equal((*prs).first,(*current).first,true)) {
					if(part1_is_number) {
						node_integer(current->second, num1);
//						(*prs).second->name=name_set.insert(to_string(num1)).first;
						}
					else {
//						txtout << "going to replace" << std::endl;
						(*current).second=tr.replace_index((*current).second, dum1.begin());
//						txtout << "replaced" << std::endl;
						}
					// Important: restoring (*prs).second in the line above.
					++current;
					}
				rep.append_child(rep.begin(), workcopy.begin());
				current=prs;
				if(!part2_is_number) 
					dum2=get_dummy(part2_class, it);
				while(current!=ind_dummy.end() && tree_exact_equal((*prs).first,(*current).first,true)) {
					if(part2_is_number) {
						node_integer(current->second, num2);
//						(*prs).second->name=name_set.insert(to_string(num2)).first;
						}
					else tr.replace_index((*current).second,dum2.begin());
					++current;
					}
				rep.append_child(rep.begin(), workcopy.begin());
				// Do not copy the multiplier; it has already been copied by cloning the original into workcopy.
            //	rep.begin()->multiplier=it->multiplier;
//				txtout << "cleaning up" << std::endl;
//				rep.print_recursive_treeform(txtout, rep.begin());
				it=tr.replace(it, rep.begin());
				cleanup_nests(tr, it);
				expression_modified=true;
				break;
				}
			}
		// skip other occurrances of this index
		index_map_t::iterator current=prs;
		while(prs!=ind_dummy.end() && tree_exact_equal((*prs).first,(*current).first,false))
			++prs;
		}

	if(expression_modified) return l_applied;
	else                    return l_no_action;
	}
