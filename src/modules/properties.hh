/* 

   $Id: properties.hh,v 1.7 2006/06/05 21:54:01 peekas Exp $

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
