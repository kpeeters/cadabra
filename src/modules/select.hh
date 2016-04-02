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

#ifndef select_hh_
#define select_hh_

#include <stddef.h>
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
