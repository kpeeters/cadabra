/* 

	Cadabra: a field-theory motivated computer algebra system.
	Copyright (C) 2001-2010  Kasper Peeters <kasper.peeters@aei.mpg.de>

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

#ifndef convert_hh_
#define convert_hh_

#include "algorithm.hh"

class frommath : public algorithm {
	public:
		frommath(exptree&, iterator);

		virtual bool     can_apply(sibling_iterator, sibling_iterator);
		virtual result_t apply(sibling_iterator&, sibling_iterator&);
};

class frommaple : public algorithm {
	public:
		frommaple(exptree&, iterator);

		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class run : public algorithm {
	public:
		run(exptree&, iterator);

		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		result_t         apply(iterator&, std::string program_name, bool mapleout);
};

class maxima : public algorithm {
	public:
		maxima(exptree&, iterator);

		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		result_t         apply(iterator&, std::string program_name, bool mapleout);

		static const char* max_to_cad[][2];
};

class maple : public algorithm {
	public:
		maple(exptree&, iterator);

		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		result_t         apply(iterator&, std::string program_name, bool mapleout);

		static const char* maple_to_cad[][2];
};

/*
class tomath : public algorithm {
	public:
		tomath(exptree&, iterator);
		
		virtual bool     can_apply(iterator);
		virtual iterator apply(iterator&);
};
*/

#endif
