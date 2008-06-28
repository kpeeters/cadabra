/* 

   $Id: dummies.cc,v 1.35 2008/06/22 12:23:33 peekas Exp $

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

#include "dummies.hh"
#include "stopwatch.hh"

void dummy::register_properties() 
	{
	properties::register_property(&create_property<Indices>);
	properties::register_property(&create_property<Coordinate>);
	}

std::string Coordinate::name() const
	{
	return "Coordinate";
	}

Indices::Indices()
	: position_free(true)
	{
	}

std::string Indices::name() const
	{
	return "Indices";
	}

property_base::match_t Indices::equals(const property_base *other) const
	{
	const Indices *cast_other = dynamic_cast<const Indices *>(other);
	if(cast_other) {
		 if(set_name == cast_other->set_name) {
			  if(parent_name == cast_other->parent_name && position_free == cast_other->position_free)
					return exact_match;
			  else
					return id_match;
			  }
		 return no_match;
		 }
	return property_base::equals(other);
	}

bool Indices::parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals)
	{
	keyval_t::const_iterator ki=keyvals.find("name");
	if(ki!=keyvals.end()) {
		 if(*ki->second->multiplier!=1) {
			  txtout << "Indices: use quotes to label names when they start with a number." << std::endl;
			  return false;
			  }
		set_name=*ki->second->name;
		}

	ki=keyvals.find("parent");
	if(ki!=keyvals.end()) 
		parent_name=*ki->second->name;

	ki=keyvals.find("position");
	if(ki!=keyvals.end()) {
		if(*ki->second->name!="free")
			position_free=false;
		}

	return true;
	}

rename_dummies::rename_dummies(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

void rename_dummies::description() const
	{
	txtout << "Rename dummies to canonical order." << std::endl;
	}

bool rename_dummies::can_apply(iterator st)
	{
	if(*st->name=="\\prod")
		return true;
	return false;
	}

algorithm::result_t rename_dummies::apply(iterator& st)
	{
	// First do a normal classify_indices both downwards and upwards.
	//
	index_map_t ind_free, ind_dummy, ind_free_up, ind_dummy_up;
	classify_indices(st, ind_free, ind_dummy);
	classify_indices_up(st, ind_free_up, ind_dummy_up);
	
	// Run through all indices once more, in order. If an index
	// occurs in the ind_dummy set, and there is no entry in repmap,
	// find the index type and get a new dummy. If the index already
	// occurs in repmap, reuse the new dummy stored there.
	//
	typedef std::map<exptree, exptree, tree_exact_less_mod_prel_obj> repmap_t;
	repmap_t    repmap;
	index_map_t added_dummies;

	exptree::index_iterator ii=tr.begin_index(st);
	while(ii!=tr.end_index(st)) {
		if(ind_dummy.find(exptree(ii))!=ind_dummy.end()) {
			repmap_t::iterator rmi=repmap.find(exptree(ii));
			if(rmi==repmap.end()) {
				const Indices *dums=properties::get<Indices>(ii);
				if(!dums)
					throw consistency_error("No index set for index "+*ii->name+" known.");

				exptree relabel=get_dummy(dums, &ind_free, &ind_free_up, &ind_dummy_up, &added_dummies);
				repmap.insert(repmap_t::value_type(exptree(ii),relabel));
				added_dummies.insert(index_map_t::value_type(relabel, ii));
				exptree::index_iterator tmp(ii);
				++tmp;
				tr.replace_index(ii, relabel.begin());
				ii=tmp;
				}
			else {
				exptree::index_iterator tmp(ii);
				++tmp;
				tr.replace_index(ii, (*rmi).second.begin());
				ii=tmp;
				}
			}
		else ++ii;
		}

	expression_modified=true;
	return l_applied;
	}
