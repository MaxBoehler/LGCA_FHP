#ifndef LATTICEGAS_SRC_VISUAL_H_
#define LATTICEGAS_SRC_VISUAL_H_

#include <memory>
#include <sstream>
#include <string>
#include <iomanip>
#include "Field.h"

enum VisualType : int {MASS = 0, VELOCITY = 1, ALL = 2};

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
  const void visualise(int currentTime);
  const void visualiseMass(std::string dt);
  const void visualiseVel(std::string dt);

};

#endif
