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

#include "exchange.hh"
#include "modules/algebra.hh"
#include "modules/gamma.hh"
#include "modules/lie.hh"
#include "modules/numerical.hh"
#include <map>


// Find groups of identical tensors. 
//
int exchange::collect_identical_tensors(exptree& tr, exptree::iterator it,
													  std::vector<identical_tensors_t>& idts) 
	{
	assert(*it->name=="\\prod");

	int total_number_of_indices=0; // refers to number of indices on gamma matrices
	exptree::sibling_iterator sib=it.begin();
	while(sib!=it.end()) {
		unsigned int i=0;
		if(exptree::number_of_indices(sib)==0) {
			++sib;
			continue;
			}
		if(properties::get_composite<GammaMatrix>(sib)) {
			total_number_of_indices+=tr.number_of_indices(sib);
			++sib;
			continue;
			}
		
		// In case of spinors, the name may be hidden inside a Dirac bar.
		exptree::sibling_iterator truetensor=sib;
		const DiracBar *db=properties::get_composite<DiracBar>(truetensor);
		if(db) 
			truetensor=tr.begin(truetensor);

		// Compare the current tensor with all other tensors encountered so far.
		for(; i<idts.size(); ++i) {
			exptree::sibling_iterator truetensor2=idts[i].tensors[0];
			const DiracBar *db2=properties::get_composite<DiracBar>(truetensor2);
			if(db2) 
				truetensor2=tr.begin(truetensor2);
			
			if(subtree_equal(truetensor2, truetensor)) {
				// If this is a spinor, check that it's connected to the one already stored
				// by a Gamma matrix, or that it is connected directly.
				if(idts[i].spino) {
					exptree::sibling_iterator tmpit=idts[i].tensors[0];
					const GammaMatrix *gmnxt=0;
					const Spinor      *spnxt=0;
					// skip objects without spinor line
					do { 
						++tmpit;
						gmnxt=properties::get_composite<GammaMatrix>(tmpit);
						spnxt=properties::get_composite<Spinor>(tmpit);
						} while(gmnxt==0 && spnxt==0);
					if(tmpit==sib) {
//						txtout << "using fermi exchange" << std::endl;
						idts[i].extra_sign++;
						break;
						}
					if(gmnxt) {
//						txtout << "gamma next " << std::endl;
						int numind=exptree::number_of_indices(tmpit);
						// skip objects without spinor line
						do {
							++tmpit;
							gmnxt=properties::get_composite<GammaMatrix>(tmpit);
							spnxt=properties::get_composite<Spinor>(tmpit);
							} while(gmnxt==0 && spnxt==0);
						if(tmpit==sib) { // yes, it's a proper Majorana spinor pair.
//							txtout << "using fermi exchange with gamma " << numind << std::endl;
							if( ((numind*(numind+1))/2)%2 == 0 )
								idts[i].extra_sign++;
							break;
							}
						}
					}
				else break;
				}
			}
		if(i==idts.size()) {
			identical_tensors_t ngr;
			ngr.comm=properties::get_composite<SelfCommutingBehaviour>(sib, true);
			ngr.spino=properties::get_composite<Spinor>(sib);
			ngr.tab=properties::get_composite<TableauBase>(sib);
			ngr.traceless=properties::get_composite<Traceless>(sib);
			ngr.gammatraceless=properties::get_composite<GammaTraceless>(sib);
			ngr.extra_sign=0;
			ngr.number_of_indices=exptree::number_of_indices(truetensor);
			ngr.tensors.push_back(sib);
			ngr.seq_numbers_of_first_indices.push_back(total_number_of_indices);
			total_number_of_indices+=ngr.number_of_indices;
			if(ngr.spino==false || ngr.spino->majorana==true)
				idts.push_back(ngr);
			}
		else {
			idts[i].tensors.push_back(sib);
			idts[i].seq_numbers_of_first_indices.push_back(total_number_of_indices);
			total_number_of_indices+=idts[i].number_of_indices;
			}
		++sib;
		}
	return total_number_of_indices;
	}

unsigned int exchange::possible_singlets(exptree& tr, exptree::iterator it)
	{
	std::vector<identical_tensors_t> idts;
	collect_identical_tensors(tr, it, idts);

	if(idts.size()==0) return 1; // no indices, so this is a singlet already

	LiE::LiE_t lie;
	// Figure out the algebra from one of the indices.
	exptree::index_iterator indit=tr.begin_index(idts[0].tensors[0]);
	const numerical::Integer *iprop=properties::get<numerical::Integer>(indit, true);
	if(!iprop) 
		throw consistency_error("Need to know about the range of the " + *indit->name + " index.");

//	iprop->difference.print_recursive_treeform(txtout, iprop->difference.begin());
	unsigned int dims=to_long(*iprop->difference.begin()->multiplier);
//	std::cout << "*** " << dims << std::endl;
	if(dims%2==0) 
		lie.algebra_type=LiE::LiE_t::alg_D;
	else
		lie.algebra_type=LiE::LiE_t::alg_B;
	lie.algebra_dim=dims/2;
	
	lie.start();

	// Find the representation for each group of tensors, taking into
	// account their exchange symmetries.
	std::vector<LiE::LiE_t::reps_t> groupreps;

	for(unsigned int i=0; i<idts.size(); ++i) {
		LiE::LiE_t::reps_t single_tensor_rep;
		if(idts[i].number_of_indices>1) {
			assert(idts[i].tab); // Every tensor with 2+ indices needs a TableauSymmetry.
			TableauBase::tab_t thetab=idts[i].tab->get_tab(tr, idts[i].tensors[0], 0);
			
			std::vector<unsigned int> topleth;
			for(unsigned int rws=0; rws<thetab.number_of_rows(); ++rws)
				topleth.push_back(thetab.row_size(rws));
			lie.plethysm(topleth, single_tensor_rep, idts[i].traceless!=0, 
							 (thetab.selfdual_column==0)?0:thetab.selfdual_column/abs(thetab.selfdual_column));
			}
		else { // vector representation
			LiE::LiE_t::rep_t tmp;
			tmp.weight.resize(lie.algebra_dim,0);
			tmp.weight[0]=1;
			if(lie.algebra_type==LiE::LiE_t::alg_D && lie.algebra_dim==2)
				tmp.weight[1]=1;
//			txtout << tmp.weight[1] << std::endl;
			single_tensor_rep.push_back(tmp);
			}
		// If it's a spinor, still tensor with a spinor rep.
		if(idts[i].spino) {
			LiE::LiE_t::reps_t vectorpart=single_tensor_rep;
			LiE::LiE_t::reps_t spinorpart;
			LiE::LiE_t::rep_t spinorrep;
			spinorrep.weight.resize(lie.algebra_dim,0);
			spinorrep.weight[lie.algebra_dim-1]=1;
			spinorpart.push_back(spinorrep);
			lie.tensor(vectorpart, spinorpart, single_tensor_rep);
			if(idts[i].gammatraceless)
				lie.keep_largest_dim(single_tensor_rep);
			}
			
		if(idts[i].tensors.size()==1) { // we're done
			groupreps.push_back(single_tensor_rep);
			}
		else {
			LiE::LiE_t::reps_t multi_tensor_rep;
			int sign=idts[i].extra_sign + (idts[i].spino!=0);
			if(idts[i].comm && idts[i].comm->sign()==-1) 
				++sign;
			lie.alt_sym_tensor(idts[i].tensors.size(), single_tensor_rep, multi_tensor_rep, sign%2==0);
			groupreps.push_back(multi_tensor_rep);
			}
		}

	// Now tensor the whole lot together.
	LiE::LiE_t::reps_t result=groupreps[0], tmpstore;

	for(unsigned int i=0; i<groupreps.size()-1; ++i) {
		lie.tensor(result, groupreps[i+1], tmpstore);
		result=tmpstore;
		}
	unsigned int retval=lie.multiplicity_of_singlet(result);
	lie.stop();

	return retval;
	}

bool exchange::get_node_gs(exptree& tr, exptree::iterator it, std::vector<std::vector<int> >& gs)
	{
	std::vector<identical_tensors_t> idts;
	int total_number_of_indices=collect_identical_tensors(tr, it, idts);
	if(idts.size()==0) return true; // no indices, so nothing to permute

	// Make a strong generating set for the permutation of identical tensors.

	for(unsigned int i=0; i<idts.size(); ++i) {
		unsigned int num=idts[i].tensors.size();
		if(idts[i].comm)
			 if(idts[i].comm->sign()==0) continue;

		if(num>1) {
			std::vector<int> gsel(total_number_of_indices+2);

			for(unsigned int sobj=0; sobj<num-1; ++sobj) {
				for(int kk=0; kk<total_number_of_indices+2; ++kk)
					gsel[kk]=kk+1;
				
				// permutation of sobj & obj for all sobj and obj > sobj.
				for(unsigned int obj=sobj; obj<num-1; ++obj) {
					 for(unsigned int kk=0; kk<idts[i].number_of_indices; ++kk) 
						  std::swap(gsel[idts[i].seq_numbers_of_first_indices[obj]+kk], 
										gsel[idts[i].seq_numbers_of_first_indices[obj+1]+kk]);
					 if(idts[i].spino) {
						  assert(num==2); // FIXME: cannot yet do more than two fermions
						  if(idts[i].extra_sign%2==1) {
								std::swap(gsel[total_number_of_indices], gsel[total_number_of_indices+1]);
								}
						  }
					 if(idts[i].comm) {
						  if(idts[i].comm->sign()==-1) 
								std::swap(gsel[total_number_of_indices], gsel[total_number_of_indices+1]);
						  }
					 if(idts[i].spino && idts[i].number_of_indices==0) {
						  if(gsel[total_number_of_indices+1]==total_number_of_indices+1)
								return false;
						  }
					 else gs.push_back(gsel);
					 }
				 }
			 }
		 }

//  	for(unsigned int i=0; i<idts.size(); ++i) {
//  		int num=idts[i].tensors.size();
//  		if(num>1) {
//  			for(int t1=0; t1<num-1; ++t1) {
//  				for(int t2=t1+1; t2<num; ++t2) {
//  					std::vector<int> gsel(total_number_of_indices+2);
//  					for(int kk=0; kk<total_number_of_indices+2; ++kk)
//  						gsel[kk]=kk+1;
//  					
//  					if(idts[i].spino) {
//  						assert(num==2); // FIXME: cannot yet do more than two fermions
//  						if(idts[i].extra_sign%2==1) {
//  //							txtout << "extra sign" << std::endl;
//  							std::swap(gsel[total_number_of_indices], gsel[total_number_of_indices+1]);
//  							}
//  						}
//  					for(unsigned int kk=0; kk<idts[i].number_of_indices; ++kk) 
//  						std::swap(gsel[idts[i].seq_numbers_of_first_indices[t1]+kk], 
//  									 gsel[idts[i].seq_numbers_of_first_indices[t2]+kk]);
//  //						for(int kk=0; kk<total_number_of_indices+2; ++kk) {
//  //							txtout << gsel[kk] << " ";
//  //							}
//  //						txtout << std::endl;
//  //						txtout << "adding gs element" << std::endl;
//  
//  					if(idts[i].comm) {
//  						if(idts[i].comm->sign()==-1) {
//  //							txtout << "anticommuting" << std::endl;
//  							std::swap(gsel[total_number_of_indices], gsel[total_number_of_indices+1]);
//  							}
//  						else if(idts[i].comm->sign()==0)
//  							return false;
//  						}
//  					
//  					if(idts[i].spino && idts[i].number_of_indices==0) {
//  						if(gsel[total_number_of_indices+1]==total_number_of_indices+1)
//  							return false;
//  						}
//  					else gs.push_back(gsel);
//  					}
//  				}
//  			}
//  		}
	return true;
	}

bool operator<(const exchange::tensor_type_t& one, const exchange::tensor_type_t& two)
	{
	if(*one.name < *two.name) return true;
	if(one.name == two.name)
		if(one.number_of_indices < two.number_of_indices) return true;
	return false;
	}
