#include "youngtab.hh"
#include <iostream>

using namespace yngtab;
using namespace combin;

//long combin::hash(const std::vector<std::string>& v) 
//	{
//	return 1; 
//	}

void tst1()
	{
	tableau tab;
	tab.add_box(0);
	tab.add_box(0);
	tab.add_box(1);
	tab.add_box(2);
	std::cout << tab << std::endl;
	filled_tableau<std::string> tab2;
	tab2.add_box(0,"1");
	tab2.add_box(0,"2");
	tab2.add_box(1,"3");
	tab2.add_box(2,"4");
	std::cout << tab2 << std::endl << *(static_cast<tableau*>(&tab2)) << std::endl;
	std::cout << tab2.hook_length_prod() << std::endl;
	}

void tst1b()
	{
	tableau tab1;
	tab1.add_box(0);
	tab1.add_box(1);
	tab1.add_box(2);
	tab1.add_box(3);
	tab1.add_box(4);
	std::cout << tab1 << std::endl;
	tableau tab2;
	tab2.add_box(0);
	tab2.add_box(1);
	tab2.add_box(2);
	tab2.add_box(3);
	tab2.add_box(4);
	std::cout << tab2 << std::endl;
	std::cout << tab2.hook_length_prod() << std::endl;
	std::ostream_iterator<tableau> oit(std::cout, "\n\n");
	LR_tensor(tab1,tab2,10,oit);
	}

void tst2()
	{
	tableau tab3;
	tab3.add_row(4);
	tab3.add_row(3);
	tab3.add_row(2);
	std::cout << tab3 << std::endl
				 << tab3.hook_length_prod() << std::endl;

	std::ostream_iterator<filled_tableau<std::string> > oit(std::cout, "\n\n");

	filled_tableau<std::string> fac1, fac2;
	fac1.add_box(0,"1");
	fac1.add_box(0,"2");
//	fac1.add_box(1,"3");
//	fac1.add_box(1,"4");
	fac2.add_box(0,"1");
//	fac2.add_box(0,"b");
	fac2.add_box(1,"2");
//	fac2.add_box(1,"d");

	std::cout << std::endl << fac1 << std::endl << fac2 << std::endl;
	std::cout << "tensor product: " << std::endl;
	LR_tensor(fac1,fac2,10,oit);
	}

void tst2c()
	{
	std::cout << "test standardform" << std::endl;
	filled_tableau<std::string> tab;
	tab.add_box(0,"d7");	
	tab.add_box(0,"d3");
	tab.add_box(1,"d4");
	tab.add_box(1,"d1");
	tab.add_box(2,"d2");
 	tab.add_box(3,"d5");
 	tab.add_box(4,"d6");
	std::cout << tab << std::endl;
	filled_tableau<std::string>::in_column_iterator it=tab.begin_column(1);
	while(it!=tab.end_column(1)) {
		std::cout << "# " << *it << std::endl;
		++it;
		}
	tab.sort_within_columns();
	std::cout << tab << " (" << tab.multiplicity << ")" << std::endl;
	tableaux<filled_tableau<std::string> > tostd;
	tostd.add_tableau(tab);
	tostd.standard_form();
	std::cout << tostd.storage.size() << " tableaux total" << std::endl;
	std::cout << tostd << std::endl;
	}

void print(symmetriser<std::string> arr) 
	{
	std::cout << arr.size() << " combinations:" << std::endl;
	for(unsigned int i=0; i<arr.size(); ++i) {
 		for(unsigned int j=0; j<arr[0].size(); j++)
 			std::cout << arr[i][j] << " ";
		std::cout << arr.signature(i) << std::endl;
 		}
	}

void tstsym()
	{
	filled_tableau<std::string> tab;
	tab.add_box(0, "a");
	tab.add_box(0, "b");
	tab.add_box(1, "c");
	symmetriser<std::string> sym;
	tab.projector(sym);
	std::cout << sym << std::endl;
	}

void test_canonicalise()
	{
	std::cout << "test_canonicalise:" << std::endl;
	filled_tableau<int> tab;
	tab.add_box(0,4);
	tab.add_box(0,1);
	tab.add_box(1,3);
	tab.add_box(1,2);
	std::cout << tab << std::endl;
	filled_tableau<int>::iterator it=tab.begin();
	while(it!=tab.end()) {
		std::cout << (*it) << " ";
		++it;
		}
	std::cout << std::endl;

	std::less<int> comp;
	tab.canonicalise(comp);
	std::cout << tab << std::endl;

	it=tab.begin();
	while(it!=tab.end()) {
		std::cout << (*it) << " ";
		++it;
		}
	std::cout << std::endl;
	}

void test_garnir()
	{
	std::cout << "test_garnir:" << std::endl;
	filled_tableau<std::string> tab;
	tab.add_box(0, "a");
	tab.add_box(0, "b");
	tab.add_box(1, "c");
	tab.add_box(1, "d");
	tab.add_box(2, "d");
	std::vector<std::string> out;

	tab.Garnir_set(std::back_insert_iterator<std::vector<std::string> >(out), 0,1);
	for(unsigned int i=0; i<out.size(); ++i)
		std::cout << out[i] << std::endl;
	}

void test_input_asym()
	{
	std::cout << "test_input_asym:" << std::endl;
	filled_tableau<std::string> tab;
	tab.add_box(0, "a");
//	tab.add_box(0, "e");
	tab.add_box(1, "b");
	tab.add_box(2, "c");
	tab.add_box(3, "d");

	symmetriser<std::string> sym;

	// Add the objects which are to be kept implicitly anti-symmetric.
	// FIXME: reinstate implicit_asym by location later
//	range_t ran;
//	range_vector_t rans;
//	ran.push_back(0); 
//	ran.push_back(2);
//	ran.push_back(3);
//	rans.push_back(ran);

	tab.projector(sym /*, rans */);
	std::cout << sym << std::endl;
	// No automatic routine yet to figure out the multiplicity; it's a bit
	// tricky to hack it in and we don't need it right now because it is
	// easy to compute by hand for our applications.
	std::cout << tab.projector_normalisation()*6 << std::endl << std::endl;

	sym.clear();
	tab.projector(sym);
	std::cout << sym << std::endl;
	std::cout << tab.projector_normalisation() << std::endl;
	}

void tst3()
	{
	filled_tableau<std::string> fac1, fac2, fac3, fac4;
	fac1.add_box(0,"a ");
	fac1.add_box(0,"c ");
	fac1.add_box(1,"b ");
	fac1.add_box(1,"d ");

	fac2.add_box(0,"e ");
	fac2.add_box(0,"c ");
	fac2.add_box(1,"a ");
	fac2.add_box(1,"f ");

	fac3.add_box(0,"f ");
	fac3.add_box(0,"d ");
	fac3.add_box(1,"b ");
	fac3.add_box(1,"e ");

	tableaux<filled_tableau<std::string> > prod1, prod2, prod3;
	LR_tensor(fac1,fac2,10,prod1.get_back_insert_iterator(),true);
//	prod1.standard_form();
//	prod1.symmetrise(fac1);
//	prod1.symmetrise(fac2);
//	prod1.multiply(fac2.
	std::cout << "after one: " << std::endl << prod1 << std::endl;
	std::cout << fac1.dimension(10)*fac2.dimension(10) << "=" 
				 << prod1.total_dimension(10) << std::endl;

	LR_tensor(prod1,fac3,10,prod2.get_back_insert_iterator(),true);
	std::cout << "after two: " << std::endl << prod2 << std::endl;
	std::cout << fac1.dimension(10)*fac2.dimension(10)*fac3.dimension(10) << "=" 
				 << prod2.total_dimension(10) << std::endl;

	std::cout << "constructing projector for" << std::endl;
	std::cout << fac1 << std::endl;
	std::cout << "normalisation = " << fac1.projector_normalisation() << std::endl;
//	std::cout << "should have " << fac1.hook_length_prod() << " elements:" << std::endl;
	symmetriser<std::string> sym;
	fac1.projector(sym);
	print(sym);
	}

int main(int, char **)
	{
	tst1();
	std::cout << "-----" << std::endl;
	tst1b();
	std::cout << "-----" << std::endl;
	tst2();
	std::cout << "-----" << std::endl;
	tst2c();
	std::cout << "-----" << std::endl;
	tst3();
	std::cout << "-----" << std::endl;
	tstsym();
	std::cout << "-----" << std::endl;
	test_canonicalise();
	std::cout << "-----" << std::endl;
	test_garnir();
	std::cout << "-----" << std::endl;
	test_input_asym();
	}


/*

 The nullifying trace stuff is a bit more complicated. The locations
 of the boxes of the tableau are directly mapping to index locations in
 the full tensor product. Only if a pair sits completely inside one
 tensor can one conclude that exchanging the indices gives the 
 symmetry indicated by the tableau. If not, one is exchanging indices
 between two different tensors. E.g.

   A_{m} B_{m}  m m  + m
                       m

 The result fully depends on whether A and B commute or anti-commute.
 Can we do this by doing the tensor product again? Note: our tensor
 products are not just LR products, as our boxes carry labels.

   A_m A_n B_{m n}  

    mnm + mn + mm + mm + m
    n     m    nn   n    n
          n         n    m
                         n

   A_n A_m B_{m n}  

    nmm + nm + nm + nm + n
    n     m    mn   m    m
          n         n    m
                         n

   Reducing these to standard form gives cancellation.

   So three things left to do:

     - change the has_nullifying_trace to take care of 
       pairs which are sitting inside a single tensor.
     - add the option of making a symmetric/antisymmetric
       tensor product.
         => Add a LR_tensor for _one_ tableaux, plus
            a sym/asym flag. This then takes care of
            the sym/asym with weight and multiplicity signs.
            There are probably even more clever ways of 
            doing this, but 6 identical tensors still give
            a reasonable number of objects, and I am not
            interested in classifying R^(10).
     - canonicalise tableaux.
     - how do we go back?

   S1_{p q} S1_{m n} 
 
      There are no contractions, so the only thing to do
      is to sort within the objects. DO not do this, always
      treat the whole expression in one go:

            pqmn + pqm + pq + mnpq + mnp + mn 
                   n     mn          q     pq 

        = 2 mnpq + mnq - mpq + 2 mn + mnp
                   p     n       pq   q
                 

          341 = 0 123 +   124 -   134
          2       4       3       2


 */                    

