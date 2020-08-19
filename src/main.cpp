#include <iostream>
#include <memory>
#include <chrono>
#include <algorithm>
#include <string>
#include "Field.h"
#include "FHP/FHP.h"
#include "FHP/FHP_I.h"
#include "FHP/FHP_II.h"
#include "Visual.h"
#include "tinyxml2.h"

int main(int argc, char const *argv[]) {

  std::string filename{argv[1]};
  std::cout << "\n--------------------" << '\n';
  std::cout << "LGCA FHP Simulation!" << '\n';
  std::cout << "--------------------" << '\n';
  std::cout << "Reading " << filename << "\n\n";

  // extracting name from filename by removing ".xml"
  std::string type{".xml"};
  std::string::size_type i = filename.find(type);
  if (i != std::string::npos) filename.erase(i, type.length());


  tinyxml2::XMLDocument doc;
	doc.LoadFile( argv[1] );

  // read general parameters from xml
  const int x         {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "x" )    ->FirstChild()->ToText()->Value())};
  const int y         {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "y" )    ->FirstChild()->ToText()->Value())};
  const int cells     {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "cells" )->FirstChild()->ToText()->Value())};
  const double prob   {std::stod(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "p" )    ->FirstChild()->ToText()->Value())};
  const int tend      {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "tend" )->FirstChild()->ToText()->Value())};
  const int fhpChoice {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "fhp" )->FirstChild()->ToText()->Value())};
  
  std::cout << "Setup field ... " << '\n';

  auto t0 = std::chrono::high_resolution_clock::now();

  // create field, collision and visualisation object
  Field field(x, y, cells, prob, &doc);
  std::cout << "Done!" << '\n';

  std::unique_ptr<FHP>fhp;
  if (fhpChoice == 1 && cells == 6) {
    std::cout << "Setup FHP_I model ..." << '\n';
    fhp = std::make_unique<FHP_I>(field);
  } else if (fhpChoice == 2 && cells == 7) {
    std::cout << "Setup FHP_II model ..." << '\n';
    fhp = std::make_unique<FHP_II>(field);
  } else {
    std::cout << "Wrong setup of FHP model. Please check your XML file!" << "\n\n";
    return 1;
  }
  Visual visual(field, filename, &doc);


  auto t1 = std::chrono::high_resolution_clock::now();

  std::cout << "Starting Simulation ... " << '\n';

  // mainloop: measure und visualize current field -> do collision -> propagate
  for (int t = 0; t <= tend; t++) {
    visual.visualise(t);
    fhp->collision();
    fhp->propagate();

    if (t % 100 == 0) {
      std::cout << "Timestep: " << t << '\n';
    }
  }

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::seconds>(t2-t0).count() << "s \n";
  std::cout << "Simulation time: " << std::chrono::duration_cast<std::chrono::seconds>(t2-t1).count() << "s \n";

  return 0;
}
