/* 

	Cadabra: a field-theory motivated computer algebra system.
	Copyright (C) 2001-2011  Kasper Peeters <kasper.peeters@aei.mpg.de>

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
