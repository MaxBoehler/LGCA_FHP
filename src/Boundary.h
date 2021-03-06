#ifndef LATTICEGAS_SRC_BOUNDARY_H_
#define LATTICEGAS_SRC_BOUNDARY_H_

#include <vector>
#include <memory>
#include <string>
#include "Field.h"


enum BoundaryCondition : int {DIRICHLET, BOUNCEBACK, NONE};

class Boundary {

private:
  Field& field;
  void staticBoundaryType  (int x0 = 0,
                            int x1 = 0,
                            int y0 = 0,
                            int y1 = 0,
                            BoundaryCondition bnc = BoundaryCondition::NONE,
                            double density = 0,
                            std::string direction = "none");

  bool dynamicBoundaryType(int x  = 0,
                           int y  = 0,
                           int x0 = 0,
                           int x1 = 0,
                           int y0 = 0,
                           int y1 = 0,
                           std::string direction = "none",
                           BoundaryCondition bnc = BoundaryCondition::NONE);

  void dirichlet (int x0,
                  int x1,
                  int y0,
                  int y1,
                  double density,
                  std::string direction);

  bool bounceback(int x,
                  int y,
                  int x0,
                  int x1,
                  int y0,
                  int y1,
                  std::string direction);

  std::vector<int>     dirichletX0;
  std::vector<int>     dirichletX1;
  std::vector<int>     dirichletY0;
  std::vector<int>     dirichletY1;
  std::vector<double>  dirichletDensity;
  std::vector<std::string> dirichletDirection;

  std::vector<int>         bouncebackX0;
  std::vector<int>         bouncebackX1;
  std::vector<int>         bouncebackY0;
  std::vector<int>         bouncebackY1;
  std::vector<std::string> bouncebackDirection;

  bool translateCoords(int& x0temp, int& x1temp, int& y0temp, int& y1temp,
                       int x0, int x1, int y0, int y1, int xNodes, int yNodes);

public:
  Boundary(Field& FIELD);
  void applyStaticBoundary ();
  bool applyDynamicBoundary(int x, int y);

};

#endif
