#include <algorithm>
#include <iostream>
#include "Boundary.h"
#include "tinyxml2.h"

Boundary::Boundary(Field& FIELD) : field{FIELD} {
  tinyxml2::XMLElement* root = field.doc->FirstChildElement( "lgca" );

  // read staticBoundaries parameters from xml
  for(tinyxml2::XMLElement* dirichlet = root->FirstChildElement("staticBoundaries")->FirstChildElement( "dirichlet" ); dirichlet != NULL; dirichlet = dirichlet->NextSiblingElement("dirichlet")) {
    dirichletX0.push_back(std::stoi(dirichlet->FirstChildElement( "x0" )->FirstChild()->ToText()->Value()));
    dirichletX1.push_back(std::stoi(dirichlet->FirstChildElement( "x1" )->FirstChild()->ToText()->Value()));
    dirichletY0.push_back(std::stoi(dirichlet->FirstChildElement( "y0" )->FirstChild()->ToText()->Value()));
    dirichletY1.push_back(std::stoi(dirichlet->FirstChildElement( "y1" )->FirstChild()->ToText()->Value()));
    dirichletCell.push_back(dirichlet->FirstChildElement( "cell" )->FirstChild()->ToText()->Value());
    dirichletValue.push_back(std::stoi(dirichlet->FirstChildElement( "value" )->FirstChild()->ToText()->Value()));
    dirichletDirection.push_back(dirichlet->FirstChildElement( "direction" )->FirstChild()->ToText()->Value());
  }

  // read dynamicBoundaries parameters from xml
  for(tinyxml2::XMLElement* bounceback = root->FirstChildElement("dynamicBoundaries")->FirstChildElement( "bounceback" ); bounceback != NULL; bounceback = bounceback->NextSiblingElement("bounceback")) {
    bouncebackX0.push_back(std::stoi(bounceback->FirstChildElement( "x0" )->FirstChild()->ToText()->Value()));
    bouncebackX1.push_back(std::stoi(bounceback->FirstChildElement( "x1" )->FirstChild()->ToText()->Value()));
    bouncebackY0.push_back(std::stoi(bounceback->FirstChildElement( "y0" )->FirstChild()->ToText()->Value()));
    bouncebackY1.push_back(std::stoi(bounceback->FirstChildElement( "y1" )->FirstChild()->ToText()->Value()));
  }
}

void Boundary::applyStaticBoundary() {
  int x0, x1, y0, y1, cell;
  uint64_t value;
  std::string direction, cells;
  for (int i = 0; i < dirichletX0.size(); i++) {
    x0 = dirichletX0.at(i) / 64;
    x1 = dirichletX1.at(i) / 64;
    y0 = dirichletY0.at(i);
    y1 = dirichletY1.at(i);
    value = uint64_t(dirichletValue.at(i));
    direction = dirichletDirection.at(i);
    direction.erase(std::remove_if(direction.begin(), direction.end(), isspace), direction.end());

    cells = dirichletCell.at(i);
    cells.erase(std::remove_if(cells.begin(), cells.end(), isspace), cells.end());

    if (direction == "right") {
      value = (value << 63);
    }
    else if ((direction == "top" || direction == "bottom") && value == uint64_t(1)) {
      value = ~uint64_t(0);
    }
    for (int j = 0; j < cells.length(); j++) {
      cell = (int)cells[j] - 48;
      staticBoundaryType(x0, x1, y0, y1, BoundaryCondition::DIRICHLET, cell, value);
    }
  }
}

bool Boundary::applyDynamicBoundary(int x, int y) {
  bool checkDynamic{false}, checkTemp;
  int x0, x1, y0, y1;

  for (int i = 0; i < bouncebackX0.size(); i++) {
    x0 = bouncebackX0.at(i) / 64;
    x1 = bouncebackX1.at(i) / 64;
    y0 = bouncebackY0.at(i);
    y1 = bouncebackY1.at(i);

    checkTemp = dynamicBoundaryType(x, y, x0, x1, y0, y1, BoundaryCondition::BOUNCEBACK);
    if (checkTemp == true) checkDynamic = true;

  }

  return checkDynamic;


}

void Boundary::staticBoundaryType(int x0, int x1, int y0, int y1, BoundaryCondition bnc, int cell, uint64_t value) {
  switch (bnc) {
    case BoundaryCondition::DIRICHLET:
      dirichlet(x0, x1, y0, y1, cell, value);
    case BoundaryCondition::BOUNCEBACK:
      break;
    case BoundaryCondition::NONE:
      break;
  }
}

bool Boundary::dynamicBoundaryType(int x, int y, int x0, int x1, int y0, int y1, BoundaryCondition bnc) {
  switch (bnc) {
    case BoundaryCondition::DIRICHLET:
      return false;
    case BoundaryCondition::BOUNCEBACK:
      return bounceback(x, y, x0, x1, y0, y1);
    case BoundaryCondition::NONE:
      return false;
  }
}

void Boundary::dirichlet(int x0, int x1, int y0, int y1, int cell, uint64_t value) {
  uint64_t tempValue;
  for (int y = y0; y < y1 + 1; y++) {
    for (int x = x0; x < x1 + 1; x++) {
      tempValue = field.getValue(field.fieldVector, x, y, cell) | value;
      field.putValue(field.fieldVector, x, y, cell, tempValue);
    }
  }
}

bool Boundary::bounceback(int x, int y, int x0, int x1, int y0, int y1) {
  uint64_t c0, c1, c2, c3, c4, c5;

  if (field.solidVector.at(y * field.getXsize() + x) == 0 || ((x >= x0 && x <= x1 ) && (y >= y0 && y <= y1)) ) {

        c0 = field.getValue(field.fieldVector, x, y, 0);
        c1 = field.getValue(field.fieldVector, x, y, 1);
        c2 = field.getValue(field.fieldVector, x, y, 2);
        c3 = field.getValue(field.fieldVector, x, y, 3);
        c4 = field.getValue(field.fieldVector, x, y, 4);
        c5 = field.getValue(field.fieldVector, x, y, 5);

        field.putValue(field.resultVector, x, y, 0, c3);
        field.putValue(field.resultVector, x, y, 1, c4);
        field.putValue(field.resultVector, x, y, 2, c5);
        field.putValue(field.resultVector, x, y, 3, c0);
        field.putValue(field.resultVector, x, y, 4, c1);
        field.putValue(field.resultVector, x, y, 5, c2);

        return true;
  }
  else {
    return false;
  }
}
