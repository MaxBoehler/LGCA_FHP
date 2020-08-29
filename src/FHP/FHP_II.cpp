#include <iostream>
#include <random>
#include <bitset>
#include "FHP_II.h"

void FHP_II::collision() {

  applyBoundary();

  bool checkDynamicBoundary;

  uint64_t c0, c1, c2, c3, c4, c5, r;
  uint64_t twoB0, twoB1, twoB2;
  uint64_t twoB_r0, twoB_r1, twoB_r2, twoB_r3, twoB_r4, twoB_r5;
  uint64_t threeB;
  uint64_t rc0, rc1, rc2, rc3, rc4, rc5;
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
        r  = field.getValue(field.fieldVector, x, y, 6);

        // two body collision
        twoB0 = (c0&c3&~(c1|c2|c4|c5));
        twoB1 = (c1&c4&~(c0|c2|c3|c5));
        twoB2 = (c2&c5&~(c0|c1|c3|c4));
        //With rest:
        twoB_r0 = (r&c0&~(c1|c2|c3|c4|c5));
        twoB_r1 = (r&c1&~(c0|c2|c3|c4|c5));
        twoB_r2 = (r&c2&~(c0|c1|c3|c4|c5));
        twoB_r3 = (r&c3&~(c0|c1|c2|c4|c5));
        twoB_r4 = (r&c4&~(c0|c1|c2|c3|c5));
        twoB_r5 = (r&c5&~(c0|c1|c2|c3|c4));

        // three body collsion
        threeB = (c0^c1)&(c1^c2)&(c2^c3)&(c3^c4)&(c4^c5);

        //Collision of a two particles at i and i+2 which leads to a rest and a moving one:
        rc0 = (c5&c1&~(r|c0|c2|c3|c4));
        rc1 = (c0&c2&~(r|c1|c3|c4|c5));
        rc2 = (c1&c3&~(r|c0|c2|c4|c5));
        rc3 = (c2&c4&~(r|c0|c1|c3|c5));
        rc4 = (c3&c5&~(r|c0|c1|c2|c4));
        rc5 = (c4&c0&~(r|c1|c2|c3 |c5));

        // generate random bit
        rnd = bitDist(mt);
        nrnd = ~rnd;

        // get solid bit
        nsbit = field.solidVector.at( y * field.getXsize() +  x );

        //Update the configuration
        field.putValue(field.resultVector, x, y, 0, c0 ^ ((twoB0|threeB|(rnd&twoB1)|(nrnd&twoB2)|twoB_r0|twoB_r1|twoB_r5|rc0|rc1|rc5)&nsbit));
        field.putValue(field.resultVector, x, y, 1, c1 ^ ((twoB1|threeB|(rnd&twoB2)|(nrnd&twoB0)|twoB_r1|twoB_r0|twoB_r2|rc1|rc0|rc2)&nsbit));
        field.putValue(field.resultVector, x, y, 2, c2 ^ ((twoB2|threeB|(rnd&twoB0)|(nrnd&twoB1)|twoB_r2|twoB_r1|twoB_r3|rc2|rc1|rc3)&nsbit));
        field.putValue(field.resultVector, x, y, 3, c3 ^ ((twoB0|threeB|(rnd&twoB1)|(nrnd&twoB2)|twoB_r3|twoB_r2|twoB_r4|rc3|rc2|rc4)&nsbit));
        field.putValue(field.resultVector, x, y, 4, c4 ^ ((twoB1|threeB|(rnd&twoB2)|(nrnd&twoB0)|twoB_r4|twoB_r3|twoB_r5|rc4|rc3|rc5)&nsbit));
        field.putValue(field.resultVector, x, y, 5, c5 ^ ((twoB2|threeB|(rnd&twoB0)|(nrnd&twoB1)|twoB_r5|twoB_r0|twoB_r4|rc5|rc0|rc4)&nsbit));
        field.putValue(field.resultVector, x, y, 6, r  ^ (twoB_r0|twoB_r1|twoB_r2|twoB_r3|twoB_r4|twoB_r5|rc0|rc1|rc2|rc3|rc4|rc5));
      }
    }
  }
}

void FHP_II::propagate() {

  uint64_t tempVal;
  uint64_t temp2Direction;
  uint64_t temp5Direction;

  for (int y = 0; y < field.getYsize(); y++){
    for (int x = 0; x < field.getXsize(); x++){

      if (y % 2 == 0) {
        // Propagation in 0 Direction
        // from odd to even rows
        tempVal = field.getValue(field.resultVector, x, y + 1, 0);
        field.putValue(field.fieldVector, x, y, 0, tempVal);

        // Propagation in 1 Direction
        // from odd to even rows
        tempVal = (field.getValue(field.resultVector, x, y + 1, 1) >> 1) ^ (field.getValue(field.resultVector, x - 1, y + 1, 1) << 63);
        field.putValue(field.fieldVector, x, y, 1, tempVal);

        // Propagation in 3 Direction
        // from odd to even rows
        tempVal = (field.getValue(field.resultVector, x, y - 1, 3) >> 1) ^ (field.getValue(field.resultVector, x - 1, y - 1, 3) << 63);
        field.putValue(field.fieldVector, x, y, 3, tempVal);

        // Propagation in 4 Direction
        // from odd to even rows
        tempVal = field.getValue(field.resultVector, x, y - 1, 4);
        field.putValue(field.fieldVector, x, y, 4, tempVal);
      } else {
        // Propagation in 0 Direction
        // from even to odd rows
        tempVal = (field.getValue(field.resultVector, x, y + 1, 0) << 1) ^ (field.getValue(field.resultVector, x + 1, y + 1, 0) >> 63);
        field.putValue(field.fieldVector, x, y, 0, tempVal);

        // Propagation in 1 Direction
        // from even to odd rows
        tempVal = field.getValue(field.resultVector, x, y + 1, 1);
        field.putValue(field.fieldVector, x, y, 1, tempVal);

        // Propagation in 3 Direction
        // from even to odd rows
        tempVal = field.getValue(field.resultVector, x, y - 1, 3);
        field.putValue(field.fieldVector, x, y, 3, tempVal);

        // Propagation in 4 Direction
        // from even to odd rows
        tempVal = (field.getValue(field.resultVector, x, y - 1, 4) << 1) ^ (field.getValue(field.resultVector, x + 1, y - 1, 4) >> 63);
        field.putValue(field.fieldVector, x, y, 4, tempVal);
      }

      // Propagation in 2 Direction
      temp2Direction = (field.getValue(field.resultVector, x - 1, y, 2) << 63);
      tempVal = (field.getValue(field.resultVector, x, y, 2) >> 1) ^ temp2Direction;
      field.putValue(field.fieldVector, x, y, 2, tempVal);

      // Propagation in 5 Direction
      temp5Direction = (field.getValue(field.resultVector, x + 1, y, 5) >> 63);
      tempVal = (field.getValue(field.resultVector, x, y, 5) << 1) ^ temp5Direction;
      field.putValue(field.fieldVector, x, y, 5, tempVal);

      // Propagation restparticle
      tempVal = field.getValue(field.resultVector, x, y, 6);
      field.putValue(field.fieldVector, x, y, 6, tempVal);
    }
  }
}
