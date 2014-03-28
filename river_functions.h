/*
 *  Extended functionality for importing models to Palabos
 *  Brock Jackman
 *
 */
#ifndef RIVER_FUNCTIONS_H
#define RIVER_FUNCTIONS_H

#include <functional>
#include "palabos3D.h"
#include "palabos3D.hh"
#include "quadtree.h"
#include <algorithm>

template <typename T>
using Vec3 = plb::Array<T, 3>;

template <typename T>
using Triangle = typename plb::TriangleSet<T>::Triangle;

//palabos is hiding its dot product someplace weird
double dot(const Vec3<double>& v1, const Vec3<double>& v2)
{
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

//returns true if the ray from origin in direction dir intersects tri
bool rayIntersects(const Triangle<double>& tri, 
    const Vec3<double>& origin, const Vec3<double>& dir);

//returns function to determine if a given point is within a mesh
std::function<int (plb::plint, plb::plint, plb::plint)> wallFlagsFunction(
    const plb::TriangleSet<double>& mesh, Vec3<plb::plint> latticeSize,
    plb::Cuboid<double> meshBounds);

namespace tree{
//function overload to allow triangles to be placed in the quadtree
template<typename T>
bool within(const plb::Array<plb::Array<T, 3>, 3>& shape,
    const tree::Rect<T>& r)
{
  for(int i = 0; i < 3; ++i) {
    if(shape[i][0] < r.xMin || shape[i][0] > r.xMax 
        || shape[i][1] < r.yMin || shape[i][1] > r.yMax) {
      return false;
    }
  }
  return true;
}
}

//Moeller-Trumbore intersection algorithm
//for rays and triangles
inline bool rayIntersects(const Triangle<double>& tri,
    const Vec3<double>& origin, const Vec3<double>& dir)
{
  static const double EPSILON = 0.00001;
  Vec3<double> edge1 = tri[1] - tri[0];
  Vec3<double> edge2 = tri[2] - tri[0];

  //calculate determinant
  Vec3<double> pVec = plb::crossProduct(dir, edge2);
  double det = dot(edge1, pVec);
  if(det > -EPSILON && det < EPSILON) {
    return false;
  }
  double inv_det = 1.0 / det;

  Vec3<double> tVec = origin - tri[0];
  double u = dot(tVec, pVec) * inv_det;
  //the intersection lies outside the triangle
  if(u < 0 || u > 1) {
    return false;
  }

  Vec3<double> qVec = plb::crossProduct(tVec, edge1);
  double v = dot(dir, qVec) * inv_det;
  if(v < 0 || v > 1) {
    return false;
  }

  double t = dot(edge2, qVec) * inv_det;

  if(t > EPSILON) {
    return true;
  }

  return false;
}

inline std::function<int (plb::plint, plb::plint, plb::plint)> wallFlagsFunction(
    const plb::TriangleSet<double>& mesh, Vec3<plb::plint> latticeSize,
    plb::Cuboid<double> meshBounds)
{
  using namespace plb;
  tree::QuadTree<double, Triangle<double>> tree(
      meshBounds.lowerLeftCorner[0], meshBounds.upperRightCorner[0],
      meshBounds.lowerLeftCorner[1], meshBounds.upperRightCorner[1]);
  Vec3<double> meshSize = meshBounds.upperRightCorner - 
    meshBounds.lowerLeftCorner;
  for(auto& tri : mesh.getTriangles()){
    tree.insert(tri);
  }

  //copying the tree, but whatever we only have to do it once
  //and capturing by move is convoluted
  return [=](plint pX, plint pY, plint pZ){
    const Vec3<double> direction = { 0.0, 0.0, 1.0 };
    Vec3<double> origin = {static_cast<double>(pX),
      static_cast<double>(pY), static_cast<double>(pZ)};
    origin *= meshSize;
    origin += meshBounds.lowerLeftCorner;
    auto first = tree.beginAt(origin[0], origin[1]);
    auto last = tree.end();
    int hits = std::count_if(first, last, [&](const Triangle<double>& t){
        return rayIntersects(t, origin, direction);
        });
    return (hits & 1) == 0 ? twoPhaseFlag::empty : twoPhaseFlag::wall;
  };
}

#endif
