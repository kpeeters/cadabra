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

class tree_dump : public algorithm {
	public:
		tree_dump(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
	private:
		void print_one(std::ostream&, sibling_iterator) const;
};

class memdump : public algorithm {
	public:
		memdump(exptree&, iterator);
			
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class depprint : public algorithm {
	public:
		depprint(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class eqs : public algorithm {
	public:
		eqs(exptree&, iterator);
		 
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class proplist : public algorithm {
	public: 
		proplist(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class print : public algorithm {
	public:
		print(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		virtual bool     is_output_module() const;
};

class indexlist : public algorithm {
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

class assert_or_exit : public algorithm {
	public:
		assert_or_exit(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class number_of_terms : public algorithm {
	public:
		number_of_terms(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

#endif
