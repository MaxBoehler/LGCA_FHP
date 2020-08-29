#include <iostream>
#include <memory>
#include <chrono>
#include <algorithm>
#include <string>
#include <mpi.h>
#include "Field.h"
#include "FHP/FHP.h"
#include "FHP/FHP_I.h"
#include "FHP/FHP_II.h"
#include "Visual.h"
#include "tinyxml2.h"

int main(int argc, char *argv[]) {

  int size, rank;

  MPI_Init(&argc , &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::string filename{argv[1]};
  if (rank == 0) {
    std::cout << "\n--------------------" << '\n'
              << "LGCA FHP Simulation!"   << '\n'
              << "--------------------"   << '\n'
              << "Reading " << filename << "\n\n";
  }

  // extracting name from filename by removing ".xml"
  std::string type{".xml"};
  std::string::size_type i = filename.find(type);
  if (i != std::string::npos) filename.erase(i, type.length());

  tinyxml2::XMLDocument doc;
	doc.LoadFile( argv[1] );

  const int MPIX {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "xMesh" )->FirstChild()->ToText()->Value())};
  const int MPIY {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "yMesh")->FirstChild()->ToText()->Value())};

  MPI_Comm CART_COMM, ROW_COMM, COL_COMM;

  int dimensions[]  = {MPIX,MPIY};
  int wrap_around[] = {0,0};
  int coordinates[2];
  int free_coords[2];
  int row_rank, col_rank, cart_rank;
  int row_size, col_size;

  MPI_Cart_create(MPI_COMM_WORLD, 2, dimensions, wrap_around, 1, &CART_COMM);
  MPI_Comm_rank(CART_COMM, &cart_rank);
  MPI_Cart_coords(CART_COMM, cart_rank, 2, coordinates);

  free_coords[0] = 1;
  free_coords[1] = 0;
  MPI_Cart_sub(CART_COMM, free_coords, &ROW_COMM);

  free_coords[0] = 0;
  free_coords[1] = 1;
  MPI_Cart_sub(CART_COMM, free_coords, &COL_COMM);

  MPI_Comm_size(ROW_COMM, &col_size);
  MPI_Comm_size(COL_COMM, &row_size);

  MPI_Comm_rank(ROW_COMM, &col_rank);
  MPI_Comm_rank(COL_COMM, &row_rank);

  MPI_Barrier(MPI_COMM_WORLD);

  // read general parameters from xml
  const int x         {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "x" )    ->FirstChild()->ToText()->Value()) / col_size};
  const int y         {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "y" )    ->FirstChild()->ToText()->Value()) / row_size};
  const int tend      {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "tend" ) ->FirstChild()->ToText()->Value())};
  const int fhpChoice {std::stoi(doc.FirstChildElement( "lgca" )->FirstChildElement( "general" )->FirstChildElement( "fhp" )  ->FirstChild()->ToText()->Value())};
  const int cells     {(fhpChoice == 1) ? 6 : 7};

  if (rank == 0) std::cout << "Setup fields ... " << '\n';

  auto t0 = std::chrono::high_resolution_clock::now();

  // create field, collision and visualisation object
  Field field(x, y, col_rank, row_rank, cells, &doc);

  MPI_Barrier(CART_COMM);

  if (cart_rank == 0) std::cout << "All fields are ready!" << "\n\n";

  std::unique_ptr<FHP>fhp;
  if (fhpChoice == 1) {
    if (cart_rank == 0) std::cout << "Setup FHP_I model ..." << '\n';
    fhp = std::make_unique<FHP_I>(field);
    if (cart_rank == 0) std::cout << "Model ready!" << "\n\n";
  } else if (fhpChoice == 2) {
    if (cart_rank == 0) std::cout << "Setup FHP_II model ..." << '\n';
    fhp = std::make_unique<FHP_II>(field);
    if (cart_rank == 0) std::cout << "Model ready!" << "\n\n";
  } else {
    if (cart_rank == 0) std::cout << "Wrong setup of FHP model. Please check your XML file!" << "\n\n";
    MPI_Finalize();
    return 1;
  }
  Visual visual(field, filename, &doc);
  if (cart_rank == 0) visual.visualInfo(MPIX, MPIY, tend);


  auto t1 = std::chrono::high_resolution_clock::now();

  if (cart_rank == 0)std::cout << "------------------------" << '\n'
                               << "Starting Simulation ... " << '\n'
                               << "------------------------" << '\n';

  // mainloop: measure und visualize current field -> do collision -> propagate
  for (int t = 0; t <= tend; t++) {
    visual.visualise(t, col_rank, row_rank);
    fhp->collision();

    field.exchangeBoundary(CART_COMM);
    MPI_Barrier(CART_COMM);

    fhp->propagate();

    if (t % 100 == 0) {
      if (cart_rank == 0) std::cout << "Timestep: " << t << '\n';
    }
  }

  MPI_Barrier(CART_COMM);
  auto t2 = std::chrono::high_resolution_clock::now();
  if (cart_rank == 0) std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::seconds>(t2-t0).count() << "s \n";
  if (cart_rank == 0) std::cout << "Simulation time: " << std::chrono::duration_cast<std::chrono::seconds>(t2-t1).count() << "s \n";

  MPI_Finalize();
  return 0;
}
