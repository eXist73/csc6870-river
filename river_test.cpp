#include "palabos3D.h"
#include "palabos3D.hh"
#include <iostream>
#include <functional>
#include "quadtree.h"
#include "river_functions.h"



using namespace plb;
using namespace std;


int main()
{
  TriangleSet<double> mesh("lip.stl");
  Vec3<plint> pBounds = {20, 20, 20};
  Cuboid<double> mBounds;
  mBounds.lowerLeftCorner = {0, 0, 0};
  mBounds.upperRightCorner = {10, 10, 10};
  auto thing = wallFlagsFunction(mesh, pBounds, mBounds);
  std::cout << "Did the thing\n";
  return 0;
}


