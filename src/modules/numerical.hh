/* 

   $Id: numerical.hh,v 1.9 2007/11/27 23:20:46 peekas Exp $

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

#ifndef numerical_hh_
#define numerical_hh_

#include "props.hh"
#include "algorithm.hh"

/// Numerical routines and properties of numerical indices
namespace numerical {
	void register_properties();
};

class Integer : public property {
	public:
		virtual ~Integer() {};
		virtual std::string name() const;
		virtual bool parse(exptree&, exptree::iterator, exptree::iterator, keyval_t&);
		virtual void display(std::ostream&) const;
		virtual std::string unnamed_argument() const { return "range"; };

		exptree from, to, difference;
};

class NumericalFlat : virtual public property {
	public:
		virtual std::string name() const;
};

class numerical_flatten : public algorithm {
	public:
		numerical_flatten(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};


#endif
