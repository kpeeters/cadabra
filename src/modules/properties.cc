/* 

   $Id: properties.cc,v 1.24 2007/09/29 19:02:58 peekas Exp $

	Cadabra: an extendable open-source symbolic tensor algebra system.
	Copyright (C) 2002  Kasper Peeters <kasper.peeters@aei.mpg.de>

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
				properties::registered_properties.find(*it->name);
			if(pit==properties::registered_properties.end()) {
				throw consistency_error("Property \"" + *it->name + "\" not registered.");
//				tr.erase(it);
				}
			else {
				property_base *thepropbase=(*pit).second();
				exptree proptree(it);
				tr.erase(it);
				keyval_t keyvals;
				if(thepropbase->preparse_arguments(proptree.begin(), keyvals)==false) {
					txtout << "Failure parsing arguments of property " << *it->name << std::endl;
					return l_error;
					}
				if(thepropbase->parse(tr,st,proptree.begin(), keyvals)) {
					list_property *thelistprop=dynamic_cast<list_property *>(thepropbase);
					if(thelistprop) {                   // a list property
						std::vector<iterator> objs;
						if(*st->name=="\\comma") {
							sibling_iterator sib=tr.begin(st);
							txtout << "Assigning list property " << propname << " to ";
							while(sib!=tr.end(st)) {
								if(sib->fl.parent_rel!=str_node::p_property) {
									txtout << *sib->name;
									objs.push_back(sib);
									}
								++sib;
								if(sib!=tr.end(st)) txtout << ", ";
								else txtout << "." << std::endl;
								}
							}
						if(objs.size()<2) {
							txtout << "A list property cannot be assigned to a single object." << std::endl;
							return l_error;
							}
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
									txtout << *sib->name;
									properties::insert_prop(sib, theprop);
									theprop=0;
									}
								++sib;
								if(sib==tr.end(st)) txtout << "." << std::endl;
								else                txtout << ", ";
								}				
							}
						else {
							txtout << "Assigning property " << propname << " to " << *st->name << "." << std::endl;
							properties::insert_prop(st, theprop);
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

