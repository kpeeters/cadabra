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

#include "properties.hh"
#include "props.hh"

extract_properties::extract_properties(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void extract_properties::description() const
	{
	txtout << "Add properties to objects." << std::endl;
	}

bool extract_properties::can_apply(iterator)
	{
	return true;
	}

algorithm::result_t extract_properties::apply(iterator& st)
	{
	sibling_iterator it=tr.begin(st);
	while(it!=tr.end(st)) {
		if(it->fl.parent_rel==str_node::p_property) {
			std::string propname=*it->name;
			properties::registered_property_map_t::iterator pit=
				properties::registered_properties.store.find(*it->name);
			if(pit==properties::registered_properties.store.end()) {
				throw consistency_error("Property \"" + *it->name + "\" not registered.");
//				tr.erase(it);
				}
			else {
				property_base *thepropbase=(*pit).second();
				exptree proptree(it);
				tr.erase(it);
				keyval_t keyvals;
				if(thepropbase->preparse_arguments(proptree.begin(), keyvals)==false) 
					throw consistency_error("Failure parsing arguments of property "+*it->name+".");

				if(thepropbase->parse(tr,st,proptree.begin(), keyvals)) {
					list_property *thelistprop=dynamic_cast<list_property *>(thepropbase);
//					bool is_index=false;
//					if(dynamic_cast<Indices *>(thepropbase)!=0)
//						is_index=true;
					if(thelistprop) {                   // a list property
						std::vector<exptree> objs;
						if(*st->name=="\\comma") {
							sibling_iterator sib=tr.begin(st);
							txtout << "Assigning list property " << propname << " to $";
							eo->print_infix(txtout, st);
							txtout << "$." << std::endl;
							eo->newline(txtout);
							while(sib!=tr.end(st)) {
								if(sib->fl.parent_rel!=str_node::p_property) {
//									if(is_index) {
//										exptree tmp(sib);
////										std::cerr << "index" << std::endl;
//										tmp.begin()->fl.parent_rel=str_node::p_super;
//										objs.push_back(tmp);
//										tmp.begin()->fl.parent_rel=str_node::p_sub;
//										objs.push_back(tmp);
//										}
//									else 
										objs.push_back(exptree(sib));
									}
								++sib;
								}
							}
						if(objs.size()<2) 
							throw consistency_error("A list property cannot be assigned to a single object.");

						properties::insert_list_prop(objs, thelistprop);
						}
					else {                              // a normal property
						property *theprop=dynamic_cast<property *>(thepropbase);
						assert(theprop);
						theprop->core_parse(keyvals);
						if(*st->name=="\\comma") {
							sibling_iterator sib=tr.begin(st);
							txtout << "Assigning property " << propname << " to ";
							while(sib!=tr.end(st)) {
								if(theprop==0) { // create a new property for each object
									thepropbase=(*pit).second();
									thepropbase->parse(tr,st,proptree.begin(),keyvals);
									theprop    =dynamic_cast<property *>(thepropbase);
									assert(theprop);
									theprop->core_parse(keyvals);
									}
								if(sib->fl.parent_rel!=str_node::p_property) {
									properties::insert_prop(exptree(sib), theprop);
									if(eo) {
										txtout << "$";
										eo->print_infix(txtout, sib);
										txtout << "$";
										}
									theprop=0;
									}
								++sib;
								if(sib==tr.end(st)) {
									txtout << "." << std::endl;
									eo->newline(txtout);
									}
								else                txtout << ", ";
								}				
							}
						else {
							properties::insert_prop(exptree(st), theprop);
							txtout << "Assigning property " << propname;
							if( *st->name!="" ) {
								txtout << " to ";
								if(eo) {
									txtout << "$";
									eo->print_infix(txtout, st);
									txtout << "$";
									}
								}
							txtout << "." << std::endl;
							if(eo) eo->newline(txtout);
							}
						}
					}
				else {
					txtout << "Arguments of property \"" << *(proptree.begin()->name) 
							 << "\" incorrect, property ignored." << std::endl;
					}
				}
			break;
			}
		++it;
		}
	return l_applied;
	}

