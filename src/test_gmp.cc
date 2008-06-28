
#include <gmpxx.h>
#include <iostream>

int main(int argc, char **argv)
	{
	mpq_class ratio1, ratio2;
	ratio1="45/89";
	ratio2="11/8";

	std::cout << ratio1*ratio2 << std::endl;
	}
