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

void FHP::propagate() {

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
    }
  }
}
