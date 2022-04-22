#include "dta.hpp"
//----------------------------------------
//----------------------------------------
int main() {

  DBI::DTA dta;

  auto inf = monitorDependences(dta);
  return 0;
}