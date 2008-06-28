/* 

   $Id: tableaux.hh,v 1.16 2007/08/04 15:27:13 peekas Exp $

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

#ifndef tableau_hh_
#define tableau_hh_

#include "manipulator.hh"
#include "combinatorics.hh"
#include "props.hh"
#include "youngtab.hh"
#include "algebra.hh"
#include "dummies.hh"

/// Young tableaux functions in the manipulator
namespace tableaux {
	void register_properties();
	void register_algorithms();
};

class Tableau : public property {
	public:
		virtual ~Tableau() {};
		virtual std::string name() const;
		virtual bool parse(exptree&, exptree::iterator, exptree::iterator, keyval_t&);

		int dimension;
};

class FilledTableau : public property {
	public:
		virtual ~FilledTableau() {};
		virtual std::string name() const;
		virtual bool parse(exptree&, exptree::iterator, exptree::iterator, keyval_t&);

		int dimension;
};

class tabdimension : public algorithm {
	public:
		tabdimension(exptree&, iterator it);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		const Tableau *tab;
		const FilledTableau *ftab;
		int dimension;
};

class tab_basics : public algorithm {
	public:
		tab_basics(exptree&, iterator);

		typedef yngtab::filled_tableau<unsigned int> uinttab_t;
		typedef yngtab::tableaux<uinttab_t>          uinttabs_t;

		/// Convert an exptree to a numerical Young tableau, using num_to_it below.
		void tree_to_numerical_tab(iterator, uinttab_t& );
		unsigned int find_obj(const exptree& other);

		/// The inverse, converting tableaux to exptree objects attached as children of the iterator.
		void tabs_to_tree(uinttabs_t&, iterator, iterator, bool even_only);
		void tabs_to_singlet_rules(uinttabs_t&, iterator);
		
		std::vector<exptree::iterator> num_to_it;
};

class tabcanonicalise : public tab_basics {
	public:
		tabcanonicalise(exptree&, iterator it);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class tabstandardform : public tab_basics {
	public:
		tabstandardform(exptree&, iterator it);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class lr_tensor : public tab_basics  {
	public:
		lr_tensor(exptree&, iterator it);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		sibling_iterator tab1, tab2;

	private:
		void do_tableau(iterator&);
		void do_filledtableau(iterator&);
};

class young_project_product : public algorithm {
	public:
		young_project_product(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class decompose_product : public algorithm {
	public:
		decompose_product(exptree& tr, iterator it);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
	private:
		typedef young_project::name_tab_t  sibtab_t;
		typedef yngtab::tableaux<sibtab_t> sibtabs_t;
		typedef young_project::pos_tab_t   numtab_t;
		typedef yngtab::tableaux<numtab_t> numtabs_t;

		const Indices *indices_equivalent(iterator it) const;
		void fill_asym_ranges(TableauBase::tab_t& tab, int offset, combin::range_vector_t&);
		void project_onto_initial_symmetries(exptree& rep, iterator rr, young_project& yp,
														 const TableauBase *tt, iterator ff, 
														 int offset, const TableauBase::tab_t& thetab,
														 bool remove_traces);

		iterator               f1, f2;
		const TableauBase     *t1, *t2;
		TableauBase::tab_t     t1tab, t2tab;
		const Indices         *ind1, *ind2;
		unsigned int dim;
		yngtab::filled_tableau<iterator> nt1, nt2;

		combin::range_vector_t asym_ranges;
};

#endif
