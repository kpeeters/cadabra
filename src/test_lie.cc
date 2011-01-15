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

#include "modules/lie.hh"
#include <iostream>
#include <cassert>

using namespace LiE;

void print_reps(const std::vector<LiE_t::rep_t>& res)
	{
	for(unsigned int i=0; i<res.size(); ++i) {
		if(i>0) std::cout << " + ";
		std::cout << res[i].multiplicity << "X[";
		for(unsigned int j=0; j<res[i].weight.size(); ++j) {
			if(j>0) std::cout << ", ";
			std::cout << res[i].weight[j];
			}
		std::cout << "]";
		}
	}

int main(int, char **)
	{
	LiE_t lie;

	lie.start();

	LiE_t::rep_t r1;
	r1.multiplicity=1;
	r1.weight.push_back(1);
	r1.weight.push_back(0);
	r1.weight.push_back(0);
	r1.weight.push_back(0);
	r1.weight.push_back(0);

	std::vector<LiE_t::rep_t> rr1, res, res2, res3;
	rr1.push_back(r1);

	lie.sym_tensor(4, rr1, res);
	print_reps(res);
	std::cout << std::endl 
				 << lie.multiplicity_of_singlet(res) << std::endl;
	assert(lie.multiplicity_of_singlet(res)==1);	
	
	lie.tensor(res, res, res2);
	print_reps(res2);
	std::cout << std::endl 
				 << lie.multiplicity_of_singlet(res2) << std::endl;
	assert(lie.multiplicity_of_singlet(res2)==3);	

	lie.keep_largest_dim(res);
	print_reps(res);
	std::cout << std::endl
				 << lie.dim(rr1) << std::endl;
	assert(lie.dim(rr1)==10);

	std::vector<unsigned int> tab;
	tab.push_back(2);
	tab.push_back(2);
	lie.plethysm(tab, rr1, res3, false);
	print_reps(res3);
	std::cout << std::endl
				 << lie.dim(res3) << std::endl;
	assert(lie.dim(res3)==825);

	lie.plethysm(tab, rr1, res3, true);
	print_reps(res3);
	std::cout << std::endl
				 << lie.dim(res3) << std::endl;
	assert(lie.dim(res3)==770);

	lie.stop();

	return 0;
	}
