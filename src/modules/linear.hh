/* 

   $Id: linear.hh,v 1.11 2006/07/28 14:33:31 peekas Exp $

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
