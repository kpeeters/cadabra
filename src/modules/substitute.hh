/* 

   $Id: substitute.hh,v 1.26 2007/04/23 16:18:17 peekas Exp $

	Cadabra: an extendable open-source symbolic tensor algebra system.
	Copyright (C) 2001-2006  Kasper Peeters <kasper.peeters@aei.mpg.de>

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

#ifndef substitute_hh_
#define substitute_hh_

#include "algorithm.hh"
#include "algebra.hh"

// STL comparators are objects which are not supposed to carry local
// data with them. They get created afresh by the stl algorithms; you
// pass the 'type' rather than the 'object'.
//
// Therefore, we introduce our own equal_subtree here, which does not
// do things this way but instead allows us to carry global information
// along.

class substitute : public algorithm {
	public:
		substitute(exptree& tr, iterator it);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
	private:
		unsigned int    use_rule;
		std::vector<sibling_iterator> factor_locations;
		std::vector<int>              factor_moving_signs;

		bool match_subproduct(sibling_iterator lhs, sibling_iterator tofind, sibling_iterator st);
		bool satisfies_conditions();
		bool equal_subtree(exptree::iterator i1, exptree::iterator i2);
		iterator        conditions;

		// Comparator:
		enum match_t { node_match, subtree_match, no_match };
		match_t compare(exptree::iterator&, exptree::iterator&, bool nobrackets=false);

		// Maps for replacement of nodes (indices, patterns) and subtrees (object patterns) respectively.
		typedef std::map<exptree, exptree, tree_exact_less_no_wildcards_mod_prel_obj>  replacement_map_t;
		typedef std::map<nset_t::iterator, exptree::iterator, nset_it_less> subtree_replacement_map_t;
		replacement_map_t         replacement_map;
		subtree_replacement_map_t subtree_replacement_map;

		std::vector<bool> lhs_contains_dummies, rhs_contains_dummies;

		stopwatch tmr;
		bool start_reporting_outside;

		// For object swap testing routines:
		prodsort    prodsort_;
};

class vary : public algorithm {
	public:
		vary(exptree& tr, iterator it);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class take_match : public algorithm {
	public:
		take_match(exptree& tr, iterator it);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class replace_match : public algorithm {
	public:
		replace_match(exptree& tr, iterator it);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class simple_rename : public algorithm {
	public:
		simple_rename(exptree& tr, iterator it);
			
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
	protected:
		sibling_iterator from_, to_;

//		void rename_existing_dummies(iterator& st, nset_t::iterator to_name) const;
};

class index_rename : public algorithm {
	public:
		index_rename(exptree& tr, iterator it);
			
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
	protected:
		sibling_iterator from_, to_;
		bool             relabel_numbered_indices;

		void rename_existing_dummies(iterator& st, nset_t::iterator to_name) const;
};

#endif
