/* 

   $Id: output.hh,v 1.16 2007/07/30 09:59:32 peekas Exp $

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

#ifndef output_hh_
#define output_hh_

#include "algorithm.hh"
#include "display.hh"

namespace output {
	void register_properties();
};

class LaTeXForm : virtual public property {
	public:
		virtual std::string name() const;
		virtual bool parse(exptree&, exptree::iterator, exptree::iterator, keyval_t&);
		virtual std::string unnamed_argument() const;
		std::string latex;
};

class tree_dump : public algorithm, public exptree_output {
	public:
		tree_dump(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
	private:
		void print_one(std::ostream&, sibling_iterator) const;
};

class memdump : public algorithm, public exptree_output {
	public:
		memdump(exptree&, iterator);
			
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class depprint : public algorithm, public exptree_output {
	public:
		depprint(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class eqs : public algorithm, public exptree_output {
	public:
		eqs(exptree&, iterator);
		 
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class proplist : public algorithm, public exptree_output {
	public: 
		proplist(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class print : public algorithm, public exptree_output {
	public:
		print(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class indexlist : public algorithm, public exptree_output {
	public:
		indexlist(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

//   class adjmatrix : public algorithm, public exptree_output {
//   	public:
//   		adjmatrix(exptree&, iterator);
//   		
//   		virtual void     description() const;
//   		virtual bool     can_apply(iterator);
//   		virtual result_t apply(iterator&);
//   };

class assert_or_exit : public algorithm, public exptree_output {
	public:
		assert_or_exit(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class number_of_terms : public algorithm, public exptree_output {
	public:
		number_of_terms(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

#endif
