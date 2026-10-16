#include "RepairCore.hpp"
#include <cassert>
#include <iostream>
int main() {
  const auto brands = mr::catalog();
  assert(brands.size() >= 10);
  assert(brands.front().brand == "Samsung");
  assert(!mr::diagnose("Bootloop").nextCheck.empty());
  assert(!mr::diagnose("unrecognized").safety.empty());
  assert(mr::validCase("Samsung", "A52", "Bootloop"));
  assert(!mr::validCase(" ", "A52", "Bootloop"));
  std::cout << "NEXVARY MR core tests passed\n";
}
