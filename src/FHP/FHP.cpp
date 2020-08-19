#include <iostream>
#include <random>
#include <bitset>
#include "FHP.h"

FHP::FHP(Field& FIELD) : field{FIELD}, boundaryController{FIELD}  {}

void FHP::applyBoundary() {
  boundaryController.applyStaticBoundary();
}

bool FHP::applyBoundary(int x, int y) {
  return boundaryController.applyDynamicBoundary(x, y);
}
