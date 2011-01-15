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

#ifndef properties_hh_k_
#define properties_hh_k_

#include "algorithm.hh"

class extract_properties : public algorithm {
	public:
		extract_properties(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

//		bool has_property(iterator, const std::string&) const;
//		// FIXME: THIS one is very dangerous, since we CANNOT just decide this based on the name
//		bool has_property(nset_t::iterator, const std::string&) const;
//		bool has_property(const std::string& key, const std::string& val) const;
};

#endif
