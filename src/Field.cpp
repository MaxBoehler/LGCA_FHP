#include <iostream>
#include <random>
#include <bitset>
#include "Field.h"

Field::Field( int X,
              int Y,
              int XRANK,
              int YRANK,
              int CELLS,
              double P,
              tinyxml2::XMLDocument* DOC) :

  xSize       {X/64},
  ySize       {Y},
  xRank       {XRANK},
  yRank       {YRANK},
  cellSize    {CELLS},
  p           {P},

  fieldVector ((X/64) * Y * CELLS, 0),
  resultVector((X/64) * Y * CELLS, 0),
  solidVector ((X/64) * Y,         1),
  mass        ((X/64) * (Y/64),    0),
  xVel        ((X/64) * (Y/64),    0),
  yVel        ((X/64) * (Y/64),    0),

  nBoundary   ((X/64) * CELLS,     0),
  eBoundary   ( Y     * CELLS,     0),
  sBoundary   ((X/64) * CELLS,     0),
  wBoundary   ( Y     * CELLS,     0),

  doc{DOC} {

  int sx0, sx1, sy0, sy1;
  int sx0temp, sx1temp, sy0temp, sy1temp;
  int xNodes{xSize * 64};
  int yNodes{ySize};
  bool xEntry, yEntry;

  tinyxml2::XMLElement* root = doc->FirstChildElement( "lgca" );
  for(tinyxml2::XMLElement* obst = root->FirstChildElement("obstacles")->FirstChildElement( "solid" ); obst != NULL; obst = obst->NextSiblingElement("solid")) {

    xEntry = false;
    yEntry = false;

    sx0 = std::stoi(obst->FirstChildElement( "x0" )->FirstChild()->ToText()->Value());
    sx1 = std::stoi(obst->FirstChildElement( "x1" )->FirstChild()->ToText()->Value());
    sy0 = std::stoi(obst->FirstChildElement( "y0" )->FirstChild()->ToText()->Value());
    sy1 = std::stoi(obst->FirstChildElement( "y1" )->FirstChild()->ToText()->Value());

    if (sx0 - xNodes*xRank >= 0 && sx0 - xNodes*xRank <= xNodes) {
      sx0temp = sx0 - xNodes*xRank;
      xEntry = true;
    } else if (sx0 - xNodes*xRank <= 0 && sx1 - xNodes*xRank >= 0 && sx1 - xNodes*xRank <= xNodes) {
      sx0temp = 0;
      xEntry = true;
    } else if (sx0 - xNodes*xRank < 0 && sx1 - xNodes*xRank > xNodes) {
      sx0temp = 0;
      xEntry = true;
    }

    if (sx1 - xNodes*xRank >= 0 && sx1 - xNodes*xRank <= xNodes) {
      sx1temp = sx1 - xNodes*xRank;
    } else if (sx1 - xNodes*xRank >= xNodes && sx0 - xNodes*xRank >= 0 && sx0 - xNodes*xRank <= xNodes ) {
      sx1temp = xNodes;
    } else if (sx0 - xNodes*xRank < 0 && sx1 - xNodes*xRank > xNodes) {
      sx1temp = xNodes;
    }


    if (sy0 - yNodes*yRank >= 0 && sy0 - yNodes*yRank <= yNodes) {
      sy0temp = sy0 - yNodes*yRank;
      yEntry = true;
    } else if (sy0 - yNodes*yRank <= 0 && sy1 - yNodes*yRank >= 0 && sy1 - yNodes*yRank <= yNodes) {
      sy0temp = 0;
      yEntry = true;
    } else if (sy0 - yNodes*yRank < 0 && sy1 - yNodes*yRank > yNodes) {
      sy0temp = 0;
      yEntry = true;
    }

    if (sy1 - yNodes*yRank >= 0 && sy1 - yNodes*yRank <= yNodes) {
      sy1temp = sy1 - yNodes*yRank;
    } else if (sy1 - yNodes*yRank >= yNodes && sy0 - yNodes*yRank >= 0 && sy0 - yNodes*yRank <= yNodes ) {
      sy1temp = yNodes;
    } else if (sy0 - yNodes*yRank < 0 && sy1 - yNodes*yRank > yNodes) {
      sy1temp = yNodes;
    }

    if ( xEntry && yEntry) {
      solidX0.push_back(sx0temp);
      solidX1.push_back(sx1temp);
      solidY0.push_back(sy0temp);
      solidY1.push_back(sy1temp);
    }
  }
  initializeField();
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
// 0 bit or 1 bit depending on probability p
void Field::initializeField() {
  std::random_device rd;
  std::mt19937 mt{rd()};
  std::uniform_real_distribution<double> bitDist(0.0, 1.0);
  bool checkSolid;

  for (int y = 0; y < ySize; y++) {
    for (int x = 0; x < xSize; x++) {
      checkSolid = false;
      for (int i = 0; i < solidX0.size(); i++) {
        if ((x >= solidX0.at(i)/64 && x < solidX1.at(i)/64) && (y >= solidY0.at(i) && y < solidY1.at(i)) ) {
          solidVector.at( y*xSize+x ) = uint64_t(0);
          checkSolid = true;
        }
      }
      if (checkSolid == false) {
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
    if (y != 0 && y % 64 == 0) yCoarse++;
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
