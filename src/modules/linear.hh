/* 

	Cadabra: a field-theory motivated computer algebra system.
	Copyright (C) 2001-2009  Kasper Peeters <kasper.peeters@aei.mpg.de>

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

#ifndef linear_hh_
#define linear_hh_

#include "manipulator.hh"
#include "props.hh"

/// Linear algebra
namespace linear {
	bool gaussian_elimination(const std::vector<std::vector<multiplier_t> >&, const std::vector<multiplier_t>& );
	bool gaussian_elimination_inplace(std::vector<std::vector<multiplier_t> >&, std::vector<multiplier_t>& );
};

/// Solve a system of linear equations.
class lsolve : public algorithm {
	public:
		lsolve(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

/// Decompose a tensor monomial on a basis of monomials constructed before.
class decompose : public algorithm {
	public:
		decompose(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

	protected:
		void add_element_to_basis(exptree&, exptree::iterator);
		std::vector<exptree>                    terms_from_yp;
		std::vector<std::vector<multiplier_t> > coefficient_matrix;

};


#endif
