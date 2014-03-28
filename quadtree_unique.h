#ifndef QUADTREE_UNIQUE_H
#define QUADTREE_UNIQUE_H

/*    Quadtree.h
 *    Templated, iterable quadtree
 *    Brock Jackman
 */

#include <iterator>
#include <exception>
#include <vector>
#include <memory>
#include <iostream>

namespace tree{

template<typename T>
  struct Rect
  {
    T xMin, xMax, yMin, yMax;
    Rect()
      : xMin(), xMax(), yMin(), yMax() {}
    Rect(T xMin, T xMax, T yMin, T yMax)
      : xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax) {}
  };

//true if S shape is within Rect area
//caller must provide a specialization/overload
//to put type S in quadtree
template<typename S, typename T>
  bool within(const S& shape, const Rect<T>& area);

//quadtree containing shapes
//shapes are placed in the largest cell that
//can contain them
template <typename T, typename S>
  class QuadTree
  {
    public:
      explicit QuadTree(Rect<T> a) : area(a) {}
      QuadTree(T xMin, T xMax, T yMin, T yMax);

      //Add shape to quadtree
      //Returns false if shape does not fit in tree
      //strong guarantee if S is copyable
      bool insert(const S& shape);

      //Remove all elements in tree
      void clear();

      //returns true if shape fits in the tree
      bool canContain(const S& shape);

      //true if point is inside area mapped by quadTree
      bool coversPoint(T x, T y) const;

      //returns true if tree has no children
      bool isLeaf() const { return !static_cast<bool>(children[0].ptr); }

      class cpoint_iterator;
      friend class cpoint_iterator;

      //returns iterator for objects near (x, y)
      cpoint_iterator beginAt(T x, T y) const
      { return cpoint_iterator(x, y, *this); }

      //returns end iterator for all points
      cpoint_iterator end() const
      { return cpoint_iterator(); }

    private:
      void split();

      struct Node;

      Rect<T> area;
      std::vector<S> elements;
      Node children[4];

      //maximum number of objects to contain before split
      //may contain more if the objects are large
      static const int MAX_OBJ = 10;
  };


//provide deep-copying for child nodes
template<typename T, typename S>
  struct QuadTree<T, S>::Node
  {
    Node() = default;
    ~Node() = default;
    Node(const Node& rhs)
    {
      if(rhs.ptr) {
        ptr.reset(new QuadTree<T, S>(*rhs.ptr.get()));
      }
    }
    Node& operator=(Node rhs)
    {
      std::swap(ptr, rhs.ptr);
      return *this;
    }
    std::unique_ptr<QuadTree<T, S> > ptr;
  };

template<typename T, typename S>
  class QuadTree<T, S>::cpoint_iterator 
  : public std::iterator<std::forward_iterator_tag, S>
  {
    public:
      cpoint_iterator() : node(nullptr) {}
      cpoint_iterator(T x, T y, const QuadTree<T,S>& tree);
      const S& operator*() { return *it; }
      cpoint_iterator& operator++();
      cpoint_iterator operator++(int);
      bool operator!=(const cpoint_iterator& rhs);
      bool operator==(const cpoint_iterator& rhs) { return !(*this != rhs); }

      //skip to next node in traversal
      void nextNode();

    private:
      T x, y;
      const QuadTree<T, S>* node;
      typename std::vector<S>::const_iterator it, end;
  };

template <typename T, typename S>
  QuadTree<T, S>::QuadTree(T xMin, T xMax, T yMin, T yMax)
  : area(xMin, xMax, yMin, yMax) {}

template <typename T, typename S>
  void QuadTree<T, S>::clear()
  {
    elements.clear();
    for(auto& child : children) {
      child.ptr.release();
    }
  }

template <typename T, typename S>
  bool QuadTree<T, S>::insert(const S& shape)
  {
    if(!canContain(shape)) {
      return false;
    }
    if(isLeaf()) {
      if(elements.size() <= MAX_OBJ) {
        elements.push_back(shape);
        return true;
      } else {
        split();
      }
    }
    for(auto& child : children) {
      if(child.ptr->insert(shape)) {
        return true;
      }
    }
    elements.push_back(shape);
    return true;
  }


template <typename T, typename S>
  bool QuadTree<T,S>::coversPoint(T x, T y) const
  {
    return (x > area.xMin && x < area.xMax && y > area.yMin && y < area.yMax);
  }


template <typename T, typename S>
  bool QuadTree<T, S>::canContain(const S& shape)
  { return within(shape, area); }

template <typename T, typename S>
  void QuadTree<T, S>::split()
  {
    using std::swap;
    T centerX = (area.xMax + area.xMin) / 2;
    T centerY = (area.yMax + area.yMin) / 2;
    QuadTree<T,S> newTree(area);
    newTree.children[0].ptr.reset(
        new QuadTree<T, S>(centerX, area.xMax, centerY, area.xMax));
    newTree.children[1].ptr.reset(
        new QuadTree<T, S>(area.xMin, centerX, centerY, area.xMax));
    newTree.children[2].ptr.reset(
        new QuadTree<T, S>(area.xMin, centerX, area.yMin, centerY));
    newTree.children[3].ptr.reset(
        new QuadTree<T, S>(centerX, area.xMax, area.yMin, centerY));

    for(auto& element : elements) {
      newTree.insert(element);
    }
    swap(*this, newTree);
  }

template <typename T, typename S>
  typename QuadTree<T, S>::cpoint_iterator& 
  QuadTree<T, S>::cpoint_iterator::operator++()
  {
    ++it;
    while(it == end && node != nullptr) {
      nextNode();
    }
    return *this;
  }

template <typename T, typename S>
  typename QuadTree<T, S>::cpoint_iterator
  QuadTree<T, S>::cpoint_iterator::operator++(int)
  {
    auto copy = *this;
    ++(*this);
    return copy;
  }

template <typename T, typename S>
  void QuadTree<T, S>::cpoint_iterator::nextNode()
  {
    std::cout << node->elements.size() << " elements in region: " << 
      '[' << node->area.xMin << ' ' << node->area.xMax << ' '
      << node->area.yMin << ' ' << node->area.yMax << ']' << '\n';
    if(node->isLeaf()) {
      node = nullptr;
      it = typename std::vector<S>::const_iterator();
      return;
    }
    for(auto& child : node->children) {
      if(child.ptr->coversPoint(x, y)) {
        node = child.ptr.get();
        it = child.ptr->elements.begin();
        end = child.ptr->elements.end();
        return;
      }
    }
  }
template <typename T, typename S>
  bool QuadTree<T, S>::cpoint_iterator::operator!=(const cpoint_iterator& rhs)
  {
    if(node != rhs.node) {
      return true;
    } else if(it != rhs.it) {
      return true;
    }
    return false;
  }


template <typename T, typename S>
  QuadTree<T, S>::cpoint_iterator::cpoint_iterator(T x, T y, 
      const QuadTree<T,S>& tree)
  : x(x), y(y), node(&tree)
  {
    if (!tree.coversPoint(x, y)) {
      node = nullptr;
    } else {
      it = tree.elements.begin();
      end = tree.elements.end();
      if (it == end) {
        nextNode();
      }
    }
  }

} //namespace
#endif
