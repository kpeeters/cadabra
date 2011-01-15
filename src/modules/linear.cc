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

//#define OLDVERSION 1

#include "linear.hh"
#include "display.hh"
#include "properties.hh"
#include "algebra.hh"
#include "dummies.hh"
#ifndef OLDVERSION
#include "tableaux.hh"
#endif
#include <utility>
#include <map>
#include <vector>
#include <algorithm>

#include "storage.hh"

bool linear::gaussian_elimination(const std::vector<std::vector<multiplier_t> >& a, 
											 const std::vector<multiplier_t>& b)
	{
	std::vector<std::vector<multiplier_t> > tmpa(a);
	std::vector<multiplier_t>               tmpb(b);

	return gaussian_elimination_inplace(tmpa, tmpb);
	}

bool linear::gaussian_elimination_inplace(std::vector<std::vector<multiplier_t> >& a, 
														std::vector<multiplier_t>& b)
	{
	assert(a.size() == b.size());

	// If there are more equations than unknowns, we first reduce the form to
	//  xxx = x
	//   xx = x
   //    x = x
   //  000 = 0
   //  000 = 0

	unsigned int number_of_eqs = a.size();
	unsigned int number_of_unk = a[0].size();
	unsigned int mineu=std::min(number_of_eqs, number_of_unk);

	// Loop over rows, creating upper-triangular matrix 'a'
	for(unsigned row=0; row<mineu; ++row) {
		multiplier_t pivot = a[row][row];
		if(pivot == 0) {
			unsigned int nrow;
			for(nrow=row+1; nrow<number_of_eqs; ++nrow)
				if((pivot = a[nrow][row]) != 0) 
					break;
			if(pivot == 0) 
				return true; // undetermined system FIXME: still minimalise
			std::swap(a[nrow],a[row]);
			std::swap(b[nrow],b[row]);
			}
		
      // Gaussian elimination of column
		for(unsigned int nrow=row+1; nrow<number_of_eqs; ++nrow) {
			multiplier_t tmp = a[nrow][row]/pivot;
			a[nrow][row]=0;
			for(unsigned int col=row+1; col<number_of_unk; ++col)
				a[nrow][col] -= tmp*a[row][col];
			b[nrow] -= tmp*b[row];
			}
		}
//	for(unsigned int i=0; i<a.size(); ++i) {
//		for(unsigned int j=0; j<a[i].size(); ++j)
//			txtout << a[i][j] << " ";
//		txtout <<  " = " << b[i] << std::endl;
//		}

	// Check that there are no inconsistencies
	for(unsigned int i=number_of_unk; i<number_of_eqs; ++i)
		if(b[i]!=0)
			return false;

//	txtout << "consistent" << std::endl;

   // Back substitution
	for(int row=mineu-1; row>=0; --row) {
		assert(a[row][row]!=0);
		for(int col=mineu-1; col>row; --col) {
			b[row]      -= a[row][col]*b[col];
			for(unsigned allcol=col; allcol<number_of_unk; ++allcol)
				a[row][allcol] -= a[row][allcol]*a[row][row];
			}
		assert(a[row][row]!=0);
		b[row]      /= a[row][row];
		a[row][row]  = (a[row][row]!=0?1:0);
		}
	return true;
	}

//  3 5 7   2  ->  3 5 7   2      ->  3 5 7    2
//    8 4   1        8 4   1          
//      2   3          1   3/2

//  3 5 7   2  ->  3 5 7   2      ->  3 5 7    2
//    8 4   1        8 4   1          



lsolve::lsolve(exptree& tr, iterator it) 
	: algorithm(tr, it)
	{
	}

void lsolve::description() const
	{
	txtout << "Solve a system of linear equations." << std::endl;
	}

bool lsolve::can_apply(iterator it) 
	{
	if(*it->name!="\\comma") return false;

	sibling_iterator sib=tr.begin(it);
	while(sib!=tr.end(it)) {
		if(*sib->name!="\\equals")
			return false;
		++sib;
		}
	return true;
	}

algorithm::result_t lsolve::apply(iterator& it)
	{
	// first find all symbols, associate them to a number
	std::map<nset_t::iterator, int, nset_it_less> name_to_number;
	std::map<int, nset_t::iterator>               number_to_name;
	unsigned int number_of_names=0;

	assert(*it->name=="\\comma");
	sibling_iterator eqsit=tr.begin(it);
	while(eqsit!=tr.end(it)) {
		assert(*eqsit->name=="\\equals");
		sibling_iterator plusit=tr.begin(eqsit);
		if(*plusit->name=="\\sum") {
			sibling_iterator termit=tr.begin(plusit);
			while(termit!=tr.end(plusit)) {
				if(name_to_number.find(termit->name)==name_to_number.end()) {
					name_to_number[termit->name]=number_of_names;
					number_to_name[number_of_names]=termit->name;
					++number_of_names;
					}
				++termit;
				}
			}
		else {
			if(name_to_number.find(plusit->name)==name_to_number.end()) {
				name_to_number[plusit->name]=number_of_names;
				number_to_name[number_of_names]=plusit->name;
				++number_of_names;
				}
			}
		// FIXME: search rhs for symbols
		++eqsit;
		}
	
	// create the matrix and vector
	std::vector<std::vector<multiplier_t> > eqs(tr.number_of_children(it));
	std::vector<multiplier_t>               rhs(tr.number_of_children(it));
	eqsit=tr.begin(it);
	unsigned int eqno=0;
	while(eqsit!=tr.end(it)) {
		eqs[eqno].resize(number_of_names,0);
		sibling_iterator plusit=tr.begin(eqsit);
		if(*plusit->name=="\\sum") {
			sibling_iterator termit=tr.begin(plusit);
			while(termit!=tr.end(plusit)) {
				eqs[eqno][name_to_number[termit->name]]=*termit->multiplier;
				++termit;
				}
			}
		else eqs[eqno][name_to_number[plusit->name]]=*plusit->multiplier;
		++plusit;
		assert(plusit->is_rational());
		rhs[eqno]=*plusit->multiplier;

		++eqsit;
		++eqno;
		}

//	for(unsigned int i=0; i<eqs.size(); ++i) {
//		for(unsigned int j=0; j<eqs[i].size(); ++j)
//			txtout << eqs[i][j] << " ";
//		txtout << " = " << rhs[i] ;
//		txtout << std::endl;
//		}

	if(linear::gaussian_elimination_inplace(eqs, rhs)) {
//		for(unsigned int i=0; i<eqs.size(); ++i) {
//			for(unsigned int j=0; j<eqs[i].size(); ++j)
//				txtout << eqs[i][j] << " ";
//			txtout << " = " << rhs[i] ;
//			txtout << std::endl;
//			}
		// fill in the result
		eqsit=tr.begin(it);
		unsigned int eqno=0;
		while(eqsit!=tr.end(it)) {
			if(eqno>=number_to_name.size()) {
				do {
					eqsit=tr.erase(eqsit);
               } while(eqsit!=tr.end(it));
				break;
				}
			tr.erase_children(eqsit);
			iterator sumit=tr.append_child(eqsit, str_node("\\sum"));
			for(unsigned int j=0; j<eqs[eqno].size(); ++j) {
				if(eqs[eqno][j]!=0) {
					tr.append_child(sumit, str_node(number_to_name[j]))->multiplier=
						rat_set.insert(eqs[eqno][j]).first;
					}
				}
			tr.append_child(eqsit, str_node("1"))->multiplier=rat_set.insert(rhs[eqno]).first;
			cleanup_expression(tr, sumit);
			++eqno;
			++eqsit;
			}
		expression_modified=true;
		return l_applied;
		}
	else {
		txtout << "system of equations admits no solution" << std::endl;
		return l_no_action;
		}
	}



decompose::decompose(exptree& tr, iterator it) 
	: algorithm(tr, it)
	{
	}

void decompose::description() const
	{
	txtout << "Decompose a monomial on a given basis of monomials, taking into account all tensor symmetries." << std::endl;
	}

bool decompose::can_apply(iterator it) 
	{
	if(*it->name!="\\prod") return false;
	if(number_of_args()!=1 && number_of_args()!=2) {
		txtout << "Need a basis on which to decompose." << std::endl;
		return false;
		}
	return true;
	}

void decompose::add_element_to_basis(exptree& projterm, exptree::iterator projtermit) 
	{
	// Add a new column for the new term in the basis
	for(unsigned int ii=0; ii<coefficient_matrix.size(); ++ii)
		coefficient_matrix[ii].push_back(0);

	if(*projtermit->name=="\\sum") {
		sibling_iterator moreit=projterm.begin(projtermit);
		while(moreit!=projterm.end(projtermit)) {
			multiplier_t remember_mult=*moreit->multiplier;
			one(moreit->multiplier);
			bool thistermfound=false;
			for(unsigned int ypi=0; ypi<terms_from_yp.size(); ++ypi) {
				if(projterm.equal_subtree(terms_from_yp[ypi].begin(), (iterator)moreit)) {
					coefficient_matrix[ypi].back()=remember_mult;
					thistermfound=true;
//					txtout << "found existing monomial" << std::endl;
					break;
					}
				}
			if(!thistermfound) { // new monomial, so add a new row to the coefficient matrix
				exptree tmp(moreit);
//				tmp.print_recursive_treeform(txtout, tmp.begin());
				terms_from_yp.push_back(tmp);
				std::vector<multiplier_t> crow(coefficient_matrix.size()>0?
														 coefficient_matrix[0].size():1,0);
				crow.back()=remember_mult;
				coefficient_matrix.push_back(crow);
//				txtout << "added new monomial" << std::endl;
				}
			++moreit;
			}
		}
	else {
		multiplier_t remember_mult=*projtermit->multiplier;
		one(projtermit->multiplier);
		bool thistermfound=false;
		for(unsigned int ypi=0; ypi<terms_from_yp.size(); ++ypi) {
			if(projterm.equal_subtree(terms_from_yp[ypi].begin(), projtermit)) {
				coefficient_matrix[ypi].back()=remember_mult;
				thistermfound=true;
				break;
				}
			}
		if(!thistermfound) { // new monomial, so add a new row to the coefficient matrix
			exptree tmp(projtermit);
			terms_from_yp.push_back(tmp);
			std::vector<multiplier_t> crow(coefficient_matrix.size()>0?
													 coefficient_matrix[0].size():1,0);
			crow.back()=remember_mult;
			coefficient_matrix.push_back(crow);
			}
		}
	}

algorithm::result_t decompose::apply(iterator& it)
	{
	bool ypproject=true;
	sibling_iterator nxtarg=args_begin();
	while(nxtarg!=args_end()) {
		 debugout << "argument " << *nxtarg->name << std::endl;
		 if(*nxtarg->name=="DontProject") {
			  ypproject=false;
			  break;
			  }
		 ++nxtarg;
		 }

	iterator basisit=args_begin();
	if(! (*basisit->name=="\\comma")) {
		sibling_iterator fr=args_begin();
		sibling_iterator nd=fr;
		++nd;
		// basis should be a list; write it as such even if there's only one element.
		basisit->fl.bracket=str_node::b_none;
		basisit=tr.wrap(basisit, str_node("\\comma"));
		}

	exptree projbasis;
	projbasis.set_head(str_node("\\expression"));
	terms_from_yp.clear();
	coefficient_matrix.clear();

	// Some overlap with code in all_contractions.
	bool nontrivial_symmetries_present=false;
	sibling_iterator factorit=tr.begin(it);
	while(factorit!=tr.end(it)) { // Do this always, even if ypproject==false, since we need it for rhs.
		if(tr.number_of_children(factorit)>1) {
			const TableauBase *tb = properties::get<TableauBase>(factorit);
			if(tb)
				if(!tb->is_simple_symmetry(tr, it)) {
					nontrivial_symmetries_present=true;
					break;
					}
			}
		++factorit;
		}

	// Setup the coefficient matrix.
	if(nontrivial_symmetries_present && ypproject) {
		 debugout << "Going to project the basis." << std::endl;
		// Need to make a Young-projected basis.
		sibling_iterator sib=tr.begin(basisit);
		while(sib!=tr.end(basisit)) {
			 debugout << "Next term in the basis." << std::endl;
			exptree projterm;
			projterm.set_head(str_node("\\expression"));
			projterm.append_child(projterm.begin(), (iterator)(sib));

#ifdef OLDVERSION
			young_project_tensor ypt(projterm, projterm.end());
			ypt.modulo_monoterm=true;
			iterator projtermit=projterm.begin(projterm.begin());
			ypt.apply_recursive(projtermit, false);

			distribute dbt(projterm, projterm.end());
			canonicalise can(projterm, projterm.end());
//			can.method=canonicalise::xperm;
			rename_dummies ren(projterm, projterm.end());
			collect_terms ct(projterm, projterm.end());
			
			dbt.apply_recursive(projtermit, false);// FIXME: URGENT: should check consistency
			ren.apply_recursive(projtermit, false);  // by far the slowest step
			if(*projtermit->name=="\\sum")
				ct.apply(projtermit);
			can.apply_recursive(projtermit, false);
			ren.apply_recursive(projtermit, false);
			if(*projtermit->name=="\\sum")
				ct.apply(projtermit);
#else
			iterator projtermit=projterm.begin(projterm.begin());
			young_project_product ypp(projterm, projterm.end());
			sumflatten sf(projterm, projterm.end());
			collect_terms ct(projterm, projterm.end());
			rename_dummies ren(projterm, projterm.end());

			ypp.apply_recursive(projtermit, false);
			sf.apply_recursive(projtermit, false);
			ren.apply_recursive(projtermit, false);  // by far the slowest step
			if(*projtermit->name=="\\sum")
				ct.apply(projtermit);
			sibling_iterator sib2=tr.begin(projtermit);
			while(sib2!=tr.end(projtermit)) {
				 sib2->fl.bracket=str_node::b_none;
				 ++sib2;
				 }
#endif
			// After young projection, we may get identically zero.
			if(projtermit->is_zero()) {
				txtout << "An element of the basis is identically zero after Young projection." << std::endl;
				return l_error;
				}
			add_element_to_basis(projterm, projtermit);
			++sib;
			}
		debugout << "Young-projected basis constructed." << std::endl;
		}
	else {
		// Copy the basis straight into the terms_from_yp.
		assert(*basisit->name=="\\comma");
		sibling_iterator sib=tr.begin(basisit);
		while(sib!=tr.end(basisit)) {
			exptree projterm(sib);
			iterator projtermit=projterm.begin();
			sibling_iterator sib2=tr.begin(projtermit);
			while(sib2!=tr.end(projtermit)) {
				 sib2->fl.bracket=str_node::b_none;
				 ++sib2;
				 }
			add_element_to_basis(projterm, projtermit);
			++sib;
			}
		debugout << "Kept old young-projected basis." << std::endl;
		}

	// Young project the rhs.
	exptree rhstree;
	rhstree.set_head(str_node("\\expression"));
	rhstree.append_child(rhstree.begin(), it);
	iterator rhsit=rhstree.begin(rhstree.begin());
	if(nontrivial_symmetries_present) {
#ifdef OLDVERSION
		young_project_tensor ypt(rhstree, rhstree.end());
		ypt.modulo_monoterm=true;
		ypt.apply_recursive(rhsit, false);
		if(*rhsit->name=="\\prod") {
			distribute dbt(rhstree, rhstree.end());
			canonicalise can(rhstree, rhstree.end());
			rename_dummies ren(rhstree, rhstree.end());
			collect_terms ct(rhstree, rhstree.end());

			dbt.apply(rhsit);
			ren.apply_recursive(rhsit, false);
			if(*rhsit->name=="\\sum")
				ct.apply(rhsit);
			can.apply_recursive(rhsit, false);
			ren.apply_recursive(rhsit, false);
			if(*rhsit->name=="\\sum")
				ct.apply(rhsit);
			}
#else
		young_project_product ypp(rhstree, rhstree.end());
		sumflatten sf(rhstree, rhstree.end());
		collect_terms ct(rhstree, rhstree.end());
		rename_dummies ren(rhstree, rhstree.end());

		debugout << "young project rhs." << std::endl;
		ypp.apply_recursive(rhsit, false);
		debugout << "sumflatten." << std::endl;
		sf.apply_recursive(rhsit, false);
		debugout << "rename." << std::endl;
		ren.apply_recursive(rhsit, false);  // by far the slowest step
		debugout << "collect terms." << std::endl;
		ct.apply_recursive(rhsit, false);
		debugout << "rhs projection done." << std::endl;

		sibling_iterator sib2=rhstree.begin(rhsit);
		while(sib2!=rhstree.end(rhsit)) {
			 sib2->fl.bracket=str_node::b_none;
			 ++sib2;
			 }
#endif		
		}	
	// debugout << "Young-projected rhs constructed" << std::endl;
	// rhstree.print_recursive_treeform(debugout, rhstree.begin());

	std::vector<multiplier_t> rhs(terms_from_yp.size(),0);
	if(*rhsit->name=="\\sum") {
		// iterate over all terms
		sibling_iterator rhssumit=rhstree.begin(rhsit);
		while(rhssumit!=rhstree.end(rhsit)) {
			bool found_in_basis=false;
			multiplier_t rhsmult=*rhssumit->multiplier;
			one(rhssumit->multiplier);
			for(unsigned int i=0; i<terms_from_yp.size(); ++i) {
				if(tr.equal_subtree(terms_from_yp[i].begin(), (iterator)(rhssumit))) {
					rhs[i]=rhsmult;
					found_in_basis=true;
					break;
					}
				}
			if(!found_in_basis) {
				txtout << "rhs contains a term not present in the basis" << std::endl;
				return l_error;
				}
			++rhssumit;
			}
		}
	else {
		// only one term in the rhs
		 if(rhsit->is_zero()==false) {
			  bool found_in_basis=false;
			  multiplier_t rhsmult=*rhsit->multiplier;
			  one(rhsit->multiplier);
			  for(unsigned int i=0; i<terms_from_yp.size(); ++i) {
					if(tr.equal_subtree(terms_from_yp[i].begin(), rhsit)) {
						 rhs[i]=rhsmult;
						 found_in_basis=true;
						 break;
						 }
					}
			  if(!found_in_basis) {
					txtout << "rhs contains a term not present in the basis" << std::endl;
					return l_error;
					}
			  }
		 }

	// debugout << "linear problem constructed" << std::endl;
	// for(unsigned int i=0; i<coefficient_matrix.size(); ++i) {
	// 	for(unsigned int j=0; j<coefficient_matrix[i].size(); ++j)
	// 		debugout << coefficient_matrix[i][j] << " ";
	// 	debugout << " " << rhs[i] << std::endl;
	// 	}
	// Now decompose 

	if(rhsit->is_zero()) {
		 // debugout << "rhs is identically zero" << std::endl;
		 exptree res;
		 res.set_head(str_node("\\comma"));
		 for(unsigned int i=0; i<coefficient_matrix[0].size(); ++i) 
			  res.append_child(res.begin(), str_node("1"))->multiplier=rat_set.insert(0).first;
		 tr.replace(it, res.begin());
		 expression_modified=true;
		 }
	else {
		 // debugout << "doing gaussian elimination" << std::endl;
		 if(linear::gaussian_elimination_inplace(coefficient_matrix, rhs)) {
			 // for(unsigned int i=0; i<coefficient_matrix.size(); ++i) {
			 // 		for(unsigned int j=0; j<coefficient_matrix[i].size(); ++j)
			 // 			 debugout << coefficient_matrix[i][j] << " ";
			 // 		debugout << " = " << rhs[i] << std::endl;
			 // 		}
			  
			  exptree res;
			  res.set_head(str_node("\\comma"));
			  for(unsigned int i=0; i<coefficient_matrix[0].size(); ++i) 
					res.append_child(res.begin(), str_node("1"))->multiplier=rat_set.insert(rhs[i]).first;
			  it=tr.replace(it, res.begin());
			  expression_modified=true;
			  }
		 else {
			  txtout << "decomposing impossible" << std::endl;
//		tr.print_recursive_treeform(txtout, it);
			  return l_error;
			  }
		 }
	

	return l_applied;
	}



