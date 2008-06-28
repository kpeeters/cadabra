#include <algorithm>
#include <string>
#include <iostream>
#include "tree.hh"

using namespace std;

int main(int, char **)
   {
   tree<string> tr;
   tree<string>::iterator top, one, two, loc, banana;

	top=tr.begin();
   one=tr.insert(top, "one");
   two=tr.append_child(one, "two");
	tr.append_child(two, "apple");
	banana=tr.append_child(two, "banana");
	tr.append_child(banana,"cherry");
	tr.append_child(two, "peach");
   tr.append_child(one,"three");

	loc=find(tr.begin(), tr.end(), "two");
	if(loc!=tr.end()) {
      tree<string>::iterator nod=tr.begin(loc);
      tree<string>::iterator end=tr.end(loc);
	   while(nod!=end) {
			if(tr.number_of_children(nod)==0) {
				tree<string>::iterator upwards=nod;
				do {
					std::cout << *upwards << " ";
					upwards=tr.parent(upwards);
					} while(tr.is_valid(upwards));
				std::cout << std::endl;
				}
         ++nod;
         }
	   }
   }
