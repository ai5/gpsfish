/* printMove.cc
 */
#include "osl/basic_type.h"
#include "osl/csa.h"
#include <iostream>

bool csa_mode = false;
using namespace osl;
void show(int move)
{
    Move m = Move::makeDirect(move);
    if (csa_mode)
	std::cout << csa::show(m) << std::endl;
    else
	std::cout << m << std::endl;
}
int main()
{
    long long move;
    while (std::cin >> move)
    {
	int imove = move;
	if (imove == move) {
	    show(imove);
	} else {
	    std::cerr << (int)imove << "\n";
	    show(imove);
	    std::cerr << (int)(move>>32) << "\n";
	    show(move >> 32);
	}
    }
}


/* ------------------------------------------------------------------------- */
