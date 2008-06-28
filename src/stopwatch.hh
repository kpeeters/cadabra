// -*- mode: C++; c-file-style: "mnemonic"; -*-

/*
  Stopwatch class for profiling purposes.

  Copyright 1998-1999 Kasper Peeters.

  Please send bugs, changes, problems to bugs@mnemonic.browser.org

  This program and its components are free software; you can
  redistribute it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software Foundation; either
  version 2, or  
 
  This program and its components are distributed in the hope that it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
  the GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License in
  the file COPYING accompanying this program; if not, write to the
  Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef stopwatch_hh__
#define stopwatch_hh__

extern "C" {
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
}

#include <iostream>

class stopwatch {
	public:
		stopwatch();

		void reset();
		void start();
		void stop();
		long seconds() const;
		long useconds() const;
		bool stopped() const;

		friend std::ostream& operator<<(std::ostream&, const stopwatch&);
	private:
		void checkpoint_() const;
		mutable struct timeval  tv1,tv2; 
		mutable struct timezone tz;
		mutable long diffsec, diffusec;
		bool stopped_;
};

std::ostream& operator<<(std::ostream&, const stopwatch&);

#endif
