#include "quadtree.h"
#include <random>
#include <iostream>

/*
 *  Rough test of quadtree
 *  Brock Jackman
 */

using namespace tree;

struct Point
{
  double x, y, z;
};

struct Triangle
{
  Point points[3];
};

bool within(const Triangle& shape, const Rect<double>& r)
{
  for(auto& p : shape.points) {
    if(p.x < r.xMin || p.x > r.xMax || p.y < r.yMin || p.y > r.yMax) {
      return false;
    }
  }
  return true;
}

std::ostream& operator<<(std::ostream& os, const Triangle& t)
{
  for(auto& p : t.points) {
    os << '[' << p.x << ", " << p.y << ", " << p.z << ']';
    os << '\t';
  }
  return os;
}

int main()
{
  std::default_random_engine rng;
  std::uniform_real_distribution<double> dist1(0, 10);
  std::uniform_real_distribution<double> dist2(-0.1, 0.1);
  QuadTree<double, Triangle> qTree(0, 10, 0, 10);
  for(int i = 0; i < 10000; ++i) {
    Triangle tri;
    double x = dist1(rng);
    double y = dist1(rng);
    for(auto& p : tri.points) {
      p.x = x + dist2(rng);
      p.y = y + dist2(rng);
      p.z = 0;
    }
    qTree.insert(tri);
  }
  auto tree2 = qTree;
  auto it = tree2.beginAt(1,1);
  auto end = tree2.end();
  while(it != end) {
    std::cout << *it << std::endl;
    ++it;
  }

  qTree.clear();
  it = qTree.beginAt(1,1);
  std::cout << "Printing cleared tree:";
  while(it != end) {
    std::cout << *it << std::endl;
    ++it;
  }
}
