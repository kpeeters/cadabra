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

#include "lie.hh"
#include "config.h"
#include <stdlib.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <cstdio>

LiE::LiE_t::LiE_t(algebra_t a, unsigned int d)
	: algebra_type(a), algebra_dim(d), lie_proc("ptywrap"), curchar(-1)
	{
	}

LiE::LiE_t::~LiE_t()
	{
	stop();
	}

LiE::LiE_t::rep_t::rep_t()
	: multiplicity(1)
	{
	}

void LiE::LiE_t::start()
	{
	if(lie_proc.get_pid()==0) {
		lie_proc << "lie";
		try {
			lie_proc.fork();
			}
		catch(std::logic_error& err) {
			throw std::logic_error("Failed to start LiE.");
			}
		pid=lie_proc.get_pid();
		wait_prompt();
		lie_proc.write("maxobjects 1000000\n");
		wait_prompt();
		}
	else throw std::logic_error("Already running");
	}

void LiE::LiE_t::stop()
	{
	if(lie_proc.get_pid()!=0) {
		lie_proc.write("quit\n");
		lie_proc.wait();
		lie_proc.close();
//		lie_proc.terminate();
		}
	}


void LiE::LiE_t::wait_prompt() 
	{
	int ch;
	while((ch=kgetc())!=EOF)
		if(ch=='>') 
			break;
	}

// Also waits for a whitespace character, so that we
// don't read the echo of the input (assumes that all
// output starts with a whitespace character).
//
void LiE::LiE_t::wait_newline()
	{
	int ch;
	bool waiting_for_space=true; //false;
	while((ch=kgetc())!=EOF) {
		if(waiting_for_space && isspace(ch)) {
			kungetc(ch);
			break;
			}
		if(!waiting_for_space && ch=='\n')
			waiting_for_space=true;
		else
			waiting_for_space=false;
		}
	}

void LiE::LiE_t::eat_white()
	{
	int ch;
	do {
		ch=kgetc();
		if(ch==EOF) break;
		} while(isspace(ch));
	}

int LiE::LiE_t::read_int()
	{
	char buf[20];
	int bufp=0;
	while(isspace(buf[bufp]=kgetc()));
	++bufp;
	while(isdigit(buf[bufp++]=kgetc()))
		if(bufp==sizeof(buf)-1)
			break;
	kungetc(buf[bufp-1]);
	buf[bufp]=0;
	return atoi(buf);
	}

void LiE::LiE_t::kungetc(int ch)
	{
	curchar=ch;
	}

int LiE::LiE_t::kgetc()
	{
	char buf[2];
	if(curchar==-1) {
//		std::cerr << "calling read" << std::endl;
		if(lie_proc.read(buf, 1)==0) {
//			std::cerr << "eof" << std::endl;
			return EOF;
			}
//		std::cerr << buf[0] << std::flush;
		}
	else {
		buf[0]=curchar;
		curchar=-1;
		}
	return buf[0];
	}

void LiE::LiE_t::read_replist(std::vector<rep_t>& reps) 
	{
	char         buffer[200];
	unsigned int bufpt=0;
	rep_t        tmp;
	int          ch;

	std::vector<parsemode_t> stck;
	stck.push_back(START);

	while(stck.size()>0 && bufpt<sizeof(buffer)) {
		ch=kgetc();
//		std::cout << ch;
		if(ch==EOF) break;
		if(isspace(ch)) continue; // whitespace is never relevant in LiE
		switch(stck.back()) {
			case START:
				if(ch=='>') {
					kungetc(ch);
					stck.clear();
					}
				else {
					kungetc(ch);
					bufpt=0;
					stck.push_back(MULT);
					}
				break;
			case MULT:
				if(ch=='X') {
					kgetc(); // read '[' too
					buffer[bufpt]=0;
					tmp.multiplicity=atoi(buffer);
					bufpt=0;
					tmp.weight.clear();
					stck.pop_back();
					stck.push_back(REP);
					}
				else {
					buffer[bufpt++]=ch;
					}
				break;
			case REP:
				if(ch==']') {
					buffer[bufpt]=0;
					tmp.weight.push_back(atoi(buffer));
					stck.pop_back();
					reps.push_back(tmp);
					}
				else if(ch==',') {
					buffer[bufpt++]=0;
					tmp.weight.push_back(atoi(buffer));
					bufpt=0;
					}
				else {
					buffer[bufpt++]=ch;
					}
				break;
			}
		}
	}

void LiE::LiE_t::putreps(std::ostream& str, const std::vector<rep_t>& reps)
	{
	for(unsigned int k=0; k<reps.size(); ++k) {
		if(k>0) str << " +";
		putrep(str, reps[k]);
		}
	}

void LiE::LiE_t::putrep(std::ostream& str, const rep_t& rep)
	{
	str << rep.multiplicity << "X[";
	for(unsigned int i=0; i<rep.weight.size(); ++i) {
		if(i>0) str << ",";
		str << rep.weight[i];
		}
	str << "]";
	}

bool LiE::LiE_t::alt_tensor(unsigned int mult, const std::vector<rep_t>& orig, std::vector<rep_t>& res)
	{
	return alt_sym_tensor(mult, orig, res, false);	
	}

bool LiE::LiE_t::sym_tensor(unsigned int mult, const std::vector<rep_t>& orig, std::vector<rep_t>& res)
	{
	return alt_sym_tensor(mult, orig, res, true);
	}

void LiE::LiE_t::putalgtype(std::ostream& str)
	{
	if(algebra_type==alg_D && algebra_dim==2)
		str << ", A1A1)\n";
	else
		str << ", " << (char)algebra_type << algebra_dim << ")\n";
	}

bool LiE::LiE_t::alt_sym_tensor(unsigned int mult, const std::vector<rep_t>& orig, 
										  std::vector<rep_t>& res, bool issym)
	{
	std::ostringstream str;

	str << (issym==true?"sym":"alt") << "_tensor(" << mult << ", ";
	putreps(str, orig);
	putalgtype(str);
	lie_proc.write(str.str());
//	debugout << str.str() << std::flush;

	wait_newline();
	res.clear();
	read_replist(res);
	wait_prompt();

	return true;
	}

bool LiE::LiE_t::tensor(const std::vector<rep_t>& orig1, const std::vector<rep_t>& orig2, 
								std::vector<rep_t>& res)
	{
	std::ostringstream str;

	str << "tensor(";
	putreps(str, orig1);
	str << ", ";
	putreps(str, orig2);
	putalgtype(str);
	lie_proc.write(str.str());
//	debugout << str.str() << std::flush;

	wait_newline();
	res.clear();
	read_replist(res);
	wait_prompt();

	return true;
	}

bool LiE::LiE_t::plethysm(const std::vector<unsigned int>& tab, std::vector<rep_t>& res, bool traceless,
								  int selfdual)
	{
	std::vector<rep_t> rep;
	rep_t tmp;
	tmp.weight.resize(algebra_dim,0);
	tmp.weight[0]=1;
	if(algebra_type==alg_D && algebra_dim==2)
		tmp.weight[1]=1;
	rep.push_back(tmp);
	return plethysm(tab, rep, res, traceless, selfdual);
	}

bool LiE::LiE_t::plethysm(const std::vector<unsigned int>& tab, 
								  const std::vector<rep_t>& rep, std::vector<rep_t>& res, bool traceless,
								  int selfdual)
	{
	std::ostringstream str;

	str << "plethysm([";
	for(unsigned int k=0; k<tab.size(); ++k) {
		if(k>0) str << ", ";
		str << tab[k];
		}
	str << "], ";
	putreps(str, rep);
	putalgtype(str);
	lie_proc.write(str.str());
//	debugout << str.str() << std::flush;

	wait_newline();
	res.clear();
	read_replist(res);
	wait_prompt();

	if(traceless)
		keep_largest_dim(res, selfdual);
	else if(abs(selfdual)==1)
		keep_largest_dim(res, selfdual);

	return true;
	}

unsigned int LiE::LiE_t::dim(const rep_t& orig)
	{
	std::vector<rep_t> tmp;
	tmp.push_back(orig);
	return dim(tmp);
	}

unsigned int LiE::LiE_t::dim(const std::vector<rep_t>& orig)
	{
	std::ostringstream str;

	str << "dim(";
	for(unsigned int k=0; k<orig.size(); ++k) {
		if(k>0) str << " +";
		putrep(str, orig[k]);
		}
	putalgtype(str);
	lie_proc.write(str.str());

	wait_newline();
	unsigned int val=read_int();
	wait_prompt();
	return val;
	}

void LiE::LiE_t::keep_largest_dim(std::vector<rep_t>& reps, int selfdual) 
	{
	if(reps.size()<=1) return;
	
	// This is slightly tricky: if we have a selfdual and anti-selfdual
	// part, these can each have a dimension which is lower than the other
	// terms. However, such selfdual/antiselfdual terms always take the
	// form [......,n,0] and [......,0,n]. So if such pairs occur, we can
	// multiply their effective dimension by 2.

	std::vector<unsigned int> realdim(reps.size());
	int wsize=reps[0].weight.size();
	for(unsigned int i=0; i<reps.size(); ++i) {
		realdim[i]=dim(reps[i]);
		if( (reps[i].weight[wsize-2]==0 && reps[i].weight[wsize-1]!=0) ||
	       (reps[i].weight[wsize-2]!=0 && reps[i].weight[wsize-1]==0) ) {
			rep_t crep=reps[i];
			std::swap(crep.weight[wsize-2], crep.weight[wsize-1]);
			for(unsigned int j=0; j<reps.size(); ++j) {
				if(reps[j].weight==crep.weight) {
					realdim[i]*=2;
					break;
					}
				}
			}
		}

	unsigned int maxdim=0;
	std::vector<unsigned int> maxdim_index;
	for(unsigned int i=0; i<reps.size(); ++i) {
		unsigned int newdim=realdim[i];
		if(newdim>maxdim) {
			maxdim_index.clear();
			maxdim=newdim;
			maxdim_index.push_back(i);
			}
		else if(newdim==maxdim) {
			maxdim_index.push_back(i);
			}
		}
	
	assert(maxdim_index.size()==1 || maxdim_index.size()==2);
	rep_t crep1, crep2;
	crep1=reps[maxdim_index[0]];
	if(maxdim_index.size()==2)
		crep2=reps[maxdim_index[1]];
	reps.clear();

	if(selfdual==-1 || selfdual==0)
		reps.push_back(crep1);
	if(maxdim_index.size()==2 && (selfdual==1  || selfdual==0)) 
		reps.push_back(crep2);
	}

unsigned int LiE::LiE_t::multiplicity_of_singlet(const std::vector<rep_t>& rep) const
	{
	for(unsigned int i=0; i<rep.size(); ++i) {
		bool allzero=true;
		for(unsigned int j=0; j<rep[i].weight.size(); ++j) {
			if(rep[i].weight[j]!=0) {
				allzero=false;
				break;
				}
			}
		if(allzero)
			return rep[i].multiplicity;
		}
	return 0;
	}
