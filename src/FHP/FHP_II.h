#ifndef LATTICEGAS_SRC_FHP_FHP_II_H_
#define LATTICEGAS_SRC_FHP_FHP_II_H_

#include <vector>
#include <memory>
#include "FHP.h"
#include "../Field.h"

class FHP_II : public FHP {

public:
  using FHP::FHP;

  void collision();
  void propagate();

};

#endif
