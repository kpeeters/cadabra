
#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include "preprocessor.hh"

bool testit(const std::string& input, const std::string& output)
	{
	std::cout << "testing: " << input << " ... " << std::flush;
	std::stringstream orig(input), res, res2;
	preprocessor pp;
	try {
		orig >> pp;
		res << pp;
		}
	catch(std::exception& ex) {
		if(output.size()==0) {
			std::cout << "ok (threw exception)." << std::endl;
			return true;
			}
		}
	if(res.str()!=output) {
		 std::cout << std::endl << std::endl << "regression check **failed** for:" << std::endl
					 << " input :" << input << std::endl
					 << " wanted:" << output << std::endl
					  << " output:" << res.str() << std::endl << std::endl;
		return false;
		}
	pp.erase();
	res >> pp;
	res2 << pp;
	if(res.str()!=res2.str()) {
		std::cout << std::endl << "iterated regression check **failed** for:" << std::endl
					 << " input : " << input << std::endl
					 << " wanted: " << res.str() << "|" << std::endl
					 << " output: " << res2.str() << "|" << std::endl;
		return false;
		}
	std::cout << std::endl 
				 << " result: " << res.str() << " ... ok." << std::endl;
	return true;
	}

int main(int argc, char **argv)
	{
	std::stringstream orig, check;
	preprocessor pp;
	int regression=1;

	try {
		regression*=testit(
			"i k^{\\mu} ( lim_{z\\rightarrow w} (z-w)^{-1/2} \\psi_{\\mu}(w) S_{\\alpha}(z) )",
			"\\prod{i}{k^{\\mu}}{\\prod(lim_{z \\rightarrow w})(\\sub(z)(w)^{\\sub{0}{\\div{1}{2}}})(\\psi_{\\mu}(w))(S_{\\alpha}(z))}");
		regression*=testit(
			"(a+b*c)",
			"\\sum(a)(\\prod{b}{c})");

		regression*=testit(
			"a+(b*c+e)",
			"\\sum{a}{\\sum(\\prod{b}{c})(e)}");

		regression*=testit(
			"a+(b)",
			"\\sum{a}(b)");

		regression*=testit(
			"sin(x)^2+cos(x)^2",
			"\\sum{sin(x)^2}{cos(x)^2}");

		regression*=testit(
			"[ T^1 T^2, T^3]",
			"\\comma[\\prod{T^1}{T^2}][T^3]");

		// This one was previously flagged as an error, but this does not make much sense
		// anymore, since the product 'y z' is perfectly fine within these brackets.
//		regression*=testit(
//			"sin(cos(acos[asin{x + y z}]))",
//			"");

		regression*=testit(
			"sin(cos(acos[asin[x + y z]]))",
			"sin(cos(acos[asin[\\sum{x}{\\prod{y}{z}}]]))");

		regression*=testit(
			"T^{mu nu}K_{mu rho} = V^{kappa} W_{kappa} M_{rho}",
			"\\equals{\\prod{T^{mu nu}}{K_{mu rho}}}{\\prod{V^{kappa}}{W_{kappa}}{M_{rho}}}");

		regression*=testit(
			"(T^{mu nu}K_{mu rho} + H^{nu}_{rho}) = V^{kappa} W_{kappa} M_{rho} + e_{rho}",
			"\\equals{\\sum(\\prod{T^{mu nu}}{K_{mu rho}})(H^{nu}_{rho})}{\\sum{\\prod{V^{kappa}}{W_{kappa}}{M_{rho}}}{e_{rho}}}");

		regression*=testit(
			"a = b c + e",
			"\\equals{a}{\\sum{\\prod{b}{c}}{e}}");

		// various forms of brackets and grouping
		regression*=testit(
			"sin(a-b+e)",
			"sin(\\sum{\\sub{a}{b}}{e})");

		regression*=testit(
			"a^{c-d+e}",
			"a^{\\sum{\\sub{c}{d}}{e}}");

		regression*=testit(
			"(a+b)",
			"\\sum(a)(b)");

		regression*=testit(
			"sin(a+b)",
			"sin(\\sum{a}{b})");

		regression*=testit(
			"(a+b)^2",
		   "\\sum(a)(b)^2");

		regression*=testit(
			"(a+b)+(c+d)",
		   "\\sum{\\sum(a)(b)}{\\sum(c)(d)}");

		regression*=testit(
			"((a+b)+(c+d))",
		   "\\sum(\\sum(a)(b))(\\sum(c)(d))");

		regression*=testit(
			"thnu((-y1-y2)/2)",
		   "thnu(\\div{\\sub(0)(y1)(y2)}{2})");

		regression*=testit(
			"a^2b",
			"\\prod{a^2}{b}");

		regression*=testit(
			"Tensor[F,{}]",
			"Tensor[\\comma{F}{{}}]");

		regression*=testit(
			"a^{} b",
			"\\prod{a^{}}{b}");

		// double brackets

		regression*=testit(
			"Tensor[{a}]",
			"Tensor[{a}]");

		// field theory constructions
		regression*=testit(
			"X^{\\mu}",
		   "X^{\\mu}");

		// weird brackets
		regression*=testit(
			"delta[\\{a,b\\},\\{c,d\\}]",
			"delta[\\comma{\\comma\\{a\\}\\{b\\}}{\\comma\\{c\\}\\{d\\}}]");

      regression*=testit(
         "q = a b + a(b)(c)",
         "\\equals{q}{\\sum{\\prod{a}{b}}{a(b)(c)}}");

		regression*=testit(
         "(a+b)_{\\mu}", 
         "\\sum(a)(b)_{\\mu}");

		regression*=testit(
         "(a+b)_\\mu", 
         "\\sum(a)(b)_\\mu");

		regression*=testit(
			"foo:= -a-b",
			"\\declare{foo}{\\sub{0}{a}{b}}");

		regression*=testit(
			"(\\Gamma_r)_{a b}",
			"(\\Gamma_r)_{a b}");

		// strings should never be expanded
		regression*=testit(
			"\"foo = bar\"",
			"\"foo = bar\"");

		// commands have special treatment
		regression*=testit(
			"@test_command!(something)",
			"@test_command!(something)");
		
		// multi-char relations
		regression*=testit(
			"a$VectorIndex(4)",
			"a$VectorIndex(4)");

		regression*=testit(
			"a::VectorIndex(4)",
			"a$VectorIndex(4)");

		regression*=testit(
			"A_{a b c} -> C_{a} D_{b c}",
			"\\arrow{A_{a b c}}{\\prod{C_{a}}{D_{b c}}}");

		// default whitespace->product
		regression*=testit(
			"{ a, b c f, d e }",
         "\\comma{a}{\\prod{b}{c}{f}}{\\prod{d}{e}}");

		regression*=testit(
			"{ a q, b c f, d e }",
         "\\comma{\\prod{a}{q}}{\\prod{b}{c}{f}}{\\prod{d}{e}}");

		regression*=testit(
			"\\partial_{a}{b c}",
         "\\partial_{a}{\\prod{b}{c}}");

		regression*=testit(
			"a*( (x^1)^2 + (x^2)^2 + (x^3)^2 )",
			"\\prod{a}{\\sum((x^1)^2)((x^2)^2)((x^3)^2)}");

		regression*=testit(
			"\\bar{\\diff{\\diff{A}_{\\nu}}_{\\rho}}",
			"\\bar{\\diff{\\diff{A}_{\\nu}}_{\\rho}}");

		regression*=testit(
			"A_{\\dot{a} \\dot{b}}",
			"A_{\\dot{a} \\dot{b}}");

//		  regression*=testit(
//			  "@substitute!(%){b_{p}->c_{p m n} A^{m n} + c_{p} }",
//			  "@substitute!(%){\\arrow{b_{p}}{\\sum{\\prod{c_{p m n}}{A^{m n}}{c_{p}}}}}");

		if(!regression) 
			std::cout << "*** Regression check failed; enter input or press CTRL-C ***" << std::endl;
		else
			std::cout << "+++ Regression check ok; enter input or press CTRL-C +++" << std::endl;

		std::cin  >> pp;
		orig  << pp;
		std::cout << orig.str() << std::endl;
		pp.erase();
		orig  >> pp;
		check << pp;
		}
	catch(std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		exit(1);
		}

	if(orig.str()==check.str()) std::cout << "iteration is identical" << std::endl;
	else std::cout << "iteration is NOT identical:" << std::endl
						<< check.str() << std::endl;

	if(regression) return 0;
	else return 1;
	}

