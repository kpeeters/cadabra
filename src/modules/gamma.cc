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


#include "algebra.hh"
#include "gamma.hh"
#include "combinatorics.hh"
#include "manipulator.hh"
#include "dummies.hh"
#include "field_theory.hh"
#include "numerical.hh"
#include <utility>
#include <algorithm>

void gamma_algebra::register_properties()
	{
	properties::register_property(&create_property<GammaMatrix>);
	properties::register_property(&create_property<SigmaMatrix>);
	properties::register_property(&create_property<SigmaBarMatrix>);
	properties::register_property(&create_property<Spinor>);
	properties::register_property(&create_property<GammaTraceless>);
	}

std::string GammaMatrix::name() const
	{
	return "GammaMatrix";
	}

void GammaMatrix::display(std::ostream& str) const
	{
	Matrix::display(str);
	}

bool GammaMatrix::parse(exptree&tr, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals)
	{
	keyval_t::iterator kv=keyvals.find("metric");
	if(kv!=keyvals.end()) {
		metric=exptree(kv->second);
		keyvals.erase(kv);
		}

	ImplicitIndex::parse(tr, pat, prop, keyvals);

//	kv=keyvals.find("delta");
//	if(kv!=keyvals.end()) delta=exptree(kv->second);
//
	return true;
	}

std::string SigmaMatrix::name() const
	{
	return "SigmaMatrix";
	}

void SigmaMatrix::display(std::ostream& str) const
	{
	Matrix::display(str);
	}

bool SigmaMatrix::parse(exptree&tr, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals)
	{
	return Matrix::parse(tr, pat, prop, keyvals);
	}

std::string SigmaBarMatrix::name() const
	{
	return "SigmaBarMatrix";
	}

void SigmaBarMatrix::display(std::ostream& str) const
	{
	Matrix::display(str);
	}

bool SigmaBarMatrix::parse(exptree&tr, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals)
	{
	return Matrix::parse(tr, pat, prop, keyvals);
	}


Spinor::Spinor()
	: dimension(10), weyl(true), chirality(positive), majorana(true)
	{
	}

std::string Spinor::name() const
	{
	return "Spinor";
	}

bool Spinor::parse(exptree& tr, exptree::iterator it, exptree::iterator prop, keyval_t& keyvals)
	{
	keyval_t::iterator ki=keyvals.find("dimension");
	if(ki!=keyvals.end()) {
		dimension=to_long(*ki->second->multiplier);
		keyvals.erase(ki);
		}
	else                  dimension=10;

	ki=keyvals.find("type");
	if(ki!=keyvals.end()) {
		if(*ki->second->name=="Weyl") {
			if(dimension%2!=0) {
				txtout << "Weyl spinors require the dimension to be even." << std::endl;
				return false;
				}
			weyl=true;
			}
		if(*ki->second->name=="Majorana") {
			weyl=false;
			if(dimension%8==2 || dimension%8==3 || dimension%8==4)
				majorana=true;
			else {
				txtout << "Majorana spinors require the dimension to be 2,3,4 mod 8." << std::endl;
				return false;
				}
			}
		if(*ki->second->name=="MajoranaWeyl") { 
			if(dimension%8==2) {
				txtout << "setting to MajoranaWeyl" << std::endl;
				weyl=true; majorana=true; 
				}
			else {
				txtout << "Majorana-Weyl spinors require the dimension to be 2 mod 8." << std::endl;
				return false;
				}
			}
		keyvals.erase(ki);
		}
	
	ki=keyvals.find("chirality");
	if(ki!=keyvals.end()) {
		if(*ki->second->name=="Positive") chirality=positive;
		if(*ki->second->name=="Negative") chirality=negative;
		keyvals.erase(ki);
		}

	ImplicitIndex::parse(tr, it, prop, keyvals);
	
	return true;
	}

std::string GammaTraceless::name() const
	{
	return "GammaTraceless";
	}

join::join(exptree& tr_, iterator it_)
	: algorithm(tr_, it_), expand(false), use_generalised_delta_(false)
	{
	}

void join::regroup_indices_(sibling_iterator gam1, sibling_iterator gam2,
									 unsigned int i, std::vector<exptree>& r1, std::vector<exptree>& r2) 
	{
	unsigned int num1=tr.number_of_children(gam1);
		
	unsigned int len1=0;
	unsigned int len2=0;
	sibling_iterator g1=tr.begin(gam1);
	while(len1<num1-i) {
		r1.push_back(*g1);
		++g1;
		++len1;
		}
	sibling_iterator g2=tr.begin(gam2);
	while(g2!=tr.end(gam2)) {
		if(len2>=i)
			r2.push_back(*g2);
		++g2;
		++len2;
		}
	if(i>0) {
		g2=tr.begin(gam2);
		g1=tr.end(gam1);
		--g1;
		len1=0;
		for(len1=0; len1<i; ++len1) {
			r1.push_back(*g1);
			r2.push_back(*g2);
			--g1;
			++g2;
			}
		}
	}

void join::append_prod_(const std::vector<exptree>& r1, const std::vector<exptree>& r2, 
								unsigned int num1, unsigned int num2, unsigned int i, multiplier_t mult,
								exptree& rep, iterator loc)
	{
	exptree::iterator gamma;

	bool hasgamma     =(num1-i>0 || num2-i>0);
	bool hasdelta     =(i>0);
	bool hasmoredeltas=(i>1 && !use_generalised_delta_);

	str_node::bracket_t subsbr=gamma_bracket_;
	if((hasgamma && hasdelta) || hasmoredeltas) {
		loc=rep.append_child(loc, str_node("\\prod", gamma_bracket_, (*loc).fl.parent_rel));
		loc->multiplier=rat_set.insert(mult).first;
		subsbr=str_node::b_none;
		}

	if(num1-i>0 || num2-i>0) {
		gamma=rep.append_child(loc, str_node(*gamma_name_->name, subsbr));
		for(unsigned int j=0; j<num1-i; ++j) 
			rep.append_child(gamma, r1[j].begin());
		for(unsigned int j=0; j<num2-i; ++j)
			rep.append_child(gamma, r2[j].begin()); //str_node(*r2[j].name, str_node::b_none, r2[j].fl.parent_rel));
		if(!hasdelta)
			gamma->multiplier=rat_set.insert(mult).first;
		}
	
	exptree::iterator delt;
	if(use_generalised_delta_ && i>0) {
		if(gm1->metric.size()==0)
			throw consistency_error("The gamma matrix property does not contain metric information.");

		delt=rep.append_child(loc, gm1->metric.begin());
		delt->fl.bracket=subsbr;
		tr.erase_children(delt);
		if(!hasgamma)
			delt->multiplier=rat_set.insert(mult).first;
		}

	for(unsigned int j=0; j<i; ++j) {
		if(!use_generalised_delta_) {
			if(gm1->metric.size()==0)
				throw consistency_error("The gamma matrix property does not contain metric information.");
			
			delt=rep.append_child(loc, gm1->metric.begin());
			delt->fl.bracket=subsbr;
			tr.erase_children(delt);
			}

		if(tree_exact_less(r1[j+num1-i], r2[j+num2-i]) || use_generalised_delta_) {
			rep.append_child(delt, r1[j+num1-i].begin()); 
			rep.append_child(delt, r2[j+num2-i].begin()); 
			}
		else {
			rep.append_child(delt, r2[j+num2-i].begin()); 
			rep.append_child(delt, r1[j+num1-i].begin()); 
			}
		}
	}

bool join::can_apply(iterator st)
	{
	if(*st->name=="\\prod") {
		sibling_iterator fc=tr.begin(st);
		while(fc!=tr.end(st)) {
			gm1=properties::get<GammaMatrix>(fc);
			if(gm1) {
				++fc;
				if(fc!=tr.end(st)) {
					gm2=properties::get<GammaMatrix>(fc);
					if(gm2) {
						expand=false;
						only_expand.clear();
						sibling_iterator it=args_begin();
						while(it!=args_end()) {
							if(*it->name=="expand")
								expand=true;
							else if(*it->name=="gendelta") 
								use_generalised_delta_=true;
							else if(it->is_rational()) {
								only_expand.push_back(to_long(*it->multiplier));
								}
							++it;
							}
						return true;
						}
					}
				}
			++fc;
			}
		}
	return false;
	}

algorithm::result_t join::apply(iterator& st)
	{
	assert(*st->name=="\\prod");
	sibling_iterator gam1=tr.begin(st);
	sibling_iterator gam2;
	while(gam1!=tr.end(st)) {
		const GammaMatrix *gm1=properties::get<GammaMatrix>(gam1);
		if(gm1) {
			gamma_name_=gam1;
			gam2=gam1; ++gam2;
			if(gam2!=tr.end(st)) {
				const GammaMatrix *gm2=properties::get<GammaMatrix>(gam2);
				if(gm2)
					break;
				}
			}
		++gam1;
		}
	if(gam1==tr.end(st)) {
		st=tr.end();
		return l_error;
		}

	gamma_bracket_=gam2->fl.bracket;

	exptree rep;
	sibling_iterator top=rep.set_head(str_node("\\sum"));
	
	// Figure out the dimension of the gamma matrix.
	long number_of_dimensions=-1; // i.e. not known.
	exptree::index_iterator firstind=tr.begin_index(gam1);
	while(firstind!=tr.end_index(gam1)) {  // select the maximum value; FIXME: be more refined...
		const numerical::Integer *ipr=properties::get<numerical::Integer>(firstind);
		if(ipr) {
			if(ipr->difference.begin()->is_integer()) {
				number_of_dimensions=std::max(number_of_dimensions, to_long(*ipr->difference.begin()->multiplier));
				}
			}
		else {
			number_of_dimensions=-1;
			break;
			}
		++firstind;
		}
	if(number_of_dimensions!=-1) {
		firstind=tr.begin_index(gam2);
		while(firstind!=tr.end_index(gam2)) {  // select the maximum value; FIXME: be more refined...
			const numerical::Integer *ipr=properties::get<numerical::Integer>(firstind);
			if(ipr) {
				if(ipr->difference.begin()->is_integer()) {
					number_of_dimensions=std::max(number_of_dimensions, to_long(*ipr->difference.begin()->multiplier));
					}
				}
			else {
				number_of_dimensions=-1;
				break;
				}
			++firstind;
			}
		}

	// iterators over the two index ranges
	unsigned int num1=tr.number_of_children(gam1);
	unsigned int num2=tr.number_of_children(gam2);
	for(unsigned int i=0; i<=std::min(num1, num2); ++i) {
		// Ignore gammas with more than 'd' indices.
		if(number_of_dimensions>0) {
			if(num1+num2 > number_of_dimensions+2*i) {
				continue;
				}
			}
		
		if(only_expand.size()!=0) {
			if(std::find(only_expand.begin(), only_expand.end(), (int)(num1+num2-2*i))==only_expand.end())
//			if((int)(num1+num2-2*i)!=only_expand)
				continue;
			}

		std::vector<exptree> r1, r2;
		regroup_indices_(gam1, gam2, i, r1, r2);

		multiplier_t mult=(combin::fact(multiplier_t(num1))*combin::fact(multiplier_t(num2)))/
			(combin::fact(multiplier_t(num1-i))*combin::fact(multiplier_t(num2-i))*combin::fact(multiplier_t(i)));
			
//		debugout << "join: contracting " << i << " indices..." << std::endl;
		if(!expand) {
			append_prod_(r1, r2, num1, num2, i, mult, rep, top);
			}
		else {
			combin::combinations<exptree> c1(r1);
			combin::combinations<exptree> c2(r2);
			if(num1-i>0)
				c1.sublengths.push_back(num1-i);
			if(num2-i>0) 
				c2.sublengths.push_back(num2-i);
			if(use_generalised_delta_ && i>0)
				c1.sublengths.push_back(i);
			else {
				for(unsigned int k=0; k<i; ++k)
					c1.sublengths.push_back(1); // the individual \deltas, antisymmetrise 'first' group
				}
			if(i>0) 
				c2.sublengths.push_back(i); // the individual \deltas, do not antisymmetrise again.

			// Collect information about which indices to write in implicit antisymmetric form.
			// FIXME: this should move into combinatorics.hh
			iterator it=args_begin();
			while(it!=args_end()) {
				if(*it->name=="\\comma") {
					sibling_iterator cst=tr.begin(it);
					combin::range_t asymrange1, asymrange2;
					while(cst!=tr.end(it)) {
						for(unsigned int i1=0; i1<r1.size(); ++i1) {
							if(subtree_exact_equal(r1[i1].begin(), cst, 0)) {
								asymrange1.push_back(i1);
								break;
								}
							}
						for(unsigned int i2=0; i2<r2.size(); ++i2) {
							if(subtree_exact_equal(r2[i2].begin(), cst, 0)) {
								asymrange2.push_back(i2);
								break;
								}
							}
						++cst;
						}
					c1.input_asym.push_back(asymrange1);
					c2.input_asym.push_back(asymrange2);
					}
				++it;
				}

			c1.permute();
			c2.permute();

			for(unsigned int k=0; k<c1.size(); ++k) {
				for(unsigned int l=0; l<c2.size(); ++l) {
					if(interrupted) {
						txtout << "join interrupted while producing GammaMatrix[" << num1+num2-2*i 
								 << "] terms." << std::endl;
						interrupted=false;
						st=tr.end();
						return l_error;
						}

					int sgn=
						combin::ordersign(c1[k].begin(), c1[k].end(), r1.begin(), r1.end())
						*combin::ordersign(c2[l].begin(), c2[l].end(), r2.begin(), r2.end());
					multiplier_t mul=1;
					if(use_generalised_delta_)
						mul=combin::fact(i);
					
					append_prod_(c1[k], c2[l], num1, num2, i, 
									 multiplier_t(c1.multiplier(k))*multiplier_t(c2.multiplier(l))*sgn*mul, rep, top);
					}
				}
			}
		}

	// Finally, replace the old product by the new sum of products.
	expression_modified=true;
	if(rep.number_of_children(rep.begin())==0) {
		multiply(st->multiplier,0);
		return l_applied;
		}
	else if(rep.number_of_children(rep.begin())==1) {
		rep.flatten(rep.begin());
		rep.erase(rep.begin());
		}

	if(tr.number_of_children(st)>2) { // erase one gamma, replace the other one
		multiply(rep.begin()->multiplier, *gam1->multiplier);
		multiply(rep.begin()->multiplier, *gam2->multiplier);
		tr.replace(tr.erase(gam1), rep.begin());
		}
	else {
		multiply(rep.begin()->multiplier, *st->multiplier);
		st = tr.replace(st, rep.begin());
		}

   cleanup_expression(tr, st);
   cleanup_nests(tr, st);
	return l_applied;
	}



gammasplit::gammasplit(exptree& tr, iterator it)
	: algorithm(tr, it), on_back(true)
	{
	}

bool gammasplit::can_apply(iterator it)
	{
	if(properties::get<GammaMatrix>(it))
		if(tr.number_of_children(it)>1) {
			on_back=true;
			if(has_argument("front")) on_back=false;
			else if(has_argument("back")) on_back=true;
			return true;
			}
	return false;
	}

algorithm::result_t gammasplit::apply(iterator& it)
	{
	// Make a new expression which is the 'join' of the result which we want.
	exptree work;
	work.set_head(str_node("\\expression"));
	iterator prodnode=work.append_child(work.begin(), str_node("\\prod"));
	iterator firstgam, secondgam;
	if(on_back) {
		firstgam =work.append_child(prodnode, it);
		secondgam=work.append_child(prodnode, *it);
		}
	else {
		secondgam=work.append_child(prodnode, *it);
		firstgam =work.append_child(prodnode, it);
		}
	sibling_iterator specind;
	if(on_back) {
		specind=work.end(firstgam);
		--specind;
		}
	else specind=work.begin(firstgam);
	work.append_child(secondgam, (iterator)(specind));
	work.erase(specind);
	
	join jn(work, work.end());
	const GammaMatrix *gm1=properties::get<GammaMatrix>(it);
	jn.gm1=gm1;
	jn.expand=true;
	jn.apply(prodnode);

	// Replace maximally-antisymmetric gamma with the product
   // in which one gamma is back-split.
	iterator maxgam=work.begin(prodnode);
	if(on_back) {
		specind=work.end(maxgam);
		--specind;
		}
	else specind=work.begin(maxgam);
	iterator newprod=work.insert(maxgam, str_node("\\prod"));
	sibling_iterator fr=maxgam, to=fr;
	++to;
	maxgam=work.reparent(newprod, fr, to);
	iterator splitgam;
	if(on_back) splitgam=work.append_child(newprod, *maxgam);
	else        splitgam=work.insert(maxgam, *maxgam);
	work.append_child(splitgam,(iterator)(specind)); 
	work.erase(specind);

	// Flip signs on all other terms.
	sibling_iterator other=work.begin(prodnode);
	++other;
	while(other!=work.end(prodnode)) {
		flip_sign(other->multiplier);
		++other;
		}
	it=tr.replace(it, prodnode);
	expression_modified=true;

	cleanup_expression(tr, it);
	cleanup_nests(tr, it);
	return l_applied;
	}


remove_gamma_trace::remove_gamma_trace(exptree& tr, iterator it)
	: algorithm(tr, it), gamma_first(false), pos(0)
	{
	}

bool remove_gamma_trace::find_contraction() 
	{
	index_map_t gamma_indices_free, gamma_indices_dummy;
	index_map_t spinor_indices_free, spinor_indices_dummy;

	classify_indices(gamma_loc,  gamma_indices_free, gamma_indices_dummy);
	classify_indices(spinor_loc, spinor_indices_free, spinor_indices_dummy);

	index_map_t contractions;
	determine_intersection(spinor_indices_free, gamma_indices_free, contractions);
	
	if(contractions.size() > 0) {
		gind=contractions.begin()->second;
		return true;
		}
	else return false;
	}

bool remove_gamma_trace::can_apply(iterator it)
	{
	if(*it->name=="\\prod") {
		gamma_first=false;
		const GammaMatrix *gm=0;
		const GammaTraceless *sp=0;
		sibling_iterator sit=tr.begin(it);
		while(sit!=tr.end(it)) {
			if(sp && !gm) {
				gm=properties::get<GammaMatrix>(sit);
				if(gm) {
					gamma_loc=sit;
					if(find_contraction()) return true; 
					// keep searching, maybe the spinor comes after the gamma
					}
				sp=0;
				}
			else if(gm && !sp) {
				sp=properties::get_composite<GammaTraceless>(sit);
				if(sp) {
					gamma_first=true;
					spinor_loc=sit;
					return find_contraction();
					}
				gm=0;
				}
			else if(!gm && !sp) {
				gm=properties::get<GammaMatrix>(sit);
				sp=properties::get_composite<GammaTraceless>(sit);
				if(gm) gamma_loc=sit;
				if(sp) spinor_loc=sit;
				}
			++sit;
			}
		}
	return false;
	}

algorithm::result_t remove_gamma_trace::apply(iterator& it)
	{
	// Find the contracting vector index.

//	FIXME: we need to do both the spinor in front and the spinor behind
//		the gamma matrix. This can be done by calling the routine twice, of course.
//									Also, we should move the test inside can_apply to avoid
//   doing a lot of work for nothing if it's not required. Then all_contractions will
//   probably be fine.

	if(tr.number_of_children(gamma_loc)==1) { // Nothing to be split, just remove
		zero(tr.parent(gamma_loc)->multiplier);
		}
	else {
		// Move this index to the back or front and do a split
		gammasplit gs(tr, tr.end());
		if(gamma_first) {
			if((tr.number_of_children(gamma_loc)-pos)%2 == 0)
				flip_sign(gamma_loc->multiplier);
			tr.move_before(tr.end(gamma_loc), gind);
			gs.on_back=true;
			}
		else {
			if(pos % 2 == 1)
				flip_sign(gamma_loc->multiplier);
			tr.move_before(tr.begin(gamma_loc), gind);
			gs.on_back=false;
			}
		iterator loc=gamma_loc;
		gs.apply(loc);
		// Erase the first term by gamma-tracelessness
		tr.erase(tr.begin(loc));
		distribute dist(tr, tr.end());
		dist.apply(it);
		eliminate_kronecker elim(tr, tr.end());
		elim.apply_recursive(it,false);
		}

	cleanup_expression(tr, it);
	cleanup_nests(tr, it);
	expression_modified=true;
	return l_applied;
	}

projweyl::projweyl(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

bool projweyl::can_apply(iterator st)
	{
	if(*st->name=="\\prod") {
		sibling_iterator it=tr.begin(st);
		unsigned int numgamma=0;
		while(it!=tr.end(st)) {
			const GammaMatrix *gm=properties::get<GammaMatrix>(it);
			if(gm) {
				const numerical::Integer *isint=properties::get<numerical::Integer>(tr.begin(gamma_loc_));
				if(isint) {
					 if(isint->difference.begin()->is_rational()) {
						  ++numgamma;
						  gamma_loc_=it;
						  }
					 }
				}
			++it;
			}
		if(numgamma==1)
			return true;
		}
	return false;
	}

algorithm::result_t projweyl::apply(iterator& st)
	{
	exptree rep;
	iterator prod=rep.insert(rep.begin(), str_node("\\prod"));
	iterator eps=rep.append_child(prod, str_node("\\epsilon"));
	unsigned int numind=rep.number_of_children(gamma_loc_);
	iterator gam;
	if(numind!=10)
		gam=rep.append_child(prod, str_node(*gamma_loc_->name));

	txtout << "projweyl: warning, using d=10 implicitly!" << std::endl;
	const Indices *dums=properties::get<Indices>(tr.begin(gamma_loc_));
	assert(dums);
	for(unsigned int i=0; i<10-numind; ++i) { // FIXME: don't hardcode dimensions
		exptree nm=get_dummy(dums, prod, st);
		iterator tmpit=rep.append_child(gam, nm.begin());
		tmpit->fl.bracket=str_node::b_none;
		tmpit->fl.parent_rel=str_node::p_sub;
		tmpit=rep.append_child(eps, nm.begin());
		tmpit->fl.bracket=str_node::b_none;
		tmpit->fl.parent_rel=str_node::p_sub;
		}
	sibling_iterator orig=gamma_loc_.begin();
	while(orig!=gamma_loc_.end()) {
		rep.append_child(eps, *orig);
		++orig;
		}
	int sign=1;
	if((numind*(numind+1)/2) % 2 == 0)
		sign=-1;

	multiply(prod->multiplier, sign/multiplier_t(combin::fact(10-numind)));

	expression_modified=true;
	if(numind==10) { // only the epsilon remains
		multiply(eps->multiplier, *prod->multiplier);
		rep.flatten(prod);
		rep.erase(prod);
		}

	multiply(rep.begin()->multiplier, *gamma_loc_->multiplier);
	tr.replace(gamma_loc_, rep.begin());

	return l_applied;
	}

// fermibilinear_sort::fermibilinear_sort(exptree& tr, iterator it)
// 	: algorithm(tr, it)
// 	{
// 	}
// 
// bool fermibilinear_sort::can_apply(iterator it)
// 	{
// 	if(*it->name=="\\fermibilinear") return true;
// 	return false;
// 	}
// 
// algorithm::result_t fermibilinear_sort::apply(iterator& it)
// 	{
// 	sibling_iterator sib=tr.begin(it);
// 	result_t res=l_no_action;
// 	sibling_iterator arg3=sib;
// 	++arg3; ++arg3;
// 
// 	// first fermion
// 	if(*sib->name=="\\prod") {
// 		sibling_iterator f1=tr.begin(sib);
// 		while(f1!=tr.end(sib)) {
// 			const Spinor *sp=properties::get<Spinor>(f1);
// 			if(!sp) {
// 				if(*arg3->name!="\\prod") {
// 					sibling_iterator prodnode=tr.insert(arg3, str_node("\\prod"));
// 					sibling_iterator arg3p1=arg3;
// 					++arg3p1;
// 					tr.reparent(prodnode, arg3, arg3p1);
// 					arg3=prodnode;
// 					}
// 				sibling_iterator f1n=f1;
// 				++f1n;
// 				tr.move_before(tr.begin(arg3), f1);
// 				f1=f1n;
// 				res=l_applied;
// 				}
// 			else ++f1;
// 			}
// 		}
// 	++sib;
// 	if(*sib->name=="\\prod") {
// 		sibling_iterator f1=tr.begin(sib);
// 		while(f1!=tr.end(sib)) {
// 			const Spinor *sp=properties::get<Spinor>(f1);
// 			if(!sp) {
// 				if(*arg3->name!="\\prod") {
// 					sibling_iterator prodnode=tr.insert(arg3, str_node("\\prod"));
// 					sibling_iterator arg3p1=arg3;
// 					++arg3p1;
// 					tr.reparent(prodnode, arg3, arg3p1);
// 					arg3=prodnode;
// 					}
// 				tr.append_child(arg3, f1);
// 				f1=tr.erase(f1);
// 				res=l_applied;
// 				}
// 			else ++f1;
// 			}
// 		}
// 
// 	if(res==l_applied)
// 		cleanup_sums_products(tr, it);
// 	return res;
// 	}


multpauli::multpauli(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

bool multpauli::can_apply(iterator it) 
	{
//	if(*it->name=="\\prod")
	return false;
	}

algorithm::result_t multpauli::apply(iterator& it)
	{
	return l_no_action;
	}


rewrite_diracbar::rewrite_diracbar(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

bool rewrite_diracbar::can_apply(iterator it) 
	{
	const DiracBar *db=properties::get<DiracBar>(it);
	if(db) {
		if(*tr.begin(it)->name=="\\prod") {
			sibling_iterator ch=tr.begin(tr.begin(it));
			const GammaMatrix *gam=properties::get<GammaMatrix>(ch);
			if(gam) {
				++ch;
				const Spinor *sp=properties::get_composite<Spinor>(ch);
				if(sp) return true;
				}
			}
		}
	return false;
	}

algorithm::result_t rewrite_diracbar::apply(iterator& it)
	{
	txtout << "Warning: assuming Minkowski signature." << std::endl;

	// \bar{\prod{\gamma_{a b} \epsilon}} -> \prod{\bar{\epsilon} \gamma_{a b}}

	sibling_iterator prodnode=tr.begin(it);
	sibling_iterator gamnode =tr.begin(prodnode);
	sibling_iterator spinnode=gamnode; 
	++spinnode;

	iterator newprod=tr.wrap(it, str_node("\\prod"));
	multiply(newprod->multiplier, *prodnode->multiplier);
	multiply(newprod->multiplier, *it->multiplier);
	one(prodnode->multiplier);
	one(it->multiplier);
	tr.move_after(it, (iterator)gamnode);
	tr.flatten(prodnode);
	tr.erase(prodnode);

	// n + (n-1) + .. 1 = 1/2 n (n+1) signs, of which the first 'n' are Minkowski.

	unsigned int n=tr.number_of_children(gamnode);
	if(n*(n+1)/2 % 2 != 0)
		flip_sign(newprod->multiplier);

	it=newprod;
	pushup_multiplier(it);
	cleanup_nests(tr, it);
					  
	expression_modified=true;
	return l_applied;
	}

fierz::fierz(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	if(number_of_args()!=1) throw constructor_error();

	if(*args_begin()->name!="\\comma") throw constructor_error();

	if(tr.number_of_children(args_begin())!=4) throw constructor_error();
	}

bool fierz::can_apply(iterator it) 
	{
	if(*it->name!="\\prod") return false;

	// Find a Dirac bar, and then continue inside the product
	// to find the gamma matrix, fermion and second fermi bilinear.

	sibling_iterator sib=tr.begin(it);
	const numerical::Integer *indit=0;
	while(sib!=tr.end(it)) {
		const DiracBar *db=properties::get_composite<DiracBar>(sib);
		if(db) {
//			txtout << "found db" << std::endl;
			spin1=sib;
			prop1=properties::get_composite<Spinor>(spin1);
			sibling_iterator ch=sib;
			const GammaMatrix *gmnxt=0;
			const Spinor      *spnxt=0;
			// Skip to next spinor-index carrying object
			do {
				++ch;
				gmnxt=properties::get_composite<GammaMatrix>(ch);
				spnxt=properties::get_composite<Spinor>(ch);
				} while(gmnxt==0 && spnxt==0);
			if(gmnxt) {
//				txtout << "found gam" << std::endl;
				// FIXME: should also work when there is a unit matrix in between.
				indit=properties::get_composite<numerical::Integer>(ch.begin());
				indprop=properties::get_composite<Indices>(ch.begin());
				if(!indit || !indprop) return false;
				dim=to_long(*indit->difference.begin()->multiplier);
				if(dim==1)
					return false;
				
				gam1=ch;
				// Skip to next spinor-index carrying object
				do {
					++ch;
					spnxt=properties::get_composite<Spinor>(ch);
					gmnxt=properties::get_composite<GammaMatrix>(ch);
					} while(gmnxt==0 && spnxt==0);
				prop2=spnxt;
				if(prop2) { // one fermi bilinear found.
//					txtout << "found spin2" << std::endl;
					spin2=ch;
					// Skip to next spinor-index carrying object
					do {
						++ch;
						spnxt=properties::get_composite<Spinor>(ch);
						gmnxt=properties::get_composite<GammaMatrix>(ch);
						} while(gmnxt==0 && spnxt==0);
					db=properties::get_composite<DiracBar>(ch);
					if(db) {
//						txtout << "found db2" << std::endl;
						spin3=ch;
						prop3=spnxt;
						// Skip to next spinor-index carrying object
						do {
							++ch;
							spnxt=properties::get_composite<Spinor>(ch);
							gmnxt=properties::get_composite<GammaMatrix>(ch);
							} while(gmnxt==0 && spnxt==0);
						if(gmnxt) {
//							txtout << "found gam2" << std::endl;
							gam2=ch;
							// Skip to next spinor-index carrying object
							do {
								++ch;
								spnxt=properties::get_composite<Spinor>(ch);
								gmnxt=properties::get_composite<GammaMatrix>(ch);
								} while(gmnxt==0 && spnxt==0);
							prop4=spnxt;
							if(prop4) {
//								txtout << "found spin4" << std::endl;
								spin4=ch;
								return true;
								}
							}
						}
					}
				}
			}
		++sib;
		}
	return false;
	}

algorithm::result_t fierz::apply(iterator& it)
	{
	sibling_iterator spt=args_begin().begin();

	// Catch terms with spinors in the right order.
	if(subtree_equal(tr.begin(spin1), spt)) {
		++spt;
		if(subtree_equal(spin2, spt)) {
			++spt;
			if(subtree_equal(tr.begin(spin3), spt)) {
				++spt;
				if(subtree_equal(spin4, spt)) {
//					txtout << "found term in right order" << std::endl;
					return l_no_action;
					}
				}
			}
		}

	// Catch terms with right spinors but wrong order.
	bool doit=false;
	spt=args_begin().begin();
	if(subtree_equal(tr.begin(spin1), spt)) {
		++spt;
		if(subtree_equal(spin4, spt)) {
			++spt;
			if(subtree_equal(tr.begin(spin3), spt)) {
				++spt;
				if(subtree_equal(spin2, spt)) {
//					txtout << "found term in wrong order" << std::endl;
					doit=true;
					}
				}
			}
		}
	if(!doit) return l_no_action;

//	txtout << "going to Fierz" << std::endl;

	exptree rep;
	rep.set_head(str_node("\\sum"));
	
	index_map_t ind_free, ind_dummy; 
	classify_indices(it, ind_free, ind_dummy);
	spinordim=(1 << dim/2);
	int maxind=dim;
	if(prop1->weyl || dim%2==1) 
		maxind/=2;

	for(int i=0; i<=maxind; ++i) {
		// Make a copy of this term, moving the gamma matrices into the 
		// first factor and inserting projector gamma matrices as well.
		exptree cpyterm;
		cpyterm.set_head(str_node("\\prod"));
		cpyterm.begin()->multiplier=it->multiplier;
		multiply(cpyterm.begin()->multiplier, multiplier_t(-1)/multiplier_t(spinordim));
		if(i>0)
			multiply(cpyterm.begin()->multiplier, multiplier_t(1)/multiplier_t(combin::fact<int>(i)));
		sibling_iterator cpit=tr.begin(it);

		// Copy and put the gammas and projector gammas in the right spot.
		iterator locgam1,  locgam2;  // locations of the projector gammas
		while(cpit!=tr.end(it)) {
			iterator tmpit;
			if(cpit==spin2) 
				tmpit=cpyterm.append_child(cpyterm.begin(), spin4);
			else if(cpit==spin4)
				tmpit=cpyterm.append_child(cpyterm.begin(), spin2);
			else tmpit=cpyterm.append_child(cpyterm.begin(), (iterator)cpit);

			if(cpit==gam1) {
				if(i>0) {
					locgam1=cpyterm.append_child(cpyterm.begin(), gam1);
					cpyterm.erase_children(locgam1);
					}
				cpyterm.append_child(cpyterm.begin(), gam2);
				}
			if(cpit==gam2) {
				locgam2=tmpit;
				if(i==0) cpyterm.erase(locgam2);
				else 		cpyterm.erase_children(locgam2);
				}
			
			++cpit;
			}

		// Insert the indices on the projector gammas.
		index_map_t ind_added;
		for(int j=1; j<=i; ++j) { 
			exptree newdum=get_dummy(indprop, &ind_free, &ind_dummy, &ind_added);
			iterator loc1=cpyterm.append_child(locgam1, newdum.begin());
			ind_added.insert(index_map_t::value_type(newdum, loc1));
			if(indprop->position_type==Indices::free)
				loc1->fl.parent_rel=str_node::p_sub;
			else
				loc1->fl.parent_rel=str_node::p_super;
			// Add the indices in opposite order in the second gamma matrix
			iterator loc2=cpyterm.insert_subtree(locgam2.begin(), newdum.begin());
			loc2->fl.parent_rel=str_node::p_sub;
			}
		
		rep.append_child(rep.begin(), cpyterm.begin());
		}

	it=tr.replace(it, rep.begin());
	cleanup_nests(tr, it);

	expression_modified=true;
	return l_applied;
	}
