
#include "settings.hh"

void settings::register_properties()
	{
	properties::register_property(&create_property<KeepHistory>);
	properties::register_property(&create_property<PreDefaultRules>);
	properties::register_property(&create_property<PostDefaultRules>);
	}

std::string KeepHistory::name() const
	{
	return "KeepHistory";
	}

bool KeepHistory::parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals)
	{
	keyval_t::const_iterator ki=keyvals.find("set");
	value=true;
	if(ki!=keyvals.end())
		if(*ki->second->name=="false")
			value=false;

	return true;
	}

void KeepHistory::display(std::ostream& str) const
	{
	str << name() << "(" << (value?"true":"false") << ")";
	}

std::string PreDefaultRules::name() const
	{
	return "PreDefaultRules";
	}

std::string PostDefaultRules::name() const
	{
	return "PostDefaultRules";
	}

std::string DefRules::unnamed_argument() const
	{
	return "rules";
	}

bool DefRules::parse(exptree& tr, exptree::iterator pat, exptree::iterator prop, keyval_t& keyvals)
	{
	if(pat!=tr.end() && (*pat->name).size()!=0) {
		txtout << name() << ": attached to '" << *pat->name << "' but can only be a global property." << std::endl;
		return false;
		}
	if(tr.number_of_children(prop)!=1) {
		txtout << name() 
				 << ": needs one argument, giving the list of default algorithms." 
				 << std::endl;
		return false;
		}
	exptree::sibling_iterator arg_node=tr.begin(prop);
	for(unsigned int i=0; i<tr.arg_size(arg_node); ++i) {
		exptree::sibling_iterator cur_command=tr.arg(arg_node,i);
		if(cur_command->is_inert_command()==false) {
			txtout << name() << ": argument " << i+1 << " is not an inert command." << std::endl;
			return false;
			}
		}
	rules.insert_subtree(rules.begin(),prop.begin());
	return true;
	}
