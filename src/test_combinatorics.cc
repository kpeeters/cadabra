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

#include <string>
#include "combinatorics.hh"
#include <iostream>
#include <vector>

using namespace std;

using namespace combin;

void print(combinations<string> arr) 
	{
	std::cout << arr.size() << " combinations, " << arr.total_permutations() << " total:" << std::endl;
	for(unsigned int i=0; i<arr.size(); ++i) {
 		for(unsigned int j=0; j<arr[0].size(); j++)
 			cout << arr[i][j] << " ";
		cout << "\t" << ordersign(arr[i].begin(), arr[i].end(), arr[0].begin(), arr[0].end())
			  << "\t" << arr.multiplier(i) << endl;
 		}
	}

void print(symmetriser<string> arr) 
	{
	std::cout << arr.size() << " combinations:" << std::endl;
	for(unsigned int i=0; i<arr.size(); ++i) {
 		for(unsigned int j=0; j<arr[0].size(); j++)
 			cout << arr[i][j] << " ";
		cout << arr.signature(i) << std::endl;
 		}
	}

bool test_sub_problem_blocksize()
	{
	combinations<string> cm;
	cm.original.push_back("1");
	cm.original.push_back("2");
	cm.original.push_back("3");
	cm.original.push_back("4");
	cm.original.push_back("5");
	cm.original.push_back("6");
	cm.set_unit_sublengths();
	cm.permute();
	assert(cm.size()==720);
	cout << cm.size() << " in total,";
	cm.clear();

	cm.original.push_back("1");
	cm.original.push_back("2");
	cm.original.push_back("3");
	cm.original.push_back("4");
	cm.original.push_back("5");
	cm.original.push_back("6");
	cm.sub_problem_blocksize=2;
	cm.set_unit_sublengths();
	cm.permute();
	cout << " but only " << cm.size() << " in sub-problem mode." << endl;
	assert(cm.size()==8);
	print(cm);
	return true;
	}

bool test_non_unit_block_length()
	{
	combinations<string> cm;
	cm.block_length=2;
	cm.sublengths.push_back(1);
	cm.sublengths.push_back(1);
	cm.original.push_back("r1");
	cm.original.push_back("r2");
	cm.original.push_back("r3");
	cm.original.push_back("r4");
	cm.permute();
	print(cm);
	return true;
	}

bool test_input_asym()
	{
	combinations<string> sm;
	sm.original.push_back("0");
	sm.original.push_back("1");
	sm.original.push_back("2");
	range_t ran;
	ran.push_back(0);
	ran.push_back(1);
	sm.input_asym.push_back(ran);
	sm.set_unit_sublengths();
	sm.permute();
	print(sm);
	return true;
	}


bool test_sublengths()
	{
   combinations<string> sm;
	sm.original.push_back("a");
	sm.original.push_back("b");
	sm.original.push_back("c");
//	sm.original.push_back("6");
	sm.block_length=1;
	sm.sublengths.push_back(2);
	sm.sublengths.push_back(1);
	sm.permute();
	print(sm);
	return true;
	}

bool test_symmetriser()
	{
	symmetriser<string> sm;
	sm.original.push_back("r1");
	sm.original.push_back("r2");
	sm.original.push_back("r3");
	sm.original.push_back("r4");
	sm.original.push_back("r5");

	sm.block_length=1;
	sm.permutation_sign=-1;
	sm.permute_blocks.push_back(0);
	sm.permute_blocks.push_back(1);
	sm.apply_symmetry();

	sm.permute_blocks.clear();
	sm.permutation_sign=-1;
	sm.permute_blocks.push_back(2);
	sm.permute_blocks.push_back(3);
	sm.apply_symmetry();

	sm.permute_blocks.clear();
	sm.block_length=2;
	sm.permutation_sign=1;
	sm.permute_blocks.push_back(0);
	sm.permute_blocks.push_back(2);
	sm.apply_symmetry();

	print(sm);
	return true;
	}

bool test_val_permute()
	{
	symmetriser<string> sm;
	sm.original.push_back("a");
	sm.original.push_back("c");
	sm.original.push_back("d");
	sm.original.push_back("b");
	sm.value_permute.push_back("b");
	sm.value_permute.push_back("c");
	sm.value_permute.push_back("d");
	sm.permutation_sign=-1;
	sm.apply_symmetry();
	print(sm);
	return true;
	}

bool test_sym_asym()
	{
	symmetriser<string> sm;
	return true;
	}

int main(int, char **)
   {
	cout << "combinatorics.hh regression tests" << endl;
	
	cout << endl << "sub_problem_blocksize test:" << endl;
	test_sub_problem_blocksize();
	cout << endl;
	cout << endl << "sublengths test:" << endl;
	test_sublengths();
	cout << endl << "input-asym test:" << endl;
	test_input_asym();
	cout << endl << "non-unit block length test:" << endl;
	test_non_unit_block_length();
	cout << endl << "symmetriser test:" << endl;
	test_symmetriser();
	cout << endl << "val permute test:" << endl;
	test_val_permute();
	cout << endl;

	combinations<string> mmm;
	mmm.multiple_pick=false;//true;
	mmm.block_length=2;
	// output ranges
	mmm.sublengths.push_back(2);

	range_t asym_range1;

	// input
   mmm.original.push_back("Q");
   mmm.original.push_back("M8");
   mmm.original.push_back("M5");
   mmm.original.push_back("Z8");
   mmm.original.push_back("Z5");
   mmm.original.push_back("Z21"); 
//   mmm.original.push_back("Z42"); 

	mmm.permute();

	cout << "done" << endl;
	print(mmm);
	cout << mmm.size() << " elements" << endl;
   }
