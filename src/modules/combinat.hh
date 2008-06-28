/* 

   $Id: combinat.hh,v 1.4 2007/01/14 17:56:21 peekas Exp $

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

#ifndef combinat_hh_
#define combinat_hh_

#include "manipulator.hh"
#include "props.hh"
#include "combinatorics.hh"

/// Combinatorical algorithms

class permute : public algorithm {
	public:
		permute(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		combin::combinations<iterator> com;
		bool                   scan_through_range;
};

#endif
