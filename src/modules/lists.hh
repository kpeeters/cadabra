/* 

   $Id: lists.hh,v 1.7 2006/10/15 09:33:06 peekas Exp $

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

#ifndef lists_hh_
#define lists_hh_

#include "algorithm.hh"

class length : public algorithm {
	public:
		length(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class take : public algorithm {
	public:
		take(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		enum { t_single, t_list, t_sequence } take_type;
		std::vector<long> nums;
};

class range : public algorithm {
	public:
		range(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		int from, to, num;
		exptree sym, pat;
};

class inner : public algorithm {
	public:
		inner(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class list_sum : public algorithm {
	public:
		list_sum(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class coefficients : public algorithm {
	public:
		coefficients(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		exptree obj;
};

// class symbolic_inner : public algorithm {
// 	public:
// 		symbolic_inner(exptree&, iterator);
// 
// 		virtual void     description() const;
// 		virtual bool     can_apply(iterator);
// 		virtual result_t apply(iterator&);
// };

#endif
