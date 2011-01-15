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

#include<string>
std::string defaults="\\prod{#}::Distributable.\n\
\\prod{#}::IndexInherit.\n\
\\prod{#}::CommutingAsProduct.\n\
\\prod{#}::DependsInherit.\n\
\\prod{#}::WeightInherit(label=all, type=Multiplicative).\n\
\\prod{#}::NumericalFlat.\n\
\n\
\\sum{#}::CommutingAsSum.\n\
\\sum{#}::DependsInherit.\n\
\\sum{#}::IndexInherit.\n\
\\sum{#}::WeightInherit(label=all, type=Additive).\n\
\n\
\\pow{#}::DependsInherit.\n\
\n\
\\indexbracket{#}::Distributable.\n\
\\indexbracket{#}::IndexInherit.\n\
\\commutator{#}::IndexInherit.\n\
\\commutator{#}::Derivative.\n\
\\anticommutator{#}::IndexInherit.\n\
\\anticommutator{#}::Derivative.\n\
";
