/* 

   $Id: dummies.hh,v 1.17 2008/06/22 12:23:33 peekas Exp $

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

#ifndef dummies_hh_
#define dummies_hh_

#include "algorithm.hh"
#include "props.hh"

namespace dummy {
	void register_properties();
};

class Coordinate : public property {
	public:
		virtual std::string name() const;
};

class Indices : public list_property {
	public:
		Indices();
		virtual bool parse(exptree&, exptree::iterator, exptree::iterator, keyval_t&);
		virtual std::string name() const;
		virtual std::string unnamed_argument() const { return "name"; };
		virtual match_t equals(const property_base *) const;
		
		std::string set_name, parent_name;
		bool        position_free;
};

//class dummies : public algorithm {
//	public:
//		dummies(exptree&, iterator);
//
//		virtual void     description() const;
//		virtual bool     can_apply(iterator);
//		virtual result_t apply(iterator&);
//
//		// Return a dummy index not yet occurring in the product pointed to by the iterators.
////		std::string      get_dummy(const std::string&, iterator, iterator) const;
//
//		// Return the class name, if known, of the index pointed to by the iterator.
//		nset_t::iterator dummy_class(iterator) const;
//	private:
////		iterator alldum;
//};

class rename_dummies : public algorithm {
	public:
		rename_dummies(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};


#endif
