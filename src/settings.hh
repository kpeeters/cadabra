
#ifndef settings_hh__
#define settings_hh__

#include "algorithm.hh"
#include "props.hh"

/// Global settings
namespace settings {
	void register_properties();
};

class KeepHistory : public property {
	public:
		virtual        std::string name() const;
		bool           parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t&);
		virtual void   display(std::ostream&) const;
		virtual std::string unnamed_argument() const { return "set"; };

		bool value;
};

class DefRules : public property {
	public:
		virtual bool parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals);
		virtual std::string unnamed_argument() const;

		exptree rules;
};

class PreDefaultRules : public DefRules {
	public:
		virtual std::string name() const;
};

class PostDefaultRules : public DefRules {
	public:
		virtual std::string name() const;
};

#endif
