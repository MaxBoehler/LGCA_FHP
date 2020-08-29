#include <iostream>
#include <random>
#include <bitset>
#include "Field.h"

Field::Field( int X,
              int Y,
              int XRANK,
              int YRANK,
              int CELLS,
              tinyxml2::XMLDocument* DOC) :

  xSize         {X/64},
  ySize         {Y},
  xRank         {XRANK},
  yRank         {YRANK},
  cellSize      {CELLS},

  fieldVector   ((X/64) * Y * CELLS, 0),
  resultVector  ((X/64) * Y * CELLS, 0),
  solidVector   ((X/64) * Y,        ~uint64_t(0)),
  mass          ((X/64) * (Y/64),    0),
  xVel          ((X/64) * (Y/64),    0),
  yVel          ((X/64) * (Y/64),    0),

  nBoundary     ((X/64) * CELLS,     0),
  eBoundary     ( Y     * CELLS,     0),
  sBoundary     ((X/64) * CELLS,     0),
  wBoundary     ( Y     * CELLS,     0),

  nBoundaryTEMP ((X/64) * CELLS,     0),
  eBoundaryTEMP ( Y     * CELLS,     0),
  sBoundaryTEMP ((X/64) * CELLS,     0),
  wBoundaryTEMP ( Y     * CELLS,     0),

  doc{DOC} {

  int x0, x1, y0, y1;
  int x0temp, x1temp, y0temp, y1temp;
  int xNodes{xSize * 64};
  int yNodes{ySize};
  bool checkCord;


  tinyxml2::XMLElement* root = doc->FirstChildElement( "lgca" );
  for(tinyxml2::XMLElement* obst = root->FirstChildElement("obstacles")->FirstChildElement( "solid" ); obst != NULL; obst = obst->NextSiblingElement("solid")) {


    x0 = std::stoi(obst->FirstChildElement( "x0" )->FirstChild()->ToText()->Value());
    x1 = std::stoi(obst->FirstChildElement( "x1" )->FirstChild()->ToText()->Value());
    y0 = std::stoi(obst->FirstChildElement( "y0" )->FirstChild()->ToText()->Value());
    y1 = std::stoi(obst->FirstChildElement( "y1" )->FirstChild()->ToText()->Value());

    checkCord = translateCoords(x0temp, x1temp, y0temp, y1temp,
                                 x0, x1, y0, y1, xNodes, yNodes);

    if ( checkCord) {
      solidX0.push_back(x0temp / 64);
      solidX1.push_back(x1temp / 64);
      solidY0.push_back(y0temp);
      solidY1.push_back(y1temp);
    }
  }

  pBackground = std::stod(doc->FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "p" ) ->FirstChild()->ToText()->Value());

  checkCord = false;
  for(tinyxml2::XMLElement* prob = root->FirstChildElement("general")->FirstChildElement( "init" ); prob != NULL; prob = prob->NextSiblingElement("init")) {


    x0 = std::stoi(prob->FirstChildElement( "x0" )->FirstChild()->ToText()->Value());
    x1 = std::stoi(prob->FirstChildElement( "x1" )->FirstChild()->ToText()->Value());
    y0 = std::stoi(prob->FirstChildElement( "y0" )->FirstChild()->ToText()->Value());
    y1 = std::stoi(prob->FirstChildElement( "y1" )->FirstChild()->ToText()->Value());

    checkCord = translateCoords(x0temp, x1temp, y0temp, y1temp,
                                 x0, x1, y0, y1, xNodes, yNodes);

    if ( checkCord) {
      pX0.push_back(x0temp / 64);
      pX1.push_back(x1temp / 64);
      pY0.push_back(y0temp);
      pY1.push_back(y1temp);
      pVal.push_back(std::stod(prob->FirstChildElement( "p" )->FirstChild()->ToText()->Value()));
    }
  }

  initializeField();
  moveToBoundary();
}

bool Field::translateCoords(int& x0temp, int& x1temp, int& y0temp, int& y1temp,
                            int x0, int x1, int y0, int y1, int xNodes, int yNodes) {

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

// get value at position x,y of given vector.
// if x or y is bigger than the given vector exchange data with boundaries
// (Usecase: MPI implementation)
const uint64_t Field::getValue(std::vector<uint64_t>& vec, int x, int y, int cell) {
  if (y == ySize) {
    // corner case
    if (x == -1) {
      return wBoundary.at( wBoundary.size() - 1 );
    }
    // corner case
    else if (x == xSize) {
      return eBoundary.at( eBoundary.size() - 1 );
    }
    // default case
    else {
      return sBoundary.at( x*cellSize + cell );
    }
  }

  else if (y == -1) {
    // corner case
    if (x == -1) {
      return wBoundary.at( cell );
    }
    // corner case
    else if (x == xSize) {
      return eBoundary.at( cell );
    }
    // default case
    else {
      return nBoundary.at( x*cellSize + cell );
    }
  }

  else if (x == xSize) {
    return eBoundary.at( y*cellSize + cell );
  }

  else if (x == -1) {
    return wBoundary.at( y*cellSize + cell );
  }

  else {
    return vec.at( y*xSize*cellSize + x*cellSize + cell );
  }
}

// put value at position x,y of given vector.
// if x or y is bigger than the given vector put data into boundary vectors
// (Usecase: MPI implementation)
void Field::putValue(std::vector<uint64_t>& vec, int x, int y, int cell, uint64_t value) {
  if (y == ySize) {
    // corner case
    if (x == -1) {
      wBoundary.at( wBoundary.size() - 1 ) = value;
    }
    // corner case
    else if (x == xSize) {
      eBoundary.at( eBoundary.size() - 1 ) = value;
    }
    // default case
    else {
      sBoundary.at( x*cellSize + cell ) = value;
    }
  }

  else if (y == -1) {
    // corner case
    if (x == -1) {
      wBoundary.at( cell ) = value;
    }
    // corner case
    else if (x == xSize) {
      eBoundary.at( cell ) = value;
    }
    // default case
    else {
      nBoundary.at( x*cellSize + cell ) = value;
    }
  }

  else if (x == xSize) {
    eBoundary.at( y*cellSize + cell ) = value;
  }
  else if (x == -1) {
    wBoundary.at( y*cellSize + cell ) = value;
  }
  else {
    vec.at( y*xSize*cellSize + x*cellSize + cell ) = value;
  }
}


// initalize each cell in field vector with either
// 0 bit or 1 bit depending on probability pBackground
void Field::initializeField() {
  std::random_device rd;
  std::mt19937 mt{rd()};
  std::uniform_real_distribution<double> bitDist(0.0, 1.0);
  bool checkSolid;
  double p;

  for (int y = 0; y < ySize; y++) {
    for (int x = 0; x < xSize; x++) {
      checkSolid = false;
      for (int i = 0; i < solidX0.size(); i++) {
        if ((x >= solidX0.at(i) && x < solidX1.at(i)) && (y >= solidY0.at(i) && y < solidY1.at(i)) ) {
          solidVector.at( y*xSize+x ) = uint64_t(0);
          checkSolid = true;
        }
      }
      if (checkSolid == false) {
        for (int i = 0; i < pX0.size(); i++) {
          if ((x >= pX0.at(i) && x < pX1.at(i)) && (y >= pY0.at(i) && y < pY1.at(i)) ) {
            p = pVal.at(i);
          } else {
            p = pBackground;
          }
          for (int cell = 0; cell < cellSize; cell++) {
            uint64_t initBits{0};
            uint64_t rndBit{0};
            for (int bit = 0; bit < 64; bit++)
            {
                rndBit = bitDist(mt) <= p ? uint64_t(1) : uint64_t(0);
                initBits = rndBit ^ (initBits << 1);
            }
            putValue(fieldVector, x, y, cell, initBits);
          }
        }
      }
    }
  }
}

void Field::moveToBoundary() {
  for (int y = 0; y < ySize; y++) {
    for (int cell = 0; cell < cellSize; cell++) {
      wBoundaryTEMP.at( y * cellSize + cell) = getValue(fieldVector, 0, y, cell);
    }
  }
  for (int y = 0; y < ySize; y++) {
    for (int cell = 0; cell < cellSize; cell++) {
      eBoundaryTEMP.at( y * cellSize + cell) = getValue(fieldVector, xSize - 1, y, cell);
    }
  }
  for (int x = 0; x < xSize; x++) {
    for (int cell = 0; cell < cellSize; cell++) {
      nBoundaryTEMP.at( x * cellSize + cell) = getValue(fieldVector, x, 0, cell);
    }
  }
  for (int x = 0; x < xSize; x++) {
    for (int cell = 0; cell < cellSize; cell++) {
      sBoundaryTEMP.at( x * cellSize + cell) = getValue(fieldVector, x, ySize - 1, cell);
    }
  }
}

void Field::exchangeBoundary(MPI_Comm CART_COMM) {
  MPI_Request req;
  MPI_Status stat;
  int ySRC, yDEST;
  int xSRC, xDEST;

  moveToBoundary();

  // y-direction
  MPI_Cart_shift(CART_COMM, 1, 1, &ySRC, &yDEST);

  // x-direction
  MPI_Cart_shift(CART_COMM, 0, 1, &xSRC, &xDEST);

  MPI_Sendrecv(&wBoundaryTEMP[0], wBoundaryTEMP.size(), MPI_UINT64_T, xSRC, 0,
               &wBoundary[0],     wBoundary.size(),     MPI_UINT64_T, xSRC, 0, CART_COMM, &stat);

  MPI_Sendrecv(&eBoundaryTEMP[0], eBoundaryTEMP.size(), MPI_UINT64_T, xDEST, 0,
               &eBoundary[0],     eBoundary.size(),     MPI_UINT64_T, xDEST, 0, CART_COMM, &stat);

  MPI_Sendrecv(&nBoundaryTEMP[0], nBoundaryTEMP.size(), MPI_UINT64_T, ySRC, 0,
               &nBoundary[0],     nBoundary.size(),     MPI_UINT64_T, ySRC, 0, CART_COMM, &stat);

  MPI_Sendrecv(&sBoundaryTEMP[0], sBoundaryTEMP.size(), MPI_UINT64_T, yDEST, 0,
               &sBoundary[0],     sBoundary.size(),     MPI_UINT64_T, yDEST, 0, CART_COMM, &stat);
}

// measure data by calculating the mean value of a 64x64 field
// (coarse graining)
void Field::measureField() {
  double Ni{0};
  double coarseGrainMass{0};
  double coarseGrainXvel{0};
  double coarseGrainYvel{0};
  int yCoarse{0};

  for (int y = 0; y < ySize; y++) {
    for (int x = 0; x < xSize; x++) {
      for (int bit = 0; bit < 64; bit++) {
        for (int cell = 0; cell < cellSize; cell++) {
          Ni = getNeighbours(x, y, cell, bit);
          coarseGrainMass += Ni;
          coarseGrainXvel += Ni * ci[cell * 2];
          coarseGrainYvel += Ni * ci[cell * 2 + 1];
        }
        mass.at ( yCoarse * xSize + x ) += coarseGrainMass;

        if (coarseGrainMass == 0) {
          xVel.at ( yCoarse * xSize + x ) += 0;
          yVel.at ( yCoarse * xSize + x ) += 0;
        }
        else {
          xVel.at ( yCoarse * xSize + x ) += coarseGrainXvel / coarseGrainMass;
          yVel.at ( yCoarse * xSize + x ) += coarseGrainYvel / coarseGrainMass;
        }
        coarseGrainMass = 0;
        coarseGrainXvel = 0;
        coarseGrainYvel = 0;
      }
    }
    if ((y + 1) % 64 == 0) yCoarse++;
  }

  for (int i = 0; i < mass.size(); i++) {
    mass.at(i) /= 4096;
    xVel.at(i) /= 4096;
    yVel.at(i) /= 4096;
  }
}


// get neighbour of given bit
double Field::getNeighbours(int x, int y, int cell, int bitCounter) {
  int sumNi{0};

  if (bitCounter == 0) {
    sumNi += getBit(getValue(fieldVector, x - 1, y - 1, cell), 63);
    sumNi += getBit(getValue(fieldVector, x, y - 1, cell), bitCounter);
    sumNi += getBit(getValue(fieldVector, x, y, cell), bitCounter + 1);
    sumNi += getBit(getValue(fieldVector, x, y + 1, cell), bitCounter);
    sumNi += getBit(getValue(fieldVector, x - 1, y + 1, cell), 63);
    sumNi += getBit(getValue(fieldVector, x - 1, y, cell), 63);

  }

  else if (bitCounter == 63) {
    sumNi += getBit(getValue(fieldVector, x, y - 1, cell), bitCounter - 1);
    sumNi += getBit(getValue(fieldVector, x, y - 1, cell), bitCounter);
    sumNi += getBit(getValue(fieldVector, x + 1, y, cell), 0);
    sumNi += getBit(getValue(fieldVector, x, y + 1, cell), bitCounter);
    sumNi += getBit(getValue(fieldVector, x, y + 1, cell), bitCounter - 1);
    sumNi += getBit(getValue(fieldVector, x, y, cell), bitCounter - 1);

  }

  else {
    sumNi += getBit(getValue(fieldVector, x, y - 1, cell), bitCounter - 1);
    sumNi += getBit(getValue(fieldVector, x, y - 1, cell), bitCounter);
    sumNi += getBit(getValue(fieldVector, x, y, cell), bitCounter + 1);
    sumNi += getBit(getValue(fieldVector, x, y + 1, cell), bitCounter);
    sumNi += getBit(getValue(fieldVector, x, y + 1, cell), bitCounter - 1);
    sumNi += getBit(getValue(fieldVector, x, y, cell), bitCounter - 1);
  }

  return sumNi / 6.0;

}

  int Field::getBit(uint64_t node, int pos) {
    return (( (node << pos ) & ( (uint64_t(1) << 63 ) )) >> 63);
  }
