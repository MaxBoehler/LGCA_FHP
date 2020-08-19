#ifndef LATTICEGAS_SRC_FHP_FHP_H_
#define LATTICEGAS_SRC_FHP_FHP_H_

#include <vector>
#include <memory>
#include "../Field.h"
#include "../Boundary.h"


class FHP {
protected:
  Field& field;
  Boundary boundaryController;

public:
  explicit FHP(Field& FIELD);
  virtual ~FHP() = default;

  void applyBoundary();
  bool applyBoundary(int x, int y);
  virtual void collision() = 0;
  virtual void propagate() = 0;

};

#endif
