/* 

   $Id: select.hh,v 1.8 2006/06/05 21:54:01 peekas Exp $

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

#ifndef select_hh_
#define select_hh_

#include "algorithm.hh"

/* class select : public algorithm {
	public:
		select(exptree&, iterator&);

		virtual void     description() const;
		virtual bool     can_apply(sibling_iterator, sibling_iterator);
		virtual iterator apply(sibling_iterator, sibling_iterator);
};

class unselect : public algorithm {
	public:
		unselect(exptree&, iterator&);

		virtual void     description() const;
		virtual bool     can_apply(sibling_iterator, sibling_iterator);
		virtual iterator apply(sibling_iterator, sibling_iterator);
};
*/

class pop : public algorithm {
	public:
		pop(exptree&, iterator);
			
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class amnesia : public algorithm {
	public:
		amnesia(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

#endif
