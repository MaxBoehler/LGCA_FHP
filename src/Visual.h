#ifndef LATTICEGAS_SRC_VISUAL_H_
#define LATTICEGAS_SRC_VISUAL_H_

#include <memory>
#include <sstream>
#include <string>
#include <iomanip>
#include "Field.h"

enum VisualType : int {MASS = 0, MOMENTUM = 1, ALL = 2};

class Visual {
private:
  Field& field;
  std::string filename;

  tinyxml2::XMLDocument* doc;

  int mdt;
  std::string inputType;
  int type;

public:
  Visual(Field& FIELD, std::string FILENAME, tinyxml2::XMLDocument* DOC);
  const void visualInfo(int MPIX, int MPIY, int tend);
  const void visualise(int currentTime, int xRank, int yRank);
  const void visualiseMass(std::string dt, std::string xr, std::string yr);
  const void visualiseVel(std::string dt, std::string xr, std::string yr);

};

#endif
