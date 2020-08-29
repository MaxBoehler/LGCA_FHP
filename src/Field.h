#ifndef LATTICEGAS_SRC_FIELD_H_
#define LATTICEGAS_SRC_FIELD_H_

#include <vector>
#include <memory>
#include <array>
#include <cmath>
#include <mpi.h>
#include "tinyxml2.h"

class Field {

private:
  const int xSize;
  const int ySize;
  const int cellSize;
  const int xRank;
  const int yRank;
        double pBackground;

  std::vector<int> solidX0;
  std::vector<int> solidX1;
  std::vector<int> solidY0;
  std::vector<int> solidY1;

  std::vector<int> pX0;
  std::vector<int> pX1;
  std::vector<int> pY0;
  std::vector<int> pY1;
  std::vector<double> pVal;

  // Node of 6 particles
  //  0   1
  //   \ /
  // 5--x--2
  //   / \
  //  4   3

  // velocity vector
  const std::array<double, 12> ci {
    -1.0/2.0, (-1) * std::sqrt(3.0)/2.0,
     1.0/2.0, (-1) * std::sqrt(3.0)/2.0,
     1.0,                           0.0,
     1.0/2.0,        std::sqrt(3.0)/2.0,
    -1.0/2.0,        std::sqrt(3.0)/2.0,
    -1.0, 0.0
  };

  void   initializeField();
  int    getBit(uint64_t node, int pos);
  void   moveToBoundary();
  double getNeighbours(int x, int y, int cell, int bitCounter);
  bool   translateCoords(int& x0temp, int& x1temp, int& y0temp, int& y1temp,
                         int x0, int x1, int y0, int y1, int xNodes, int yNodes);



public:
  Field( int X,
         int Y,
         int XRANK,
         int YRANK,
         int CELLS,
         tinyxml2::XMLDocument* doc);

  std::vector<uint64_t> fieldVector;
  std::vector<uint64_t> resultVector;
  std::vector<uint64_t> solidVector;
  std::vector<double>   mass;
  std::vector<double>   xVel;
  std::vector<double>   yVel;

  // north (ymin), east (xmax), south (ymax), west (xmin) boundary vectors
  std::vector<uint64_t> nBoundary;
  std::vector<uint64_t> eBoundary;
  std::vector<uint64_t> sBoundary;
  std::vector<uint64_t> wBoundary;

  // buffer vector for MPI communication
  std::vector<uint64_t> nBoundaryTEMP;
  std::vector<uint64_t> eBoundaryTEMP;
  std::vector<uint64_t> sBoundaryTEMP;
  std::vector<uint64_t> wBoundaryTEMP;

  tinyxml2::XMLDocument* doc;

  void measureField();
  void exchangeBoundary(MPI_Comm CART_COMM);

  // getter
  const uint64_t    getValue(std::vector<uint64_t>& vec, int x, int y, int cell);
  const inline auto getXsize() { return xSize;    };
  const inline auto getYsize() { return ySize;    };
  const inline auto getCells() { return cellSize; };
  const inline auto getXRank() { return xRank;    };
  const inline auto getYRank() { return yRank;    };

  // setter
  void putValue(std::vector<uint64_t>& vec, int x, int y, int cell, uint64_t value);


};

#endif
