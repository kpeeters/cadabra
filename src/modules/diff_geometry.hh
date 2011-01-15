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

#ifndef diff_geometry_hh_
#define diff_geometry_hh_

#include "props.hh"
#include "storage.hh"
#include "algorithm.hh"

/// Coordinate-free differential geometry
namespace diff_geometry {
	void register_properties();
	void register_algorithms();
};

class DifferentialForm : public property {
	public:
		virtual ~DifferentialForm() {};
		virtual std::string name() const;
};

#endif
