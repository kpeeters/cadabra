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

#ifndef gamma_hh_
#define gamma_hh_

#include "algorithm.hh"
#include "algebra.hh"
#include "field_theory.hh"
#include "dummies.hh"

/// Dirac gamma matrix algebra
namespace gamma_algebra {
	void register_properties();
};

class GammaMatrix : public AntiSymmetric, public Matrix, virtual public property {
	public:
		virtual std::string name() const;
		virtual void        display(std::ostream&) const;
		virtual bool        parse(exptree&, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals);
		exptree metric;
};

class SigmaMatrix : public AntiSymmetric, public Matrix, virtual public property {
	public:
		virtual std::string name() const;
		virtual void        display(std::ostream&) const;
};

class SigmaBarMatrix : public AntiSymmetric, public Matrix, virtual public property {
	public:
		virtual std::string name() const;
		virtual void        display(std::ostream&) const;
};

class Spinor : public ImplicitIndex, virtual public property {
	public:
		Spinor();
		virtual ~Spinor() {};
		virtual std::string name() const;
		virtual void        display(std::ostream&) const;
		virtual bool        parse(exptree&, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals);

		int  dimension;
		bool weyl;
		enum { positive, negative } chirality;  // only in combination with weyl
		bool majorana;
};

class DiracBar : public Accent, public Distributable, virtual public property {
	public:
		virtual std::string name() const;
};

class GammaTraceless : public property {
	public:
		virtual ~GammaTraceless() {};
		virtual std::string name() const;
};

class join : public algorithm {
	public:
		join(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

		bool expand;
		std::vector<int>     only_expand;
		const GammaMatrix   *gm1, *gm2;
	private:
		void regroup_indices_(sibling_iterator, sibling_iterator, unsigned int,
									 std::vector<exptree>&, std::vector<exptree>& );
		void append_prod_(const std::vector<exptree>& r1, const std::vector<exptree>& r2, 
								unsigned int num1, unsigned int num2, unsigned int i, multiplier_t mult,
								exptree& rep, iterator loc);

		bool                use_generalised_delta_;
		exptree::iterator   gamma_name_;
		str_node::bracket_t gamma_bracket_;
};

class remove_gamma_trace : public algorithm {
	public:
		remove_gamma_trace(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);		
		
		sibling_iterator gind, gamma_loc, spinor_loc;
		bool gamma_first;
		int  pos;
	private:
		bool find_contraction();
};

class gammasplit : public algorithm {
	public:
		gammasplit(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);		

		bool on_back;
};

class projweyl : public algorithm {
	public:
		projweyl(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
	private:
		sibling_iterator gamma_loc_;
};

class multpauli : public algorithm {
	public:
		multpauli(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);
};

class rewrite_diracbar : public algorithm {
	public:
		rewrite_diracbar(exptree&, iterator);

		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);		
};

class fierz : public algorithm {
	public:
		fierz(exptree&, iterator);
		
		virtual void     description() const;
		virtual bool     can_apply(iterator);
		virtual result_t apply(iterator&);

	private:
		iterator        spin1, spin2, spin3, spin4;
		const Spinor   *prop1,*prop2,*prop3,*prop4;
		iterator        gam1, gam2;
		int             dim, spinordim;
		const Indices  *indprop;
};

#endif
