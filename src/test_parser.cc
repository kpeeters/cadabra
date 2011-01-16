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

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include "preprocessor.hh"
#include "parser.hh"
#include "display.hh"

bool interrupted;

int main(int argc, char **argv)
	{
	interrupted=false;
	std::stringstream frompre1, frompre2, orig, check;
	preprocessor pp;
	parser       pa;

	try {
		std::cin >> pp;
		frompre1 << pp;
		std::cout << "preprocessor 1: " << frompre1.str() << std::endl;
		frompre1 >> pa;
		orig << pa;
		std::cout << "parser       1: " << orig.str() << std::endl;
		pa.tree.print_recursive_treeform(std::cout, pa.tree.begin()) << std::endl;
//		pa.tree.select(3,1);
		exptree_output eo(pa.tree, std::cout);
		eo.print_infix(pa.tree.begin());
		std::cout << std::endl;

		pp.erase();
		pa.erase();

		orig >> pp;
		frompre2 << pp;
		std::cout << "preprocessor 2: " << frompre2.str() << std::endl;
		std::stringstream again(frompre2.str().substr(12,frompre2.str().size()-13));
		std::cout << "feedback      : " << again.str() << std::endl;
		again >> pa;
		check << pa;
		std::cout << "parser       2: " << check.str() << std::endl;
		}
	catch(std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		exit(1);
		}

	if(orig.str()==check.str()) std::cout << "iteration is identical" << std::endl;
	else std::cout << "iteration is NOT identical:" << std::endl
						<< check.str() << std::endl;
	}

