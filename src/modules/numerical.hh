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

#ifndef numerical_hh_
#define numerical_hh_

#include "props.hh"
#include "algorithm.hh"

/// Numerical routines and properties of numerical indices
namespace numerical {
	void register_properties();

	/// Property indicating that a symbolic object always takes integer values.
	/// Optionally takes a range over which it runs, which can be symbolic.
	class Integer : public property {
		public:
			virtual ~Integer() {};
			virtual std::string name() const;
			virtual bool parse(exptree&, exptree::iterator, exptree::iterator, keyval_t&);
			virtual void display(std::ostream&) const;
			virtual std::string unnamed_argument() const { return "range"; };
			
			exptree from, to, difference;
	};
	
	/// Property indicating that an operator is numerically flat, so that
	/// numerical factors in the argument can be taken outside.
	class NumericalFlat : virtual public property {
		public:
			virtual std::string name() const;
	};
	
	/// Algorithm to move numerical factors inside NumericalFlat objects outside.
	class numerical_flatten : public algorithm {
		public:
			numerical_flatten(exptree&, iterator);
			
			virtual void     description() const;
			virtual bool     can_apply(iterator);
			virtual result_t apply(iterator&);
	};
};

#endif
