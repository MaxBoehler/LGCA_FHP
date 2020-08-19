#ifndef LATTICEGAS_SRC_FHP_FHP_I_H_
#define LATTICEGAS_SRC_FHP_FHP_I_H_

#include <vector>
#include <memory>
#include "FHP.h"
#include "../Field.h"

class FHP_I : public FHP {

public:
  using FHP::FHP;

  void collision();
  void propagate();

};

#endif
