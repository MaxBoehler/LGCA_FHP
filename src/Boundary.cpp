#include <algorithm>
#include <iostream>
#include "Boundary.h"
#include "tinyxml2.h"

Boundary::Boundary(Field& FIELD) : field{FIELD} {

  int bx0, bx1, by0, by1;
  int bx0temp, bx1temp, by0temp, by1temp;
  int xNodes{field.getXsize() * 64};
  int yNodes{field.getYsize()};
  bool checkBoundary;

  tinyxml2::XMLElement* root = field.doc->FirstChildElement( "lgca" );

  // read staticBoundaries parameters from xml
  for(tinyxml2::XMLElement* dirichlet = root->FirstChildElement("staticBoundaries")->FirstChildElement( "dirichlet" ); dirichlet != NULL; dirichlet = dirichlet->NextSiblingElement("dirichlet")) {

    bx0 = std::stoi(dirichlet->FirstChildElement( "x0" )->FirstChild()->ToText()->Value());
    bx1 = std::stoi(dirichlet->FirstChildElement( "x1" )->FirstChild()->ToText()->Value());
    by0 = std::stoi(dirichlet->FirstChildElement( "y0" )->FirstChild()->ToText()->Value());
    by1 = std::stoi(dirichlet->FirstChildElement( "y1" )->FirstChild()->ToText()->Value());

    checkBoundary = translateCoords(bx0temp, bx1temp, by0temp, by1temp,
                                    bx0, bx1, by0, by1, xNodes, yNodes);

    if ( checkBoundary) {
      dirichletX0.push_back(bx0temp);
      dirichletX1.push_back(bx1temp);
      dirichletY0.push_back(by0temp);
      dirichletY1.push_back(by1temp);
      dirichletCell.push_back(dirichlet->FirstChildElement( "cell" )->FirstChild()->ToText()->Value());
      dirichletValue.push_back(std::stoi(dirichlet->FirstChildElement( "value" )->FirstChild()->ToText()->Value()));
      dirichletDirection.push_back(dirichlet->FirstChildElement( "direction" )->FirstChild()->ToText()->Value());
    }
  }

  // read dynamicBoundaries parameters from xml
  for(tinyxml2::XMLElement* bounceback = root->FirstChildElement("dynamicBoundaries")->FirstChildElement( "bounceback" ); bounceback != NULL; bounceback = bounceback->NextSiblingElement("bounceback")) {

    bx0 = std::stoi(bounceback->FirstChildElement( "x0" )->FirstChild()->ToText()->Value());
    bx1 = std::stoi(bounceback->FirstChildElement( "x1" )->FirstChild()->ToText()->Value());
    by0 = std::stoi(bounceback->FirstChildElement( "y0" )->FirstChild()->ToText()->Value());
    by1 = std::stoi(bounceback->FirstChildElement( "y1" )->FirstChild()->ToText()->Value());

    checkBoundary = translateCoords(bx0temp, bx1temp, by0temp, by1temp,
                                    bx0, bx1, by0, by1, xNodes, yNodes);

    if ( checkBoundary) {
      bouncebackX0.push_back(bx0temp);
      bouncebackX1.push_back(bx1temp);
      bouncebackY0.push_back(by0temp);
      bouncebackY1.push_back(by1temp);
    }
  }
}

bool Boundary::translateCoords(int& x0temp, int& x1temp, int& y0temp, int& y1temp,
                            int x0, int x1, int y0, int y1, int xNodes, int yNodes) {

  int xRank{field.getXRank()};
  int yRank{field.getYRank()};

  bool xEntry{false}, yEntry {false};

  if (x0 - xNodes*xRank >= 0 && x0 - xNodes*xRank <= xNodes) {
    x0temp = x0 - xNodes*xRank;
    xEntry = true;
  } else if (x0 - xNodes*xRank <= 0 && x1 - xNodes*xRank >= 0 && x1 - xNodes*xRank <= xNodes) {
    x0temp = 0;
    xEntry = true;
  } else if (x0 - xNodes*xRank < 0 && x1 - xNodes*xRank > xNodes) {
    x0temp = 0;
    xEntry = true;
  }

  if (x1 - xNodes*xRank >= 0 && x1 - xNodes*xRank <= xNodes) {
    x1temp = x1 - xNodes*xRank;
  } else if (x1 - xNodes*xRank >= xNodes && x0 - xNodes*xRank >= 0 && x0 - xNodes*xRank <= xNodes ) {
    x1temp = xNodes;
  } else if (x0 - xNodes*xRank < 0 && x1 - xNodes*xRank > xNodes) {
    x1temp = xNodes;
  }


  if (y0 - yNodes*yRank >= 0 && y0 - yNodes*yRank <= yNodes) {
    y0temp = y0 - yNodes*yRank;
    yEntry = true;
  } else if (y0 - yNodes*yRank <= 0 && y1 - yNodes*yRank >= 0 && y1 - yNodes*yRank <= yNodes) {
    y0temp = 0;
    yEntry = true;
  } else if (y0 - yNodes*yRank < 0 && y1 - yNodes*yRank > yNodes) {
    y0temp = 0;
    yEntry = true;
  }

  if (y1 - yNodes*yRank >= 0 && y1 - yNodes*yRank <= yNodes) {
    y1temp = y1 - yNodes*yRank;
  } else if (y1 - yNodes*yRank >= yNodes && y0 - yNodes*yRank >= 0 && y0 - yNodes*yRank <= yNodes ) {
    y1temp = yNodes;
  } else if (y0 - yNodes*yRank < 0 && y1 - yNodes*yRank > yNodes) {
    y1temp = yNodes;
  }

  if (xEntry && yEntry) {
    return true;
  } else {
    return false;
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

  if (bouncebackX0.size() == 0) {
    checkTemp = dynamicBoundaryType(x, y, 0, 0, 0, 0, BoundaryCondition::BOUNCEBACK);
    if (checkTemp == true) checkDynamic = true;
  } else {
    for (int i = 0; i < bouncebackX0.size(); i++) {
      x0 = bouncebackX0.at(i) / 64;
      x1 = bouncebackX1.at(i) / 64;
      y0 = bouncebackY0.at(i);
      y1 = bouncebackY1.at(i);

      checkTemp = dynamicBoundaryType(x, y, x0, x1, y0, y1, BoundaryCondition::BOUNCEBACK);
      if (checkTemp == true) checkDynamic = true;
    }
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
  for (int y = y0; y <= y1; y++) {
    for (int x = x0; x <= x1; x++) {
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
  } else {
    return false;
  }
}
