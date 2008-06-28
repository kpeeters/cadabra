/* 

   $Id: convert.hh,v 1.7 2006/06/05 21:54:00 peekas Exp $

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

#ifndef convert_hh_
#define convert_hh_

#include "algorithm.hh"

class frommath : public algorithm {
	public:
		frommath(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(sibling_iterator, sibling_iterator);
		virtual result_t apply(sibling_iterator&, sibling_iterator&);
};

class frommaple : public algorithm {
	public:
		frommaple(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class run : public algorithm {
	public:
		run(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		result_t         apply(iterator&, std::string program_name, bool mapleout);
};

/*
class tomath : public algorithm {
	public:
		tomath(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual iterator apply(iterator&);
};
*/

#endif
