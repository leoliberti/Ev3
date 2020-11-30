//                                               -*- C++ -*-
/**
 *  @brief template for tree construction
 *
 *  Copyright (C) 2008-2010 Leo Liberti
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __EV3TREECXX__
#define __EV3TREECXX__

#define RCS7 "$Id: tree.cxx,v 1.13 2005/09/08 23:37:23 liberti Exp liberti $"

#include<iostream>
#include<vector>
#include<string>

#include "exceptions.h"

template<class NodeType> class Pointer {

private:

  // pointer to node data
  NodeType* node;

  // how many Pointers point to the NodeType pointed to by *node
  Int* ncount;

public:

  // constructors  
  Pointer() {
    node = new NodeType;
    ncount = new Int(1);
#ifdef MEMDEBUG
    memcheckdebugpair.first = (void*) node;
    memcheckdebugpair.second.first = memcheckdebugcounter;
    memcheckdebugpair.second.second = ncount;
    if (ncount && *ncount == 1)
      memcheckdebug.push_back(memcheckdebugpair);
    cerr << "MEMCHECK: c1 (" << memcheckdebugcounter 
	 << "): node = " << node << "; ncount = " 
	 << ncount;
    memcheckdebugcounter++;
    if (ncount) {
      cerr << "; *ncount = " << *ncount << endl;
    } else {
      cerr << endl;
    }
#endif
  }
  Pointer(NodeType& n) {
    node = new NodeType(n);
    ncount = new Int(1);
#ifdef MEMDEBUG
    memcheckdebugpair.first = (void*) node;
    memcheckdebugpair.second.first = memcheckdebugcounter;
    memcheckdebugpair.second.second = ncount;
    if (ncount && *ncount == 1)
      memcheckdebug.push_back(memcheckdebugpair);
    cerr << "MEMCHECK: c2 (" << memcheckdebugcounter 
	 << "): node = " << node << "; ncount = " 
	 << ncount;
    memcheckdebugcounter++;
    if (ncount) {
      cerr << "; *ncount = " << *ncount << endl;
    } else {
      cerr << endl;
    }
#endif
  }
  Pointer(double v) {
    node = new NodeType(v);
    ncount = new Int(1);
#ifdef MEMDEBUG
    memcheckdebugpair.first = (void*) node;
    memcheckdebugpair.second.first = memcheckdebugcounter;
    memcheckdebugpair.second.second = ncount;
    if (ncount && *ncount == 1)
      memcheckdebug.push_back(memcheckdebugpair);
    cerr << "MEMCHECK: c3 (" << memcheckdebugcounter 
	 << "): node = " << node << "; ncount = " 
	 << ncount;
    memcheckdebugcounter++;
    if (ncount) {
      cerr << "; *ncount = " << *ncount << endl;
    } else {
      cerr << endl;
    }
#endif
  }
  Pointer(double c, int vi, string vn) {
    node = new NodeType(c, vi, vn);
    ncount = new Int(1);
#ifdef MEMDEBUG
    memcheckdebugpair.first = (void*) node;
    memcheckdebugpair.second.first = memcheckdebugcounter;
    memcheckdebugpair.second.second = ncount;
    if (ncount && *ncount == 1)
      memcheckdebug.push_back(memcheckdebugpair);
    cerr << "MEMCHECK: c4 (" << memcheckdebugcounter 
	 << "): node = " << node << "; ncount = " 
	 << ncount;
    memcheckdebugcounter++;
    if (ncount) {
      cerr << "; *ncount = " << *ncount << endl;
    } else {
      cerr << endl;
    }
#endif
  }
  Pointer(bool notinitialized) {
    if (!notinitialized) {
      node = new NodeType;
      ncount = new Int(1);
    } else {
      node = NULL;
      ncount = NULL;
    }
#ifdef MEMDEBUG
    memcheckdebugpair.first = (void*) node;
    memcheckdebugpair.second.first = memcheckdebugcounter;
    memcheckdebugpair.second.second = ncount;
    if (ncount && *ncount == 1)
      memcheckdebug.push_back(memcheckdebugpair);
    cerr << "MEMCHECK: c5 (" << memcheckdebugcounter 
	 << "): node = " << node << "; ncount = " 
	 << ncount;
    memcheckdebugcounter++;
    if (ncount) {
      cerr << "; *ncount = " << *ncount << endl;
    } else {
      cerr << endl;
    }
#endif
  }
  
  // copy constructor
  void SetTo(const Pointer<NodeType>& t) {
    if (node != t.node) {
      Destroy();
      node = t.node;
      ncount = t.ncount;
    }
    (*ncount)++;
#ifdef MEMDEBUG
    memcheckdebugpair.first = (void*) node;
    memcheckdebugpair.second.first = memcheckdebugcounter;
    memcheckdebugpair.second.second = ncount;
    if (ncount && *ncount == 1)
      memcheckdebug.push_back(memcheckdebugpair);
    cerr << "MEMCHECK: setto (" << memcheckdebugcounter 
	 << "): node = " << node << "; ncount = " 
	 << ncount;
    memcheckdebugcounter++;
    if (ncount) {
      cerr << "; *ncount = " << *ncount << endl;
    } else {
      cerr << endl;
    }
#endif
  }    
  Pointer(const Pointer<NodeType>& t) {
    node = NULL;
    ncount = NULL;
    SetTo(t);
  }

  // copy assignment: does NOT copy, it just references.
  Pointer<NodeType>& operator = (const Pointer<NodeType>& t) {
    SetTo(t);
    return *this;
  }

  // copy factory
  // 1. this is a copy of pointer
  void SetToCopyOf(const Pointer<NodeType>& t) {
    if (node != t.node || node == t.node && ncount && *ncount == 1) {
      Destroy();
    } else if (node == t.node) {
      assert(ncount);
      *ncount--;
    }
    // destroys generality to use the user-defined constructor to force
    // the copy, but what the heck, can't work miracles.
    node = new NodeType(t, true); 
    ncount = new Int(1);
#ifdef MEMDEBUG
    memcheckdebugpair.first = (void*) node;
    memcheckdebugpair.second.first = memcheckdebugcounter;
    memcheckdebugpair.second.second = ncount;
    if (ncount && *ncount == 1)
      memcheckdebug.push_back(memcheckdebugpair);
    cerr << "MEMCHECK: setcpof (" << memcheckdebugcounter 
	 << "): node = " << node << "; ncount = " 
	 << ncount;
    memcheckdebugcounter++;
    if (ncount) {
      cerr << "; *ncount = " << *ncount << endl;
    } else {
      cerr << endl;
    }
#endif
  }
  // 2. returns a copy of this
  Pointer<NodeType> Copy(void) const {
    Pointer<NodeType> ret(true); // uninitialized
    ret.SetToCopyOf(*this);
    return ret;
  }

  // destructor
  void Destroy(void) {
#ifdef MEMDEBUG
    bool found = false;
    for(memcheckdebugit = memcheckdebug.begin(); 
	memcheckdebugit != memcheckdebug.end();
	memcheckdebugit++) {
      if (memcheckdebugit->first == (void*) node) {
	found = true;
	break;
      }
    }
    int lineno = -1;
    if (found) {
      lineno = memcheckdebugit->second.first;
      if  (ncount && *ncount == 1) {
	memcheckdebug.erase(memcheckdebugit);
      }
    }
    cerr << "MEMCHECK: deleting (" << lineno 
	 << "): node = " << node << "; ncount = " 
	 << ncount;
    if (ncount) {
      cerr << "; *ncount = " << *ncount - 1;
    }
#endif
    if (ncount) {
      if (--(*ncount) == 0) {
	if (node) {
	  delete node; 
	  node = NULL;
	}
	delete ncount;
	ncount = NULL;
#ifdef MEMDEBUG
	cerr << "; deleted";
#endif
      }
    }
#ifdef MEMDEBUG
    cerr << endl;
#endif    
  }

  ~Pointer() {
    Destroy();
  }  

  // overload of ->
  NodeType* operator->() const {
    return node;
  }

  NodeType GetPointee(void) const {
    return *node;
  }

  // check for equality
  bool operator == (const Pointer<NodeType>& t) {
    if (node == t.node) {
      // fast check
      return true;
    } else {
      // use the NodeType::operator==
      if (*node == *(t.node)) {
	return true;
      }
    }
    return false;
  }

#ifdef MEMDEBUG
  NodeType* MemDebugGetNodePtr(void) {
    return node;
  }
  int MemDebugGetNCountPtr(void) {
    return ncount;
  }
#endif

};

template<class NodeType> class Tree {

private:

protected:

  // the vector containing the nodes
  vector<Pointer<NodeType> > nodes;
  
public:
  
  // constructor
  Tree() { }
  
  // destructor
  ~Tree() { 
    /* // SIGSEGVs -- investigate or bear the memleak
    int sz = nodes.size();
    for(int i = 0; i < sz; i++) {
      nodes[i].Destroy();
    }
    */
    DeleteAllNodes();
  }

  // Tree's methods
  
  // add a subnode
  void AddNode(const Pointer<NodeType> n) {
    nodes.push_back(n);
  }

  void AddCopyOfNode(const Pointer<NodeType> n) {
    nodes.push_back(n.Copy());
  }

  // delete a subnode
  bool DeleteNode(const Int i) {
    if (i >= (Int) nodes.size())
      return false;
    else {
      nodes.erase(nodes.begin() + i);
      return true;
    }
  }  

  // I would love to just return the iterators to this vector,
  // but GCC3.2 issues warnings against it, says it's deprecated.
  // I really don't see how, but still... Never mind, do it this
  // way. It's only ever used in Expression::ReorderNodes() anyway
  vector<Pointer<NodeType> >* GetNodeVectorPtr(void) {
    return &nodes;
  }

  // delete all subnodes
  void DeleteAllNodes(void) {
    nodes.erase(nodes.begin(), nodes.end());    
  }

  // get a subnode 
  Pointer<NodeType> GetNode(const Int i) const {
    //cout << "Tree::GetNode(): " << i << ", " << nodes.size() << endl;
    if (nodes.size() == 0) {
      throw ErrNotPermitted(30, "Tree", "GetNode", "nodes.size()==0",
			    "vector of nodes is empty", HELPURL);
    }
    if (i < (Int)nodes.size()) 
      return nodes[i];
    else
      throw ErrNotPermitted(32, "Tree", "GetNode", "i>=nodes.size()",
			    "not enough nodes in vector", HELPURL);
  }

  // get a subnode 
  Pointer<NodeType>* GetNodePtr(const Int i) {
    //cout << "Tree::GetNode(): " << i << ", " << nodes.size() << endl;
    if (nodes.size() == 0) {
      throw ErrNotPermitted(33, "Tree", "GetNodePtr", "nodes.size()==0",
			    "vector of nodes is empty", HELPURL);
    }
    if (i < (Int)nodes.size()) 
      return &(nodes[i]);
    else
      throw ErrNotPermitted(34, "Tree", "GetNodePtr", "i>=nodes.size()",
			    "not enough nodes in vector", HELPURL);
  }

  // get a copy of subnode 
  Pointer<NodeType> GetCopyOfNode(const Int i) {    
    Int j = i;
    if (nodes.size() == 0) {
      throw ErrNotPermitted(31, "Tree", "GetNode", "nodes.size()==0",
			    "vector of nodes is empty", HELPURL);
    }
    if (j >= (Int) nodes.size()) {
      throw ErrNotPermitted(35, "Tree", "GetCopyOfNode", "i>=nodes.size()",
			    "not enough nodes in vector", HELPURL);
    }
    Pointer<NodeType> ret;
    ret.SetToCopyOf(nodes[j]);
    return ret;
  }

  // get the size of nodes
  Int GetSize(void) const {
    return nodes.size();
  }

  // compare two trees
  bool operator == (const Tree<NodeType>& t) const {
    if (this == &t) {
      // fast check
      return true;
    } else {
      // recurse
      Int s = GetSize();
      if (s == t.GetSize()) {
	Int i;
	for(i = 0; i < s; i++) {
	  if (!(GetNode(i) == t.GetNode(i)))
	    return false;
	}
	return true;
      } else
	return false;
    }
  }

};

#endif
