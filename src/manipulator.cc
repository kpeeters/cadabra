/* 

   $Id: manipulator.cc,v 1.233 2008/06/28 09:44:33 peekas Exp $

	Cadabra: an extendable open-source symbolic tensor algebra system.
	Copyright (C) 2002  Kasper Peeters <kasper.peeters@aei.mpg.de>

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

#include <sstream>
#include <fstream>
#include <iomanip>
#include "modules/modules.hh"
#include "manipulator.hh"
#include "preprocessor.hh"
#include "settings.hh"
#include "parser.hh"
#include <stdexcept>

extern std::string defaults;

stream_end_error::stream_end_error()
	{
	}

exit_exception::exit_exception()
	{
	}

manipulator::algo_info::algo_info(std::auto_ptr<algorithm> (*cr)(exptree&, iterator))
	: create(cr), calls(0)
	{
	}

manipulator::manipulator()
	: output_format(exptree_output::out_plain), editing_equation(0), last_used_equation_number(0), 
	  utf8_output(getenv("CDB_USE_UTF8")), status_output(false), prompt_string(">")
	{
	properties::register_properties();
	settings::register_properties();

	// pertstring
	algorithms["@aticksen"]       =new algo_info(&create<aticksen>);
	algorithms["@riemannid"]      =new algo_info(&create<riemannid>);

	// tableaux
	tableaux::register_properties();
	algorithms["@lr_tensor"]         =new algo_info(&create<lr_tensor>);
	algorithms["@tabdimension"]      =new algo_info(&create<tabdimension>);
	algorithms["@tabcanonicalise"]   =new algo_info(&create<tabcanonicalise>);
	algorithms["@tabstandardform"]   =new algo_info(&create<tabstandardform>);
	algorithms["@decompose_product"] =new algo_info(&create<decompose_product>);
	algorithms["@young_project_product"]  =new algo_info(&create<young_project_product>);

	// algebra
	algebra::register_properties();
	algorithms["@distribute"]     =new algo_info(&create<distribute>); 
	algorithms["@expand_power"]   =new algo_info(&create<expand_power>);
	algorithms["@prodrule"]       =new algo_info(&create<prodrule>); 
	algorithms["@prodflatten"]    =new algo_info(&create<prodflatten>);
	algorithms["@sumflatten"]     =new algo_info(&create<sumflatten>);
	algorithms["@listflatten"]    =new algo_info(&create<listflatten>);
	algorithms["@remove_indexbracket"]    =new algo_info(&create<remove_indexbracket>);
	algorithms["@prodcollectnum"] =new algo_info(&create<prodcollectnum>);
	algorithms["@collect_terms"]  =new algo_info(&create<collect_terms>);
	algorithms["@collect_factors"]=new algo_info(&create<collect_factors>);
	algorithms["@factorise"]      =new algo_info(&create<factorise>);
	algorithms["@factor_in"]      =new algo_info(&create<factor_in>);
	algorithms["@factor_out"]     =new algo_info(&create<factor_out>);
	algorithms["@canonicalise"]   =new algo_info(&create<canonicalise>);
	algorithms["@reduce"]         =new algo_info(&create<reduce>);
	algorithms["@ratrewrite"]     =new algo_info(&create<ratrewrite>);
	algorithms["@canonicalorder"] =new algo_info(&create<canonicalorder>);
	algorithms["@acanonicalorder"]=new algo_info(&create<acanonicalorder>);
	algorithms["@prodsort"]       =new algo_info(&create<prodsort>);
	algorithms["@sumsort"]        =new algo_info(&create<sumsort>);
	algorithms["@spinorsort"]     =new algo_info(&create<spinorsort>);
//	algorithms["@subseq"]         =new algo_info(&create<subseq>);
//	algorithms["@drop"]           =new algo_info(&create<drop>);
	algorithms["@drop_weight"]    =new algo_info(&create<drop_weight>);
	algorithms["@keep_weight"]    =new algo_info(&create<keep_weight>);
	algorithms["@sym"]            =new algo_info(&create<sym>); 
	algorithms["@asym"]           =new algo_info(&create<asym>); 
	algorithms["@"]               =new algo_info(&create<eqn>); 
	algorithms["@indexsort"]      =new algo_info(&create<indexsort>); 
	algorithms["@asymprop"]       =new algo_info(&create<asymprop>);
	algorithms["@impose_asym"]    =new algo_info(&create<impose_asym>);
	algorithms["@young_project"]  =new algo_info(&create<young_project>);
	algorithms["@young_project_tensor"]  =new algo_info(&create<young_project_tensor>);
	algorithms["@keep_terms"]     =new algo_info(&create<keep_terms>);

	// linear
	algorithms["@lsolve"]         =new algo_info(&create<lsolve>);
	algorithms["@decompose"]      =new algo_info(&create<decompose>);

	// combinat
	algorithms["@permute"]        =new algo_info(&create<permute>);

	// convert
	algorithms["@from_math"]      =new algo_info(&create<frommath>);  // not documented yet
	algorithms["@from_maple"]     =new algo_info(&create<frommaple>); // not documented yet
	algorithms["@run"]            =new algo_info(&create<run>);

	// differential geometry
	diff_geometry::register_properties();

	// gamma
	gamma_algebra::register_properties();
	algorithms["@join"]              =new algo_info(&create<join>);
	algorithms["@projweyl"]          =new algo_info(&create<projweyl>);
	algorithms["@remove_gamma_trace"]=new algo_info(&create<remove_gamma_trace>); // not documented yet
	algorithms["@gammasplit"]        = new algo_info(&create<gammasplit>);
	algorithms["@rewrite_diracbar"]  = new algo_info(&create<rewrite_diracbar>);
	algorithms["@fierz"]             = new algo_info(&create<fierz>);

	// field_theory
	field_theory::register_properties();
	algorithms["@eliminate_kr"]    =new algo_info(&create<eliminate_kronecker>);
	algorithms["@einsteinify"]     =new algo_info(&create<einsteinify>);
	algorithms["@combine"]         =new algo_info(&create<combine>); 
	algorithms["@expand"]          =new algo_info(&create<expand>);
	algorithms["@debracket"]       =new algo_info(&create<debracket>); // not documented yet (what is it?)
	algorithms["@reduce_gendelta"] =new algo_info(&create<reduce_gendelta>);
	algorithms["@breakgendelta"]   =new algo_info(&create<break_gendelta>);
	algorithms["@eliminateeps"]    =new algo_info(&create<eliminate_eps>); // buggy 
	algorithms["@dualise_tensor"]  =new algo_info(&create<dualise_tensor>);
	algorithms["@epsprod2gendelta"]=new algo_info(&create<epsprod2gendelta>); 
	algorithms["@product_shorthand"]=new algo_info(&create<product_shorthand>); 
	algorithms["@expand_product_shorthand"]=new algo_info(&create<expand_product_shorthand>);
	algorithms["@remove_eoms"]     =new algo_info(&create<remove_eoms>); // not documented yet (deprecate?)
	algorithms["@pintegrate"]      =new algo_info(&create<pintegrate>);
	algorithms["@impose_bianchi"]  =new algo_info(&create<impose_bianchi>); 
	algorithms["@all_contractions"]=new algo_info(&create<all_contractions>);
	algorithms["@unique_indices"]  =new algo_info(&create<unique_indices>);  // not properly specified
//	algorithms["@remove_vanishing_derivatives"]  =new algo_info(&create<remove_vanishing_derivatives>);
	algorithms["@unwrap"]          =new algo_info(&create<unwrap>);

	// dummies
	algorithms["@rename_dummies"] =new algo_info(&create<rename_dummies>);

	// select
//	algorithms["@select"]         =&create<select>);
//	algorithms["@unselect"]       =&create<unselect>);
	algorithms["@pop"]            =new algo_info(&create<pop>);
	algorithms["@amnesia"]        =new algo_info(&create<amnesia>);

	// output
	output::register_properties();
	algorithms["@tree"]           =new algo_info(&create<tree_dump>);
	algorithms["@print"]          =new algo_info(&create<print>);
	algorithms["@depprint"]       =new algo_info(&create<depprint>); 
	algorithms["@indexlist"]      =new algo_info(&create<indexlist>); // internal: not documented yet
//	algorithms["@adjmatrix"]      =new algo_info(&create<adjmatrix>);
	algorithms["@number_of_terms"]=new algo_info(&create<number_of_terms>); 
	algorithms["@assert"]         =new algo_info(&create<assert_or_exit>);
	algorithms["@mem"]            =new algo_info(&create<memdump>);
	algorithms["@eqs"]            =new algo_info(&create<eqs>);
	algorithms["@proplist"]       =new algo_info(&create<proplist>);

	// numerical
	numerical::register_properties();
	algorithms["@numerical_flatten"] = new algo_info(&create<numerical_flatten>);

	// properties
	algorithms["@props"]          =new algo_info(&create<extract_properties>); // internal: not documented yet

	// relativity
	relativity::register_properties();
//	algorithms["@remove_weyl_traces"]   =new algo_info(&create<remove_weyl_traces>);
//	algorithms["@ricci_identity"]       =new algo_info(&create<ricci_identity>);
//	algorithms["@weyl_index_order"]     =new algo_info(&create<weyl_index_order>);
	algorithms["@riemann_index_regroup"]=new algo_info(&create<riemann_index_regroup>);
	algorithms["@split_index"]          =new algo_info(&create<split_index>);
	algorithms["@rewrite_indices"]      =new algo_info(&create<rewrite_indices>);
	algorithms["@eliminate_vielbein"]   =new algo_info(&create<eliminate_vielbein>);
	algorithms["@eliminate_metric"]     =new algo_info(&create<eliminate_metric>);

	// substitute
	algorithms["@substitute"]     =new algo_info(&create<substitute>);
	algorithms["@vary"]           =new algo_info(&create<vary>);
	algorithms["@take_match"]     =new algo_info(&create<take_match>);
	algorithms["@replace_match"]  =new algo_info(&create<replace_match>);
	algorithms["@rename"]         =new algo_info(&create<simple_rename>); // not documented yet
	algorithms["@index_rename"]   =new algo_info(&create<index_rename>);

	// lists
	algorithms["@length"]         =new algo_info(&create<length>);
	algorithms["@take"]           =new algo_info(&create<take>);
	algorithms["@range"]          =new algo_info(&create<range>);
	algorithms["@inner"]          =new algo_info(&create<inner>);
	algorithms["@list_sum"]       =new algo_info(&create<list_sum>);
	algorithms["@coefficients"]   =new algo_info(&create<coefficients>); // not documented yet; unfinished!
	}

manipulator::~manipulator()
	{
	algorithm_map_t::iterator beg=algorithms.begin();
	while(beg!=algorithms.end()) {
		delete (*beg).second;
		++beg;
		}
	properties::clear();
	expressions.clear();
	name_set.clear();
	rat_set.clear();
	}

bool manipulator::is_whitespace_(const std::string& str) const
	{
	for(unsigned int i=0; i<str.size(); ++i)
		if(!isblank(str[i])) return false;
	return true;
	}

void manipulator::open_stream(const std::string& nm)
	{
	std::ifstream *str=new std::ifstream(nm.c_str());
	if(!str->is_open()) 
		 output_comment("Cannot open \""+nm+"\".");
	else
		 output_comment("Opening \""+nm+"\" ...");
	streamstack.push(str);
	}

// Ownership is transferred to manipulator if str derives from std::fstream (only!)
void manipulator::open_stream(std::istream *str)
	{
	streamstack.push(str);
	}

void manipulator::cleanup_stream()
	{
	assert(streamstack.size()>0);
	std::ifstream *fst=dynamic_cast<std::ifstream *>(streamstack.top());
	if(fst) {
		if(fst->is_open()) {
			output_comment("Closing input file.");
			fst->close();
			}
		}
	streamstack.top()->clear();
	if(fst)
		delete streamstack.top();
	streamstack.pop();
	}

void manipulator::replace_cmdline_args(std::string& oneline)
	{
	std::string::size_type pos=0;
	for(unsigned int i=0; i<cmdline_arguments.size(); ++i) {
		std::ostringstream str;
		str << "\\argv[" << i << "]";
		while((pos=oneline.find(str.str(),pos))!=std::string::npos) {
			oneline.replace(pos, str.str().size(), cmdline_arguments[i]);
			}
		}
	}

void manipulator::read_program_file()
	{

	}

bool manipulator::getline_precut(std::istream& str, std::string& buf) 
	{
	if(getline_precut_buffer.size()==0) {
		if(getline(str, getline_precut_buffer)==false)
			return false;
		}

	if(getline_precut_buffer[0]=='#') {
		buf=getline_precut_buffer;
		getline_precut_buffer="";
		return true;
		}
	unsigned int i=0;
	bool exit_after_string=false;
	bool scan_for_closing_quote=false;
	getline_precut_buffer+="   ";
	while(i<getline_precut_buffer.size()) {
		if(scan_for_closing_quote) {
			if(getline_precut_buffer[i]=='\"') {
				 scan_for_closing_quote=false;
				 if(exit_after_string==true) {
					  ++i;
					  break;
					  }
				}
			}
		else if(getline_precut_buffer[i]=='\"') { 
			 scan_for_closing_quote=true;
			 }
		else {
			bool endofline=false;
			if(getline_precut_buffer[i]==';') endofline=true;
			else if(getline_precut_buffer[i]==':') {
				++i;
				if(getline_precut_buffer[i]!=':' && getline_precut_buffer[i]!='=') endofline=true;				
				}
			else if(getline_precut_buffer[i]=='.') {
				++i;
				if(getline_precut_buffer[i]!='.') endofline=true;
				}

			if(endofline) {
				++i;
				if(getline_precut_buffer[i]==' ' && getline_precut_buffer[i+1]=='\"') {
					++i;
					scan_for_closing_quote=true;
					exit_after_string=true;
					}
				else { break; }
				}
			}
		++i;
		}
	buf=getline_precut_buffer.substr(0,i);
	getline_precut_buffer=getline_precut_buffer.substr(i);
	if(is_whitespace_(getline_precut_buffer))
		getline_precut_buffer="";
	debugout << "sending " << buf << std::endl;

	return true;
	}

void manipulator::output_comment(const std::string& comment) const
	{
	if(status_output) 
		 txtout << "\n<comment>\n";
	txtout << comment + "\n";
	if(status_output) 
		 txtout << "</comment>\n";
	}

// Reads and processes input from streamstack.top() until end of stream.
// Will close and delete the stream if it derives from std::ifstream. 
bool manipulator::handle_input()
	{
//	long number_of_lines=0;
	std::string   oneline;
	std::string   filename;
	std::string   procedure_name;
	std::string   procedure_code;
	bool          reading_procedure=false;
	iterator      procedure_it;

	// variables for output redirection (redirected output is always in standard cadabra format so we 
	// need to remember the output format which is in use for the display).
	std::ofstream output_file;
	exptree_output::output_format_t remember_output_format=exptree_output::out_plain;
	bool remember_status_output=false;

	reinput:
	while(getline_precut(*(streamstack.top()),oneline)) {
		replace_cmdline_args(oneline);
		if(loginput)
			debugout << oneline << std::endl;
		if(oneline.substr(0,10)=="#cellstart" || oneline.substr(0,8)=="#cellend") {
			txtout << "\n" << oneline << "\n";
			}
		else if(!is_whitespace_(oneline) && oneline.size()>0 && oneline[0]!='#') {
		   display_result=true;
			keep_result=true;
			if(!reading_procedure) {
				if(goto_label.size()>0) {
					if(oneline.substr(0,goto_label.size())==goto_label) {
						goto_label.clear();
						}
					else goto anotherline;
					}
				if(oneline.substr(0,11)=="@procedure{") {
					procedure_name=oneline.substr(11,oneline.size()-13);
					output_comment("reading procedure "+procedure_name+"...");
					procedure_it=expressions.insert(expressions.end(), 
															  str_node("\\procedure", str_node::b_no));
					iterator lit=expressions.append_child(procedure_it, str_node("\\label", str_node::b_no));
					expressions.append_child(lit, str_node(procedure_name, str_node::b_none));
					reading_procedure=true;
					goto anotherline;
					}
				if(oneline.substr(0,9)=="@bailout{") {
					bailout_label=oneline.substr(9,oneline.size()-11);
					goto anotherline;
					}
				if(oneline.substr(0,6)=="@goto{") {
					goto_label=oneline.substr(6,oneline.size()-8);
//					txtout << "|" << goto_label << "|" << std::endl;
					goto anotherline;
					}
				if(oneline.substr(0,5)=="@end;") {
					output_comment("Stream closed by @end command.");
					if(streamstack.size()==1) {
						cleanup_stream();
						throw stream_end_error();
						}
					goto jumpout;
					}
				}
			else if(oneline.substr(0,15)=="@procedure_end;") {
				output_comment("procedure "+procedure_name+" read.");
				reading_procedure=false;
				goto anotherline;
				}

			unsigned int pos=oneline.size()-1;
		   again:
			while(pos>0 && isblank(oneline[pos]))
				--pos;
			switch(oneline[pos]) {
				case '\"':{
					int quotepos=pos;
					--pos;
					while(pos>0 && oneline[pos]!='\"')
						--pos;
					filename=oneline.substr(pos+1,quotepos-pos-1);
					oneline=oneline.substr(0,pos);
					--pos;
					goto again;
					}
				case '<': // input redirect
					if(filename.size()!=0) {
						open_stream(filename);
						filename.clear();
						goto reinput;
						}
					break;
				case ';':
					if(filename.size()!=0) {
						// first try to open
						std::ifstream testfile;
						testfile.open(filename.c_str());
						if(testfile.is_open()) {
							output_comment("file \""+filename
												 +"\" already exists, sending output to display instead.");
							testfile.close();
							filename.clear();
							}
						else {
							assert(output_file.is_open()==false);
							output_comment("Opening \""+filename+"\" for output...");
							output_file.open(filename.c_str(), std::ofstream::out);
							if(output_file.fail()) 
								 output_comment("Error opening "+filename+".");
							else { 
								fake_txtout=&output_file;
								fake_forcedout=&output_file;
								// turn off status output and switch to plain output format
								remember_output_format=output_format;
								remember_status_output=status_output;
								output_format=exptree_output::out_plain;
								status_output=false;
								}
							filename.clear();
							}
						}
					display_result=true;
					oneline=oneline.substr(0,pos);
					break;
				case ':':
					display_result=false;
					oneline=oneline.substr(0,pos);
					break;
				case '.':
					display_result=true;
					keep_result=false;
					oneline=oneline.substr(0,pos);
					break;
				default:
					input_buffer+=oneline;
					goto anotherline;
				}
			input_buffer=input_buffer+oneline;
			if(is_whitespace_(input_buffer) || input_buffer.size()==0 || input_buffer[0]=='#') {
				input_buffer.clear();
				goto anotherline;
				}

			// At this stage the input buffer has been filled with one properly terminated
			// expression. We now feed it to the parser.
			
		   inputbuffer_ready:
			std::stringstream str(input_buffer);
			parser pa(true);

			try {
				str >> pa;
				}
			catch(std::exception& ex) {
				if(status_output) txtout << "<error>\n";
				if(status_output) txtout << "ERROR: " << texify(ex.what()) << std::endl;
				else              txtout << "ERROR: " << ex.what() << std::endl;
//				txtout << "       Expression removed." << std::endl << std::endl;
				if(status_output) txtout << "</error>\n";
				input_buffer.clear();
				goto anotherline;
				}
			input_buffer.clear();

			// The parser has accepted the input and we now start processing the
			// generated tree. The first step is to collect the label and make sure
			// that this expression is stored at the right location.

			exptree::iterator it;
			if(editing_equation) {
				exptree::iterator exp=expressions.equation_by_number(editing_equation);
				it=expressions.append_child(exp, pa.tree.begin());
				}
			else {
            exptree::iterator topit;
            if(!reading_procedure) {
               topit=expressions.insert(expressions.end(), 
                                        str_node("\\history", str_node::b_no));
               }
            else topit=procedure_it;
            // FIXME: next is to trick buggy routines, it does not do anything yet
            expressions.append_child(pa.tree.begin(), str_node("\\asymimplicit", str_node::b_no));
            it=expressions.append_child(topit, pa.tree.begin());
				it->fl.keep_after_eval=keep_result;
				nset_t::iterator explabel=collect_labels_(expressions, it);
				// FIX HERE
				if(explabel!=name_set.end()) { 
					// Determine whether an expression with this label already exists; if so
					// erase it from the tree.
					iterator oldeq=expressions.equation_by_name(explabel);
					if(oldeq!=expressions.end() && oldeq!=topit) {
						topit=expressions.replace(oldeq, topit);
						expressions.erase(topit);
						}
					}
				cleanup_new_expression_(it);
				}
			if(reading_procedure) goto anotherline;

			// The bare expression is now in the tree and properly
			// labelled.  We cannot yet check index consistency because
			// there may be active nodes which produce objects with
			// indices. So handle all active nodes now.

			editing_equation=false;
			exptree::iterator pit=it;
			bool incomment=false;
			try {
				if(status_output) {
					incomment=true;
					txtout << "\n<comment>\n";
					}
				pit=it;
				it=apply_pre_default_rules_(it);
				it=expressions.named_parent(it, "\\expression");
				pit=it;
				pit=handle_active_nodes_(it);
				// The "pit" iterator does not necessarily refer to the top of the
				// expression.

				// Also, the expression, or in fact the entire tree, may have been
				// removed by the algorithm. Proceed with care.
				if(pit!=expressions.end()) {
					 // We need to walk back up to the top of the subtree for this 
					 // expression.
					 pit = expressions.named_parent(pit, "\\expression");
//					 exptree::print_recursive_treeform(txtout, pit);
					 pit=apply_post_default_rules_(pit);
					extract_properties_(pit);
					last_used_equation_number=expressions.equation_number(pit);
					if(incomment) {
						incomment=false;
						txtout << "</comment>" << std::endl;
						}
					if(display_result) {
						if(status_output) output_status();
						exptree_output eo(expressions, txtout, output_format);
						if(status_output) eo.xml_structured=true;
						eo.utf8_output=utf8_output;
						if(keep_result) { // change wrt. old setup
							eo.print_full_standardform(pit, keep_result);
							txtout << std::endl;
							}
						}
					if(!keep_result) {
						expressions.erase_expression(pit);
						last_used_equation_number=0;
						}
					}
				}
			catch(display_interrupted& ex) {
				txtout << "display interrupted (expression preserved)" << std::endl;
				}
			catch(algorithm_interrupted& ex) {
				interrupted=false;
				if(pit!=expressions.end()) {
					expressions.erase(pit);
					pit=expressions.end();
					}
				if(output_format==exptree_output::out_texmacs)
					txtout << DATA_BEGIN << "verbatim:";
				if(status_output) txtout << "<error>\n";
				txtout << std::endl;
				if(status_output) txtout << "ERROR: " << texify(ex.what()) << "." << std::endl;
				else              txtout << "ERROR: " << ex.what() << "." << std::endl;
//				txtout << "       Expression removed." << std::endl << std::endl;
				if(status_output) txtout << "</error>\n";
				if(output_format==exptree_output::out_texmacs)
					txtout << DATA_END;
				}
			catch(consistency_error& ex) {
				debugout << std::endl << "ERROR: " << ex.what() << std::endl;
				if(pit!=expressions.end()) {
					debugout << std::endl << "History dump of the entire tree:" << std::endl << std::endl;
					expressions.print_recursive_treeform(debugout, expressions.begin());
					}
				expressions.erase(pit);
				pit=expressions.end();
				// Output, depending on format.
				if(output_format==exptree_output::out_texmacs)
					txtout << DATA_BEGIN << "verbatim:";
				if(status_output) txtout << "<error>\n";
				txtout << std::endl;
				if(status_output) txtout << "ERROR: " << texify(ex.what()) << std::endl;
				else              txtout << "ERROR: " << ex.what() << std::endl;
//				txtout << "       Expression removed." << std::endl << std::endl;
				if(status_output) txtout << "</error>\n";
				if(output_format==exptree_output::out_texmacs)
					txtout << DATA_END;
				if(getenv("CDB_ERRORS_ARE_FATAL")) {
					if(incomment) {
						incomment=false;
						txtout << "</comment>\n";
						}
					throw consistency_error("Error triggered program abort by CDB_ERRORS_ARE_FATAL setting.");
					}
				}
			
			// If an error occurred, either normal or because an exception got thrown,
			// close the open streams to get back to the top-level input stream.

			if(pit==expressions.end()) {
				if(streamstack.size()>1) { 
					output_comment("Error occurred, closing all input and output streams.");
					while(streamstack.size()>1)
						cleanup_stream();
					}
				input_buffer=refill_input_buffer;
				refill_input_buffer="";
				if(output_file.is_open()) {
					output_file.close();
					output_file.clear();
					fake_txtout=real_txtout;
					fake_forcedout=real_forcedout;
					output_format=remember_output_format;
					status_output=remember_status_output;
					output_comment("Output file closed.");
					}
				print_prompt();
				if(input_buffer.size()>0) goto inputbuffer_ready;
				else {
					if(output_format==exptree_output::out_texmacs)
						txtout << DATA_BEGIN << "verbatim: " << DATA_END; 
					goto reinput;
					}
				}
			}

		// Ready to process another line of input. Handle some remaining output
		// issues with the texmacs backend, remove output redirection if any was
		// enabled for the current line, print a prompt, and then back to start.

	   anotherline:
		debugout << std::flush;
		if(output_format==exptree_output::out_texmacs) {
			if(getline_precut_buffer.size()==0) {
				txtout << DATA_BEGIN << "verbatim: " << DATA_END; 
				}
			}
		
		// Reset output to display.
		if(output_file.is_open()) {
			output_file.close();
			output_file.clear();
			fake_txtout=real_txtout;
			fake_forcedout=real_forcedout;
			output_format=remember_output_format;
			status_output=remember_status_output;
			output_comment("Output file closed.");
			}

		if(streamstack.size()==1)
			print_prompt();
		}

	jumpout:
	cleanup_stream();
	if(streamstack.size()>0) goto reinput;
	return true;
	}

bool manipulator::receive_command(modglue::ipipe& ip)
	{
	assert(streamstack.size()==0); // command line input is always the first ifstream
	streamstack.push(&ip);
	handle_input();
	return true;
	}

void manipulator::set_prompt(const std::string& str)
	{
	prompt_string=str;
	}

void manipulator::print_prompt() const
	{
	if(output_format!=exptree_output::out_texmacs)
		txtout << prompt_string;
	else {
		if(getline_precut_buffer.size()==0) {
			txtout << DATA_BEGIN << "channel:prompt" << DATA_END;
			txtout << DATA_BEGIN << "latex: $\\red{>}$" << DATA_END;
			}
		}
	txtout << std::flush;
	}

void manipulator::output_status() const
	{
	txtout << "<status>" << std::endl;
	txtout << "last_used_equation_number=" << last_used_equation_number << std::endl;
	txtout << "</status>" << std::endl;
	}

nset_t::iterator manipulator::collect_labels_(exptree& tr, exptree::iterator it)
	{
	// collect labels from \declare constructions:
	iterator sit=tr.begin(it);
	iterator eit=it;
	eit.skip_children();
	++eit;
	while(sit!=eit) {
		if(*sit->name=="\\declare") {
			nset_t::iterator name=tr.child(sit,0)->name;
			// insert a label
			iterator lit=tr.insert(it, str_node("\\label", str_node::b_no));
			iterator nameit=tr.append_child(lit, str_node(*name, str_node::b_none));
			// remove the declare node
			tr.flatten(sit);
			sit=tr.erase(sit);
			sit=tr.erase(sit);
			return name;
			}
		++sit;
		}
	return name_set.end();
	}

void manipulator::cleanup_new_expression_(exptree::iterator it)
	{
	cleanup_expression(expressions, it);
	generate_indexbracket gb(expressions, expressions.end());
	gb.apply_recursive(it,false); 

   // the last algorithm; check consistency too!
	// Does not do the right job yet, should be called _after_ the
	// algorithms have been worked out.
	}

exptree::iterator manipulator::apply_pre_default_rules_(exptree::iterator it)
	{
	const PreDefaultRules *dr=properties::get<PreDefaultRules>();
	if(dr) {
		unsigned int eqnumbuf=last_used_equation_number;
		iterator expression_to_print; // dummy
		last_used_equation_number=expressions.equation_number(it);
//		last_used_equation_number=expressions.number_of_equations();
		debugout << "applying pre default rules..." << std::endl;
	   iterator rlist=dr->rules.begin();
		for(unsigned int i=0; i<dr->rules.arg_size(rlist); ++i) {
			debugout << *(dr->rules.arg(rlist, i)->name) << std::endl;
			iterator original_expression=it;
			handle_external_commands_(original_expression, dr->rules.arg(rlist,i), expression_to_print);
			}
		last_used_equation_number=eqnumbuf;
		}
	return it;
	}

exptree::iterator manipulator::apply_post_default_rules_(exptree::iterator it)
	{
	const PostDefaultRules *dr=properties::get<PostDefaultRules>();
	if(dr) {
		iterator expression_to_print; // dummy
		last_used_equation_number=expressions.equation_number(it);
		debugout << "applying post default rules..." << std::endl;
//		exptree::print_recursive_treeform(debugout, it);
	   iterator rlist=dr->rules.begin();
		for(unsigned int i=0; i<dr->rules.arg_size(rlist); ++i) {
			debugout << "applying " << *(dr->rules.arg(rlist, i)->name) << std::endl;
//			expressions.print_recursive_treeform(txtout, it);
			iterator original_expression=it;
			handle_external_commands_(original_expression, dr->rules.arg(rlist,i), expression_to_print);
			}
		}
//	expressions.print_recursive_treeform(txtout, it);
	return it;
	}

void manipulator::extract_properties_(exptree::iterator it)
	{
	// Only extract properties after the cleanup! (otherwise things
	// like [a?,b?]::Derivative would not be recognised as being
	// \commutator{a?}{b?}::Derivative).

	// FIXME: properties are not required on anything but top-level objects.

//	txtout << "extracting properties" << std::endl;
	extract_properties props(expressions, expressions.end());
	props.apply_recursive(it, false);
//	txtout << "extracting properties done" << std::endl;
	}

exptree::iterator manipulator::handle_active_nodes_(exptree::iterator original_expression)
	{
//	txtout << "handle_active_nodes " << *original_expression->name << std::endl;
	assert(expressions.begin()!=expressions.end());
	bool once_modified=false; // if nothing happens, do a check_consistency at the end
	
   original_expression=expressions.active_expression(original_expression);
	iterator expression_to_print=original_expression;

	post_order_iterator it=expressions.begin(original_expression);
	it.descend_all();
	while(original_expression!=expressions.end() && expressions.depth(it)>1) {
		if(interrupted) {
			interrupted=false;
			original_expression=expressions.end();
			expression_to_print=expressions.end();
			txtout << "calculation interrupted." << std::endl;
			break;
			}
		if(it->is_command()==false || (*it->name).size()==0) {
			++it;
			continue;
			}
		post_order_iterator next=it;
		++next;

		// There are four bail-out cases which cannot be handled in modules:
		if(*it->name=="@timing") {
			algorithm_map_t::iterator it=algorithms.begin();
			while(it!=algorithms.end()) {
				txtout << std::setw(30) << it->first << "  " 
						 << std::setw(10) << it->second->calls << "  "
						 << it->second->sw << std::endl;
				++it;
				}
			txtout << std::setw(30) << "classify_indices" << "  " << algorithm::index_sw << std::endl;
			txtout << std::setw(30) << "get_dummy       " << "  " << algorithm::get_dummy_sw << std::endl;
			expressions.erase_expression(original_expression);
			original_expression=expressions.end();
			return expressions.end();
			}
		else if(*it->name=="@properties") {
			 properties::registered_property_map_t::iterator pit=properties::registered_properties.begin();
			 while(pit!=properties::registered_properties.end()) {
				  if(output_format==exptree_output::out_xcadabra)
						txtout << "<property>\n";
				  txtout << "::" << pit->first << std::endl;
				  if(output_format==exptree_output::out_xcadabra)
						txtout << "</property>\n";
				  ++pit;
				  }
			expressions.erase_expression(original_expression);
			original_expression=expressions.end();
			return expressions.end();
			}
		else if(*it->name=="@algorithms") {
			algorithm_map_t::iterator it=algorithms.begin();
			while(it!=algorithms.end()) {
				 if(output_format==exptree_output::out_xcadabra)
						txtout << "<algorithm>\n";
				 txtout << it->first << std::endl;
				 if(output_format==exptree_output::out_xcadabra)
						txtout << "</algorithm>\n";
				++it;
				}
			expressions.erase_expression(original_expression);
			original_expression=expressions.end();
			return expressions.end();
			}
		else if(*it->name=="@reserved") {
			 // If any of these change, the documentation in cadabra.tex needs to
			 // be updated as well.
			 txtout << "<reserved>\n\\anticommutator\n</reserved>\n"
				     << "<reserved>\n\\arrow\n</reserved>\n"
					  << "<reserved>\n\\comma\n</reserved>\n"
					  << "<reserved>\n\\commutator\n</reserved>\n"
					  << "<reserved>\n\\conditional\n</reserved>\n"
					  << "<reserved>\n\\div\n</reserved>\n"
					  << "<reserved>\n\\dot\n</reserved>\n"
					  << "<reserved>\n\\equals\n</reserved>\n"
					  << "<reserved>\n\\expression\n</reserved>\n"
					  << "<reserved>\n\\factorial\n</reserved>\n"
					  << "<reserved>\n\\indexbracket\n</reserved>\n"
					  << "<reserved>\n\\infty\n</reserved>\n"
					  << "<reserved>\n\\label\n</reserved>\n"
					  << "<reserved>\n\\pow\n</reserved>\n"
				     << "<reserved>\n\\prod\n</reserved>\n"
					  << "<reserved>\n\\regex\n</reserved>\n"
					  << "<reserved>\n\\sequence\n</reserved>\n"
					  << "<reserved>\n\\sum\n</reserved>\n"
					  << "<reserved>\n\\unequals\n</reserved>\n";
			 expressions.erase_expression(original_expression);
			 original_expression=expressions.end();
			 return expressions.end();
			 }
		else if(*it->name=="@quit") {
			debugout << "@quit encountered" << std::endl;
			throw exit_exception();
			}
		else if(*it->name=="@output_format") {
			sibling_iterator sib=expressions.begin(it);
			if(*sib->name=="cadabra")          output_format=exptree_output::out_plain;
			else if(*sib->name=="mathematica") output_format=exptree_output::out_mathematica;
			else if(*sib->name=="reduce")      output_format=exptree_output::out_reduce;
			else if(*sib->name=="maple")       output_format=exptree_output::out_maple;
			else if(*sib->name=="texmacs")     output_format=exptree_output::out_texmacs;
			else if(*sib->name=="xcadabra")    output_format=exptree_output::out_xcadabra;
			else if(*sib->name=="mathml")      output_format=exptree_output::out_mathml;
			expressions.erase_expression(original_expression);
			if(last_used_equation_number!=0)
				return expressions.equation_by_number(last_used_equation_number);
//			txtout << "handle_active_nodes end" << std::endl;
			return expressions.end();
			}
		else if(*it->name=="@utf8_output") {
			sibling_iterator sib=expressions.begin(it);
			if(*sib->name=="true")          utf8_output=true;
			else                            utf8_output=false;
			expressions.erase_expression(original_expression);
			if(last_used_equation_number!=0)
				return expressions.equation_by_number(last_used_equation_number);
			return expressions.end();
			}
		else if(*it->name=="@print_status") {
			sibling_iterator sib=expressions.begin(it);
			if(*sib->name=="true")          status_output=true;
			else                            status_output=false;
			expressions.erase_expression(original_expression);
			if(last_used_equation_number!=0)
				return expressions.equation_by_number(last_used_equation_number);
			return expressions.end();
			}
		else if(*it->name=="@call") {
			if(last_used_equation_number==0) {
				txtout << "need existing current expression" << std::endl;
				expressions.erase_expression(original_expression);
				return expressions.end();
				}
			sibling_iterator args=expressions.begin(it);
			iterator procit=expressions.procedure_by_name(args->name);
			if(procit==expressions.end()) {
				txtout << "no procedure '" << *args->name << "' known." << std::endl;
				expressions.erase_expression(original_expression);
				return expressions.end();
				}
			else {
				++args;
				long collect_after=10;
				if(args!=expressions.end(it))
					collect_after=to_long(*args->multiplier);
				iterator newit=run_procedure(procit,collect_after);
				expressions.erase_expression(original_expression);
				txtout << "procedure completed." << std::endl;
				return newit;
				}
			}
		else if(*it->name=="@xterm_title") {
			if(expressions.begin(it)!=expressions.end(it)) { 
				txtout << "\033]0;";
				sibling_iterator nm=expressions.begin(it);
				if(nm->is_quoted_string())
					txtout << (*nm->name).substr(1,(*nm->name).size()-2);
				else
					txtout << *nm->name;
				txtout  << "\007" << std::flush;
				}
			expressions.erase_expression(original_expression);
			if(last_used_equation_number!=0)
				return expressions.equation_by_number(last_used_equation_number);
			return expressions.end();
			}
		else if(*it->name=="@reset") {
			properties::clear();
			expressions.clear();
			name_set.clear();
			rat_set.clear();
			txtout << "All expressions and object properties erased." << std::endl;
			refill_input_buffer=defaults;
			return expressions.end();
			}
		
		// All the rest is handled externally:
		if(handle_external_commands_(original_expression, it, expression_to_print))
			 once_modified=true;
		
		it=next;
		}
	
	// If a new expression has been entered (expression_to_print equals original_expression)
	// check index consistency. This throws an exception if there is a problem.
	if(original_expression!=expressions.end()) { 
		iterator top_now =expressions.active_expression(expression_to_print);
		iterator top_orig=expressions.active_expression(original_expression);
		if(/* getenv("CDB_CHECK_INPUT") && */ top_now==top_orig) {
//			txtout << "checking input index consistency" << std::endl;
			collect_terms ct(expressions, expressions.end());
			ct.check_index_consistency(original_expression);
			}
		}

//	txtout << "returning from handle_active_nodes_" << std::endl;
	return expression_to_print;
	}

exptree::iterator manipulator::run_procedure(exptree::iterator proc, long collect_after)
	{
	collect_terms collector(expressions, expressions.end());
	long termcount=0;
	long collectcount=0;
	bool collected=false;
	// for each term in current expression,
	iterator curr=expressions.equation_by_number(last_used_equation_number);
	iterator act=expressions.begin(expressions.active_expression(curr));
	if(*act->name!="\\sum") {
		iterator tmpsum=expressions.insert(act,str_node("\\sum"));
		iterator tmpchild=expressions.append_child(tmpsum, str_node("dummy"));
		expressions.move_ontop(tmpchild, act);
		act=tmpsum;
		}
//	assert(*act->name=="\\sum"); // for the time being
	sibling_iterator sib=expressions.begin(act);
	unsigned int backup_last_used=last_used_equation_number;	
	long totalterms = expressions.number_of_children(act);
	std::vector<stopwatch> timers(expressions.number_of_children(proc));
	stopwatch expired_so_far;
	expired_so_far.start();
	while(sib!=expressions.end(act)) {
		++termcount;
		sibling_iterator next_sib=sib;
		++next_sib;
		//    duplicate term
		iterator dup_hi=expressions.insert(expressions.end(), str_node("\\history", str_node::b_no));
		iterator dup_ex=expressions.append_child(dup_hi, str_node("\\expression", str_node::b_no));
		expressions.append_child(dup_ex, iterator(sib));

		// make this one current
		last_used_equation_number=expressions.equation_number(dup_ex);

		// loop over all lines in procedure
		sibling_iterator procl=expressions.begin(proc);
		int line_no=0;
		while(procl!=expressions.end(proc)) {
			// copy to new expression
			if(*procl->name=="\\expression") {
				if(!nowarnings)
					txtout << "running line " << line_no << std::endl;
				timers[line_no].start();
				iterator dup_proc_hi=expressions.insert(expressions.end(), 
																  str_node("\\history", str_node::b_no));
				iterator dup_proc_ex=expressions.append_child(dup_proc_hi, iterator(procl));
				// call handle_external_commands on them
				iterator ret=handle_active_nodes_(dup_proc_hi);
				if(ret!=expressions.end()) {
					extract_properties_(ret);
					if(dup_proc_ex->fl.keep_after_eval==false) 
						expressions.erase_expression(dup_proc_ex);
					}
				timers[line_no].stop();
				}
			++line_no;
			++procl;
			}
		// statistics output
		expired_so_far.stop();
		txtout << "term " << termcount << " of " << totalterms;
		txtout << " remains " << expired_so_far.seconds()*(float)(totalterms-termcount)/(float)(termcount)/60
				 << " min" << std::endl;
		expired_so_far.start();

		// replace term with new current expression.
		dup_ex=expressions.active_expression(dup_ex);
		iterator newterm=expressions.move_ontop(sib, dup_ex.begin());
		expressions.erase_expression(dup_ex);
		cleanup_nests(expressions,newterm);

		sib=next_sib;
		// collect terms
		if(++collectcount>=collect_after) {
			collectcount=0;
			collected=true;
			txtout << "collecting terms; of " << expressions.number_of_children(act) << " terms ";
			sibling_iterator from(expressions.begin(act)), to(sib);
			collector.apply(from, to);
			txtout << expressions.number_of_children(act) << " remain." << std::endl;
			if(getenv("JUSTPROFILE"))
				goto justforprofiling;
			}
		}
//	expressions.print_recursive_treeform(txtout, act) << std::endl;
//	if(collected) {
//		if(expressions.number_of_children(act)==1) {
//			expressions.begin(act)->fl.bracket=act->fl.bracket;
//			expressions.begin(act)->fl.parent_rel=act->fl.parent_rel;
//			expressions.flatten(act);
//			act=expressions.erase(act);
//			}
//		else if(expressions.number_of_children(act)==0) {
//			act->multiplier=rat_set.insert(0).first;
////			propagate_zeroes(act, expressions.parent(act));
//			}
//		}
	justforprofiling:
	if(collected)
		cleanup_expression(expressions, act); // to remove possible zeroes
	last_used_equation_number=backup_last_used;
	for(unsigned int i=0; i<timers.size(); ++i)
		txtout << "line " << i << " total " << timers[i] << std::endl;
	return expressions.active_expression(expressions.equation_by_number(last_used_equation_number));
	}

bool manipulator::handle_external_commands_(exptree::iterator& original_expression, exptree::iterator it,
														  exptree::iterator& expression_to_print)
	{
	bool algorithm_modified_tree=false;

	std::string node_name=*it->name;
	bool        act_multiple=false;
	int         act_at_level=-1;
	bool        act_until_nochange=false;
	std::string::size_type   excl_pos;

	if((excl_pos=node_name.find('!'))!=std::string::npos) {
		 act_multiple=true;
		 std::string level=node_name.substr(excl_pos+1);
		 if(excl_pos+1 < node_name.size() )
			 if(node_name[excl_pos+1]=='!') {
				  act_until_nochange=true;
				  level=level.substr(1);
				  }
		 node_name=node_name.substr(0,excl_pos);
		 if(level.size()>0) 
			  act_at_level=atoi(level.c_str());
		 }
	debugout << "action: " << act_until_nochange << " " << act_multiple << std::endl;

	if(it->is_inert_command())  // turn inert command into
		node_name=node_name.substr(1);
	algorithm_map_t::iterator ait=algorithms.find(node_name);
	if(ait!=algorithms.end()) {
		try {
			std::auto_ptr<algorithm> thealg=ait->second->create(expressions, it);
			thealg->output_format=output_format;

			if(last_used_equation_number==0)
				last_used_equation_number=expressions.number_of_equations()-1;
			// propagate output flags if this is an output module
			exptree_output *eo=dynamic_cast<exptree_output *>(&(*thealg));
			if(eo) {
				eo->output_format=output_format;
				eo->utf8_output=utf8_output;
				}

			bool make_copies=true;
			const KeepHistory *kh=properties::get<KeepHistory>();
			if(kh && kh->value==false) // default when unset is 'true'.
				make_copies=false;

			bool is_inert_command=it->is_inert_command();

			ait->second->sw.start();
			thealg->apply(last_used_equation_number, act_multiple, act_until_nochange, 
							  make_copies, act_at_level, true);
			ait->second->sw.stop();
			ait->second->calls+=thealg->number_of_calls;
			// "it" may no longer be valid!!
			// NOTE: thealg->subtree does not have to be valid now.
//			it=thealg->subtree;

			expression_to_print=thealg->subtree;
			switch(thealg->global_success) {
				case algorithm::g_not_yet_started:
					txtout << ait->first << ": failed to start." << std::endl;
					break;
				case algorithm::g_arguments_accepted:
					txtout << ait->first << ": no such expression." << std::endl;
					break;
				case algorithm::g_operand_determined:
//					txtout << "dangerous" << std::endl;
					if(it->is_inert_command()==false) {
						if(!silentfail)
							txtout << ait->first << ": not applicable." << std::endl;
						}
					break;
				case algorithm::g_applied:
					algorithm_modified_tree=true;
					last_used_equation_number=thealg->equation_number;
					if(thealg->suppress_normal_output)
						display_result=false;
					break;
				case algorithm::g_apply_failed:
					txtout << ait->first << ": error" << std::endl;
					break;
				}
//			expressions.print_entire_tree(txtout);
			if(thealg->discard_command_node && is_inert_command==false) {
				iterator ths=expressions.erase(thealg->this_command);
				// If there was no previous entry in the history of this expression, 
				// remove the expression altogether.
				if(expressions.number_of_children(expressions.active_expression(ths))==1) {
					expressions.erase_expression(original_expression);
					original_expression=expressions.end();
//					expression_to_print=expressions.end();
					}
				}
			}
		catch(algorithm::constructor_error& ex) {
			txtout << ait->first << ": illegal arguments." << std::endl;
			if(it->is_inert_command()==false)
				expressions.erase_expression(original_expression);
			original_expression=expressions.end();
			expression_to_print=expressions.end();
			}
		catch(display_interrupted& ex) {
			txtout << "display interrupted" << std::endl;
			original_expression=expressions.end();
			expression_to_print=expressions.end();
			}
		catch(display_error& ex) {
			unsigned int termnum=1;
			expressions.print_recursive_treeform(debugout, it, termnum) << std::endl; 
			throw std::logic_error("Error in expression display routines.");
			}
		}
	else {
		throw consistency_error("Unknown active node \'"+node_name+"\'.");
		// FIXME: this of course does not get executed anymore, but it should be the
		// job of the higher level anyway.
		if(it->is_inert_command()==false)
			expressions.erase_expression(original_expression);
		original_expression=expressions.end();
		expression_to_print=expressions.end();
		}
//	txtout << "returning" << std::endl;
	return algorithm_modified_tree;
	}
	
	
/*
		else if(*it->name=="@edit") {
			if(expressions.number_of_children(it)<1) {
				txtout << "@edit needs to know the equation number." << std::endl;
				expressions.erase_expression(cit);
				cit=expressions.end();
				}
			else {
				exptree::iterator eit;
				exptree::sibling_iterator eqs=expressions.begin(it);
				unsigned int eqno=atoi((*eqs->name).c_str());
				eit=expressions.equation_by_number(eqno);
				if(eit!=expressions.end() && eit!=cit) {
					print_prompt();
					editing_equation=eqno;
					txtout << "\033$";
					display_expression_(txtout, eit, false, false);
					txtout << "$" << std::flush;
					}
				else {
					txtout << "equation number " << eqno << " does not exist." << std::endl;
					}
				expressions.erase_expression(cit);
				cit=expressions.end();
				}
			}
*/
/*
		else if(*it->name=="@erase") {
			// FIXME: allow for labels instead of numbers
			exptree::iterator eit;
			exptree::sibling_iterator eqs=expressions.begin(it);
			while(eqs!=expressions.end(it)) {
				eit=expressions.equation_by_number(atoi((*eqs->name).c_str()));
				if(eit!=expressions.end() && eit!=cit) {
					expressions.erase_expression(eit);
					txtout << "expression (" << atoi((*eqs->name).c_str()) << ") erased." << std::endl;
					}
				else {
					txtout << "expression (" << atoi((*eqs->name).c_str()) << ") does not exist." << std::endl;
					}
				++eqs;
				}
			expressions.erase_expression(cit);
			cit=expressions.end();
			}
*/
/*
		else if(*it->name=="@label") {
			exptree::sibling_iterator argit=expressions.begin(it);
			unsigned int num=atoi((*argit->name).c_str()); ++argit;
			std::string  lab=*argit->name;
			
			exptree::iterator eit=expressions.equation_by_number(num);
			exptree::iterator lit=expressions.insert(eit.begin(), str_node("\\label", str_node::b_no));
			expressions.append_child(lit, str_node(lab));
			unsigned int termnum=1;
			expressions.print_recursive_treeform(txtout, expressions.begin(), termnum) << std::endl;
			expressions.erase_expression(cit);
			cit=expressions.end();
			}
*/

std::string manipulator::texify(const std::string& str) const
	{
	std::string res;
	for(unsigned int i=0; i<str.size(); ++i) {
		if(str[i]=='_') res+='\\';
		res+=str[i];
		}
	return res;
	}
