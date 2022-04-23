#include "dta.hpp"
//----------------------------------------
//----------------------------------------
int main ()
{
    DBI::DTA dta;

    auto inf = leakDetector (dta);
    return 0;
}