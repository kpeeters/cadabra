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

#include "tree.hh"
#include "tree_util.hh"

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <list>
#include <utility>
#include <stdexcept>

void status_message(const std::string& name,  bool res)
	{
	std::cout << name << "\t: " << (res?"passed":"failed") << std::endl;
	}

int compare_tree(const std::string& name, const tree<std::string>& tr, const std::string& exp) 
	{
	std::ostringstream str;
	kptree::print_tree_bracketed(tr, str);
	if(str.str()==exp) {
		status_message(name, true);
		return 1;
		}
	else {
		status_message(name, false);
		std::cout << "  expected:" << std::endl << exp << std::endl
					 << "  received:" << std::endl << str.str() << std::endl;
		return 0;
		}
	}

int compare_subtree(const std::string& name, const tree<std::string>& tr, tree<std::string>::iterator it,
						  const std::string& exp) 
	{
	std::ostringstream str;
	kptree::print_subtree_bracketed(tr, it, str);
	if(str.str()==exp) {
		status_message(name, true);
		return 1;
		}
	else {
		status_message(name, false);
		std::cout << "  expected:" << exp << std::endl
					 << "  received:" << str.str() << std::endl << std::endl;
		return 0;
		}
	}

int test1() 
	{
	tree<std::string> tst;
	return compare_tree("empty tree      ", tst, "");
	}

int test2()
	{
	int res=1;

	tree<std::string> tr;
	tree<std::string>::pre_order_iterator html, body, h1, h3, bh1, mv1, sometext;
	
	html=tr.set_head("html");
	tr.insert(html,"extra");
	body=tr.append_child(html, "body");
	h1  =tr.append_child(body, "h1");
	bh1 =tr.insert(h1,"before h1");
	sometext = tr.append_child(h1, "some text");
	tree<std::string>::sibling_iterator more_text=tr.append_child(body, "more text");
	
	res*=compare_tree("basic algorithms", tr, "extra\nhtml(body(before h1, h1(some text), more text))");

	tr.swap(bh1);
	res*=compare_tree("element swapping", tr, "extra\nhtml(body(h1(some text), before h1, more text))");
	tr.swap(h1);
	res*=compare_tree("swapping back   ", tr, "extra\nhtml(body(before h1, h1(some text), more text))");

	tree<std::string> copytree(h1);
	res*=compare_tree("copy constructor", copytree, "h1(some text)");

	tree<std::string>::pre_order_iterator it;
	it=std::find(tr.begin(),tr.end(),std::string("h1"));
	if(it==tr.end()) {
		status_message("stl find       ", false);
		res=0;
		}
	else res*=compare_subtree("stl find               ", tr, it, "h1(some text)");

	it=std::find(tr.begin(),tr.end(), std::string("kasper"));
	if(it!=tr.end()) {
		status_message("stl find 2       ", false);
		res=0;
		}
	else status_message("stl find 2         ", true);

	// depth & max_depth
	//
	if(tr.depth(sometext)==3) 
		status_message("depth            ", true);
	else {
		status_message("depth            ", false);
		res=0;
		}

	if(tr.max_depth(tr.begin())==0)
		status_message("max_depth 1      ", true);
	else {
		status_message("max_depth 1      ", false);
		res=0;
		}
	if(tr.max_depth(html)==3)
		status_message("max_depth 2      ", true);
	else {
		status_message("max_depth 2      ", false);
		res=0;
		}
	if(tr.max_depth()==3)
		status_message("max_depth 3      ", true);
	else {
		status_message("max_depth 3      ", false);
		res=0;
		}
	tree<std::string> tr2;
	if(tr2.max_depth()==-1)
		status_message("max_depth 4      ", true);
	else {
		status_message("max_depth 4      ", false);
		res=0;
		}


	return res;
	}

int test3()
	{
	int res=1;

	tree<std::string> tr;
	tree<std::string>::iterator l1,l2,l3,l4,l5,tr1;

	l1=tr.set_head("1");
	l2=tr.append_child(l1,"11");
	tr1=tr.append_child(l1,"12");
	tr.append_child(l1,"13");
	l3=tr.append_child(l1,"14");
	l4=tr.append_child(l2,"111");
	l5=tr.append_child(l3,"141");
	tr.append_child(l4,"1111");
	tr.append_child(l5,"1411");
	tr.append_child(tr1,"bad");
	
//	kptree::print_subtree_bracketed(tr, tr.begin(), std::cout);
//	std::cout << std::endl;
	tree<std::string>::fixed_depth_iterator fd=tr.begin_fixed(tr.begin(),3);
	if(tr.is_valid(fd) && *fd=="1111") status_message("fixed_depth_iterator 1", true);
	else {
		res=0;
		status_message("fixed_depth_iterator 1", false);
		}
	fd=tr.next_at_same_depth(fd);
	if(tr.is_valid(fd) && *fd=="1411") status_message("fixed_depth_iterator 2", true);
	else {
		res=0;
		status_message("fixed_depth_iterator 2", false);
		}
	fd=tr.next_at_same_depth(fd);
	if(tr.is_valid(fd)==false) status_message("fixed_depth_iterator 3", true);
	else {
		res=0;
		status_message("fixed_depth_iterator 3", false);
		}

	return res;
	}

int main(int, char **)
   {
	int result=1;
	result*=test1()*test2()*test3();

	if(result!=1) {
		std::cout << "*** Regression tests failed ***" << std::endl;
		return -1;
		}
	else {
		std::cout << "Regression tests passed." << std::endl;
		return 0;
		}
	}
