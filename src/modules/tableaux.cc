/* 

   $Id: tableaux.cc,v 1.39 2007/10/02 22:01:36 peekas Exp $

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

#include "tableaux.hh"
#include "numerical.hh"

void tableaux::register_properties()
	{
	properties::register_property(&create_property<Tableau>);
	properties::register_property(&create_property<FilledTableau>);
	}

std::string Tableau::name() const
	{
	return "Tableau";
	}

bool Tableau::parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals)
	{
	keyval_t::const_iterator kv=keyvals.find("dimension");
	if(kv!=keyvals.end()) dimension=to_long(*(kv->second->multiplier));
	else dimension=-1;
	return true;
	}

std::string FilledTableau::name() const
	{
	return "FilledTableau";
	}

bool FilledTableau::parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals)
	{
	keyval_t::const_iterator kv=keyvals.find("dimension");
	if(kv!=keyvals.end()) dimension=to_long(*(kv->second->multiplier));
	else dimension=-1;
	return true;
	}

tabcanonicalise::tabcanonicalise(exptree& tr, iterator it)
	: tab_basics(tr,it)
	{
	}

void tabcanonicalise::description() const
	{
	txtout << "Canonicalise a FilledTableau tableau." << std::endl;
	}

bool tabcanonicalise::can_apply(iterator it) 
	{
	if(properties::get<FilledTableau>(it)) return true;
	return false;
	}

algorithm::result_t tabcanonicalise::apply(iterator& it)
	{
	yngtab::filled_tableau<exptree> one;
	
	sibling_iterator sib=tr.begin(it);
	unsigned int currow=0;
	while(sib!=tr.end(it)) {
		if(*sib->name=="\\comma") {
			sibling_iterator sib2=tr.begin(sib);
			while(sib2!=tr.end(sib)) {
				one.add_box(currow, exptree(sib2));
				++sib2;
				}
			}
		else one.add_box(currow, exptree(sib));
		++sib;
		++currow;
		}

	tree_exact_less_obj cmp;
	one.canonicalise(cmp);

	// put back
	tr.erase_children(it);
	multiply(it->multiplier, one.multiplicity);
	for(unsigned int r=0; r<one.number_of_rows(); ++r) {
		unsigned int rs=one.row_size(r);
		if(rs==1) 
			tr.append_child(it, one(r,0).begin());
		else {
			iterator tmp=tr.append_child(it, str_node("\\comma"));
			for(unsigned int c=0; c<rs; ++c) 
				tr.append_child(tmp, one(r,c).begin());
			}
		}
	
	expression_modified=true;
	return l_applied;
	}


tabstandardform::tabstandardform(exptree& tr, iterator it)
	: tab_basics(tr,it)
	{
	}

void tabstandardform::description() const
	{
	txtout << "Write a FilledTableau as the sum of tableaux in standard form." << std::endl;
	}

bool tabstandardform::can_apply(iterator it) 
	{
	if(properties::get<FilledTableau>(it)) return true;
	return false;
	}

algorithm::result_t tabstandardform::apply(iterator& it)
	{
	uinttab_t one;
	num_to_it.clear();
	tree_to_numerical_tab(it, one);
	
	uinttabs_t tabs;
	tabs.add_tableau(one);
	if(tabs.standard_form())
		 return l_no_action;

	// put the result back
	exptree rep;
	iterator top=rep.set_head(str_node("\\sum"));

	tabs_to_tree(tabs, top, it, false);

	expression_modified=true;
	multiply(rep.begin()->multiplier, *it->multiplier);
	pushup_multiplier(rep.begin());

	tr.replace(it, rep.begin());
	cleanup_sums_products(tr, it);
	return l_applied;
	}

tabdimension::tabdimension(exptree& tr, iterator it)
	: algorithm(tr,it)
	{
	}

void tabdimension::description() const
	{
	txtout << "Compute the dimension of a Young tableau." << std::endl;
	}

bool tabdimension::can_apply(iterator it) 
	{
	dimension=-1;

	tab=properties::get<Tableau>(it);
	if(tab) {
		dimension=tab->dimension;
		if(dimension>0) 
			return true;
		}

	ftab=properties::get<FilledTableau>(it);
	if(ftab) {
		dimension=ftab->dimension;
		if(dimension>0) 
			return true;
		}

	return false;
	}

algorithm::result_t tabdimension::apply(iterator& it)
	{
	if(ftab) {
		yngtab::filled_tableau<exptree> one;
	
		sibling_iterator sib=tr.begin(it);
		unsigned int currow=0;
		while(sib!=tr.end(it)) {
			if(*sib->name=="\\comma") {
				sibling_iterator sib2=tr.begin(sib);
				while(sib2!=tr.end(sib)) {
					one.add_box(currow, exptree(sib2));
					++sib2;
					}
				}
			else one.add_box(currow, exptree(sib));
			++sib;
			++currow;
			}
		node_one(it);
		multiply(it->multiplier, one.dimension(dimension));
		}
	else {
		yngtab::tableau one;
		sibling_iterator sib=tr.begin(it);
		while(sib!=tr.end(it)) {
			one.add_row(to_long(*sib->multiplier));
			++sib;
			}
		node_one(it);
		multiply(it->multiplier, one.dimension(dimension));
		}
	cleanup_sums_products(tr, it); // FIXME: does not work because we're not at the prod node
	expression_modified=true;
	return l_applied;
	}

lr_tensor::lr_tensor(exptree& tr, iterator it)
	: tab_basics(tr,it)
	{
	}

void lr_tensor::description() const
	{
	txtout << "Compute the tensor product of two Tableaux or two FilledTableaux." << std::endl;
	}

bool lr_tensor::can_apply(iterator it) 
	{
	if(*it->name=="\\prod") {
		sibling_iterator sib=tr.begin(it);
		tab1=tr.end(it);
		tab2=tr.end(it);
		while(sib!=tr.end(it)) {
			if(properties::get<Tableau>(sib)) {
				if(tab1==tr.end(it))
					tab1=sib;
				else {
					tab2=sib;
					break;
					}
				}
			++sib;
			}
		if(tab2!=tr.end(it)) return true;
		
		sib=tr.begin(it);
		tab1=tr.end(it);
		tab2=tr.end(it);
		while(sib!=tr.end(it)) {
			if(properties::get<FilledTableau>(sib)) {
				if(tab1==tr.end(it))
					tab1=sib;
				else {
					tab2=sib;
					break;
					}
				}
			++sib;
			}
		if(tab2!=tr.end(it)) return true;
		}
	return false;
	}

algorithm::result_t lr_tensor::apply(iterator& it)
	{
	if(properties::get<Tableau>(tab1)) do_tableau(it);
	else                               do_filledtableau(it);

	expression_modified=true;
	return l_applied;
	}

tab_basics::tab_basics(exptree& tr, iterator it) 
	: algorithm(tr, it)
	{
	}

unsigned int tab_basics::find_obj(const exptree& other)
	{
	for(unsigned int i=0; i<num_to_it.size(); ++i) {
		 if(tree_exact_equal(num_to_it[i], other))
			  return i;
		 }
	throw std::logic_error("internal error in tab_basics::find_obj");
	}

void tab_basics::tree_to_numerical_tab(iterator tab1, uinttab_t& one) 
	{
	unsigned int prevsize=num_to_it.size();

	// First determine the sort order of the children of tab1.
	sibling_iterator sib=tr.begin(tab1);
	while(sib!=tr.end(tab1)) {
		if(*sib->name=="\\comma") {
			sibling_iterator sib2=tr.begin(sib);
			while(sib2!=tr.end(sib)) {
				 num_to_it.push_back(sib2);
				 ++sib2;
				 }
			}
		else {
			 num_to_it.push_back(sib);
			 }			
		 ++sib;
		 }

	tree_exact_less_obj cmp;
	std::vector<exptree::iterator>::iterator startit=num_to_it.begin();
	startit+=prevsize;
	std::sort(startit, num_to_it.end(), cmp);
	
	// Now fill the uinttab.
	sib=tr.begin(tab1);
	unsigned int currow=0;
	while(sib!=tr.end(tab1)) {
		if(*sib->name=="\\comma") {
			sibling_iterator sib2=tr.begin(sib);
			while(sib2!=tr.end(sib)) {
				 one.add_box(currow, find_obj(exptree(sib2)) );
				 ++sib2;
				}
			}
		else {
			 one.add_box(currow, find_obj(exptree(sib)) );
			 }
		++sib;
		++currow;
		}
	}

void tab_basics::tabs_to_singlet_rules(uinttabs_t& tabs, iterator top)
	{
	uinttabs_t::tableau_container_t::iterator tabit=tabs.storage.begin();

	while(tabit!=tabs.storage.end()) {
		 // Keep only the diagrams which lead to a singlet.
		 for(unsigned int r=0; r<(*tabit).number_of_rows(); ++r) 
			  if((*tabit).row_size(r)%2!=0)
					goto next_tab;

		 { 
		 iterator tprod=tr.append_child(top, str_node("\\prod"));
		 for(unsigned int r=0; r<(*tabit).number_of_rows(); ++r) {
			  for(unsigned int c=0; c<(*tabit).row_size(r); ++c) {
					iterator tt=tr.append_child(tprod, str_node("\\delta"));
					tr.append_child(tt, num_to_it[(*tabit)(r,c++)]);
					tr.append_child(tt, num_to_it[(*tabit)(r,c)]);
					}
			  }
		 }

  	    next_tab:
		 ++tabit;
		 }
	}


void tab_basics::tabs_to_tree(uinttabs_t& tabs, iterator top, iterator tabpat, bool even_only)
	{
	uinttabs_t::tableau_container_t::iterator tabit=tabs.storage.begin();

	while(tabit!=tabs.storage.end()) {
		 // Keep only the diagrams which lead to a singlet if requested.
		 if(even_only)
			  for(unsigned int r=0; r<(*tabit).number_of_rows(); ++r) 
					if((*tabit).row_size(r)%2!=0)
						 goto next_tab;

		 { iterator tt=tr.append_child(top, str_node(tabpat->name));
		 multiply(tt->multiplier, tabit->multiplicity);
		 for(unsigned int r=0; r<(*tabit).number_of_rows(); ++r) {
			  unsigned int rs=(*tabit).row_size(r);
			  if(rs==1) 
					tr.append_child(tt, num_to_it[(*tabit)(r,0)]);
			  else {
					iterator tmp=tr.append_child(tt, str_node("\\comma"));
					for(unsigned int c=0; c<rs; ++c) 
						 tr.append_child(tmp, num_to_it[(*tabit)(r,c)]);
					}
			  }
			  }

	    next_tab:
		 ++tabit;
		 }
	}


// The format is \ftab{a,b,c}{d,e}{f}.
//
void lr_tensor::do_filledtableau(iterator& it)
	{
	bool even_only=false;
	bool singlet_rules=false;

	if(has_argument("EvenOnly"))
		 even_only=true;
	if(has_argument("SingletRules"))
		 singlet_rules=true;

	uinttab_t one, two;

	// For efficiency we store integers in the tableaux, not the actual
	// exptree objects.
	uinttabs_t prod;

	num_to_it.clear();
	tree_to_numerical_tab(tab1, one);
	tree_to_numerical_tab(tab2, two);

	yngtab::LR_tensor(one,two,999,prod.get_back_insert_iterator());

	exptree rep;
	iterator top=rep.set_head(str_node("\\sum"));

	if(singlet_rules) tabs_to_singlet_rules(prod, top);
	else              tabs_to_tree(prod, top, tab1, even_only);

	sibling_iterator sib=rep.begin(top);
	while(sib!=rep.end(top)) {
		 sib->fl.bracket=str_node::b_round;
		 ++sib;
		 }

	expression_modified=true;
	tr.replace(tab1, rep.begin());
	tr.erase(tab2);
	cleanup_sums_products(tr, it);
	}

void lr_tensor::do_tableau(iterator& it)
	{
	bool even_only=false;
	if(has_argument("EvenOnly"))
		 even_only=true;

	yngtab::tableau one, two;
	yngtab::tableaux<yngtab::tableau> prod;
	
	sibling_iterator sib=tr.begin(tab1);
	while(sib!=tr.end(tab1)) {
		one.add_row(to_long(*sib->multiplier));
		++sib;
		}
	sib=tr.begin(tab2);
	while(sib!=tr.end(tab2)) {
		two.add_row(to_long(*sib->multiplier));
		++sib;
		}
	yngtab::LR_tensor(one,two,999,prod.get_back_insert_iterator());

	exptree rep;
	iterator top=rep.set_head(str_node("\\sum"));
	yngtab::tableaux<yngtab::tableau>::tableau_container_t::iterator tabit=prod.storage.begin();
	while(tabit!=prod.storage.end()) {
		// Keep only the diagrams which lead to a singlet if requested.
		if(even_only)
			for(unsigned int r=0; r<(*tabit).number_of_rows(); ++r) 
				if((*tabit).row_size(r)%2!=0)
					goto next_tab;

		{iterator tt=tr.append_child(top, str_node(tab1->name));
		multiply(tt->multiplier, tabit->multiplicity);
		for(unsigned int r=0; r<(*tabit).number_of_rows(); ++r) 
			multiply(tr.append_child(tt, str_node("1"))->multiplier, (*tabit).row_size(r));
			}

	   next_tab:
		++tabit;
		}

	expression_modified=true;
	tr.replace(tab1, rep.begin());
	tr.erase(tab2);
	cleanup_sums_products(tr, it);
	}



young_project_product::young_project_product(exptree& tr, iterator it)
	 : algorithm(tr, it)
	{
	}

void young_project_product::description() const
	{
	txtout << "Project a product of tensors by projecting each factor in turn." << std::endl;
	}

bool young_project_product::can_apply(iterator it)
	{
	if(*it->name=="\\prod") return true;
	return false;
	}

algorithm::result_t young_project_product::apply(iterator& it)
	{
	exptree rep;
	iterator topsum = rep.set_head(str_node("\\sum"));

	sibling_iterator sib=tr.begin(it);
	bool first=true;
	while(sib!=tr.end(it)) {
		 young_project_tensor ypt(tr, tr.end());
		 sibling_iterator nxt=sib;
		 ++nxt;
		 if(ypt.can_apply(sib)) {
			  ypt.modulo_monoterm=true;
			  iterator ii(sib);
			  ypt.apply(ii);
			  if(*ii->name!="\\sum") 
					ii=tr.wrap(ii, str_node("\\sum"));

			  expression_modified=true;
			  if(first) {
					// Add a new \prod node to rep for each term in the projected factor.
					first=false;
					sibling_iterator trm=tr.begin(ii);
					while(trm!=tr.end(ii)) {
						 iterator prod=rep.append_child(topsum, str_node("\\prod"));
						 prod->fl.bracket=str_node::b_round;
						 prod->multiplier=it->multiplier;
						 rep.append_child(prod, iterator(trm));
						 ++trm;
						 }
					}
			  else {
					// Distribute this projected factor over all existing terms in rep,
					sibling_iterator trm=rep.begin(topsum);
					while(trm!=rep.end(topsum)) {
						 iterator tmp=trm;
						 sibling_iterator nxttrm=trm;
						 ++nxttrm;

						 // Copy this term out of the sum and append projected factor.
						 exptree work(trm);
						 iterator workit=work.begin();
						 iterator put=rep.append_child(work.begin(), str_node());
						 if(tr.number_of_children(ii)==1)
							  rep.replace(put, tr.begin(ii));
						 else {
							  rep.replace(put, ii);
							  
							  // Distribute the product.
							  distribute   dis(work, work.end());
							  if(dis.can_apply(workit)) 
									dis.apply(workit);
							  }
//						 txtout << "GOOD?" << std::endl;
//						 work.print_recursive_treeform(txtout, work.begin());

						 // Canonicalise all new products.
						 canonicalise can(work, work.end());
//						 txtout << *workit->name << " " << *put->name << std::endl;
						 can.apply_recursive(workit, false);
						 if(*work.begin()->multiplier!=0) {
							  // The upcoming move wants to see a sum, even if there is only one term
							  if(*work.begin()->name!="\\sum")
									work.wrap(work.begin(), str_node("\\sum"));
							  
							  // Move back to the original tree.
							  sibling_iterator cpyit=work.begin(work.begin());
							  while(cpyit!=work.end(work.begin())) {
									sibling_iterator cpyitnxt=cpyit;
									++cpyitnxt;
									rep.move_before(trm, cpyit);
									cpyit=cpyitnxt;
									}
							  }
						 rep.erase(trm);
						 
						 trm=nxttrm;
						 }
					collect_terms coll(rep, rep.end());
					if(coll.can_apply(topsum)) 
						 coll.apply(topsum);

					// If collect terms removed the sum because there was only
					// one term (or zero) left, put it back in.
					if(*topsum->name!="\\sum")
						 topsum=tr.wrap(topsum, str_node("\\sum"));
					}
			  }
		 else {
			  if(first) {
					first=false;
					iterator prod=rep.append_child(topsum, str_node("\\prod"));
					prod->fl.bracket=str_node::b_round;
					prod->multiplier=it->multiplier;
					rep.append_child(prod, iterator(sib));
					}
			  else {
					// just multiply all terms with this factor
					sibling_iterator trm=rep.begin(topsum);
					while(trm!=rep.end(topsum)) {
						 iterator put=rep.append_child(trm,str_node());
						 tr.replace(put, iterator(sib));
						 ++trm;
						 }
					}
			  }
//		 rep.print_recursive_treeform(debugout, rep.begin());
		 sib=nxt;
		 }
	
	if(expression_modified) {
		 it=tr.replace(it, rep.begin());
		 expression_modified=true;
		 // FIXME: this canonicalise should really not be necessary
//		 txtout << "WHOOAAH " << *it->name << std::endl;
//		 tr.print_recursive_treeform(txtout, it);
//		 canonicalise can(tr, tr.end());
//		 can.apply_recursive(it, false);
		 cleanup_sums_products(tr, it);
		 return l_applied;
		 }
	else return l_no_action;
	}

decompose_product::decompose_product(exptree&tr, iterator it)
	: algorithm(tr, it), t1(0), t2(0)
	{
	}

void decompose_product::description() const
	{
	txtout << "Decompose tensor product according to Tableau symmetries." << std::endl;
	}

const Indices *decompose_product::indices_equivalent(iterator it) const
	{
	exptree::index_iterator ii=tr.begin_index(it);
	const Indices *ret=0, *tmp=0;
	while(ii!=tr.end_index(it)) {
		tmp=properties::get<Indices>(ii);
		if(tmp==0) return 0;
		if(ret==0) ret=tmp;
		else if(ret!=tmp) return 0;
		++ii;
		}
	return ret;
	}

bool decompose_product::can_apply(iterator it)
	{
	// Act on products. Find the first object which either has a
	// TableauSymmetry or has one vector index only. Then find the next
	// indexed object in the product and return true if this is a
	// one-indexed or TableauSymmetry object, and if the index types
	// of all indices match. 

	if(*it->name=="\\prod") {
		sibling_iterator fc=tr.begin(it);
		while(fc!=tr.end(it)) {
			t1=properties::get<TableauBase>(fc);
			if(t1 || tr.number_of_indices(fc)==1) {
				f1=fc;
				ind1=indices_equivalent(fc);
				if(ind1) {
					++fc;
					if(fc!=tr.end(it)) {
						t2=properties::get<TableauBase>(fc);
						if(t2 || tr.number_of_indices(fc)==1) {
							f2=fc;
							ind2=indices_equivalent(fc);
							if(ind2 && ind1==ind2) {
								const Integer *itg=properties::get<Integer>( tr.begin_index(fc) );
								if(itg) {
									dim=to_long(*itg->difference.begin()->multiplier);
									if(dim>0)
										return true;
									}
								}
							}
						}
					}
				}
			++fc;
			}
		}
	return false;
	}

std::ostream& operator<<(std::ostream& str, exptree::sibling_iterator it)
	{
	str << *it->name;
	return str;
	}

void decompose_product::fill_asym_ranges(TableauBase::tab_t& tab, int offset, 
													  combin::range_vector_t& ranges)
	{
	// FIXME: we could also look at all other factors, and see if the index
	// _name_ in the slot is contracted to the index name in an antisymmetric
	// slot range. But that is more tricky, because index names move, whereas
	// slots stay.

	for(unsigned int i=0; i<tab.row_size(0); ++i) {
		TableauBase::tab_t::in_column_iterator ci=tab.begin_column(i);
		combin::range_t tmprange;
		while(ci!=tab.end_column(i)) {
			tmprange.push_back((*ci)+offset);
			++ci;
			}
		if(tmprange.size()>=2) 
			ranges.push_back(tmprange);
		}
	}

algorithm::result_t decompose_product::apply(iterator& it)
	{
	// Create the tensor product Young tableaux.
	sibtab_t  m1,m2;
	sibtabs_t prod;
	numtabs_t numprod;

	unsigned int ioffset1=0, ioffset2=0;

	if(t1) {
		if(t1->size(tr, f1)>1)
			throw consistency_error("sorry, only for single-tableau tensors");
		t1tab=t1->get_tab(tr, f1, 0);
		for(unsigned int r=0; r<t1tab.number_of_rows(); ++r) 
			for(unsigned int c=0; c<t1tab.row_size(r); ++c) {
				exptree::index_iterator tmpii=tr.begin_index(f1);
				tmpii+=t1tab(r,c);
				m1.add_box(r, tmpii);
				}
		}
	else m1.add_box(0, tr.begin_index(f1));

	if(t2) {
		if(t2->size(tr, f2)>1)
			throw consistency_error("sorry, only for single-tableau tensors");
		t2tab=t2->get_tab(tr, f2, 0);
		for(unsigned int r=0; r<t2tab.number_of_rows(); ++r) 
			for(unsigned int c=0; c<t2tab.row_size(r); ++c) {
				exptree::index_iterator tmpii=tr.begin_index(f2);
				tmpii+=t2tab(r,c);
				m2.add_box(r, tmpii);
				}
		}
	else m2.add_box(0, tr.begin_index(f2));

	// Determine the position of the first index of the two 
	// factors relative to the product (not to the tensors themselves).
	exptree::index_iterator srch=tr.begin_index(it);
	while(srch!=tr.end_index(it)) {
		if(iterator(srch)==iterator(tr.begin_index(f1)))
			break;
		++ioffset1;
		++srch;
		}
	srch=tr.begin_index(it);
	while(srch!=tr.end_index(it)) {
		if(iterator(srch)==iterator(tr.begin_index(f2)))
			break;
		++ioffset2;
		++srch;
		}

	// Determine slot ranges which are anti-symmetric.

	asym_ranges.clear();
	if(t1) fill_asym_ranges(t1tab, ioffset1, asym_ranges);
	if(t2) fill_asym_ranges(t2tab, ioffset2, asym_ranges);

	// Make the tensor product tableaux.

	yngtab::LR_tensor(m1, m2, dim, prod.get_back_insert_iterator(), true);
	
	// The tableaux in 'prod' contain in their boxes iterators to
	// the indices in the original expression. We convert these to
	// numerical positions so they can be applied to copies of the
	// expression as well.

	sibtabs_t::tableau_container_t::iterator tt=prod.storage.begin();
	while(tt!=prod.storage.end()) {
		numtab_t tmptab;
		tmptab.copy_shape(*tt);
		sibtab_t::iterator si=tt->begin();
		numtab_t::iterator ni=tmptab.begin();
		while(si!=tt->end()) {
			exptree::index_iterator fnd=tr.begin_index(it);
			unsigned int inum=0;
			while(fnd!=tr.end_index(it)) {
				if(iterator(fnd) == (*si)) {
					*ni=inum;
					break;
					}
				++inum;
				++fnd;
				}
			assert(inum!=tr.number_of_indices(it));
			++ni;
			++si;
			}
		numprod.storage.push_back(tmptab);
		++tt;
		}

	// Now create a Young projector sum of terms with the indices
	// distributed according to the tensor product tableaux.

// 	txtout << numprod << std::endl;

	exptree rep;
	rep.set_head(str_node("\\tmp"));  // not \sum to prevent auto flattening

	numtabs_t::tableau_container_t::iterator ntt=numprod.storage.begin();

	while(ntt!=numprod.storage.end()) {
		// TESTINGONLY
///		++ntt; ++ntt; ++ntt;

//		txtout << "another tableau" << std::endl;
		young_project yp(tr, tr.end());
		yp.tab=(*ntt);

//		if(getenv("SMART")) 
		yp.asym_ranges=asym_ranges;

		// The asym ranges contain ranges of index locations. What we need
		// to convert this to is box numbers. This is a value->location
		// conversion in combinatorics.hh language. This will be done
		// inside the youngtab.hh routines.
		
		// Apply the product projector.
		iterator rr=rep.append_child(rep.begin(), it);
		assert(yp.can_apply(rr));
		yp.apply(rr);

		// We cannot use any algorithms which re-order indices, as the
		// order in yp.sym must match the order in the expression. Also,
		// we cannot remove terms without removing the corresponding entries
		// in yp.sym. So for the time being we have decided to put this 
		// simplification in young_project.

		// Now apply the symmetries of the original tableaux (if any).
		// For each of the permutations in the product projector,
		// we need to figure out where the indices went which sat on
		// tensor 1 and 2. This information is stored in the symmetriser
		// of young_project. These indices then have to be projected using
		// the tensor projectors.


		// TESTINGONLY
//		txtout << "one ..." << std::flush;
		if(t1) project_onto_initial_symmetries(rep, rr, yp, t1, f1, ioffset1, t1tab, false);
//		txtout << "done" << std::endl;
//		txtout << "two ..." << std::flush;
		if(t2) project_onto_initial_symmetries(rep, rr, yp, t2, f2, ioffset2, t2tab, true);
//		txtout << "done" << std::endl;

//    TESTINGONLY
///		break; 

		++ntt;
		}

	rep.begin()->name=name_set.insert("\\sum").first;

	expression_modified=true;
	it=tr.replace(it, rep.begin());

	// flatten sums
//	txtout << "flattening... " << std::flush;
	sumflatten sf(tr, tr.end());
	sf.apply_recursive(it, false);
//	txtout << "done" << std::endl;
	cleanup_nests(tr, it);

	return l_applied;
	}


void decompose_product::project_onto_initial_symmetries(exptree& rep, iterator rr, young_project& yp,
																		  const TableauBase *tt, iterator ff, 
																		  int ioffset, const TableauBase::tab_t& thetab,
																		  bool remove_traces)
	{
	// Sample: S_{m n} D_{p}{ A_{n q} } with S symmetric and A antisymmetric.
	// The tensor product contains one tableau which leads to a symmetriser
	// with as first entry 0 3 2 1 4. This means that the 'm' and 'n' index
	// names are associated, in the original, to box 0 and 3 respectively.
	// Now one of the terms in this symmetriser reads 2 4 0 1 3. In order
	// to apply the individual tensor projectors, we read off that the
	// 'm' and 'n' indices have now been moved to slot 2 and 1 respectively.
	// So we create a [1 1] tableau with numbers 2 and 1 in the boxes,
	// and apply this tensor projector to the full product tensor.
	
	unsigned int termnum=0;
	
	// Run through all terms in this tableau of the tensor product.
	sibling_iterator term=rep.begin(rr);
	while(term!=rep.end(rr)) {
		// Setup the tableau for initial-tensor projection.
		young_project ypinitial(tr, tr.end());
		ypinitial.tab.copy_shape(thetab);
		numtab_t::iterator tabit=ypinitial.tab.begin();
		numtab_t::iterator origtabit=thetab.begin();

		sibling_iterator nxt=term;
		++nxt;
		exptree::index_iterator ii=tr.begin_index(ff);
		while(ii!=tr.end_index(ff)) {
			unsigned int ipos=ioffset + (*origtabit);
			assert(termnum<yp.sym.size());
			
			// Find ipos in the first entry of yp.sym
			// and store the new position in the tableau.
			for(unsigned int i=0; i<yp.sym[termnum].size(); ++i) {
				if(yp.sym[termnum][i]==ipos) {
					*tabit=yp.sym[0][i];
//					txtout << ipos << " has moved to " << yp.sym[0][i] << std::endl;
					break;
					}
				}
			++tabit;
			++origtabit;
			++ii;
			}
		
		// Now we can finally project.
		yp.remove_traces=remove_traces;

		if(*term->name=="\\sum") { // apply to all terms in the sum
			// THIS IS NOT CORRECT?! If we turn on asym_ranges  here
			// the result breaks.
//			if(getenv("SMART")) 
//				ypinitial.asym_ranges=asym_ranges;
			sibling_iterator trmit=tr.begin(term);
			while(trmit!=tr.end(term)) {
				iterator tmp=trmit;
				sibling_iterator nxt2=trmit;
				++nxt2;

				// Now apply the projector.
				assert(ypinitial.can_apply(tmp));
				ypinitial.apply(tmp);
				trmit=nxt2;
				}
			}
		else { // just a single term
//			if(getenv("SMART")) 
				ypinitial.asym_ranges=asym_ranges;
			iterator tmp=term;
			assert(ypinitial.can_apply(tmp));
			ypinitial.apply(tmp);
			}

		++termnum;
		term=nxt;
		}
	}
