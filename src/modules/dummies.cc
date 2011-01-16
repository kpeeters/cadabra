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

#include "dummies.hh"
#include "stopwatch.hh"

rename_dummies::rename_dummies(exptree& tr, iterator it)
	: algorithm(tr, it)
	{
	}

bool rename_dummies::can_apply(iterator st)
	{
	if(*st->name!="\\prod") 
		if(!is_single_term(st))
			return false;
	return true;
	}

algorithm::result_t rename_dummies::apply(iterator& st)
	{
	prod_wrap_single_term(st);
	
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
				const Indices *dums=properties::get<Indices>(ii, true);
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

	prod_unwrap_single_term(st);

	expression_modified=true;
	return l_applied;
	}
