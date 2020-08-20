#include <fstream>
#include <algorithm>
#include "Visual.h"


Visual::Visual(Field& FIELD,
               std::string FILENAME,
               tinyxml2::XMLDocument* DOC) :

  field{FIELD},
  filename{FILENAME},
  doc{DOC} {

  mdt = std::stoi(doc->FirstChildElement( "lgca" )->FirstChildElement( "measure" )->FirstChildElement( "time" )->FirstChild()->ToText()->Value());
  inputType = doc->FirstChildElement( "lgca" )->FirstChildElement( "measure" )->FirstChildElement( "type" )->FirstChild()->ToText()->Value();
  inputType.erase(std::remove_if(inputType.begin(), inputType.end(), isspace), inputType.end());

  if (inputType == "mass") {
    type = 0;
  }
  else if (inputType == "velocity") {
    type = 1;
  }
  else {
    type = 2;
  }

}

const void Visual::visualise(int currentTime, int xRank, int yRank) {
  if (currentTime % mdt == 0 ) {
    field.measureField();
    std::ostringstream ct, xr, yr;

    ct << std::setw(6) << std::setfill('0') << currentTime;
    xr << std::setw(3) << std::setfill('0') << xRank;
    yr << std::setw(3) << std::setfill('0') << yRank;

    switch (type) {
      case VisualType::MASS:
        visualiseMass(ct.str(), xr.str(), yr.str());
        break;
      case VisualType::VELOCITY:
        visualiseVel(ct.str(), xr.str(), yr.str());
        break;
      case VisualType::ALL:
        visualiseMass(ct.str(), xr.str(), yr.str());
        visualiseVel(ct.str(), xr.str(), yr.str());
        break;
    }
  }
}

const void Visual::visualiseMass(std::string dt, std::string xr, std::string yr) {
  std::ofstream outFile;
  outFile.open(filename + "_mass_t_" + dt + "_xr_" + xr + "_yr_" + yr + ".csv");

  for (int y = 0; y < field.getYsize()/64; y++) {
    for (int x = 0; x < field.getXsize(); x++) {
      if ( x == field.getXsize() - 1 ) {
        outFile << field.mass.at(y * field.getXsize() + x);
      }
      else {
        outFile << field.mass.at(y * field.getXsize() + x) << ',';
      }
    }
    outFile << '\n';
  }
  outFile.close();
}

const void Visual::visualiseVel(std::string dt, std::string xr, std::string yr) {
  std::ofstream outFile;
  outFile.open(filename + "_velocity_t_" + dt + "_xr_" + xr + "_yr_" + yr + ".csv");
  outFile << field.getXsize() << '\n';
  outFile << field.getYsize() << '\n';

  for (int i = 0; i < field.xVel.size(); i++) {
    outFile << field.xVel.at(i);
    if ( i != field.xVel.size() - 1) {
      outFile << ',';
    }
  }
  outFile << '\n';
  for (int i = 0; i < field.yVel.size(); i++) {
    outFile << field.yVel.at(i);
    if ( i != field.yVel.size() - 1) {
      outFile << ',';
    }
  }
  outFile.close();
}
