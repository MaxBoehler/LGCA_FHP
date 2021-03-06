#include <iostream>
#include <random>
#include <bitset>
#include "FHP_I.h"

void FHP_I::collision() {

  applyBoundary();

  bool checkDynamicBoundary;

  uint64_t c0, c1, c2, c3, c4, c5;
  uint64_t twoB0, twoB1, twoB2;
  uint64_t threeB;
  uint64_t rnd, nrnd;
  uint64_t nsbit;

  std::random_device rd;
  std::mt19937 mt{rd()};
  std::uniform_int_distribution<uint64_t> bitDist(uint64_t(0), ~uint64_t(0));

  for (int y = 0; y < field.getYsize(); y++){
    for (int x = 0; x < field.getXsize(); x++){

      checkDynamicBoundary = applyBoundary(x, y);

      if (checkDynamicBoundary == false) {
        c0 = field.getValue(field.fieldVector, x, y, 0);
        c1 = field.getValue(field.fieldVector, x, y, 1);
        c2 = field.getValue(field.fieldVector, x, y, 2);
        c3 = field.getValue(field.fieldVector, x, y, 3);
        c4 = field.getValue(field.fieldVector, x, y, 4);
        c5 = field.getValue(field.fieldVector, x, y, 5);

        // two body collision
        twoB0 = (c0&c3&~(c1|c2|c4|c5));
        twoB1 = (c1&c4&~(c0|c2|c3|c5));
        twoB2 = (c2&c5&~(c0|c1|c3|c4));

        // three body collsion
        threeB = (c0^c1)&(c1^c2)&(c2^c3)&(c3^c4)&(c4^c5);

        // generate random bit
        rnd = bitDist(mt);
        nrnd = ~rnd;

        // get solid bit
        nsbit = field.solidVector.at( y * field.getXsize() +  x );

        //Update the configuration
        field.putValue(field.resultVector, x, y, 0, c0 ^ ((threeB|twoB0|(rnd&twoB1)|(nrnd&twoB2))&nsbit));
        field.putValue(field.resultVector, x, y, 1, c1 ^ ((threeB|twoB1|(rnd&twoB2)|(nrnd&twoB0))&nsbit));
        field.putValue(field.resultVector, x, y, 2, c2 ^ ((threeB|twoB2|(rnd&twoB0)|(nrnd&twoB1))&nsbit));
        field.putValue(field.resultVector, x, y, 3, c3 ^ ((threeB|twoB0|(rnd&twoB1)|(nrnd&twoB2))&nsbit));
        field.putValue(field.resultVector, x, y, 4, c4 ^ ((threeB|twoB1|(rnd&twoB2)|(nrnd&twoB0))&nsbit));
        field.putValue(field.resultVector, x, y, 5, c5 ^ ((threeB|twoB2|(rnd&twoB0)|(nrnd&twoB1))&nsbit));
      }
    }
  }
}
