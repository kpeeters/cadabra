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

		iterator        conditions;

		exptree_comparator comparator;
		std::vector<bool>  lhs_contains_dummies, rhs_contains_dummies;

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
