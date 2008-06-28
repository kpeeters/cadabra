#ifndef lie_hh__
#define lie_hh__

#include <vector>
#include <modglue/process.hh>

namespace LiE {

	/// A frontend to use LiE algorithms from within C++. 
	class LiE_t {
		public:
			enum algebra_t { alg_A='A', alg_B='B', alg_C='C', alg_D='D', alg_E='E', alg_F='F', alg_G='G' };

			LiE_t(algebra_t alg=alg_D, unsigned int d=5);
			~LiE_t();

			void start();
			void stop();
			
			class rep_t {
				public:
					rep_t();

					int              multiplicity;
					std::vector<int> weight;
			};
			typedef std::vector<rep_t> reps_t;

			/// Global setting for algebra dimension and type (not much point in making these
			/// changeable on the fly).
			algebra_t    algebra_type;
			unsigned int algebra_dim;

			/// Routines which map 1-1 to a LiE command.
			bool         tensor(const std::vector<rep_t>&, const std::vector<rep_t>&, std::vector<rep_t>&);
			bool         sym_tensor(unsigned int, const std::vector<rep_t>&, std::vector<rep_t>&);
			bool         alt_tensor(unsigned int, const std::vector<rep_t>&, std::vector<rep_t>&);
			bool         alt_sym_tensor(unsigned int, const std::vector<rep_t>&, std::vector<rep_t>&, 
												 bool issym);
			unsigned int dim(const rep_t&);
			unsigned int dim(const std::vector<rep_t>&);
			bool         plethysm(const std::vector<unsigned int>& tab, 
										 std::vector<rep_t>& res, bool traceless, int selfdual=0);
			bool         plethysm(const std::vector<unsigned int>&, 
										 const std::vector<rep_t>&, std::vector<rep_t>&,
										 bool traceless=false, int selfdual=0);

			/// Keep the representation with the largest dimension.
			void         keep_largest_dim(std::vector<rep_t>&, int selfdual=0);

			/// Find the number of singlets in a list of representations.
			unsigned int multiplicity_of_singlet(const std::vector<rep_t>&) const;
		private:
			modglue::child_process lie_proc; //int  fd;
			char curchar;
			int  pid;
			enum parsemode_t { START, MULT, REP };

			int  kgetc();
			void kungetc(int);
			void wait_prompt();
			void wait_newline();
			void eat_white();
			void putreps(std::ostream& str, const std::vector<rep_t>& reps);
			void putrep(std::ostream& str, const rep_t& rep);
			void putalgtype(std::ostream& str);
			int  read_int();
			void read_replist(std::vector<rep_t>&);

	};

};

#endif
