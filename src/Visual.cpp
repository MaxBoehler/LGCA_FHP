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
  else if (inputType == "momentum") {
    type = 1;
  }
  else {
    type = 2;
  }

}

const void Visual::visualInfo(int MPIX, int MPIY, int tend) {
  int xSize{field.getXsize()};
  int ySize{field.getYsize()};
  std::ofstream outFile(filename + ".info", std::ios::out | std::ios::binary);
  outFile.write(reinterpret_cast<char*>( &MPIX  ), sizeof( MPIX  ));
  outFile.write(reinterpret_cast<char*>( &MPIY  ), sizeof( MPIY  ));
  outFile.write(reinterpret_cast<char*>( &tend  ), sizeof( tend  ));
  outFile.write(reinterpret_cast<char*>( &mdt   ), sizeof( mdt   ));
  outFile.write(reinterpret_cast<char*>( &xSize ), sizeof( xSize ));
  outFile.write(reinterpret_cast<char*>( &ySize ), sizeof( ySize ));
  outFile.close();
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
      case VisualType::MOMENTUM:
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
  std::ofstream outFile(filename + "_" + dt + "_" + xr + "_" + yr + ".mass", std::ios::out | std::ios::binary);

  for (int y = 0; y < field.getYsize()/64; y++) {
    for (int x = 0; x < field.getXsize(); x++) {
      outFile.write(reinterpret_cast<char*>( &(field.mass.at(y * field.getXsize() + x))  ), sizeof( field.mass.at(y * field.getXsize() + x) ));
    }
  }
  outFile.close();
}

const void Visual::visualiseVel(std::string dt, std::string xr, std::string yr) {
  std::ofstream outFileX(filename + "_" + dt + "_" + xr + "_" + yr + ".x_momentum_density", std::ios::out | std::ios::binary);
  std::ofstream outFileY(filename + "_" + dt + "_" + xr + "_" + yr + ".y_momentum_density", std::ios::out | std::ios::binary);

  for (int y = 0; y < field.getYsize()/64; y++) {
    for (int x = 0; x < field.getXsize(); x++) {
      outFileX.write(reinterpret_cast<char*>( &(field.xVel.at(y * field.getXsize() + x))  ), sizeof( field.xVel.at(y * field.getXsize() + x) ));
      outFileY.write(reinterpret_cast<char*>( &(field.yVel.at(y * field.getXsize() + x))  ), sizeof( field.yVel.at(y * field.getXsize() + x) ));
    }
  }
  outFileX.close();
  outFileY.close();
}
