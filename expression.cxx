//                                               -*- C++ -*-
/**
 *  @brief symbolic expression (base classes and functionality)
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

#define RCS4 "$Id: expression.cxx,v 1.27 2006/07/30 05:36:39 liberti Exp liberti $"

using namespace std;

#include <string>
#if __GNUC__ >= 3
#include <sstream>
#else
#include <strstream>
#endif
#include <cmath>
#include <algorithm>
#include <cassert>
#include "expression.h"

////////////// MEMCHECK debug ////////////////////
#ifdef MEMDEBUG
vector<pair<void*,pair<int,int*> > > memcheckdebug;
vector<pair<void*,pair<int,int*> > >::iterator memcheckdebugit;
pair<void*, pair<int, int*> > memcheckdebugpair;
int memcheckdebugcounter = 0;
#endif
///////////// END MEMCHECK debug ////////////////

////////////// auxiliary functions ///////////////
	   
#define PEV3PI 3.1415926535897932385
#define NOTVARNAME "_var_not_found_"
//#define VNAMEIDXCHAR "_"
#define VNAMEIDXCHAR ""

#define ISINTTOLERANCE 1e-8
bool is_integer(double a) {
  double b = fabs(a);
  int bi = (int) rint(b);
  if (fabs(b - bi) < ISINTTOLERANCE) {
    return true;
  } else {
    return false;
  }
}

bool is_even(double a) {
  if (is_integer(a)) {
    int ai = (int) rint(a);
    if (ai % 2 == 0) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool is_odd(double a) {
  if (is_integer(a)) {
    int ai = (int) rint(a);
    if (ai % 2 == 1) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

double Ev3NearZero(void) {
  // returns a very small positive value
  return 1/LARGE;
}

double Ev3Infinity(void) {
  // returns a very large positive value
  return LARGE;
}

double argmin(double a1, double a2, double a3, double a4) {
  double ret;
  ret = a1;
  if (a2 < ret)
    ret = a2;
  if (a3 < ret)
    ret = a3;
  if (a4 < ret)
    ret = a4;
  return ret;
}

double argmax(double a1, double a2, double a3, double a4) {
  double ret;
  ret = a1;
  if (a2 > ret)
    ret = a2;
  if (a3 > ret)
    ret = a3;
  if (a4 > ret)
    ret = a4;
  return ret;
}

void bilinearprodmkrange(double a, double b, double c, double d, 
			 double* wl, double *wu) {
  if ((a >= 0 && b >= 0 && c >= 0 && d >= 0) || \
      (a <  0 && b <  0 && c <  0 && d <  0)) {
    *wl = a * c;
    *wu = b * d;
  } else if (a <  0 && b <  0 && c >= 0 && d >= 0) {
    *wl = a * d;
    *wu = b * c;
  } else if (a >= 0 && b >= 0 && c <  0 && d <  0) {
    *wl = b * c;
    *wu = a * d;
  } else if (a <  0 && b >= 0 && c >= 0 && d >= 0) {
    *wl = a * d;
    *wu = b * d;
  } else if (a <  0 && b <  0 && c <  0 && d >= 0) {
    *wl = a * d;
    *wu = a * c;
  } else if (a >= 0 && b >= 0 && c <  0 && d >= 0) {
    *wl = b * c;
    *wu = b * d;
  } else if (a <  0 && b >= 0 && c <  0 && d <  0) {
    *wl = b * c;
    *wu = a * c;
  } else if (a <  0 && b >= 0 && c <  0 && d >= 0) {
    *wl = min(a * d, b * c);
    *wu = max(a * c, b * d);
  }
}
	
void fractionmkrange(double a, double b, double c, double d,
		     double* wl, double* wu) {
  if ((a >= 0 && b >= 0 && c >  0 && d >  0) || 
      (a <  0 && b <  0 && c <  0 && d <  0)) { 
    *wl = a / d;
    *wu = b / c;
  } else if ((a <  0 && b <  0 && c >  0 && d >  0) || 
	     (a >= 0 && b >= 0 && c <  0 && d <  0)) {
    *wl = b / c;
    *wu = a / d;
  } else if (a <  0 && b >= 0 && c <  0 && d <  0) {
    *wl = b / d;
    *wu = a / d;
  } else if (a <  0 && b >= 0 && c >  0 && d >  0) {
    *wl = a / c;
    *wu = b / c;
  } else if (c <  0 && d >  0) {
    *wl = -Ev3Infinity();
    *wu = Ev3Infinity();
  }
  if (c == 0 && d > 0) {
    if (a >= 0 && b >= 0) {
      *wl = a / d;
      *wu = Ev3Infinity();
    } else if (a < 0 && b >= 0) {
      *wl = -Ev3Infinity();
      *wu = Ev3Infinity();
    } else if (a < 0 && b < 0) {
      *wl = -Ev3Infinity();
      *wu = b / d;
    }
  } else if (c == 0 && d == 0) {
    *wl = -Ev3Infinity();
    *wu = Ev3Infinity();
  } else if (c < 0 && d == 0) {
    if (a >= 0 && b >= 0) {
      *wl = b / c;
      *wu = Ev3Infinity();
    } else if (a < 0 && b >= 0) {
      *wl = -Ev3Infinity();
      *wu = Ev3Infinity();
    } else if (a < 0 && b < 0) {
      *wl = -Ev3Infinity();
      *wu = a / c;
    }
  }
}   
	
void constpowermkrange(double a, double b, double c,
		       double* wl, double* wu) {
  double tmp = 0;
  double tmp1 = 0;
  if (is_integer(c)) {
    // exponent is integer
    tmp = std::pow(a, c);
    tmp1 = std::pow(b, c);
    if (is_even(c)) {
      if (a >= 0 && b >= 0) {
	*wl = tmp;
	*wu = tmp1;
      } else if (a < 0 && b < 0) {
	*wl = tmp1;
	*wu = tmp;
      } else {
	*wl = 0;
	*wu = max(tmp, tmp1);
      }
    } else {
      *wl = tmp;
      *wu = tmp1;
    }
  } else {
    // exponent not an integer: range can't go into negative numbers
    if (a < 0) {
      a = 0;
    }
    if (b < 0) {
      b = 0;
    }
    // calculate range
    tmp = std::pow(a, c);
    tmp1 = std::pow(b, c);
    if (tmp > tmp1) {
      double t = tmp;
      tmp = tmp1;
      tmp1 = t;
    }
    *wl = tmp;
    *wu = tmp1;
  }
  if (*wl <= -Ev3Infinity()) 
    *wl = -Ev3Infinity();
  if (*wu >= Ev3Infinity())
    *wu = Ev3Infinity();
}

void powermkrange(double a, double b, double c, double d,
		  double* wl, double* wu) {
  if (fabs(d-c) < 1/LARGE) {
    constpowermkrange(a, b, c, wl, wu);
    return;
  }
  // WARNING: don't have time to do it properly now
  double t1 = 0;
  double t2 = 0;
  double t3 = 0;
  double t4 = 0;
  t1 = std::pow(a, c);
  t2 = std::pow(a, d);
  t3 = std::pow(b, c);
  t4 = std::pow(b, d);
  *wl = argmin(t1, t2, t3, t4);
  *wu = argmax(t1, t2, t3, t4);

  cerr << "expression.cxx::powermkrange(): method not functional\n";
  exit(148);
}

///////////// classes ////////////////

// constructors
Operand::Operand() : 
  oplabel(CONST), constant(0), varindex(NOVARIABLE), 
  coefficient(1), exponent(1), dependency(0), 
  depconstant(NULL), depcoefficient(NULL), depexponent(NULL) { }

Operand::Operand(double t) : 
  oplabel(CONST), constant(t), varindex(NOVARIABLE),
  coefficient(1), exponent(1), dependency(0),
  depconstant(NULL), depcoefficient(NULL), depexponent(NULL) { }

Operand::Operand(const Int t) : 
  oplabel(CONST), constant(t), varindex(NOVARIABLE),
  coefficient(1), exponent(1), dependency(0),
  depconstant(NULL), depcoefficient(NULL), depexponent(NULL) { }

Operand::Operand(const Int t, const bool isvar) :   
  coefficient(1), exponent(1), dependency(0),
  depconstant(NULL), depcoefficient(NULL), depexponent(NULL) { 
  if (isvar) {
    // make it a variable
    oplabel = VAR;
    constant = 0;
    varindex = t;
  } else {
    // make it an operator label
    oplabel = (int) t;
    constant = 0;
    varindex = NOVARIABLE;
  }
}

// create an (empty) operator or a variable leaf and set coefficient
Operand::Operand(const double c, const Int t, string vn) : 
  coefficient(c), exponent(1) { 
  // make it a variable
  oplabel = VAR;
  constant = 0;
  varindex = t;
  varname = vn;
  dependency = 0;
  depconstant = NULL;
  depcoefficient = NULL;
  depexponent = NULL;
}

// copy constructor
Operand::Operand(const Operand& t) : 
  oplabel(t.oplabel), constant(t.constant), depconstant(t.depconstant), 
  varindex(t.varindex), varname(t.varname), coefficient(t.coefficient), 
  depcoefficient(t.depcoefficient), exponent(t.exponent), 
  depexponent(t.depexponent), dependency(t.dependency) { }

// destructor
Operand::~Operand() { }

// Operand class methods:

string Operand::ToString(void) const {
#if __GNUC__ >= 3
  stringstream outbuf;  
#else
  strstream outbuf;
#endif
  string vn;
  if (GetCoeff() == 0) {
    // coefficient is 0
    outbuf << 0;
  } else if (GetOpType() == CONST) {
    // constant
    outbuf << GetValue();
  } else if (GetOpType() == VAR) {
    // variable
    if (GetCoeff() == 1) {
      int vi = GetVarIndex();
      if (vi == NOVARIABLE) {
	if (GetExponent() == 1) {
	  outbuf << NOTVARNAME;
	} else {
	  outbuf << NOTVARNAME << "^" << GetExponent();
	}
      } else {
	vn = GetVarName();
	if (GetExponent() == 1) {
	  outbuf << vn << VNAMEIDXCHAR << vi;
	} else {
	  outbuf << vn << VNAMEIDXCHAR << vi << "^" << GetExponent();
	}
      }
    } else {
      int vi = GetVarIndex();
      if (vi == NOVARIABLE) {
	if (GetExponent() == 1) {
	  outbuf << GetCoeff() << "*_" << NOTVARNAME;
	} else {
	  outbuf << GetCoeff() << "*" << NOTVARNAME << "^" 
                 << GetExponent();
	}
      } else {
	vn = GetVarName();
	if (GetExponent() == 1) {
	  outbuf << GetCoeff() << "*" << vn << VNAMEIDXCHAR << vi;
	} else {
	  outbuf << GetCoeff() << "*" << vn << VNAMEIDXCHAR << vi << "^" 
		 << GetExponent();
	}
      }
    }
  } else {
    // operand, don't print anything
    ;
  }
  return outbuf.str();
}

// get operator type
int Operand::GetOpType(void) const { return oplabel; }
  
// get constant value - in CONSTs, multiply by coeff. and raise 
// to exponent, first
double Operand::GetValue(void) const { 
  double ret = 0;
  if (oplabel == CONST && dependency == 0) {
    if (exponent == 1) 
      ret = coefficient * constant;
    else if (exponent == 2)
      ret = coefficient * constant * constant;
    else
      ret = coefficient * pow(constant, exponent);
  } else if (oplabel == CONST && dependency == 1 && depconstant) {
    ret = *depconstant;
  } else {
    ret = constant; 
  }
  return ret;
}

// just get the value in any case
double Operand::GetSimpleValue(void) const { 
  if (dependency == 1 && depconstant) {
    return *depconstant;
  } else {
    return constant; 
  }
}

// node bounds
double Operand::GetLB(void) const {
  return lb;
}
double Operand::GetUB(void) const {
  return ub;
}
void Operand::SetLB(double theLB) {
  lb = theLB;
}
void Operand::SetUB(double theUB) {
  ub = theUB;
}

// get variable index
Int Operand::GetVarIndex(void) const { return varindex; }

// get variable name
string Operand::GetVarName(void) const { return varname; }

// set operator type
void Operand::SetOpType(const int t) { oplabel = t; }
  
// set constant value
void Operand::SetValue(const double t) { oplabel = CONST; constant = t; }

// set variable index
void Operand::SetVarIndex(const Int t) { oplabel = VAR; varindex = t; }

// set variable name
void Operand::SetVarName(const string vn) { oplabel = VAR; varname = vn; }

// is operand a constant?
bool Operand::IsConstant(void) const {
  return (GetOpType() == CONST);
}

// is operand a variable?
bool Operand::IsVariable(void) const {
  return (GetOpType() == VAR);
}

// is operand a leaf node?
bool Operand::IsLeaf(void) const {
  return (IsConstant() || IsVariable());
}

void Operand::SetCoeff(const double coeff) {
  coefficient = coeff;
}

double Operand::GetCoeff(void) const {
  if (dependency == 2 && depcoefficient) {
    return *depcoefficient;
  } else {
    return coefficient;
  }
}

void Operand::SetExponent(const double expon) {
  exponent = expon;
}

double Operand::GetExponent(void) const {
  if (dependency == 3 && depexponent) {
    return *depexponent;
  } else {
    return exponent;
  }
}

void Operand::SetDependencyOnOperand(const int whichconstant, 
				     double** depvalue) {
  dependency = whichconstant + 1;
  switch(dependency) {
  case 1:
    depconstant = *depvalue;
    break;
  case 2:
    depcoefficient = *depvalue;
    break;
  case 3:
    depexponent = *depvalue;
    break;
  }
}

void Operand::EnforceDependencyOnOperand(void) {
  switch(dependency) {
  case 1:
    constant = *depconstant;
    break;
  case 2:
    coefficient = *depcoefficient;
    break;
  case 3:
    exponent = *depexponent;
    break;
  }
}  

void Operand::ConsolidateValue(void) {
  SetValue(GetValue());
  SetCoeff(1.0);
  SetExponent(1.0);
}

// is operand a zero constant?
bool Operand::IsZero(void) const {
  if (GetOpType() == CONST) {
    double c = GetValue();
    if (fabs(c) < Ev3NearZero()) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}
  
// is operand a constant having value v?
bool Operand::HasValue(double v) const {
  if (GetOpType() == CONST) {
    double c = GetValue();
    double t1 = v + Ev3NearZero();
    double t2 = v - Ev3NearZero();
    if (c < t1 && c > t2) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}  

// is operand a negative constant?
bool Operand::IsLessThan(double v) const {
  if (GetOpType() == CONST) {
    if (GetValue() < v + Ev3NearZero()) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

// is operand a negative constant?
bool Operand::IsGreaterThan(double v) const {
  if (GetOpType() == CONST) {
    if (GetValue() > v - Ev3NearZero()) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}


// is operand this == operand t?
bool Operand::operator == (const Operand& t) {
  if (this == &t) {
    // first fast check
    return true;
  } else {
    // not the same operand - check data fields
    return (GetOpType() == t.GetOpType() &&
	    GetValue() == t.GetValue() &&
	    GetVarIndex() == t.GetVarIndex());
  }
}      

// substitute a variable with a constant
void Operand::SubstituteVariableWithConstant(int varindex, double c) {
  if (GetOpType() == VAR && GetVarIndex() == varindex) {  
    SetOpType(CONST);
    SetVarIndex(NOVARIABLE);
    double t;
    t = GetCoeff() * pow(c, GetExponent());
    SetCoeff(1);
    SetExponent(1);
    SetValue(t);    
  }
}


// create empty
BasicExpression::BasicExpression() { FastEvalAllocated = false; }

// create a constant leaf
BasicExpression::BasicExpression(const double t) : Operand(t) { 
  FastEvalAllocated = false;
}

// create a constant (integer-valued) leaf
BasicExpression::BasicExpression(const Int t) : Operand(t) { 
  FastEvalAllocated = false;
}

// create an (empty) operator or a variable leaf
BasicExpression::BasicExpression(const Int t, const bool isvar) : 
  Operand(t, isvar) { 
  FastEvalAllocated = false;
}
  
// create a variable leaf and set coefficient
BasicExpression::BasicExpression(const double c, const Int t, string vn) : 
  Operand(c, t, vn) { 
  FastEvalAllocated = false;
}

// user-defined copy constructor with two options
BasicExpression::BasicExpression(const Expression& t, const bool iscopy) :
  Operand(t.GetPointee()) {
  Int s = t->GetSize();
  if (iscopy) {
    // create a _copy_ of t, subnode by subnode
    for (int i = 0; i < s; i++) {
      nodes.push_back(t->GetCopyOfNode(i));
    }
  } else {
    // create a copy of the pointers in t
    for (int i = 0; i < s; i++) {
      nodes.push_back(t->GetNode(i));
    }
  }
  FastEvalAllocated = false;
}

// copy constructor
BasicExpression::BasicExpression(const BasicExpression& t) : Operand(t) { 
  Int s = t.GetSize();
  // necessary for constructs "(BasicExpression) e =" where e already 
  // existed prior to assignment  
  // -- segvs (?)
  for(int i = 0; i < GetSize(); i++) {
    RecursiveDestroy(GetNodePtr(i));
  }
  DeleteAllNodes(); 
  // create a copy of the subnode pointers in t
  for (int i = 0; i < s; i++) {
    nodes.push_back(t.GetNode(i));
  }
  FastEvalAllocated = false;
}

// destructor
BasicExpression::~BasicExpression() { 
  if (FastEvalAllocated) {
    DeleteFastEvalTree();
  }
}

// BasicExpression class methods:

void BasicExpression::DeleteFastEvalTree(void) {
  if (FastEvalAllocated) {
    DeleteFastEvalTreeRecursive(fetree);
  }
  delete fetree;
  FastEvalAllocated = false;
}

void BasicExpression::DeleteFastEvalTreeRecursive(FastEvalTree* fet) {
  for(Int i = 0; i < fet->nodesize; i++) {
    DeleteFastEvalTreeRecursive(&fet->nodes[i]);
  }
  delete [] fet->nodes;
}

void BasicExpression::CreateFastEvalTree(void) {
  if (FastEvalAllocated) {
    DeleteFastEvalTree();
  }
  if (!FastEvalAllocated) {
    fetree = new FastEvalTree;
    CreateFastEvalTreeRecursive(fetree);
  }
}

void BasicExpression::CreateFastEvalTreeRecursive(FastEvalTree* fet) {
  Int s = GetSize();
  fet->optype = GetOpType();
  fet->coeff = GetCoeff();
  fet->depcoeff = depcoefficient;
  fet->exponent = GetExponent();
  fet->depexponent = depexponent;
  fet->value = GetValue();
  fet->depvalue = depconstant;
  fet->varindex = GetVarIndex();
  fet->nodesize = s;
  fet->nodes = new FastEvalTree [fet->nodesize];
  for(Int i = 0; i < s; i++) {
    GetNode(i)->CreateFastEvalTreeRecursive(&fet->nodes[i]);
  }
}

void BasicExpression::Debug (void) const {
  Int s = GetSize();
  cerr << "BasicExpression: Debug:\n";
  cerr << "\tthis   = " << this << endl;
  cerr << "\toptype = " << GetOpType() << endl;
  cerr << "\tnodes  = " << s << endl;
  Int i;
  for (i = 0; i < s; i++) {
    cerr << "\tnode " << i << ": " << GetNode(i)->GetOpType() << endl;
  }
}  

void BasicExpression::Zero(void) {
  // -- segvs (?)
  for(int i = 0; i < GetSize(); i++) {
    RecursiveDestroy(GetNodePtr(i));
  }
  DeleteAllNodes();
  SetCoeff(1.0);
  SetExponent(1.0);
  SetValue(0.0);
  SetOpType(CONST);
}

void BasicExpression::One(void) {
  // -- segvs (?)
  for(int i = 0; i < GetSize(); i++) {
    RecursiveDestroy(GetNodePtr(i));
  }
  DeleteAllNodes();
  SetCoeff(1.0);
  SetExponent(1.0);
  SetValue(1.0);
  SetOpType(CONST);
}

string BasicExpression::PrintTree(int blanks, int tabs) const {
#if __GNUC__ >= 3
  stringstream outbuf;
#else
  strstream outbuf;
#endif 
  string b(blanks, ' ');
  if (IsLeaf()) {
    outbuf << b << Operand::ToString();
  } else {
    outbuf << b << "OP[" << GetOpType() << "](" << endl;
    for(int i = 0; i < GetSize(); i++) {
      outbuf << GetNode(i)->PrintTree(blanks + tabs, tabs);
      if (i < GetSize() - 1) {
        outbuf << ",";
      }
      outbuf << endl;
    }
    outbuf << b << ")";
  }
  return outbuf.str();
}

string BasicExpression::ToString(void) const {
#if __GNUC__ >= 3
  stringstream outbuf;
#else
  strstream outbuf;
#endif  
  Int i, s;
  if (IsLeaf()) {
    // leaf node, use Operand::ToString()
    return Operand::ToString();
  } else {
    // operator node
    if (GetCoeff() != 1) {
      outbuf << "(" << GetCoeff() << "*(";
    }
    s = GetSize();
    if (s > 1) {
      string t;
      for (i = 0; i < s; i++) {
	t = GetNode(i)->ToString();
	outbuf << "(" << t << ")";
	if (i < s - 1) {
	  switch(GetOpType()) {
	  case SUM:
	    outbuf << "+";
	    break;
	  case DIFFERENCE:
	    outbuf << "-";
	    break;
	  case PRODUCT:
	    outbuf << "*";
	    break;
	  case FRACTION:
	    outbuf << "/";
	    break;
	  case POWER:
	    outbuf << "^";
	    break;
	  default:
	    outbuf << "UNKNOWNOP";
	    break;
	  }
	}
      }    
    } else {
      switch(GetOpType()) {
      case PLUS:
	break;
      case MINUS:
	outbuf << "-";
	break;
      case LOG:
	outbuf << "log";
	break;
      case EXP:
	outbuf << "exp";
	break;
      case SIN:
	outbuf << "sin";
	break;
      case COS:
	outbuf << "cos";
	break;
      case TAN:
	outbuf << "tan";
	break;
      case COT:
	outbuf << "cot";
	break;
      case SINH:
	outbuf << "sinh";
	break;
      case COSH:
	outbuf << "cosh";
	break;
      case TANH:
	outbuf << "tanh";
	break;
      case COTH:
	outbuf << "coth";
	break;
      case SQRT:
	outbuf << "sqrt";
	break;
      default:
	outbuf << "UNKNOWNOP";
      }
      if (s == 1) {
	string t(GetNode(0)->ToString());
	outbuf << "(" << t << ")";
      } else {
	// no arguments - error
	outbuf << "(NOARG)";
      }
    }
    if (GetCoeff() != 1) {
      outbuf << "))"; 
    }
    return outbuf.str();
  }
}

// is expression this == expression t?
bool BasicExpression::IsEqualTo(const Expression& t) const {
  if (IsEqualToNoCoeff(t)) {
    if (GetCoeff() == t->GetCoeff() && GetExponent() == t->GetExponent())
      return true;
    else
      return false;
  } else 
    return false;
}

bool BasicExpression::operator == (const BasicExpression& t) const {
  // fast checks
  if (IsLeaf() && t.IsLeaf()) {
    if (GetOpType() == t.GetOpType()) {
      if (GetOpType() == CONST) {
	// if both are constants, they're always equal up to coefficient
	return true;
      } else if (GetOpType() == VAR && GetVarIndex() == t.GetVarIndex() &&
		 GetExponent() == t.GetExponent())
	return true;
      else
	return false;
    } else
      return false;
  } else if ((!IsLeaf()) && (!t.IsLeaf())) {
    // both BasicExpressions are not leaves, recurse using PRECISE
    // (i.e. not up to coefficient) form
    if (GetSize() != t.GetSize()) 
      return false;
    if (GetOpType() != t.GetOpType())
      return false;
    Int i;
    bool ret = true;
    for (i = 0; i < GetSize(); i++) {
      if (!(GetNode(i)->IsEqualTo(t.GetNode(i)))) {
	ret = false;
	break;
      }
    }
    if (ret) {
      return true;
    }
    return false;
  } else
    return false;
}

bool BasicExpression::IsEqualTo(const double t) const {
  return (IsLeaf() && GetOpType() == CONST && GetValue() == t);
}

int BasicExpression::NumberOfVariables() const {
  int maxvi = 0;
  return NumberOfVariables(maxvi);
}

int BasicExpression::NumberOfVariables(int& maxvi) const {
  int newvi = 0;
  if (IsVariable()) {
    newvi = GetVarIndex();
    if (newvi > maxvi) {
      maxvi = newvi;
    }
    return maxvi;
  } else if (!IsLeaf()) {
    for(int i = 0; i < GetSize(); i++) {
      newvi = GetNode(i)->NumberOfVariables(maxvi);
      if (newvi > maxvi) {
	maxvi = newvi;
      }
    }
    return maxvi;
  } else {
    return 0;
  }
}


// taken and adapted from operator ==
bool BasicExpression::IsEqualToNoCoeff(const Expression& t) const {
  // fast checks
  if (IsLeaf() && t->IsLeaf()) {
    if (GetOpType() == t->GetOpType()) {
      if (GetOpType() == CONST) {
	// if both are constants, they're always equal up to coefficient
	return true;
      } else if (GetOpType() == VAR && GetVarIndex() == t->GetVarIndex() &&
		 GetExponent() == t->GetExponent())
	return true;
      else
	return false;
    } else
      return false;
  } else if ((!IsLeaf()) && (!t->IsLeaf())) {
    // both BasicExpressions are not leaves, recurse using PRECISE
    // (i.e. not up to coefficient) form
    if (GetSize() != t->GetSize()) 
      return false;
    if (GetOpType() != t->GetOpType())
      return false;
    Int i;
    bool ret = true;
    for (i = 0; i < GetSize(); i++) {
      if (!(GetNode(i)->IsEqualTo(t->GetNode(i)))) {
	ret = false;
	break;
      }
    }
    if (ret) {
      return true;
    }
    return false;
  } else
    return false;
}

bool BasicExpression::IsEqualBySchema(const Expression& t) const {
  if (IsLeaf() && t->IsLeaf()) {
    if (GetOpType() == t->GetOpType() && 
	IsLinear() == t->IsLinear()) {
      return true;
    } else {
      return false;
    }
  } else if ((!IsLeaf()) && (!t->IsLeaf())) {
    // both BasicExpressions are not leaves, recurse 
    if (GetSize() != t->GetSize()) {
      return false;
    }
    if (GetOpType() != t->GetOpType()) {
      return false;
    }
    Int i;
    bool ret = true;
    for (i = 0; i < GetSize(); i++) {
      if (!(GetNode(i)->IsEqualBySchema(t->GetNode(i)))) {
	ret = false;
	break;
      }
    }
    if (ret) {
      return true;
    }
    return false;
  } else {
    return false;
  }
}

bool BasicExpression::IsEqualByOperator(const int theoplabel) const {
  if (GetOpType() == theoplabel) {
    return true;
  } else {
    return false;
  }
}

double BasicExpression::Eval(double* varvalues, Int vsize) const {
  map<Int,Int> varmap;
  for(Int i = 1; i <= vsize; i++) {
    varmap[i] = i;
  }
  return Eval(varvalues, varmap, vsize);
}

double BasicExpression::Eval(double* varvalues, map<int,int>& varmap, 
			     Int vsize) const {
  // evaluate expression
  if (GetOpType() == CONST) {
    return GetValue();
  } else if (GetOpType() == VAR) {
    if (GetVarIndex() > vsize) {
      throw ErrNotPermitted(0, "BasicExpression", "Eval", "varindex > vsize",
			    "varindex should be <= vsize", HELPURL);
    } else if (GetVarIndex() <= 0) {
      throw ErrNotPermitted(1, "BasicExpression", "Eval", "varindex <= 0", 
                            "varindex should be > 0", HELPURL);
    }
    double ret = GetCoeff() * pow(varvalues[varmap[varindex] - 1], 
				  GetExponent());
    return ret;
  } else { 
    Int i;
    double ret = 0;
    double tmp = 0;
    int op = GetOpType();
    if (GetSize() == 0) {
      throw ErrNotPermitted(5, "BasicExpression", "Eval", "GetSize()==0", 
			    "non-leaf expressions without subnodes", 
			    HELPURL);
    }
    switch(op) {
    case SUM:
      for(i = 0; i < GetSize(); i++) {
	ret += GetNode(i)->Eval(varvalues, varmap, vsize);
      }
      ret *= GetCoeff();
      break;
    case DIFFERENCE:
      for(i = 0; i < GetSize(); i++) {
	ret -= GetNode(i)->Eval(varvalues, varmap, vsize);
      }
      ret *= GetCoeff();
      break;
    case PRODUCT:
      ret = 1;
      for(i = 0; i < GetSize(); i++) {
	ret *= GetNode(i)->Eval(varvalues, varmap, vsize);
      }
      ret *= GetCoeff();
      break;
    case FRACTION:
      if (GetSize() != 2) {
	cerr << "BasicExpression::Eval: in [GetSize()!=2]: "
	     << "fractions should have just 2 operands, but going ahead "
	     << "anyway...\n";
	//throw ErrNotPermitted(1, "BasicExpression", "Eval", "GetSize()!=2", 
	//"fractions should have just 2 operands", 
	//HELPURL);
      }
      for(i = 0; i < GetSize(); i++) {
	tmp = GetNode(i)->Eval(varvalues, varmap, vsize);
	if (i > 0 && tmp == 0) {
	  cerr << "BasicExpression::Eval: division by zero not allowed (node "
	       << i << " evaluates to 0), setting to a large value" << endl;
	  //throw ErrDivideByZero(2, "BasicExpression", "Eval", "divisor==0", 
	  //"division by zero not allowed", HELPURL);
	  tmp = Ev3NearZero();
	}
	if (i == 0) 
	  ret = tmp;
	else 
	  ret /= tmp;
      }
      ret *= GetCoeff();
      break;
    case POWER:
      if (GetSize() != 2) {
	cerr << "BasicExpression::Eval: in [GetSize()!=2]: "
	     << "powers should have just 2 operands, but going ahead "
	     << "anyway...\n";
	//throw ErrNotPermitted(3, "BasicExpression", "Eval", "GetSize()!=2", 
	//"powers should have just 2 operands", 
	//HELPURL);
      }
      ret = GetNode(0)->Eval(varvalues, varmap, vsize);
      for(i = 1; i < GetSize(); i++) {
	ret = pow(ret, GetNode(i)->Eval(varvalues, varmap, vsize));
      }
      ret *= GetCoeff();
      break;
    // for all unary functions - should check that GetSize() == 1
    case PLUS:
      ret = GetCoeff() * GetNode(0)->Eval(varvalues, varmap, vsize);
      break;
    case MINUS:
      ret = -GetCoeff() * GetNode(0)->Eval(varvalues, varmap, vsize);
      break;
    case LOG:
      tmp = GetNode(0)->Eval(varvalues, varmap, vsize);
      if (tmp < 0) {
	cerr << "BasicExpression::Eval: log of negative not allowed ("
	     << "argument evaluates to < 0), taking absolute value" << endl;
	//throw ErrNotPermitted(6, "BasicExpression", "Eval", "log arg < 0", 
	//"log of negative not allowed", HELPURL);
	tmp = -tmp;
      } else if (tmp == 0) {
	cerr << "BasicExpression::Eval: log of zero not allowed ("
	     << "argument evaluates to 0), setting to large negative value" 
	     << endl;
	//throw ErrNotPermitted(7, "BasicExpression", "Eval", "log arg == 0", 
	//"log of zero not allowed", HELPURL);
	tmp = Ev3NearZero();
      }
      ret = GetCoeff() * log(tmp);
      break;
    case EXP:
      ret = GetCoeff() * exp(GetNode(0)->Eval(varvalues, varmap, vsize));
      break;
    case SIN:
      ret = GetCoeff() * sin(GetNode(0)->Eval(varvalues, varmap, vsize));
      break;
    case COS:
      ret = GetCoeff() * cos(GetNode(0)->Eval(varvalues, varmap, vsize));
      break;
    case TAN:
      ret = GetCoeff() * tan(GetNode(0)->Eval(varvalues, varmap, vsize));
      // should check that cos() is nonzero
      break;
    case SINH:
      ret = GetCoeff() * sinh(GetNode(0)->Eval(varvalues, varmap, vsize));
      break;
    case COSH:
      ret = GetCoeff() * cosh(GetNode(0)->Eval(varvalues, varmap, vsize));
      break;
    case TANH:
      ret = GetCoeff() * tanh(GetNode(0)->Eval(varvalues, varmap, vsize));
      // should check that cosh is nonzero
      break; 
    case COTH:
      ret = GetCoeff() / tanh(GetNode(0)->Eval(varvalues, varmap, vsize));
      // should check that tanh is nonzero
      break;
    case SQRT:
      tmp = GetNode(0)->Eval(varvalues, varmap, vsize);
      if (tmp < 0) {
	cerr << "BasicExpression::Eval: sqrt of negative not allowed, "
	     << "taking absolute value" << endl;
	//throw ErrNotPermitted(9, "BasicExpression", "Eval", "sqrt arg < 0", 
	//"sqrt of negative not allowed", HELPURL);
	tmp = -tmp;
      }
      ret = GetCoeff() * sqrt(tmp);
      break;
    }
    return ret;
  }
}

double BasicExpression::FastEval(double* varvalues, Int vsize) {
  if (!FastEvalAllocated) {
    CreateFastEvalTree();
  }
  return FastEvalRecursive(fetree, varvalues, NULL, vsize);
}

double BasicExpression::FastEval(double* varvalues, map<int,int>& varmap, 
				 Int vsize) {
  if (!FastEvalAllocated) {
    CreateFastEvalTree();
  }
  return FastEvalRecursive(fetree, varvalues, &varmap, vsize);
}

double BasicExpression::FastEval(FastEvalTree* fet, 
				 double* varvalues, Int vsize) {
  if (!FastEvalAllocated) {
    CreateFastEvalTree();
  }
  return FastEvalRecursive(fet, varvalues, NULL, vsize);
}

double BasicExpression::FastEval(FastEvalTree* fet, double* varvalues, 
				 map<int,int>& varmap, Int vsize) {
  if (!FastEvalAllocated) {
    CreateFastEvalTree();
  }
  return FastEvalRecursive(fet, varvalues, &varmap, vsize);
}

double BasicExpression::FastEvalRecursive(FastEvalTree* fet, 
					  double* varvalues, 
					  map<int,int>* varmap, 
					  Int vsize) {
  double thecoeff = fet->depcoeff ? *(fet->depcoeff) : fet->coeff;
  double theexpon = fet->depexponent ? *(fet->depexponent) : fet->exponent;
  double thevalue = fet->depvalue ? *(fet->depvalue) : fet->value;
  // evaluate expression
  if (fet->optype == CONST) {
    return thevalue;
  } else if (fet->optype == VAR) {
    if (fet->varindex > vsize) {
      throw ErrNotPermitted(0, "BasicExpression", "FastEvalRecursive", 
			    "fet->varindex > vsize",
			    "fet->varindex should be <= vsize", HELPURL);
    } else if (fet->varindex <= 0) {
      throw ErrNotPermitted(1, "BasicExpression", "FastEvalRecursive", 
			    "fet->varindex <= 0", 
			    "fet->varindex should be > 0", 
			    HELPURL);
    }
    double ret = 0;
    if (varmap) {
      ret = varvalues[(*varmap)[fet->varindex] - 1];
    } else {
      ret = varvalues[fet->varindex - 1];
    }
    ret = thecoeff * pow(ret, theexpon);
    return ret;
  } else { 
    Int i;
    double ret = 0;
    double tmp = 0;
    int op = fet->optype;
    if (fet->nodesize == 0) {
      throw ErrNotPermitted(5, "BasicExpression", "FastEvalRecursive", 
			    "GetSize()==0", 
			    "non-leaf expressions without subnodes", 
			    HELPURL);
    }
    switch(op) {
    case SUM:
      for(i = 0; i < fet->nodesize; i++) {
	ret += FastEvalRecursive(&fet->nodes[i], varvalues, varmap, vsize);
      }
      ret *= thecoeff;
      break;
    case DIFFERENCE:
      for(i = 0; i < fet->nodesize; i++) {
	ret -= FastEvalRecursive(&fet->nodes[i], varvalues, varmap, vsize);
      }
      ret *= thecoeff;
      break;
    case PRODUCT:
      ret = 1;
      for(i = 0; i < fet->nodesize; i++) {
	ret *= FastEvalRecursive(&fet->nodes[i], varvalues, varmap, vsize);
      }
      ret *= thecoeff;
      break;
    case FRACTION:
      if (fet->nodesize != 2) {
	cerr << "BasicExpression::FastEvalRecursive: in [GetSize()!=2]: "
	     << "fractions should have just 2 operands, but going ahead "
	     << "anyway...\n";
	//throw ErrNotPermitted(1, "BasicExpression", "Eval", "GetSize()!=2", 
	//"fractions should have just 2 operands", 
	//HELPURL);
      }
      for(i = 0; i < fet->nodesize; i++) {
	tmp = FastEvalRecursive(&fet->nodes[i], varvalues, varmap, vsize);
	if (i > 0 && tmp == 0) {
	  cerr << "BasicExpression::FastEvalRecursive: "
	       << "division by zero not allowed (node "
	       << i << " evaluates to 0), setting to a large value" << endl;
	  //throw ErrDivideByZero(2, "BasicExpression", "Eval", "divisor==0", 
	  //"division by zero not allowed", HELPURL);
	  tmp = Ev3NearZero();
	}
	if (i == 0) 
	  ret = tmp;
	else 
	  ret /= tmp;
      }
      ret *= thecoeff;
      break;
    case POWER:
      if (fet->nodesize != 2) {
	cerr << "BasicExpression::FastEvalRecursive: in [GetSize()!=2]: "
	     << "powers should have just 2 operands, but going ahead "
	     << "anyway...\n";
	//throw ErrNotPermitted(3, "BasicExpression", "Eval", "GetSize()!=2", 
	//"powers should have just 2 operands", 
	//HELPURL);
      }
      ret = FastEvalRecursive(&fet->nodes[0], varvalues, varmap, vsize);
      for(i = 1; i < fet->nodesize; i++) {
	ret = pow(ret, FastEvalRecursive(&fet->nodes[i], varvalues, varmap, 
					 vsize));
      }
      ret *= thecoeff;
      break;
    // for all unary functions - should check that GetSize() == 1
    case PLUS:
      ret = thecoeff * FastEvalRecursive(&fet->nodes[0], 
					   varvalues, varmap, vsize);
      break;
    case MINUS:
      ret = -thecoeff * FastEvalRecursive(&fet->nodes[0], 
					    varvalues, varmap, vsize);
      break;
    case LOG:
      tmp = FastEvalRecursive(&fet->nodes[0], varvalues, varmap, vsize);
      if (tmp < 0) {
	cerr << "BasicExpression::FastEvalRecursive: log of negative not allowed ("
	     << "argument evaluates to < 0), taking absolute value" << endl;
	//throw ErrNotPermitted(6, "BasicExpression", "Eval", "log arg < 0", 
	//"log of negative not allowed", HELPURL);
	tmp = -tmp;
      } else if (tmp == 0) {
	cerr << "BasicExpression::FastEvalRecursive: log of zero not allowed ("
	     << "argument evaluates to 0), setting to large negative value" 
	     << endl;
	//throw ErrNotPermitted(7, "BasicExpression", "Eval", "log arg == 0", 
	//"log of zero not allowed", HELPURL);
	tmp = Ev3NearZero();
      }
      ret = thecoeff * log(tmp);
      break;
    case EXP:
      ret = thecoeff * exp(FastEvalRecursive(&fet->nodes[0], 
					       varvalues, varmap, vsize));
      break;
    case SIN:
      ret = thecoeff * sin(FastEvalRecursive(&fet->nodes[0], 
					       varvalues, varmap, vsize));
      break;
    case COS:
      ret = thecoeff * cos(FastEvalRecursive(&fet->nodes[0], 
					       varvalues, varmap, vsize));
      break;
    case TAN:
      ret = thecoeff * tan(FastEvalRecursive(&fet->nodes[0], 
					       varvalues, varmap, vsize));
      // should check that cos() is nonzero
      break;
    case SINH:
      ret = thecoeff * sinh(FastEvalRecursive(&fet->nodes[0], 
						varvalues, varmap, vsize));
      break;
    case COSH:
      ret = thecoeff * cosh(FastEvalRecursive(&fet->nodes[0], 
						varvalues, varmap, vsize));
      break;
    case TANH:
      ret = thecoeff * tanh(FastEvalRecursive(&fet->nodes[0], 
						varvalues, varmap, vsize));
      // should check that cosh is nonzero
      break; 
    case COTH:
      ret = thecoeff / tanh(FastEvalRecursive(&fet->nodes[0], 
					      varvalues, varmap, vsize));
      // should check that tanh is nonzero
      break;
    case SQRT:
      tmp = FastEvalRecursive(&fet->nodes[0], varvalues, varmap, vsize);
      if (tmp < 0) {
	cerr << "BasicExpression::FastEvalRecursive: "
	     << "sqrt of negative not allowed, "
	     << "taking absolute value" << endl;
	//throw ErrNotPermitted(9, "BasicExpression", "Eval", "sqrt arg < 0", 
	//"sqrt of negative not allowed", HELPURL);
	tmp = -tmp;
      }
      ret = thecoeff * sqrt(tmp);
      break;
    }
    return ret;
  }
}

FastEvalTree* BasicExpression::GetFastEvalTree(void) {
  if (!FastEvalAllocated) {
    CreateFastEvalTree();
  }
  return fetree;
}

bool BasicExpression::DependsOnVariable(Int vi) const {
  if (IsLeaf()) {
    if (GetOpType() == VAR) {
      if (GetVarIndex() == vi) 
	return true;
      else
	return false;
    } else
      return false;
  } else {
    Int i;
    bool ret = false;
    for (i = 0; i < GetSize(); i++) {
      ret = GetNode(i)->DependsOnVariable(vi);
      if (ret)
	return true;
    }
    return false;
  }
}

int BasicExpression::DependsLinearlyOnVariable(Int vi) const {
  if (IsVariable()) {
    if (GetVarIndex() == vi) {
      if (GetExponent() == 1) {
	return 1; // depends linearly
      } else { 
	return 0; // depends nonlinearly
      }
    } else {
      return 2; // doesn't depend on vi at all
    }
  } else {
    int i;
    int d;
    bool dependsatall = false;
    // if node is linear:
    if (GetOpType() == SUM || GetOpType() == DIFFERENCE || 
	GetOpType() == PLUS || GetOpType() == MINUS) {
      for(i = 0; i < GetSize(); i++) {
	d = GetNode(i)->DependsLinearlyOnVariable(vi);
	if (d == 0) {
	  // depends nonlinearly, return 0
	  return 0;
	}
	if (d == 1) {
	  // depends linearly, record
	  dependsatall = true;
	}
      }
      if (dependsatall) {
	return 1;
      } else {
	return 2;
      }
    } else {
      if (DependsOnVariable(vi)) {
	return 0;  // depends nonlinearly
      } else {
	return 2;  // doesn't depend on vi at all
      }
    }
  }
}

void BasicExpression::ConsolidateProductCoeffs(void) {
  if (GetOpType() == PRODUCT) {
    Int i;
    double tc = GetCoeff();
    for (i = 0; i < GetSize(); i++) {
      tc *= GetNode(i)->GetCoeff();
      GetNode(i)->SetCoeff(1);
    }
    if (fabs(tc) < Ev3NearZero()) {
      Zero();
    } else {
      SetCoeff(tc);
    }
  }
}

void BasicExpression::DistributeCoeffOverSum(void) {
  if (GetOpType() == SUM) {
    double tc = GetCoeff();
    if (tc != 1) {
      SetCoeff(1);
      Int i;
      for(i = 0; i < GetSize(); i++) {
	GetNode(i)->SetCoeff(tc * GetNode(i)->GetCoeff());
	GetNode(i)->DistributeCoeffOverSum();
      }
    }
  }
}

void BasicExpression::DistributeCoeffOverProduct(void) {
  if (GetOpType() == PRODUCT) {
    double tc = GetCoeff();
    if (tc != 1) {
      SetCoeff(1);
      Int i;
      for(i = 0; i < GetSize(); i++) {
	GetNode(i)->SetCoeff(tc * GetNode(i)->GetCoeff());
	GetNode(i)->DistributeCoeffOverProduct();
      }
    }
  }
}

// enforce constant dependencies (added for MORON - see ../PROGNOTES)
// this only acts on the proper leaf nodes
void BasicExpression::EnforceDependency(void) {
  if (IsLeaf()) {
    // nonrecursive
    EnforceDependencyOnOperand();
  } else {
    // recursive
    int i;
    for (i = 0; i < GetSize(); i++) {
      GetNode(i)->EnforceDependency();
    }
  } 
}

// substitute a variable with a constant
void BasicExpression::VariableToConstant(int varindex, double c) {
  if (IsLeaf()) {
    // nonrecursive
    SubstituteVariableWithConstant(varindex, c);
  } else {
    // recursive
    int i;
    for (i = 0; i < GetSize(); i++) {
      GetNode(i)->VariableToConstant(varindex, c);
    }
  }
}

// replace variable indexed v1 with variable indexed v2
void BasicExpression::ReplaceVariable(int v1, int v2, string vn) {
  if (DependsOnVariable(v1)) {
    if (IsVariable() && GetVarIndex() == v1) {
      SetVarIndex(v2);
      SetVarName(vn);
    } else {
      int i;
      for(i = 0; i < GetSize(); i++) {
	GetNode(i)->ReplaceVariable(v1, v2, vn);
      }
    }
  }
}
void BasicExpression::ReplaceVariable(int v1, int v2, string vn,
				      double c2) {
  if (DependsOnVariable(v1)) {
    if (IsVariable() && GetVarIndex() == v1) {
      SetVarIndex(v2);
      SetVarName(vn);
      SetCoeff(GetCoeff() * c2);
    } else {
      int i;
      for(i = 0; i < GetSize(); i++) {
	GetNode(i)->ReplaceVariable(v1, v2, vn, c2);
      }
    }
  }
}

// replace with a variable the deepest node conforming to schema and
// return replaced term or zero expression if no replacement occurs
Expression BasicExpression::ReplaceBySchema(int vi, string vn, 
					    Expression schema) {
  Expression ret(0.0);
  ret = ReplaceBySchemaRecursive(vi, vn, schema);
  if (ret->IsZero()) {
    // no subnodes with schema found, wor on this one
    if (IsEqualBySchema(schema)) {
      // this node is according to schema
      // save (recursively) this into ret
      ret.SetToCopyOf(*this);
      // replace with w_vi
      // -- segvs (?)
      for(int i = 0; i < GetSize(); i++) {
	RecursiveDestroy(GetNodePtr(i));
      }
      DeleteAllNodes();
      SetOpType(VAR);
      SetVarIndex(vi);
      SetVarName(vn);
      SetCoeff(1.0);
      SetExponent(1.0);
    }
  } 
  return ret;
}

// recursive version - works on subnodes, not on current node
Expression BasicExpression::ReplaceBySchemaRecursive(int vi, string vn, 
						     Expression schema) {
  bool done = false;
  Expression ret(0.0);
  for(int i = 0; i < GetSize(); i++) {
    if (!GetNode(i)->IsLeaf()) {
      ret = GetNode(i)->ReplaceBySchemaRecursive(vi, vn, schema);
      if (!ret->IsZero()) {
	done = true;
	break;
      }
    }
    if (!done) {
      if (GetNode(i)->IsEqualBySchema(schema)) {
	ret = GetNode(i);
	Expression w(1, vi, vn);
	GetNodePtr(i)->SetTo(w);
	done = true;
	break;
      }
    }
  }
  return ret;
}

// replace with a variable the deepest node with given operator label
// return replaced term or zero expression if no replacement occurs
Expression BasicExpression::ReplaceByOperator(int vi, string vn, 
					      int theoplabel) {
  Expression ret(0.0);
  ret = ReplaceByOperatorRecursive(vi, vn, theoplabel);
  if (ret->IsZero()) {
    // no subnodes with schema found, wor on this one
    if (IsEqualByOperator(theoplabel)) {
      // this node is according to schema
      // save (recursively) this into ret
      ret.SetToCopyOf(*this);
      // replace with w_vi
      // -- segvs
      for(int i = 0; i < GetSize(); i++) {
	RecursiveDestroy(GetNodePtr(i));
      }
      DeleteAllNodes();
      SetOpType(VAR);
      SetVarIndex(vi);
      SetVarName(vn);
      SetCoeff(1.0);
      SetExponent(1.0);
    }
  } 
  return ret;
}

// recursive version - works on subnodes, not on current node
Expression BasicExpression::ReplaceByOperatorRecursive(int vi, string vn, 
						       int theoplabel) {
  bool done = false;
  Expression ret(0.0);
  for(int i = 0; i < GetSize(); i++) {
    if (!GetNode(i)->IsLeaf()) {
      ret = GetNode(i)->ReplaceByOperatorRecursive(vi, vn, theoplabel);
      if (!ret->IsZero()) {
	done = true;
	break;
      }
    }
    if (!done) {
      if (GetNode(i)->IsEqualByOperator(theoplabel)) {
	ret = GetNode(i);
	Expression w(1, vi, vn);
	GetNodePtr(i)->SetTo(w);
	done = true;
	break;
      }
    }
  }

  return ret;  
}

void BasicExpression::ReplaceWithExpression(Expression replace) {
  /* // -- segvs (?)
  for(int i = 0; i < GetSize(); i++) {
    RecursiveDestroy(GetNodePtr(i));
  }
  */
  DeleteAllNodes();
  if (replace->GetOpType() == VAR) {
    SetVarIndex(replace->GetVarIndex());
    SetVarName(replace->GetVarName());
    SetCoeff(replace->GetCoeff());
    SetExponent(replace->GetExponent());
  } else if (replace->GetOpType() == CONST) {
    SetValue(replace->GetValue());
  } else {
    SetCoeff(replace->GetCoeff());
    SetExponent(replace->GetExponent());
    SetOpType(replace->GetOpType());
  }
  for (int i = 0; i < replace->GetSize(); i++) {
    nodes.push_back(replace->GetNode(i));
  }    
}


int BasicExpression::ReplaceSubexpression(Expression needle, 
					  Expression replace) {
  int ret = 0;
  if (!IsLeaf()) {
    // recurse
    for(int i = 0; i < GetSize(); i++) {
      ret += GetNode(i)->ReplaceSubexpression(needle, replace);
    }
  }
  // act on this node
  if (IsEqualTo(needle)) {
    ret++;
    ReplaceWithExpression(replace);
  }
  return ret;
}

// smith's standard form
void BasicExpression::SmithStandardForm(int defvarindex,
					string defvarname,
					vector<int>& oplabels, 
					vector<Expression>& schemata,
					vector<Expression>& defcons) {
  defcons.erase(defcons.begin(), defcons.end());
  SmithStandardFormRecursive(defvarindex, defvarname, 
			     oplabels, schemata, defcons);
}

void BasicExpression::SmithStandardFormRecursive(int defvarindex,
						 string defvarname,
						 vector<int>& oplabels, 
						 vector<Expression>& schemata,
						 vector<Expression>& defcons) {
  // standardize depth-first
  int sz = GetSize();
  if (!IsLeaf()) {
    int defconsize;
    for(int i = 0; i < sz; i++) {
      defconsize = defcons.size();
      GetNode(i)->SmithStandardFormRecursive(defvarindex, defvarname, 
					     oplabels, schemata, defcons);
      defvarindex += defcons.size() - defconsize;
    }
  }
  // do this node
  Expression e(0.0);
  bool done = false;
  for(vector<Expression>::iterator schi = schemata.begin(); 
      schi != schemata.end() && !done; schi++) {
    if (IsEqualBySchema(*schi)) {
      done = true;
      // save this into defcons
      e.SetTo(*this);
      defcons.push_back(e);
      // replace this node with w_defvarindex
      // -- segvs (?)
      for(int i = 0; i < GetSize(); i++) {
	RecursiveDestroy(GetNodePtr(i));
      }
      DeleteAllNodes();
      SetOpType(VAR);
      SetVarIndex(defvarindex);
      SetVarName(defvarname);
      SetCoeff(1.0);
      SetExponent(1.0);
      break;
    }
  }
  if (!done) {
    for(vector<int>::iterator opi = oplabels.begin(); 
	opi != oplabels.end() && !done; opi++) {
      if (IsEqualByOperator(*opi)) {
	done = true;
	if (*opi == PRODUCT && sz > 2) {
	  // treat this case separately -- only case of n-ary product
	  // whose Smith reformulation is binary: w = x1 (x2 ... (x_{n-1}x_n)) 
	  e = GetNode(sz - 1) * GetNode(sz - 2);
	  defcons.push_back(e);
	  for(int i = sz-3; i >= 0; i--) {
	    Expression wi(1.0, defvarindex, defvarname);
	    Expression e2 = GetNode(i) * wi;
	    defcons.push_back(e2);
	    defvarindex++;
	  }
	} else {
	  // save this into defcons
	  e.SetTo(*this);
	  defcons.push_back(e);
	}
	// replace this node with w_defvarindex
	/* // -- segvs (why does this and not the other similar ones?)
	for(int i = 0; i < GetSize(); i++) {
	  RecursiveDestroy(GetNodePtr(i));
	}
	*/
	DeleteAllNodes();
	SetOpType(VAR);
	SetVarIndex(defvarindex);
	SetVarName(defvarname);
	SetCoeff(1.0);
	SetExponent(1.0);
	break;
      }
    }
  }
}

int BasicExpression::ProdBinCont(int defvarindex, string defvarname, 
				 map<int,bool>& integrality,
				 map<int,double>& vlb, 
				 map<int,double>& vub,
				 map<int,pair<double,double> >& addvarbounds,
				 vector<Expression>& defcons) {
  int ret = 0;
  defcons.erase(defcons.begin(), defcons.end());
  addvarbounds.erase(addvarbounds.begin(), addvarbounds.end());
  ret = ProdBinContRecursive(defvarindex, defvarname, integrality, 
			     vlb, vub, addvarbounds, defcons);
  return ret;
}

int BasicExpression::ProdBinContRecursive(int defvarindex, string defvarname, 
					  map<int,bool>& integrality,
					  map<int,double>& vlb, 
					  map<int,double>& vub,
					  map<int,pair<double,double> >& 
					  addvarbounds,
					  vector<Expression>& defcons){
  int ret = 0;
  // apply depth-first
  int sz = GetSize();
  if (!IsLeaf()) {
    int defconsize;
    for(int i = 0; i < sz; i++) {
      defconsize = defcons.size();
      ret += GetNode(i)->ProdBinContRecursive(defvarindex, defvarname, 
					      integrality, vlb, vub, 
					      addvarbounds, defcons);
    }
    defvarindex += ret;
  }
  // do this node
  if (GetOpType() == PRODUCT && GetSize() == 2 && 
      GetNode(0)->GetOpType() == VAR && GetNode(1)->GetOpType() == VAR) {
    // x*y, check binary
    int v1 = GetNode(0)->GetVarIndex();
    int v2 = GetNode(1)->GetVarIndex();
    if ((integrality[v1] && vlb[v1] == 0 && vub[v1] == 1) || 
	(integrality[v2] && vlb[v2] == 0 && vub[v2] == 1)) {
      // one of the vars is binary, call it x and call the other y
      int xi = (integrality[v1] && vlb[v1] == 0 && vub[v1] == 1) ? v1 : v2;
      int yi = (xi == v1) ? v2 : v1;
      // compute bounds for added variable
      pair<double,double> bnd(vlb[yi], vub[yi]);
      bool isfound = false;
      for(map<int,pair<double,double> >::iterator mi = addvarbounds.begin();
	  mi != addvarbounds.end(); mi++) {
	if (mi->first == defvarindex) {
	  isfound = true;
	  break;
	}
      }
      if (isfound) {
	pair<double,double> oldbnd = addvarbounds[defvarindex];
	if (oldbnd.first > bnd.first) {
	  bnd.first = oldbnd.first;
	}
	if (oldbnd.second < bnd.second) {
	  bnd.second = oldbnd.second;
	}
      }
      addvarbounds[defvarindex] = bnd;
      // compute linearization expressions
      Expression w(1.0, defvarindex, defvarname);
      Expression xl(vlb[xi], xi, "x");
      Expression xu(vub[xi], xi, "x");
      Expression xL(vlb[xi]);
      Expression xU(vub[xi]);
      Expression y(1.0, yi, "y");
      Expression c1 = w - xu;
      Expression c2 = xl - w;
      Expression c3 = -xL + w + xl - y;
      Expression c4 = -xU + xu - w + y;
      Simplify(&c1);
      Simplify(&c2);
      Simplify(&c3);
      Simplify(&c4);
      defcons.push_back(c1);
      defcons.push_back(c2);
      defcons.push_back(c3);
      defcons.push_back(c4);
      // now replace this product with w
      // -- segvs (?)
      for(int i = 0; i < GetSize(); i++) {
	RecursiveDestroy(GetNodePtr(i));
      }
      DeleteAllNodes();
      SetOpType(VAR);
      SetVarIndex(defvarindex);
      SetVarName(defvarname);
      SetCoeff(1.0);
      SetExponent(1.0);      
    }
  }
  return ret;
}

void BasicExpression::ResetVarNames(string vn, int lid, int uid) {
  // set all variable names in the expression to vn
  if (!IsLeaf()) {
    for(int i = 0; i < GetSize(); i++) {
      GetNode(i)->ResetVarNames(vn, lid, uid);
    }
  } else {
    if (GetOpType() == VAR) {
      int vi = GetVarIndex();
      if (vi >= lid && vi <= uid) {
	SetVarName(vn);
      }
    }
  }
}


bool BasicExpression::DistributeProductsOverSums(void) {
  // recursive part
  bool ret = false;
  if (!IsLeaf()) {
    for(int i = 0; i < GetSize(); i++) {
      bool haschanged = GetNode(i)->DistributeProductsOverSums();
      if (haschanged) {
	ret = true;
      }
    }
  }
  // deal with this node
  Expression e(0.0);
  if (GetOpType() == PRODUCT) {
    for(int i = 0; i < GetSize(); i++) {
      if (GetNode(i)->GetOpType() == SUM) {
	// found occurrence of *(+), distribute
	ret = true;
	Expression f = (*this) / GetNode(i);
	Simplify(&f);
	for(int j = 0; j < GetNode(i)->GetSize(); j++) {
	  e = e + f * GetNode(i)->GetNode(j);
	}
	// now replace this with e
	ReplaceWithExpression(e);
      }
    }
  }
  return ret;
}

void BasicExpression::GetVarIndices(vector<int>& vidx) {
  if (!IsLeaf()) {
    for(int i = 0; i < GetSize(); i++) {
      GetNode(i)->GetVarIndices(vidx);
    }
  } else if (IsVariable()) {
    int vi = GetVarIndex();
    if (find(vidx.begin(), vidx.end(), vi) == vidx.end()) {
      vidx.push_back(vi);
    }
  }
}

void BasicExpression::GetVarIndicesInSchema(vector<int>& vidx, 
					    Expression schema) {
  // recurse
  if (!IsLeaf()) {
    for(int i = 0; i < GetSize(); i++) {
      GetNode(i)->GetVarIndicesInSchema(vidx, schema);
    }
  }
  // deal with this node
  if (IsEqualBySchema(schema)) {
    GetVarIndices(vidx);
  }
}

// find the variable name corresponding to variable index vi
string BasicExpression::FindVariableName(int vi) {
  string vn;
  if (IsVariable()) {
    if (GetVarIndex() == vi) {
      return GetVarName();
    } else {
      return "";
    }
  } else {
    int i;
    for(i = 0; i < GetSize(); i++) {
      vn = GetNode(i)->FindVariableName(vi);
      if (vn.length() > 0) {
	return vn;
      }
    }
  }
  return "";
}

// is this expression linear?
bool BasicExpression::IsLinear(void) const {
  if (IsVariable()) {
    if (GetExponent() != 0 && GetExponent() != 1) {
      return false;
    } else {
      return true;
    }
  }
  if (IsConstant()) {
    return true;
  }
  if(GetOpType() == SUM || GetOpType() == DIFFERENCE) {
    int i;
    for(i = 0; i < GetSize(); i++) {
      if (!GetNode(i)->IsLinear()) {
	return false;
      }
    }
    return true;
  } else {
    return false;
  }
}

// is this expression a quadratic product?
bool BasicExpression::IsQuadratic(int& prodtype) const {
  bool ret = false;
  if ((GetOpType() == PRODUCT && 
       GetNode(0)->GetOpType() == VAR && GetNode(1)->GetOpType() == VAR) || 
      (GetOpType() == POWER && 
       GetNode(0)->GetOpType() == VAR && GetNode(1)->GetValue() == 2) ||
      (GetOpType() == VAR && GetExponent() == 2)) {
    prodtype = GetOpType();
    ret = true;
  }
  return ret;
}
bool BasicExpression::IsQuadratic(void) const {
  int ret = 0;
  return IsQuadratic(ret);
}

// return info about the linear part (assumes Simplify() has already been
// called on this) - return false if expression has no linear part
// by "linear part" we mean lin(x) in expr(x,y) = lin(x) + nonlin(y)
bool BasicExpression::GetLinearInfo(vector<double>& lincoeff, 
				    vector<int>& linvi,
				    vector<string>& linvn,
				    double& c) {  
  c = 0;
  bool ret = false;
  Expression nl(GetPureNonlinearPart());
  if (IsLinear()) {
    nl->Zero();
  }
  if (lincoeff.size() > 0) {
    lincoeff.erase(lincoeff.begin(), lincoeff.end());
    linvi.erase(linvi.begin(), linvi.end());
    linvn.erase(linvn.begin(), linvn.end());
  }
  if (IsLeaf()) {
    if (IsConstant()) {
      // just a constant
      c = GetValue();
      ret = true;
    } else if (IsVariable() && GetExponent() == 1) {
      // just a variable
      linvi.push_back(GetVarIndex());
      lincoeff.push_back(GetCoeff());
      linvn.push_back(GetVarName());
      ret = true;
    }
  } else {
    if (GetOpType() == SUM) {
      int i, vi;
      c = 0;
      for(i = 0; i < GetSize(); i++) {
	if (GetNode(i)->IsConstant()) {
	  if (i > 0) {
	    cerr << "BasicExpression::GetLinearInfo: WARNING: "
		 << "run Simplify() first\n";
	  }
	  c += GetNode(i)->GetValue();
	} else if (GetNode(i)->IsVariable() && 
		   GetNode(i)->GetExponent() == 1) {
	  vi = GetNode(i)->GetVarIndex();
	  if (!nl->DependsOnVariable(vi)) {
	    // vi depends only linearly on expression
	    linvi.push_back(vi);
	    lincoeff.push_back(GetNode(i)->GetCoeff());
	    linvn.push_back(GetNode(i)->GetVarName());
	    ret = true;	    
	  }
	}
      }
    }
  }
  return ret;
}

// return info about the purely linear part 
// (assumes Simplify() has already been
// called on this) - return false if expression has no linear part
// by "pure linear part" we mean e.g. x+y in x+y+y^2
bool BasicExpression::GetPureLinearInfo(vector<double>& lincoeff, 
					vector<int>& linvi,
					vector<string>& linvn,
					double& c) {  
  c = 0;
  bool ret = false;
  int i;
  Expression nl(GetPureNonlinearPart());
  if (IsLinear()) {
    nl->Zero();
  }
  if (lincoeff.size() > 0) {
    lincoeff.erase(lincoeff.begin(), lincoeff.end());
    linvi.erase(linvi.begin(), linvi.end());
    linvn.erase(linvn.begin(), linvn.end());
  }
  if (IsLeaf()) {
    if (IsConstant()) {
      // just a constant
      c = GetValue();
      ret = true;
    } else if (IsVariable() && GetExponent() == 1) {
      // just a variable
      linvi.push_back(GetVarIndex());
      lincoeff.push_back(GetCoeff());
      linvn.push_back(GetVarName());
      ret = true;
    }
  } else {
    if (GetOpType() == SUM) {
      int vi;
      c = 0;
      for(i = 0; i < GetSize(); i++) {
	if (GetNode(i)->IsConstant()) {
	  if (i > 0) {
	    cerr << "BasicExpression::GetLinearInfo: WARNING: "
		 << "run Simplify() first\n";
	  }
	  c += GetNode(i)->GetValue();
	} else if (GetNode(i)->IsVariable() && 
		   GetNode(i)->GetExponent() == 1) {
	  vi = GetNode(i)->GetVarIndex();
	  linvi.push_back(vi);
	  lincoeff.push_back(GetNode(i)->GetCoeff());
	  linvn.push_back(GetNode(i)->GetVarName());
	  ret = true;	    
	}
      }
    }
  }
  return ret;
}

// get the linear part - x in x+y+y^2
Expression BasicExpression::GetLinearPart(void) {
  vector<double> lincoeff;
  vector<int> linvi;
  vector<string> linvn;
  double c;
  GetLinearInfo(lincoeff, linvi, linvn, c);
  int i;
  Expression ret;
  if (lincoeff.size() > 0) {
    ret->SetOpType(VAR);
    ret->SetVarIndex(linvi[0]);
    ret->SetCoeff(lincoeff[0]);
    ret->SetVarName(linvn[0]);
    ret->SetExponent(1);
    if (lincoeff.size() > 1) {
      Expression addend(1.0, -1, NOTVARNAME);
      for(i = 1; i < (int) lincoeff.size(); i++) {
	addend->SetVarIndex(linvi[i]);
	addend->SetCoeff(lincoeff[i]);
	addend->SetVarName(linvn[i]);
	ret = ret + addend;
      }
    }
  }
  return ret;
}

// get the pure linar part - x+y in x+y+y^2
Expression BasicExpression::GetPureLinearPart(void) {
  vector<double> lincoeff;
  vector<int> linvi;
  vector<string> linvn;
  double c;
  GetPureLinearInfo(lincoeff, linvi, linvn, c);
  int i;
  Expression ret(0.0);
  if (lincoeff.size() > 0) {
    ret->SetOpType(VAR);
    ret->SetVarIndex(linvi[0]);
    ret->SetCoeff(lincoeff[0]);
    ret->SetVarName(linvn[0]);
    ret->SetExponent(1);
    if (lincoeff.size() > 1) {
      for(i = 1; i < (int) lincoeff.size(); i++) {
	Expression addend(lincoeff[i], linvi[i], linvn[i]);
	ret = SumLink(ret, addend);
      }
    }
  }
  return ret;
}

// get the nonlinear part - nonlin(y) in expr(x,y) = lin(x) + nonlin(y)
Expression BasicExpression::GetNonlinearPart(void) {
  Expression ret(GetPureNonlinearPart());
  vector<double> linval;
  vector<int> linidx;
  vector<string> linvn;
  double c = 0;
  GetPureLinearInfo(linval, linidx, linvn, c);
  int i;
  Expression addend(1.0, -1, NOTVARNAME);
  // we cycle backwards to keep the order of addends in ret
  for(i = linidx.size() - 1; i >= 0 ; i--) {
    if (ret->DependsOnVariable(linidx[i])) {
      // pure nonlinear part depends on varindex i, add it to ret
      addend->SetCoeff(linval[i]);
      addend->SetVarIndex(linidx[i]);
      addend->SetVarName(linvn[i]);
      ret = addend + ret;
    }
  }
  return ret;
}


// get the purely nonlinear part - e.g. only y^2 in x+y+y^2
Expression BasicExpression::GetPureNonlinearPart(void) {
  Expression ret(0.0);
  if (!IsLeaf()) {
    if (GetOpType() == SUM) {
      int i;
      for(i = 0; i < GetSize(); i++) {
	if (!GetNode(i)->IsLinear()) {
	  ret = SumLink(ret, GetNode(i));
	}
      }
    } else if (GetOpType() == DIFFERENCE) {
      int i;
      for(i = 0; i < GetSize(); i++) {
	if (!GetNode(i)->IsLinear()) {
	  ret = ret - GetNode(i);
	}
      }
    } else if (GetOpType() == PLUS) {
      ret = GetNode(0);
    } else if (GetOpType() == MINUS) {
      ret = GetNode(0);
      ret->SetCoeff(- ret->GetCoeff());
    } else {
      ret = *this;
    }
  } else {
    // leaf but can be a power
    if (GetExponent() != 0 && GetExponent() != 1) {
      ret = *this;
    }
  }
  return ret;
}

// get value of additive constant
double BasicExpression::GetConstantPart(void) {
  double ret = 0;
  if (IsConstant()) {
    ret = GetValue();
  } else if (!IsLeaf()) {
    int op = GetOpType();
    if (op == SUM || op == DIFFERENCE) {
      int i = 0;
      int sz = GetSize();
      while(i < sz) {
	if (GetNode(i)->IsConstant()) {
	  if (op == SUM || (op == DIFFERENCE && i == 0)) {
	    ret += GetNode(i)->GetValue();
	  } else {
	    ret -= GetNode(i)->GetValue();
	  }
	}
	i++;
      }
    }
  }
  return ret;
}

// doesn't deal with additive constants in PLUS/MINUS operands
double BasicExpression::RemoveAdditiveConstant(void) {
  double ret = 0;
  if (IsConstant()) {
    ret = GetValue();
    SetValue(0);
  } else if (!IsLeaf()) {
    int op = GetOpType();
    if (op == SUM || op == DIFFERENCE) {
      int i = 0;
      int sz = GetSize();
      while(i < sz) {
	if (GetNode(i)->IsConstant()) {
	  if (op == SUM || (op == DIFFERENCE && i == 0)) {
	    ret += GetNode(i)->GetValue();
	  } else {
	    ret -= GetNode(i)->GetValue();
	  }
	  DeleteNode(i);
	  sz--;
	} else {
	  i++;
	}
      }
    }
  }
  return ret;
}

// perform interval arithmetics on the expression;
// Vlb[i] is the lower bound of varindex i, Vub is similar,
// returned elb and eub hold values to expression interval
void BasicExpression::Interval(map<int,double>& Vlb, map<int,double>& Vub, 
			       double& elb, double& eub) {
  int i;
  if (IsLeaf()) {
    // leaf
    if (IsConstant()) {
      elb = GetValue();
      eub = elb;
    } else if (IsVariable()) {
      double expon = GetExponent();
      double coeff = GetCoeff();
      int vi = GetVarIndex();
      if (expon == 1) {
	// exponent is 1, simple variable
	elb = coeff * Vlb[vi];
	eub = coeff * Vub[vi];
      } else {
	// var ^ constant
	constpowermkrange(Vlb[vi], Vub[vi], expon, &elb, &eub);
	elb = coeff * elb;
	eub = coeff * eub;
      }
    }
  } else {
    // operator recurse
    vector<double> tmplb, tmpub;
    double tlb, tub;
    for(i = 0; i < GetSize(); i++) {
      tlb = -Ev3Infinity();
      tub = Ev3Infinity();
      GetNode(i)->Interval(Vlb, Vub, tlb, tub);
      tmplb.push_back(tlb);
      tmpub.push_back(tub);
    }
    // do the gig
    int op = GetOpType();
    int sz = GetSize();
    double t1 = 0;
    double t2 = 0;
    double t3 = 0;
    double t4 = 0;
    double t5 = 0;
    switch(op) {
    case SUM:
      for(i = 0; i < sz; i++) {
	t1 += tmplb[i];
	t2 += tmpub[i];
      }
      break;
    case DIFFERENCE:
      for(i = 1; i < sz; i++) {
	t1 += tmplb[i];
	t2 += tmpub[i];
      }
      t1 = tmplb[0] - t2;
      t2 = tmpub[0] - t1;
      break;
    case PRODUCT:
      assert(sz > 1);
      bilinearprodmkrange(tmplb[0], tmpub[0], tmplb[1], tmpub[1], &t1, &t2);
      for(i = 2; i < sz; i++) {
	t3 = t1;
	t4 = t2;
	bilinearprodmkrange(t3, t4, tmplb[1], tmpub[1], &t1, &t2);
      }      
      break;
    case FRACTION:
      fractionmkrange(tmplb[0], tmpub[0], tmplb[1], tmpub[1], &t1, &t2);
      break;      
    case POWER:
      powermkrange(tmplb[0], tmpub[0], tmplb[1], tmpub[1], &t1, &t2);
      break;
    case MINUS:
      t1 = -tmpub[0];
      t2 = -tmplb[0];
      break;
    case LOG:
      if (tmplb[0] <= 0 && tmpub[0] <= 0) {
	t1 = 0;
	t2 = 0;
      } else if (tmplb[0] <= 0) {
	t1 = -Ev3Infinity();
	t2 = std::log(tmpub[0]);
      } else {
	t1 = std::log(tmplb[0]);
	t2 = std::log(tmpub[0]);
      } 
      break;
    case EXP:
      t1 = std::exp(tmplb[0]);
      t2 = std::exp(tmpub[0]);
      break;
    case SIN:
      t3 = tmpub[0] - tmplb[0];
      t4 = tmplb[0] / PEV3PI + 0.5;
      t5 = tmpub[0] / PEV3PI + 0.5;
      if (t3 < PEV3PI && std::floor(t4) == std::floor(t5)) {
	t1 = std::sin(tmplb[0]);
	t2 = std::sin(tmpub[0]);
	if (t1 > t2) {
	  double ttmp = t1;
	  t1 = t2;
	  t2 = ttmp;	
	}
      } else {
	// should provide an "else if" for case where we have 
	// -1 < v < UB or LB < v < 1 but I can't be bothered
	t1 = -1;
	t2 = 1;
      }
      break;
    case COS:
      t3 = tmpub[0] - tmplb[0];
      t4 = tmplb[0] / PEV3PI + 0.5;
      t5 = tmpub[0] / PEV3PI + 0.5;
      if (t3 < PEV3PI && std::floor(t4) == std::floor(t5)) {
	t1 = std::cos(tmplb[0]);
	t2 = std::cos(tmpub[0]);
	if (t1 > t2) {
	  double ttmp = t1;
	  t1 = t2;
	  t2 = ttmp;	
	}
      } else {
	// should provide an "else if" for case where we have 
	// -1 < v < UB or LB < v < 1 but I can't be bothered
	t1 = -1;
	t2 = 1;
      }
      break;
    case TAN:
      t3 = tmpub[0] - tmplb[0];
      t4 = tmplb[0] / PEV3PI + 0.5;
      t5 = tmpub[0] / PEV3PI + 0.5;
      if (t3 < PEV3PI && std::floor(t4) == std::floor(t5)) {
	// lb and ub within the same tangent branch
	t1 = std::tan(tmplb[0]);
	t2 = std::tan(tmpub[0]);
	if (t1 > t2) {
	  double ttmp = t1;
	  t1 = t2;
	  t2 = ttmp;	
	}
      } else {
	// should provide an "else if" for case where we have 
	// -1 < v < UB or LB < v < 1 but I can't be bothered
	t1 = -Ev3Infinity();
	t2 = Ev3Infinity();
      }
      break;
    case COT:
      cerr << "expression.cxx::Interval(): cot() not implemented\n";
      break;
    case SINH:
      cerr << "expression.cxx::Interval(): sinh() not implemented\n";
      break;
    case COSH:
      cerr << "expression.cxx::Interval(): cosh() not implemented\n";
      break;
    case TANH:
      cerr << "expression.cxx::Interval(): tanh() not implemented\n";
      break;
    case COTH:
      cerr << "expression.cxx::Interval(): coth() not implemented\n";
      break;
    case SQRT:
      t1 = std::sqrt(tmplb[0]);
      t2 = std::sqrt(tmpub[0]);
      break;
    default:
      break;
    }
    elb = t1;
    eub = t2;
  }
  if (elb > eub) {
    // switch if needed (it might happen in certain cases that we end
    // up with an inverted range)
    double tmp = elb;
    elb = eub;
    eub = tmp;
  }
  // store the new bounds in the node's bounds (for FBBT)
  SetLB(elb);
  SetUB(eub);
}


void BasicExpression::FBBTUpDown(map<int,double>& Vlb, map<int,double>& Vub, 
				 double elb, double eub) {
  double ielb = -LARGE;
  double ieub = LARGE;
  // UP
  Interval(Vlb, Vub, ielb, ieub);
  if (ielb <= elb && ieub >= eub) {
    // FBBT has no effecton this expression
    return;
  }
  // [elb,eub] = UP \cap [elb,eub]
  if (ielb > elb) {
    elb = ielb;
  }
  if (ieub < eub) {
    eub = ieub;
  }
  // DOWN

}

void BasicExpression::FBBTDown(double elb, double eub) {
  if (!IsLeaf()) {
    // can only do DOWN on non-leaf nodes
    int k = GetSize();
    double l0, u0, l1, u1, l2, u2, l3, u3;
    // case DIFFERENCE 
    if (GetOpType() == DIFFERENCE && k == 2) {
      // inverse operator is SUM (of two operands)
      l0 = GetNode(0)->GetLB();
      u0 = GetNode(0)->GetUB();
      l1 = GetNode(1)->GetLB();
      u1 = GetNode(1)->GetUB();
      // set bounds for left node 
      GetNode(0)->SetLB(max(l0, elb + l1));
      GetNode(0)->SetUB(min(u0, eub + u1));
      // set bounds for right node
      GetNode(1)->SetLB(max(l1, elb + l0));
      GetNode(1)->SetUB(min(u1, eub + u0));
    } else if (GetOpType() == SUM && k >= 2) {
      // inverse operator is DIFFERENCE (top - sum(subnodes but one))
      // compute sum of all intervals
      l0 = 0;
      u0 = 0;
      for(int i = 0; i < k; i++) {
	l0 += GetNode(i)->GetLB();
	u0 += GetNode(i)->GetUB();
      }
      // assign new intervals to each node
      for(int i = 0; i < k; i++) {
	l1 = GetNode(i)->GetLB();
	u1 = GetNode(i)->GetUB();
	GetNode(i)->SetLB(max(l1, elb - (u0 - u1)));
	GetNode(i)->SetUB(min(u1, eub - (l0 - l1)));
      }
    }
  }
}

bool BasicExpression::IsSmithStandard(void) {
  return IsSmithStandard(0, 0);
}

bool BasicExpression::IsSmithStandard(double l, double u) {
  bool ret = false;
  if (!IsLeaf()) {
    int op = GetOpType();
    if (GetSize() == 2 && (op == SUM || op == DIFFERENCE)) {
      if (!GetNode(0)->IsConstant() && !GetNode(1)->IsConstant()) {
	int v1 = -1;
	int v2 = -1;
	if (GetNode(0)->IsVariable()) {
	  v1 = GetNode(0)->GetVarIndex();
	} 
	if (GetNode(1)->IsVariable()) {
	  v2 = GetNode(1)->GetVarIndex();
	}
	if (!(v1 > -1 && v2 > -1)) {
	  int posv = -1;
	  int posop = -1;
	  if (v1 > -1) {
	    posv = 0;
	    posop = 1;
	  } else if (v2 > -1) {
	    posv = 1;
	    posop = 0;
	  }
	  if (posv > -1) {
	    if (GetNode(posv)->GetExponent() == 1) {
	      // we have finally determined that expression is of the form
	      // c*w_i +/- something or something +/- c*w_i, that
	      // something is an expression which is not a leaf, and that
	      // it is in GetNode(pos)
	      Expression e = GetNode(posop);
#ifdef NOSMITH1VARTRIPLES
	      int vi = GetNode(posv)->GetVarIndex();
	      if (e->DependsOnVariable(vi)) {
		// the nonlinear part depends on the "added variable",
		// this is not a good "triple", must reformulate
		ret = false;
	      } else 
#endif
		{
		if (!(e->IsLinear())) {
		  int sz = e->GetSize();
		  if (sz == 1 && e->GetNode(0)->IsLeaf()) {
		    // unary function with leaf argument: OK
		    ret = true;
		  } else if (sz == 2 && e->GetNode(0)->IsLeaf() && 
			     e->GetNode(1)->IsLeaf()){
		    // binary operation with two leaves arguments
		    if (e->GetOpType() == PRODUCT) {
		      // binary product with two leaves arguments
		      // and whatever constraint bounds: OK
		      ret = true;
		    } else if (e->GetOpType() == FRACTION && 
			       l == u && u == 0) {
		      // binary fraction with two leaves arguments
		      // and constraint bounds l == u == 0: OK
		      ret = true;
		    } else if (e->GetOpType() == POWER && e->GetSize() == 2 &&
			       e->GetNode(1)->IsConstant()) {
		      if (e->GetNode(1)->GetValue() >= 0) {
			// binary power with constant positive exponent and
			// any constraint bounds: OK
			ret = true;
		      } else {
			if (l == u) {
			  // constant negative exponent and
			  // constraint is equation: OK
			  ret = true;
			} else {
			  // constant negative exponent and 
			  // inequality constraint: not ok
			  ret = false;
			}
		      }
		    } else if (e->GetOpType() == LOG ||
			       e->GetOpType() == EXP ||
			       e->GetOpType() == SQRT) {
		      // univariate convex/concave with any 
		      // constraint bounds: OK
		      // (can do some more complex testing here to
		      // include univariate convex/concave parts of
		      // trigonometric/hyperbolic functions)
		      ret = true;
		    } else {
		      // anything else is false, needs standardization proper
		      ret = false;
		    }
		  }
		}
	      }
	    }
	  }
	} else {
	  // both addends are variables, check that one has exponent 1
	  // and the other != 1 and != 0
	  double expon1 = GetNode(0)->GetExponent();
	  double expon2 = GetNode(1)->GetExponent();
	  if (expon1 != 0 && expon2 != 0) {
	    if (((expon1 == 1 && expon2 != 1) ||
		 (expon1 != 1 && expon2 == 1)) 
#ifdef NOSMITH1VARTRIPLES
		&& (v1 != v2)
#endif
		) {
	      ret = true;
	    }
	  }
	}
      }
    }
  }
  return ret;
}

bool BasicExpression::IsOptStandard(void) {
  bool ret = false;
  // not implemented yet
  return ret;
}

bool BasicExpression::IsEvidentlyConvex(void) {
  if (IsLinear()) {
    return true;
  }
  bool ret = true;
  int sz = GetSize();
  int op = GetOpType();
  int i;
  double c = 0;
  double expon = 0;
  switch(op) {
  case SUM:
    for(i = 0; i < sz; i++) {
      if (!GetNode(i)->IsLinear()) {
	if (!GetNode(i)->IsEvidentlyConvex()) {
	  ret = false;
	  break;
	}
      }
    }
    // invert if this' coefficient is negative
    c = GetCoeff();
    if (ret) {
      if (c < 0) {
	ret = false;
      }
    }
    break;
  case DIFFERENCE:
    if (!GetNode(0)->IsLinear()) {
      if (!GetNode(0)->IsEvidentlyConcave()) {
	ret = false;
	break;
      }
    } 
    if (ret) {
      for(i = 1; i < sz; i++) {
	if (!GetNode(i)->IsLinear()) {
	  if (!GetNode(i)->IsEvidentlyConcave()) {
	    ret = false;
	    break;
	  }
	}
      }
    }
    c = GetCoeff();
    if (ret) {
      if (c < 0) {
	ret = false;
      }
    }
    break;
  case PRODUCT: case FRACTION: case SIN: case COS: case TAN: case COT:
  case SINH: case COSH: case TANH: case COTH:
    // in general, neither convex nor concave
    ret = false;
    break;
  case LOG: case SQRT:
    // depending on coefficient, Concave (c>0) or convex
    c = GetCoeff();
    if (c < 0) {
      ret = true;
    } else {
      ret = false;
    }
    break;
  case EXP:
    // depending on coefficient, Convex (c>0) or concave
    c = GetCoeff();
    if (c < 0) {
      ret = true;
    } else {
      ret = false;
    }
    break;
  case POWER:
    if (GetSize() == 2 && GetNode(1)->IsConstant()) {
      c = GetCoeff();
      expon = GetNode(1)->IsConstant();
      if (!is_odd(expon) && expon > 1) {
	// positive non-odd rational exponent > 1
	if (c > 0) {
	  ret = true;
	} else {
	  ret = false;
	}
      } else if (expon > 0) {
	// positive 0 < exponent < 1
	if (c < 0) {
	  ret = true;
	} else {
	  ret = false;
	}
      } else {
	// all other cases are in general neither concave nor convex
	// (we don't know the range of the variable here)
	ret = false;
      }
    } else {
      ret = false;
    }
    break;
  case VAR:
    if (GetExponent() != 1) {
      // this is like POWER above
      c = GetCoeff();
      expon = GetExponent();
      if (!is_odd(expon) && expon > 1) {
	// positive non-odd rational exponent > 1
	if (c > 0) {
	  ret = true;
	} else {
	  ret = false;
	}
      } else if (expon > 0) {
	// positive 0 < exponent < 1
	if (c < 0) {
	  ret = true;
	} else {
	  ret = false;
	}
      } else {
	// all other cases are in general neither concave nor convex
	// (we don't know the range of the variable here)
	ret = false;
      }
    } else {
      // this should have been caught by the IsLinear() at the beginning
      // anyway
      ret = true;
    }
  }
  return ret;
}

bool BasicExpression::IsEvidentlyConcave(void) {
  if (IsLinear()) {
    return true;
  }
  bool ret = false;
  double c = GetCoeff();
  SetCoeff(-c);
  ret = IsEvidentlyConvex();
  SetCoeff(c);
  return ret;
}

bool BasicExpression::IsEvidentlyConvex(double l, double u) {
  bool ret = true;
  if (IsLinear()) {
    return true;
  }
  if (l > -Ev3Infinity() && u < Ev3Infinity()) {
    // a nonlinear constraint with both bounds finite always
    // defines a nonconvex set
    ret = false;
  } else if (l <= -Ev3Infinity() && u >= Ev3Infinity()) {
    // this is an inactive constraint: always convex
    ret = true;
  } else if (l <= -Ev3Infinity()) {
    // situation is -inf < expr < u. If expr is concave, then
    // we have a convex constraint
    if (IsEvidentlyConcave()) {
      ret = true;
    } else {
      ret = false;
    }
  } else if (u >= Ev3Infinity()) {
    // situation is l < expr < inf. If expr is convex, then
    // we have a convex constraint
    if (IsEvidentlyConvex()) {
      ret = true;
    } else {
      ret = false;
    }
  }
  return ret;
}


/************** expression creation (no change to args) ***************/

// BIG FAT WARNING: when you change these operators, also please
// change their "-Link" counterparts!

// sums:
Expression operator + (Expression a, Expression b) {
  Expression ret;
  // make a preliminary check
  if (a->GetCoeff() == 0 || a->HasValue(0)) {
    ret.SetToCopyOf(b);
    return ret;
  }
  if (b->GetCoeff() == 0 || b->HasValue(0)) {
    ret.SetToCopyOf(a);
    return ret;  
  }
  if (!(a->IsConstant() && b->IsConstant()) && a->IsEqualToNoCoeff(b)) {
    a->SetCoeff(a->GetCoeff() + b->GetCoeff());
    if (fabs(a->GetCoeff()) < Ev3NearZero()) {
      // simplify to zero - for differences
      Expression zero(0.0);
      return zero;
    } else {
      ret.SetToCopyOf(a);
      return ret;
    }
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST &&
      b->IsLeaf() && b->GetOpType() == CONST) {
    // a, b are numbers - add them
    ret.SetToCopyOf(a);
    ret->SetValue(a->GetValue() + b->GetValue());
    ret->SetCoeff(1);
    ret->SetExponent(1);
    return ret;
  } else if (a->IsLeaf() && a->GetOpType() == VAR &&
	     b->IsLeaf() && b->GetOpType() == VAR &&
	     a->GetVarIndex() == b->GetVarIndex() &&
	     a->GetExponent() == b->GetExponent()) {
    // a, b are the same variable - add coefficients
    ret.SetToCopyOf(a);    
    ret->SetCoeff(a->GetCoeff() + b->GetCoeff());
    return ret;
  } else if (a->GetOpType() == SUM && b->GetOpType() != SUM) {    
    // a is a sum and b isn't - just add the term b
    ret.SetToCopyOf(a);    
    ret->DistributeCoeffOverSum();
    Int i = 0;
    bool couldsimplify = false;
    Expression tmp;
    // if t is a leaf and there is a like leaf in this,
    // just add it to the value/coefficient
    if (b->IsLeaf() && b->GetOpType() == CONST) {
      // b is a constant
      for (i = 0; i < ret->GetSize(); i++) {
	tmp = ret->GetNode(i);
	if (tmp->IsLeaf() && tmp->GetOpType() == CONST) {
	  tmp->SetValue(tmp->GetValue() + b->GetValue() / ret->GetCoeff());
	  tmp->SetCoeff(1);
	  tmp->SetExponent(1); // NB: changing tmp should also change a
	  couldsimplify = true;
	  break;
	}
      }
    } else if (b->IsLeaf() && b->GetOpType() == VAR) {
      // b is a variable
      for (i = 0; i < ret->GetSize(); i++) {
	if (ret->GetNode(i)->IsLeaf() && ret->GetNode(i)->GetOpType() == VAR &&
	    b->GetVarIndex() == ret->GetNode(i)->GetVarIndex() &&
	    b->GetExponent() == ret->GetNode(i)->GetExponent()) {
	  double tc = ret->GetNode(i)->GetCoeff() + 
	    b->GetCoeff() / ret->GetCoeff();
	  // warning: tc could be zero, but it would be cumbersome
	  // to simplify it here - do it in SimplifyConstant
	  ret->GetNode(i)->SetCoeff(tc);
	  couldsimplify = true;
	  break;
	}
      }
    } else if (!b->IsLeaf()) {
      // a is a sum, b is a nonleaf, look for a subnode of a similar to b
      for (i = 0; i < ret->GetSize(); i++) {
	if (ret->GetNode(i)->IsEqualTo(b)) {
	  // found one, add coefficients - notice, as above, coeff could
	  // be zero, but deal with that case in SimplifyConstant
	  ret->GetNode(i)->SetCoeff(ret->GetNode(i)->GetCoeff() 
				    + b->GetCoeff());
	  couldsimplify = true;
	  break;
	}
      }
    }
    if (!couldsimplify) {
      // either could not simplify in steps above, or b is an operator
      ret->AddCopyOfNode(b);
    }
    return ret;
  } else if (a->GetOpType() == SUM && b->GetOpType() == SUM) {    
    // a, b are sums - add terms of b to a
    b->DistributeCoeffOverSum();
    ret.SetToCopyOf(a);
    Int i = 0;
    Int s = b->GetSize();
    for (i = 0; i < s; i++) {
      ret = ret + b->GetNode(i);
    }
    return ret;
  } else if (a->GetOpType() != SUM && b->GetOpType() == SUM) {
    // a is not a sum but b is - transform this into a sum
    ret.SetToCopyOf(b);
    ret = ret + a;
    return ret;
  } else {
    // all other cases - make new node on top of the addends
    ret->SetOpType(SUM);
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->AddCopyOfNode(a);
    ret->AddCopyOfNode(b);
    return ret;
  }
}


// product
Expression operator * (Expression a, Expression t) {
  Expression ret;
  // make a preliminary check  
  if (a->GetCoeff() == 0 || t->GetCoeff() == 0 || 
      a->HasValue(0) || t->HasValue(0)) {
    Expression zero(0.0);
    return zero;
  }
  if (a->HasValue(1)) {
    ret.SetToCopyOf(t);
    return ret;
  }
  if (t->HasValue(1)) {
    ret.SetToCopyOf(a);
    return ret; 
  }
  if (!(a->IsConstant() && t->IsConstant()) && a->IsEqualToNoCoeff(t)) {
    Expression two(2.0);
    ret.SetToCopyOf(a);
    ret->SetCoeff(a->GetCoeff() * t->GetCoeff());
    ret = ret^two;
    return ret;
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST &&
      t->IsLeaf() && t->GetOpType() == CONST) {
    // a, t are numbers - multiply them
    ret.SetToCopyOf(a);    
    ret->SetValue(a->GetValue() * t->GetValue());
    ret->SetCoeff(1);
    ret->SetExponent(1);
    return ret;
  } else if (a->IsLeaf() && a->GetOpType() == VAR &&
	     t->IsLeaf() && t->GetOpType() == VAR &&
	     a->GetVarIndex() == t->GetVarIndex()) {
    // a, t are the same variable - multiply coefficients
    // and add exponents
    ret.SetToCopyOf(a);
    ret->SetCoeff(a->GetCoeff() * t->GetCoeff());
    ret->SetExponent(a->GetExponent() + t->GetExponent());
    return ret;
  } else if (t->IsConstant()) {
    // t is constant, set coeff of a
    ret.SetToCopyOf(a);
    ret->SetCoeff(a->GetCoeff() * t->GetValue());
    ret->DistributeCoeffOverSum();
    return ret;
  } else if (a->IsConstant()) {
    // a is constant, set coeff of t
    ret.SetToCopyOf(t);
    ret->SetCoeff(t->GetCoeff() * a->GetValue());
    ret->DistributeCoeffOverSum();
    return ret;
  } else if (a->GetOpType() == PRODUCT && t->GetOpType() != PRODUCT) {
    // a is a product and t isn't - just multiply the term t
    ret.SetToCopyOf(a);
    Int i = 0;
    bool couldsimplify = false;
    if (t->IsLeaf() && t->GetOpType() == VAR) {
      // t is a variable
      Expression tmp;
      for (i = 0; i < ret->GetSize(); i++) {
	tmp = ret->GetNode(i);
	if (tmp->IsLeaf() && tmp->GetOpType() == VAR &&
	    t->GetVarIndex() == tmp->GetVarIndex()) {
	  // found same variable in a, multiply coeffs and add exponents
	  tmp->SetCoeff(tmp->GetCoeff() * t->GetCoeff());	
	  tmp->SetExponent(tmp->GetExponent() + t->GetExponent());
	  couldsimplify = true;
	  break;
	}
      }
    } 
    // here we shan't try to simplify f*f <-- f^2 (f nonleaf) 
    // because a product of nonleaves is easier to manipulate than 
    // a power (as it adds a depth level)
    if (!couldsimplify) {
      // either could not simplify in steps above, or t is an operator
      ret->AddCopyOfNode(t);
    }
    return ret;
  } else if (a->GetOpType() == PRODUCT && t->GetOpType() == PRODUCT) {
    // a, t are products - multiply terms of t to a
    t->DistributeCoeffOverProduct();
    ret.SetToCopyOf(a);
    Int i = 0;
    Int s = t->GetSize();
    for (i = 0; i < s; i++) {
      ret = ret * t->GetNode(i);
    }
    return ret;
  } else if (a->GetOpType() != PRODUCT && t->GetOpType() == PRODUCT) {
    // a is not a products but t is - transform this into a product
    ret.SetToCopyOf(t);
    ret = ret * a;
    return ret;
  } else {
    // all other cases - make new node on top of the addends
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(PRODUCT);
    ret->AddCopyOfNode(a);
    ret->AddCopyOfNode(t);
    return ret;
  }
}

// fractions:
Expression operator / (Expression a, Expression t) throw(ErrDivideByZero) {
  Expression ret;
  // make a preliminary check
#ifdef DEBUG2
  cout << "----diff----" << endl;
  cout << a->PrintTree(2, 2) << endl;
  cout << "  --over--" << endl;
  cout << t->PrintTree(2, 2) << endl;
  cout << "------------" << endl;
#endif
  if (t->GetCoeff() == 0) {
    // divide by zero
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("operator/");
    string myop("t.GetCoeff()==0");
    string mydesc("Divisor cannot be zero");
    string myinfo(HELPURL);
    string mydiv(NONE);
    throw ErrDivideByZero(mycode,myif,myscope,myop,mydesc,myinfo,mydiv);
  }
  if (a->GetCoeff() == 0 || a->HasValue(0)) {
    // dividend has zero coeff, return zero
    Expression zero(0.0);
    return zero;
  }
  if (t->HasValue(1)) {
    ret.SetToCopyOf(a);    
    return ret;
  }
  if (!(a->IsConstant() && t->IsConstant()) && a->IsEqualToNoCoeff(t)) {
    // dividend = divisor, return ratio of coefficients
    Expression one(1.0);
    one->SetCoeff(a->GetCoeff() / t->GetCoeff());
    return one;
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST &&
      t->IsLeaf() && t->GetOpType() == CONST) {
    // a, t are numbers - divide them
    if (t->GetValue() == 0) {
      unsigned long mycode(0);
      string myif("Expression Building");
      string myscope("operator/");
      string myop("t.GetValue()==0");
      string mydesc("Divisor cannot be zero");
      string myinfo(HELPURL);
      string mydiv(NONE);
      throw ErrDivideByZero(mycode,myif,myscope,myop,mydesc,myinfo,mydiv);
    } else {
      ret.SetToCopyOf(a);
      ret->SetValue(a->GetValue() / t->GetValue());
      ret->SetCoeff(1);
      ret->SetExponent(1);
      return ret;
    }
  } else if (t->HasValue(1)) {
    // divide by constant 1, don't do anything
    ret.SetToCopyOf(a);
    return ret;
  } else if (t->IsConstant()) {
    // t is constant, set coeff of a
    ret.SetToCopyOf(a);
    ret->SetCoeff(a->GetCoeff() / t->GetValue());
    ret->DistributeCoeffOverSum();
    return ret;
  } else if (a->IsVariable() && t->IsVariable() &&
	     a->GetVarIndex() == t->GetVarIndex()) {
    // cx^e / dx^f = (c/d)x^(e-f)
    ret.SetToCopyOf(a);
    double te = a->GetExponent() - t->GetExponent();
    double tc = a->GetCoeff() / t->GetCoeff();
    if (fabs(te) < Ev3NearZero()) {
      Expression c(tc);
      return tc;
    }
    ret->SetCoeff(tc);
    ret->SetExponent(te);
    return a;
  } else if (a->IsVariable() && t->GetOpType() == PRODUCT) {
    // a is a variable, t is a product - see if a appears in t
    // and cancel common term
    // first simplify coeffs of divisor
    Expression at;
    at.SetToCopyOf(a);
    ret.SetToCopyOf(t);
    ret->ConsolidateProductCoeffs();
    // denominator
    if (fabs(ret->GetCoeff()) < Ev3NearZero()) {
      // divide by zero
      unsigned long mycode(22);
      string myif("Expression Building");
      string myscope("operator/");
      string myop("t->GetCoeff()");
      string mydesc("Divisor cannot be zero");
      string myinfo(HELPURL);
      string mydiv(NONE);
      throw ErrDivideByZero(mycode,myif,myscope,myop,mydesc,myinfo,mydiv);
    }
    if (fabs(at->GetCoeff()) < Ev3NearZero()) {
      Expression zero(0.0);
      return zero;
    }
    double accumulatedcoeff = at->GetCoeff() / ret->GetCoeff();
    at->SetCoeff(1.0);
    ret->SetCoeff(1.0);
    // now try simplification
    Int i;
    for (i = 0; i < ret->GetSize(); i++) {
      if (ret->GetNode(i)->GetOpType() == VAR && 
	  at->GetVarIndex() == ret->GetNode(i)->GetVarIndex()) {
	double te = at->GetExponent() - ret->GetNode(i)->GetExponent();
	if (fabs(te) < Ev3NearZero()) {
	  // exponents are the same, just cancel
	  at->One();
	  ret->DeleteNode(i);
	} else if (te > 0) {
	  // numerator remains, cancel denominator
	  at->SetExponent(te);
	  ret->DeleteNode(i);
	} else if (te < 0) {
	  // numerator goes to one, denominator remains
	  at->One();
	  ret->GetNode(i)->SetExponent(-te);
	}
	// exit loop
	break;
      }
    }
    // check that denominator (t) has more than one operand;
    // if not, bring up a rank level
    if (ret->GetSize() == 1) {
      ret = ret->GetNode(0);
    }
    // build ratio
    Expression ret2;
    ret2->SetOpType(FRACTION);
    ret2->SetCoeff(accumulatedcoeff);
    ret2->SetExponent(1);
    ret2->AddCopyOfNode(at);
    ret2->AddCopyOfNode(ret);  
    return ret2;
  } else if (t->IsVariable() && a->GetOpType() == PRODUCT) {
    // t is a variable, a is a product - see if t appears in a
    // and cancel common term
    // first simplify coeffs of divisor
    Expression bt;
    bt.SetToCopyOf(t);
    ret.SetToCopyOf(a);    
    ret->ConsolidateProductCoeffs();
    // denominator - already checked
    if (fabs(ret->GetCoeff()) < Ev3NearZero()) {
      Expression zero(0.0);
      return zero;
    }
    double accumulatedcoeff = ret->GetCoeff() / bt->GetCoeff();
    ret->SetCoeff(1.0);
    bt->SetCoeff(1.0);
    // now try simplification
    Int i;
    for (i = 0; i < ret->GetSize(); i++) {
      if (ret->GetNode(i)->GetOpType() == VAR && 
	  bt->GetVarIndex() == ret->GetNode(i)->GetVarIndex()) {
	double te = ret->GetNode(i)->GetExponent() - bt->GetExponent();
	if (fabs(te) < Ev3NearZero()) {
	  // exponents are the same, just cancel
	  bt->One();
	  ret->DeleteNode(i);
	} else if (te > 0) {
	  // numerator remains, cancel denominator
	  bt->One();
	  ret->GetNode(i)->SetExponent(te);
	} else if (te < 0) {
	  // numerator goes to one, denominator remains
	  bt->SetExponent(-te);
	  ret->DeleteNode(i);
	}
	// exit loop
	break;
      }
    }
    // check that numerator (a) has more than one operands;
    // if not, bring up a rank level
    if (ret->GetSize() == 1) {
      ret = ret->GetNode(0);
    }
    // build ratio
    Expression ret2;
    ret2->SetOpType(FRACTION);
    ret2->SetCoeff(accumulatedcoeff);
    ret2->SetExponent(1);
    ret2->AddCopyOfNode(ret);
    ret2->AddCopyOfNode(bt);  
    return ret2;
  } else if (a->GetOpType() == PRODUCT && t->GetOpType() == PRODUCT) {
    // a, t are products, try to cancel common terms
    Expression at;
    Expression bt;
    at.SetToCopyOf(a);
    bt.SetToCopyOf(t);
    Int i = 0, j = 0;
    double accumulatedcoeff;
    // first simplify coefficients of operands
    at->ConsolidateProductCoeffs();
    bt->ConsolidateProductCoeffs();
    // denominator
    if (fabs(bt->GetCoeff()) < Ev3NearZero()) {
      // divide by zero
      unsigned long mycode(21);
      string myif("Expression Building");
      string myscope("operator/");
      string myop("t->GetCoeff()");
      string mydesc("Divisor cannot be zero");
      string myinfo(HELPURL);
      string mydiv(NONE);
      throw ErrDivideByZero(mycode,myif,myscope,myop,mydesc,myinfo,mydiv);
    }
    if (fabs(at->GetCoeff()) < Ev3NearZero()) {
      Expression zero(0.0);
      return zero;
    }
    // save ratio of coeffs of products
    accumulatedcoeff = at->GetCoeff() / bt->GetCoeff();
    at->SetCoeff(1.0);
    bt->SetCoeff(1.0);
    // now try simplification
    i = 0;
    bool isnumeratorempty = false;
    bool isdenominatorempty = false;
    int szi = at->GetSize();
    int szj = bt->GetSize();
    while(!isnumeratorempty && !isdenominatorempty && i < szi) {
      j = 0;
      while(!isnumeratorempty && !isdenominatorempty && j < szj) {
	if (at->GetNode(i)->IsEqualTo(bt->GetNode(j))) {
	  // found like terms i and j
	  at->DeleteNode(i);
	  szi--;
	  if(szi == 0) {
	    isnumeratorempty = true;
	    at->One();
	  }
	  bt->DeleteNode(j);
	  szj--;
	  if (szj == 0) {
	    isdenominatorempty = true;
	    bt->One();
	  }
	  i--;   // cancel the effect of the later i++
	  break; // go to outer cycle
	} else {
	  j++;
	}
      }	
      i++;
    }
    if (bt->HasValue(1)) {
      // denominator is 1, return a
      at->SetCoeff(accumulatedcoeff);
      return at;
    }
    // now construct fraction
    // check that numerator, denominator have more than one operands;
    // if not, bring up a rank level
    if (at->GetSize() == 1) {
      at = at->GetNode(0);
    }
    if (bt->GetSize() == 1) {
      bt = bt->GetNode(0);
    }
    ret->SetCoeff(accumulatedcoeff); // already contains coeffs of a, t
    ret->SetExponent(1);
    ret->SetOpType(FRACTION);
    ret->AddCopyOfNode(at);
    ret->AddCopyOfNode(bt);
    return ret;
  } else {
    Expression at;
    Expression bt;
    at.SetToCopyOf(a);
    bt.SetToCopyOf(t);
    ret->SetCoeff(at->GetCoeff() / bt->GetCoeff());
    at->SetCoeff(1);
    bt->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(FRACTION);
    ret->AddCopyOfNode(at);
    ret->AddCopyOfNode(bt);
    return ret;
  }
}    

// unary minus:
Expression operator - (Expression a) {
  Expression ret;
  ret.SetToCopyOf(a);
  if (ret->IsLeaf() && ret->GetOpType() == CONST) {
    ret->SetValue(- ret->GetValue());
    ret->SetCoeff(1);
    ret->SetExponent(1);    
  } else {
    ret->SetCoeff(- ret->GetCoeff());
  }
  return ret;
}

// binary minus:
Expression operator - (Expression a, Expression b) {
  Expression ret;
  if (a->HasValue(0)) 
    return -b;
  if (b->HasValue(0)) {
    ret.SetToCopyOf(a);
    return a;
  }
  ret = a + (-b);
  return ret;
}

// power:
Expression operator ^ (Expression a, Expression t) {
  // make a preliminary check
  Expression ret;
  if (a->GetCoeff() == 0) {
    // *this is zero, just return zero
    Expression zero(0.0);
    return zero;
  }
  if (t->HasValue(0.0)) {
    // exponent is 0, just return 1
    Expression one(1.0);
    return one;
  } else if (t->HasValue(1.0)) {
    // exponent is 1, just return a
    ret.SetToCopyOf(a);
    return ret;
  } 
  if (a->HasValue(0.0)) {
    // base is zero, return 0
    Expression zero(0.0);
    return zero;
  } else if (a->HasValue(1.0)) {
    // base is one, return 1
    Expression one(1.0);
    return one;
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST &&
      t->IsLeaf() && t->GetOpType() == CONST) {
    // constant to constant
    ret.SetToCopyOf(a);
    ret->SetValue(pow(ret->GetValue(), t->GetValue()));
    ret->SetCoeff(1);
    ret->SetExponent(1);
    return ret;
  } else if (a->IsLeaf() && a->GetOpType() == VAR &&
	     t->IsLeaf() && t->GetOpType() == CONST) {
    // variable to constant
    ret.SetToCopyOf(a);
    ret->SetExponent(ret->GetExponent() * t->GetValue());
    return ret;
  } else {
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(POWER);
    ret->AddCopyOfNode(a);
    ret->AddCopyOfNode(t);
    return ret;
  }
}

Expression Log(Expression a) throw(ErrNotPermitted) {
  // make a preliminary check
  if (a->IsZero()) {
    // *this is zero, can't do
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("Log");
    string myop("IsZero()");
    string mydesc("log(0) is undefined");
    string myinfo(HELPURL);
    throw ErrNotPermitted(mycode,myif,myscope,myop,mydesc,myinfo);
  }
  if (a->IsLessThan(0)) {
    // argument is < zero, can't do
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("Log");
    string myop("value <= 0");
    string mydesc("log(<=0) is undefined");
    string myinfo(HELPURL);
    throw ErrNotPermitted(mycode,myif,myscope,myop,mydesc,myinfo);
  } 
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    double t = ret->GetValue();
    assert(t >= 0);
    ret->SetCoeff(1);    
    ret->SetValue(log(t));
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(LOG);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

Expression Exp(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    ret->SetCoeff(1);
    ret->SetValue(exp(a->GetValue()));
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(EXP);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

Expression Sin(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    ret->SetCoeff(1);
    ret->SetValue(sin(a->GetValue()));
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(SIN);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

Expression Cos(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    ret->SetCoeff(1);
    ret->SetValue(cos(a->GetValue()));
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(COS);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

Expression Tan(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    ret->SetCoeff(1);
    ret->SetValue(tan(a->GetValue()));
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(TAN);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

Expression Cot(Expression a)  throw(ErrNotPermitted) {
  // make a preliminary check
  if (a->IsZero()) {
    // *this is zero, can't do
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("Cot");
    string myop("IsZero()");
    string mydesc("cot(0) is undefined");
    string myinfo(HELPURL);
    throw ErrNotPermitted(mycode,myif,myscope,myop,mydesc,myinfo);
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    double t = tan(a->GetValue());
    assert(t != 0);
    ret->SetCoeff(1);
    ret->SetValue(1 / t);
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(COT);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

Expression Sinh(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    ret->SetCoeff(1);
    ret->SetValue(sinh(a->GetValue()));
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(SINH);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

Expression Cosh(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    ret->SetCoeff(1);
    ret->SetValue(cosh(a->GetValue()));
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(COSH);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

Expression Tanh(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    ret->SetCoeff(1);
    ret->SetValue(tanh(a->GetValue()));
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(TANH);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

Expression Coth(Expression a)  throw(ErrNotPermitted) {
  // make a preliminary check
  if (a->IsZero()) {
    // *this is zero, can't do
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("Coth");
    string myop("IsZero()");
    string mydesc("coth(0) is undefined");
    string myinfo(HELPURL);
    throw ErrNotPermitted(mycode,myif,myscope,myop,mydesc,myinfo);
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    double t = tanh(a->GetValue());
    assert(t != 0);
    ret->SetCoeff(1);
    ret->SetValue(1 / t);
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(COTH);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

Expression Sqrt(Expression a) throw(ErrNotPermitted) {
  // make a preliminary check
  if (a->IsLessThan(0) && !a->HasValue(0)) {
    // argument is < zero, can't do
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("Sqrt");
    string myop("value < 0");
    string mydesc("sqrt(<0) is complex, can't do");
    string myinfo(HELPURL);
    throw ErrNotPermitted(mycode,myif,myscope,myop,mydesc,myinfo);
  } 
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    Expression ret;
    ret.SetToCopyOf(a);
    double t = a->GetValue();
    assert(t >= 0);
    ret->SetCoeff(1);
    ret->SetValue(sqrt(t));
    ret->SetExponent(1);
    ret->SetOpType(CONST);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(SQRT);
    ret->AddCopyOfNode(a);
    return ret;
  }
}

/***************** expression creation (affects arguments) ***********/

// sums:
Expression SumLink(Expression a, Expression b) {
  // make a preliminary check
  if (a->GetCoeff() == 0 || a->HasValue(0))
    return b;
  if (b->GetCoeff() == 0 || b->HasValue(0))
    return a;  
  if (!(a->IsConstant() && b->IsConstant()) && a->IsEqualToNoCoeff(b)) {
    a->SetCoeff(a->GetCoeff() + b->GetCoeff());
    if (fabs(a->GetCoeff()) < Ev3NearZero()) {
      // simplify to zero - for differences
      Expression zero(0.0);
      return zero;
    } else
      return a;
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST &&
      b->IsLeaf() && b->GetOpType() == CONST) {
    // a, b are numbers - add them
    a->SetValue(a->GetValue() + b->GetValue());
    a->SetCoeff(1);
    a->SetExponent(1);
    return a;
  } else if (a->IsLeaf() && a->GetOpType() == VAR &&
	     b->IsLeaf() && b->GetOpType() == VAR &&
	     a->GetVarIndex() == b->GetVarIndex() &&
	     a->GetExponent() == b->GetExponent()) {
    // a, b are the same variable - add coefficients
    a->SetCoeff(a->GetCoeff() + b->GetCoeff());
    return a;
  } else if (a->GetOpType() == SUM && b->GetOpType() != SUM) {    
    // a is a sum and b isn't - just add the term b
    a->DistributeCoeffOverSum();
    Int i = 0;
    bool couldsimplify = false;
    Expression tmp;
    // if t is a leaf and there is a like leaf in this,
    // just add it to the value/coefficient
    if (b->IsLeaf() && b->GetOpType() == CONST) {
      // b is a constant
      for (i = 0; i < a->GetSize(); i++) {
	tmp = a->GetNode(i);
	if (tmp->IsLeaf() && tmp->GetOpType() == CONST) {
	  tmp->SetValue(tmp->GetValue() + b->GetValue() / a->GetCoeff());
	  tmp->SetCoeff(1);
	  tmp->SetExponent(1); // NB: changing tmp should also change a
	  couldsimplify = true;
	  break;
	}
      }
    } else if (b->IsLeaf() && b->GetOpType() == VAR) {
      // b is a variable
      for (i = 0; i < a->GetSize(); i++) {
	if (a->GetNode(i)->IsLeaf() && a->GetNode(i)->GetOpType() == VAR &&
	    b->GetVarIndex() == a->GetNode(i)->GetVarIndex() &&
	    b->GetExponent() == a->GetNode(i)->GetExponent()) {
	  double tc = a->GetNode(i)->GetCoeff() + b->GetCoeff()/a->GetCoeff();
	  // warning: tc could be zero, but it would be cumbersome
	  // to simplify it here - do it in SimplifyConstant
	  a->GetNode(i)->SetCoeff(tc);
	  couldsimplify = true;
	  break;
	}
      }
    } else if (!b->IsLeaf()) {
      // a is a sum, b is a nonleaf, look for a subnode of a similar to b
      for (i = 0; i < a->GetSize(); i++) {
	if (a->GetNode(i)->IsEqualTo(b)) {
	  // found one, add coefficients - notice, as above, coeff could
	  // be zero, but deal with that case in SimplifyConstant
	  a->GetNode(i)->SetCoeff(a->GetNode(i)->GetCoeff() + b->GetCoeff());
	  couldsimplify = true;
	  break;
	}
      }
    }
    if (!couldsimplify) {
      // either could not simplify in steps above, or b is an operator
      a->AddNode(b);
    }
    return a;
  } else if (a->GetOpType() == SUM && b->GetOpType() == SUM) {    
    // a, b are sums - add terms of b to a
    b->DistributeCoeffOverSum();
    Int i = 0;
    Int s = b->GetSize();
    for (i = 0; i < s; i++) {
      a = SumLink(a, b->GetNode(i));
    }
    return a;
  } else if (a->GetOpType() != SUM && b->GetOpType() == SUM) {
    // a is not a sum but b is - transform this into a sum
    b = SumLink(b, a);
    return b;
  } else {
    // all other cases - make new node on top of the addends
    Expression ret;
    ret->SetOpType(SUM);
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->AddNode(a);
    ret->AddNode(b);
    return ret;
  }
}


// product
Expression ProductLink(Expression a, Expression t) {
  // make a preliminary check
  if (a->GetCoeff() == 0 || t->GetCoeff() == 0 || 
      a->HasValue(0) || t->HasValue(0)) {
    Expression zero(0.0);
    return zero;
  }
  if (a->HasValue(1))
    return t;
  if (t->HasValue(1))
    return a; 
  if (!(a->IsConstant() && t->IsConstant()) && a->IsEqualToNoCoeff(t)) {
    Expression two(2.0);
    a->SetCoeff(a->GetCoeff() * t->GetCoeff());
    return a^two;
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST &&
      t->IsLeaf() && t->GetOpType() == CONST) {
    // a, t are numbers - multiply them
    a->SetValue(a->GetValue() * t->GetValue());
    a->SetCoeff(1);
    a->SetExponent(1);
    return a;
  } else if (a->IsLeaf() && a->GetOpType() == VAR &&
	     t->IsLeaf() && t->GetOpType() == VAR &&
	     a->GetVarIndex() == t->GetVarIndex()) {
    // a, t are the same variable - multiply coefficients
    // and add exponents
    a->SetCoeff(a->GetCoeff() * t->GetCoeff());
    a->SetExponent(a->GetExponent() + t->GetExponent());
    return a;
  } else if (t->IsConstant()) {
    // t is constant, set coeff of a
    a->SetCoeff(a->GetCoeff() * t->GetValue());
    a->DistributeCoeffOverSum();
    return a;
  } else if (a->IsConstant()) {
    // a is constant, set coeff of t
    t->SetCoeff(t->GetCoeff() * a->GetValue());
    t->DistributeCoeffOverSum();
    return t;
  } else if (a->GetOpType() == PRODUCT && t->GetOpType() != PRODUCT) {
    // a is a product and t isn't - just multiply the term t
    Int i = 0;
    bool couldsimplify = false;
    if (t->IsLeaf() && t->GetOpType() == VAR) {
      // t is a variable
      Expression tmp;
      for (i = 0; i < a->GetSize(); i++) {
	tmp = a->GetNode(i);
	if (tmp->IsLeaf() && tmp->GetOpType() == VAR &&
	    t->GetVarIndex() == tmp->GetVarIndex()) {
	  // found same variable in a, multiply coeffs and add exponents
	  tmp->SetCoeff(tmp->GetCoeff() * t->GetCoeff());	
	  tmp->SetExponent(tmp->GetExponent() + t->GetExponent());
	  couldsimplify = true;
	  break;
	}
      }
    } 
    // here we shan't try to simplify f*f <-- f^2 (f nonleaf) 
    // because a product of nonleaves is easier to manipulate than 
    // a power (as it adds a depth level)
    if (!couldsimplify) {
      // either could not simplify in steps above, or t is an operator
      a->AddNode(t);
    }
    return a;
  } else if (a->GetOpType() == PRODUCT && t->GetOpType() == PRODUCT) {
    // a, t are products - multiply terms of t to a
    t->DistributeCoeffOverProduct();
    Int i = 0;
    Int s = t->GetSize();
    for (i = 0; i < s; i++) {
      a = ProductLink(a, t->GetNode(i));
    }
    return a;
  } else if (a->GetOpType() != PRODUCT && t->GetOpType() == PRODUCT) {
    // a is not a products but t is - transform this into a product
    t = ProductLink(t, a);
    return t;
  } else {
    // all other cases - make new node on top of the addends
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(PRODUCT);
    ret->AddNode(a);
    ret->AddNode(t);
    return ret;
  }
}

// fractions:
Expression FractionLink(Expression a, Expression t) 
  throw(ErrDivideByZero) {
  // make a preliminary check
  if (t->GetCoeff() == 0) {
    // divide by zero
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("FractionLink");
    string myop("t.GetCoeff()==0");
    string mydesc("Divisor cannot be zero");
    string myinfo(HELPURL);
    string mydiv(NONE);
    throw ErrDivideByZero(mycode,myif,myscope,myop,mydesc,myinfo,mydiv);
  }
  if (a->GetCoeff() == 0 || a->HasValue(0)) {
    // dividend has zero coeff, return zero
    Expression zero(0.0);
    return zero;
  }
  if (t->HasValue(1))
    return a;
  if (!(a->IsConstant() && t->IsConstant()) && a->IsEqualToNoCoeff(t)) {
    // dividend = divisor, return ratio of coefficients
    Expression one(1.0);
    one->SetCoeff(a->GetCoeff() / t->GetCoeff());
    return one;
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST &&
      t->IsLeaf() && t->GetOpType() == CONST) {
    // a, t are numbers - divide them
    if (t->GetValue() == 0) {
      unsigned long mycode(0);
      string myif("Expression Building");
      string myscope("FractionLink");
      string myop("t.GetValue()==0");
      string mydesc("Divisor cannot be zero");
      string myinfo(HELPURL);
      string mydiv(NONE);
      throw ErrDivideByZero(mycode,myif,myscope,myop,mydesc,myinfo,mydiv);
    } else {
      a->SetValue(a->GetValue() / t->GetValue());
      a->SetCoeff(1);
      a->SetExponent(1);
      return a;
    }
  } else if (t->HasValue(1)) {
    // divide by constant 1, don't do anything
    return a;
  } else if (t->IsConstant()) {
    // t is constant, set coeff of a
    a->SetCoeff(a->GetCoeff() / t->GetValue());
    a->DistributeCoeffOverSum();
    return a;
  } else if (a->IsVariable() && t->IsVariable() &&
	     a->GetVarIndex() == t->GetVarIndex()) {
    // cx^e / dx^f = (c/d)x^(e-f)
    double te = a->GetExponent() - t->GetExponent();
    double tc = a->GetCoeff() / t->GetCoeff();
    if (fabs(te) < Ev3NearZero()) {
      Expression c(tc);
      return tc;
    }
    a->SetCoeff(tc);
    a->SetExponent(te);
    return a;
  } else if (a->IsVariable() && t->GetOpType() == PRODUCT) {
    // a is a variable, t is a product - see if a appears in t
    // and cancel common term
    // first simplify coeffs of divisor
    t->ConsolidateProductCoeffs();
    // denominator
    if (fabs(t->GetCoeff()) < Ev3NearZero()) {
      // divide by zero
      unsigned long mycode(22);
      string myif("Expression Building");
      string myscope("FractionLink");
      string myop("t->GetCoeff()");
      string mydesc("Divisor cannot be zero");
      string myinfo(HELPURL);
      string mydiv(NONE);
      throw ErrDivideByZero(mycode,myif,myscope,myop,mydesc,myinfo,mydiv);
    }
    if (fabs(a->GetCoeff()) < Ev3NearZero()) {
      Expression zero(0.0);
      return zero;
    }
    double accumulatedcoeff = a->GetCoeff() / t->GetCoeff();
    a->SetCoeff(1.0);
    t->SetCoeff(1.0);
    // now try simplification
    Int i;
    for (i = 0; i < t->GetSize(); i++) {
      if (t->GetNode(i)->GetOpType() == VAR && 
	  a->GetVarIndex() == t->GetNode(i)->GetVarIndex()) {
	double te = a->GetExponent() - t->GetNode(i)->GetExponent();
	if (fabs(te) < Ev3NearZero()) {
	  // exponents are the same, just cancel
	  a->One();
	  t->DeleteNode(i);
	} else if (te > 0) {
	  // numerator remains, cancel denominator
	  a->SetExponent(te);
	  t->DeleteNode(i);
	} else if (te < 0) {
	  // numerator goes to one, denominator remains
	  a->One();
	  t->GetNode(i)->SetExponent(-te);
	}
	// exit loop
	break;
      }
    }
    // check that denominator (t) has more than one operands;
    // if not, bring up a rank level
    if (t->GetSize() == 1) {
      t = t->GetNode(0);
    }
    // build ratio
    Expression ret;
    ret->SetOpType(FRACTION);
    ret->SetCoeff(accumulatedcoeff);
    ret->SetExponent(1);
    ret->AddNode(a);
    ret->AddNode(t);  
    return ret;
  } else if (t->IsVariable() && a->GetOpType() == PRODUCT) {
    // t is a variable, a is a product - see if t appears in a
    // and cancel common term
    // first simplify coeffs of divisor
    a->ConsolidateProductCoeffs();
    // denominator - already checked
    if (fabs(a->GetCoeff()) < Ev3NearZero()) {
      Expression zero(0.0);
      return zero;
    }
    double accumulatedcoeff = a->GetCoeff() / t->GetCoeff();
    a->SetCoeff(1.0);
    t->SetCoeff(1.0);
    // now try simplification
    Int i;
    for (i = 0; i < a->GetSize(); i++) {
      if (a->GetNode(i)->GetOpType() == VAR && 
	  t->GetVarIndex() == a->GetNode(i)->GetVarIndex()) {
	double te = a->GetNode(i)->GetExponent() - t->GetExponent();
	if (fabs(te) < Ev3NearZero()) {
	  // exponents are the same, just cancel
	  t->One();
	  a->DeleteNode(i);
	} else if (te > 0) {
	  // numerator remains, cancel denominator
	  t->One();
	  a->GetNode(i)->SetExponent(te);
	} else if (te < 0) {
	  // numerator goes to one, denominator remains
	  t->SetExponent(-te);
	  a->DeleteNode(i);
	}
	// exit loop
	break;
      }
    }
    // check that numerator (a) has more than one operands;
    // if not, bring up a rank level
    if (a->GetSize() == 1) {
      a = a->GetNode(0);
    }
    // build ratio
    Expression ret;
    ret->SetOpType(FRACTION);
    ret->SetCoeff(accumulatedcoeff);
    ret->SetExponent(1);
    ret->AddNode(a);
    ret->AddNode(t);  
    return ret;
  } else if (a->GetOpType() == PRODUCT && t->GetOpType() == PRODUCT) {
    // a, t are products, try to cancel common terms
    Int i = 0, j = 0;
    double accumulatedcoeff;
    // first simplify coefficients of operands
    a->ConsolidateProductCoeffs();
    t->ConsolidateProductCoeffs();
    // denominator
    if (fabs(t->GetCoeff()) < Ev3NearZero()) {
      // divide by zero
      unsigned long mycode(21);
      string myif("Expression Building");
      string myscope("FractionLink");
      string myop("t->GetCoeff()");
      string mydesc("Divisor cannot be zero");
      string myinfo(HELPURL);
      string mydiv(NONE);
      throw ErrDivideByZero(mycode,myif,myscope,myop,mydesc,myinfo,mydiv);
    }
    if (fabs(a->GetCoeff()) < Ev3NearZero()) {
      Expression zero(0.0);
      return zero;
    }
    // save ratio of coeffs of products
    accumulatedcoeff = a->GetCoeff() / t->GetCoeff();
    a->SetCoeff(1.0);
    t->SetCoeff(1.0);
    // now try simplification
    i = 0;
    bool isnumeratorempty = false;
    bool isdenominatorempty = false;
    int szi = a->GetSize();
    int szj = t->GetSize();
    while(!isnumeratorempty && !isdenominatorempty && i < szi) {
      j = 0;
      while(!isnumeratorempty && !isdenominatorempty && j < szj) {
	if (a->GetNode(i)->IsEqualTo(t->GetNode(j))) {
	  // found like terms i and j
	  a->DeleteNode(i);
	  szi--;
	  if(szi == 0) {
	    isnumeratorempty = true;
	    a->One();
	  }
	  t->DeleteNode(j);
	  szj--;
	  if (szj == 0) {
	    isdenominatorempty = true;
	    t->One();
	  }
	  i--;   // cancel the effect of the later i++
	  break; // go to outer cycle
	} else {
	  j++;
	}
      }	
      i++;
    }
    if (t->HasValue(1)) {
      // denominator is 1, return a
      a->SetCoeff(accumulatedcoeff);
      return a;
    }
    // now construct fraction
    // check that numerator, denominator have more than one operands;
    // if not, bring up a rank level
    if (a->GetSize() == 1) {
      a = a->GetNode(0);
    }
    if (t->GetSize() == 1) {
      t = t->GetNode(0);
    }
    Expression ret;
    ret->SetCoeff(accumulatedcoeff); // already contains coeffs of a, t
    ret->SetExponent(1);
    ret->SetOpType(FRACTION);
    ret->AddNode(a);
    ret->AddNode(t);
    return ret;
  } else {
    Expression ret;
    ret->SetCoeff(a->GetCoeff() / t->GetCoeff());
    a->SetCoeff(1);
    t->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(FRACTION);
    ret->AddNode(a);
    ret->AddNode(t);
    return ret;
  }
}    

// unary minus:
Expression MinusLink(Expression a) {
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    a->SetValue(- a->GetValue());
    a->SetCoeff(1);
    a->SetExponent(1);    
  } else {
    a->SetCoeff(- a->GetCoeff());
  }
  return a;
}

// binary minus:
Expression DifferenceLink(Expression a, Expression b) {
  if (a->HasValue(0))
    return MinusLink(b);
  if (b->HasValue(0))
    return a;
  return SumLink(a, MinusLink(b));
}

// power:
Expression PowerLink(Expression a, Expression t) {
  // make a preliminary check
  if (a->GetCoeff() == 0) {
    // *this is zero, just return zero
    Expression zero(0.0);
    return zero;
  }
  if (t->HasValue(0.0)) {
    // exponent is 0, just return 1
    Expression one(1.0);
    return one;
  } else if (t->HasValue(1.0)) {
    // exponent is 1, just return a
    return a;
  } 
  if (a->HasValue(0.0)) {
    // base is zero, return 0
    Expression zero(0.0);
    return zero;
  } else if (a->HasValue(1.0)) {
    // base is one, return 1
    Expression one(1.0);
    return one;
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST &&
      t->IsLeaf() && t->GetOpType() == CONST) {
    // constant to constant
    a->SetValue(pow(a->GetValue(), t->GetValue()));
    a->SetCoeff(1);
    a->SetExponent(1);
    return a;
  } else if (a->IsLeaf() && a->GetOpType() == VAR &&
	     t->IsLeaf() && t->GetOpType() == CONST) {
    // variable to constant
    a->SetExponent(a->GetExponent() * t->GetValue());
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(POWER);
    ret->AddNode(a);
    ret->AddNode(t);
    return ret;
  }
}

Expression LogLink(Expression a) throw(ErrNotPermitted) {
  // make a preliminary check
  if (a->IsZero()) {
    // *this is zero, can't do
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("LogLink");
    string myop("IsZero()");
    string mydesc("log(0) is undefined");
    string myinfo(HELPURL);
    throw ErrNotPermitted(mycode,myif,myscope,myop,mydesc,myinfo);
  }
  if (a->IsLessThan(0)) {
    // argument is < zero, can't do
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("LogLink");
    string myop("value <= 0");
    string mydesc("log(<=0) is undefined");
    string myinfo(HELPURL);
    throw ErrNotPermitted(mycode,myif,myscope,myop,mydesc,myinfo);
  } 
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    double t = a->GetValue();
    assert(t >= 0);
    a->SetCoeff(1);    
    a->SetValue(log(t));
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(LOG);
    ret->AddNode(a);
    return ret;
  }
}

Expression ExpLink(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    a->SetCoeff(1);
    a->SetValue(exp(a->GetValue()));
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(EXP);
    ret->AddNode(a);
    return ret;
  }
}

Expression SinLink(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    a->SetCoeff(1);
    a->SetValue(sin(a->GetValue()));
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(SIN);
    ret->AddNode(a);
    return ret;
  }
}

Expression CosLink(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    a->SetCoeff(1);
    a->SetValue(cos(a->GetValue()));
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(COS);
    ret->AddNode(a);
    return ret;
  }
}

Expression TanLink(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    a->SetCoeff(1);
    a->SetValue(tan(a->GetValue()));
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(TAN);
    ret->AddNode(a);
    return ret;
  }
}

Expression CotLink(Expression a)  throw(ErrNotPermitted) {
  // make a preliminary check
  if (a->IsZero()) {
    // *this is zero, can't do
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("CotLink");
    string myop("IsZero()");
    string mydesc("cot(0) is undefined");
    string myinfo(HELPURL);
    throw ErrNotPermitted(mycode,myif,myscope,myop,mydesc,myinfo);
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    double t = tan(a->GetValue());
    assert(t != 0);
    a->SetCoeff(1);
    a->SetValue(1 / t);
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(COT);
    ret->AddNode(a);
    return ret;
  }
}

Expression SinhLink(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    a->SetCoeff(1);
    a->SetValue(sinh(a->GetValue()));
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(SINH);
    ret->AddNode(a);
    return ret;
  }
}

Expression CoshLink(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    a->SetCoeff(1);
    a->SetValue(cosh(a->GetValue()));
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(COSH);
    ret->AddNode(a);
    return ret;
  }
}

Expression TanhLink(Expression a) {
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    a->SetCoeff(1);
    a->SetValue(tanh(a->GetValue()));
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(TANH);
    ret->AddNode(a);
    return ret;
  }
}

Expression CothLink(Expression a)  throw(ErrNotPermitted) {
  // make a preliminary check
  if (a->IsZero()) {
    // *this is zero, can't do
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("CothLink");
    string myop("IsZero()");
    string mydesc("coth(0) is undefined");
    string myinfo(HELPURL);
    throw ErrNotPermitted(mycode,myif,myscope,myop,mydesc,myinfo);
  }
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    double t = tanh(a->GetValue());
    assert(t != 0);
    a->SetCoeff(1);
    a->SetValue(1 / t);
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(COTH);
    ret->AddNode(a);
    return ret;
  }
}

Expression SqrtLink(Expression a) throw(ErrNotPermitted) {
  // make a preliminary check
  if (a->IsLessThan(0) && !a->HasValue(0)) {
    // argument is < zero, can't do
    unsigned long mycode(0);
    string myif("Expression Building");
    string myscope("SqrtLink");
    string myop("value < 0");
    string mydesc("sqrt(<0) is complex, can't do");
    string myinfo(HELPURL);
    throw ErrNotPermitted(mycode,myif,myscope,myop,mydesc,myinfo);
  } 
  // go for it
  if (a->IsLeaf() && a->GetOpType() == CONST) {
    double t = a->GetValue();
    assert(t >= 0);
    a->SetCoeff(1);
    a->SetValue(sqrt(t));
    a->SetExponent(1);
    a->SetOpType(CONST);
    return a;
  } else {
    Expression ret;
    ret->SetCoeff(1);
    ret->SetExponent(1);
    ret->SetOpType(SQRT);
    ret->AddNode(a);
    return ret;
  }
}


/***************** differentiation ******************/

// differentiate w.r.t. variable varindex
Expression Diff(const Expression& ac, Int vi) {
  Expression ret(DiffNoSimplify(ac, vi));
  Simplify(&ret);
  return ret;
}

Expression DiffNoSimplify(const Expression& ac, Int vi) {
  Expression a;
  a.SetToCopyOf(ac);
  Expression zero(0.0);
  Expression c(1.0);
  if (a->DependsOnVariable(vi)) {
    if (a->IsLeaf()) {
      if (a->GetOpType() == CONST || a->GetVarIndex() != vi) {
	// safety check
	cerr << "Expression::Diff: warning: this node should "
	     << "not diff to zero\n";
	return zero;
      } else {
	// variable vi, check exponent
	if (a->GetExponent() == 0) {
	  // exponent is zero, node is actually constant
	  return zero;
	} else if (a->GetExponent() == 1) {
	  // exponent is one, node is variable
	  c->SetValue(a->GetCoeff());
	  return c;
	} else {
	  // for all other cases, apply rule x^c = c*x^(c-1)
	  double expon = a->GetExponent();
	  Expression ret(a.Copy());
	  ret->SetExponent(expon - 1);
	  ret->SetCoeff(ret->GetCoeff() * expon);
	  return ret;
	}
      }
    } else {
      // non-leaf node. build derivative.
      int op = a->GetOpType();
      Int sz = a->GetSize();
      double opcoeff = a->GetCoeff();
      if (sz == 0) {
	throw ErrNotPermitted(10, "Expression", "Diff", "GetSize() == 0",
			      "non-leaf node can't have size 0", HELPURL);
      }
      Int i, j;
      Expression ret(0.0);
      Expression tmp(1.0);
      Expression two(2.0);
      switch(op) {
      case SUM:
	ret = Diff(a->GetNode(0), vi); // f_0'
	for(i = 1; i < sz; i++) {
	  ret = ret + Diff(a->GetNode(i), vi); // ... + g_i'
	}
	break;
      case DIFFERENCE:
	ret = Diff(a->GetNode(0), vi);  // f_0'
	for(i = 1; i < sz; i++) {
	  ret = ret - Diff(a->GetNode(i), vi);  // ... - g_i'
	}
	break;
      case PRODUCT:
	if (sz == 1) {
	  // warn about product with one operand 
	  cerr << "Expression::Diff: warning: product with 1 operand "
	       << "should not occur\n";
	} 
	ret = Diff(a->GetNode(0), vi);  // f_0'
	for(j = 1; j < sz; j++) {
	  // get copies, not references
	  ret = ret * a->GetCopyOfNode(j); // ... * f_i[i!=0]
	}
	tmp->One(); // reset temporary to 1.0
	for(i = 1; i < sz; i++) {
	  tmp = Diff(a->GetNode(i), vi); // tmp = f_i'
	  for(j = 0; j < sz; j++) {
	    if (j != i) 
	      // get references, and copy later (in sum)
	      tmp = tmp * a->GetNode(j); // ... * f_j[i!=i]
	  }
	  ret = ret + tmp.Copy();  // ... + tmp
	  tmp->One(); // reset temporary to 1.0 
	}
	break;
      case FRACTION:
	if (sz != 2) {
	  // check that there are exactly two operands
	  throw ErrNotPermitted(11, "Expression", "Diff", "GetSize() != 2",
				"fraction must have exactly 2 operands", 
				HELPURL);
	}
	if (a->GetNode(1)->IsZero()) {
	  // check denominator is not zero
	  throw ErrDivideByZero(20, "Expression", "Diff", 
				"GetNode(1)->IsZero()", 
				"cannot divide by zero", HELPURL, 
				a->GetNode(1)->ToString());
	}
	tmp->One();
	ret = Diff(a->GetNode(0), vi); // f'
	ret = ret * a->GetCopyOfNode(1);  // f'g
	// can dispense from using GetCopyOf because tmp gets copied anyway
	tmp = a->GetNode(0);           // tmp = f
	tmp = tmp * Diff(a->GetNode(1), vi);  // tmp = fg'
	ret = ret - tmp.Copy();    // f'g - fg'
	tmp->One();
	tmp = a->GetNode(1);  // tmp = g
	tmp = tmp ^ two; // g^2
	// can dispense from using copy here - tmp is not used thereafter
	// and when tmp is deleted, its subnodes are not automatically
	// deleted unless reference counter is zero - which won't be.
	ret = ret / tmp;   // (f'g - fg')/g^2
	break;
      case POWER:
	if (sz != 2) {
	  // check that there are exactly two operands
	  throw ErrNotPermitted(12, "Expression", "Diff", "GetSize() != 2",
				"power must have exactly 2 operands", 
				HELPURL);
	}
	// check exponent
	if (a->GetNode(1)->IsZero()) {
	  // exponent is zero, whole node is one, diff is zero
	  ret->Zero();
	} else if (a->GetNode(1)->HasValue(1.0)) {
	  // exponent is one, whole node is first operand, diff
	  // is diff of first operand
	  ret = Diff(a->GetNode(0), vi);
	} else if (a->GetNode(1)->HasValue(2.0)) { 
	  // exponent is two, diff is 2 * first op * diff of first operand
	  ret = Diff(a->GetNode(0), vi);  // f'
	  ret = ret * a->GetCopyOfNode(0);   // f'f
	  ret->SetCoeff(ret->GetCoeff() * 2.0);  // 2f'f
	} else {
	  // all other cases
	  if (a->GetNode(1)->IsConstant()) {
	    // numeric exponent != 0,1,2
	    ret = Diff(a->GetNode(0), vi); // f'	    
	    tmp = a->GetCopyOfNode(0);     // f
	    tmp = tmp ^ a->GetCopyOfNode(1);  // f^c
	    tmp->GetNode(1)->ConsolidateValue();
	    tmp->SetCoeff(tmp->GetCoeff() * tmp->GetNode(1)->GetValue());//cf^c
	    tmp->GetNode(1)->SetValue(ret->GetNode(1)->GetValue()-1);//cf^(c-1)
	    // can dispense from using copy here - Diff returns copies anyway.
	    // when temporary is deleted, its subnodes are not automatically
	    // deleted unless their reference counter is zero - which won't be.
	    ret = ret * tmp; // f'(cf^(c-1))
	  } else {
	    // symbolic exponent f^g    
	    ret = a->GetCopyOfNode(0); // f
	    ret = Log(ret); // log(f)
	    // can dispense from using copy here - Diff returns copies anyway.
	    // when temporary is deleted, its subnodes are not automatically
	    // deleted unless their reference counter is zero - which won't be.
	    ret = ret * Diff(a->GetNode(1), vi); // g' log(f)
	    tmp = Diff(a->GetNode(0), vi);  // f'
	    tmp = tmp * a->GetCopyOfNode(1);   // gf'
	    tmp = tmp / a->GetCopyOfNode(0);    // gf'/f
	    // can dispense from using copy here - tmp is not used thereafter
	    // and when tmp is deleted, its subnodes are not automatically
	    // deleted unless their reference counter is zero - which won't be.
	    ret = ret / tmp;           // g'log(f) + gf'/f
	    ret = ret * a.Copy();        // (g'log(f) + gf'/f)(f^g)
	  }
	}
	break;
      case MINUS:
	if (sz != 1) {
	  // check that there is exactly one operand
	  throw ErrNotPermitted(13, "Expression", "Diff", "GetSize() != 1",
				"unary minus must have exactly 1 operand", 
				HELPURL);
	}
	ret = Diff(a->GetNode(0), vi);
	ret->SetCoeff(- ret->GetCoeff());
	break;
      case LOG:
	if (sz != 1) {
	  // check that there is exactly one operand
	  throw ErrNotPermitted(14, "Expression", "Diff", "GetSize() != 1",
				"log must have exactly 1 operand", 
				HELPURL);
	}
	if (a->GetNode(0)->IsLessThan(0)) {
	  throw ErrNotPermitted(15, "Expression", "Diff", "arg <= 0",
				"log argument must be symbolic or positive",
				HELPURL);
	}
	ret = Diff(a->GetNode(0), vi);  // f'
	ret = ret / a->GetCopyOfNode(0);  // f'/f       
	break;
      case EXP:
	if (sz != 1) {
	  // check that there is exactly one operand
	  throw ErrNotPermitted(16, "Expression", "Diff", "GetSize() != 1",
				"exp must have exactly 1 operand", 
				HELPURL);
	}
	ret = Diff(a->GetNode(0), vi);  // f'
	ret = ret * a.Copy();  // f' e^f
	break;
      case SIN:
	if (sz != 1) {
	  // check that there is exactly one operand
	  throw ErrNotPermitted(17, "Expression", "Diff", "GetSize() != 1",
				"sin must have exactly 1 operand", 
				HELPURL);
	}
	ret = Diff(a->GetNode(0), vi) * Cos(a->GetCopyOfNode(0));  // f' cos(f)
	break;
      case COS:
	if (sz != 1) {
	  // check that there is exactly one operand
	  throw ErrNotPermitted(18, "Expression", "Diff", "GetSize() != 1",
				"cos must have exactly 1 operand", 
				HELPURL);
	}
	ret = -Diff(a->GetNode(0), vi) * Sin(a->GetCopyOfNode(0)); // -f'sin(f)
	break;
      case TAN:
	if (sz != 1) {
	  // check that there is exactly one operand
	  throw ErrNotPermitted(19, "Expression", "Diff", "GetSize() != 1",
				"tan must have exactly 1 operand", 
				HELPURL);
	}
	ret = a.Copy();  // tan(f)
	ret = ret ^ two;      // tan(f)^2
	c->One();
	ret = ret + c;         // tan(f)^2 + 1
	ret = ret * Diff(a->GetNode(0), vi);    // f' * (tan(f)^2 + 1)
	break;
      default:
	// missing cases: COT, SINH, COSH, TANH, COTH, SQRT
	break;
      }
      ret->SetCoeff(ret->GetCoeff() * opcoeff);
      return ret;
    }
  } else {
    return zero;
  }
  return zero;
}

/************************ simplifications **********************/

bool TrigSimp(Expression a) {
  Int i = 0;
  Int ret = 0;
  bool bret = false;
  bool ischanged = false;
  // first, recurse over all subnodes of a
  for(i = 0; i < a->GetSize(); i++) {
    ischanged = TrigSimp(a->GetNode(i));
    if (!bret && ischanged) {
      bret = true;
    }
  }
  // now try simplification on a
  if (a->GetOpType() == SUM && a->GetSize() > 1) {    
    // try to look for a sin^2 and a cos^2
    Int sinpos = -1;
    Int cospos = -1;
    Int sinpossimple = -1;
    Int cospossimple = -1;
    for (i = 0; i < a->GetSize(); i++) {
      // cycle over subnodes
      if (a->GetNode(i)->GetOpType() == POWER) {
	if (a->GetNode(i)->GetNode(0)->GetOpType() == SIN &&
	    a->GetNode(i)->GetNode(1)->HasValue(2))
	  sinpos = i;
      }
      if (a->GetNode(i)->GetOpType() == POWER) {
	if (a->GetNode(i)->GetNode(0)->GetOpType() == COS &&
	    a->GetNode(i)->GetNode(1)->HasValue(2))
	  cospos = i;
      }
      if (a->GetNode(i)->GetOpType() == SIN &&
	  a->GetNode(i)->GetExponent() == 2) {
	sinpossimple = i;
      } 
      if (a->GetNode(i)->GetOpType() == COS &&
	  a->GetNode(i)->GetExponent() == 2) {
	cospossimple = i;
      }
    }
    if (sinpos != -1 && cospos != -1) {
      // found both, check their arguments
      if (a->GetNode(sinpos)->GetNode(0)->GetNode(0)->IsEqualTo
	  (a->GetNode(cospos)->GetNode(0)->GetNode(0))) {
	ret++; // augment simplification counter
	bret = true;
	// arguments are equal, can simplify
	Int f = sinpos < cospos ? sinpos : cospos; // last to delete
	Int l = sinpos > cospos ? sinpos : cospos; // first to delete
	a->DeleteNode(l); 
	a->DeleteNode(f); 
	// verify that there are still some nodes left
	if (a->GetSize() == 0) {
	  // there aren't any, set a to one
	  a->One();
	} else {
	  // there are some, check whether there is a constant part
	  // we can add the 1 to
	  bool addflag = false;
	  for (i = 0; i < a->GetSize(); i++) {
	    if (a->GetNode(i)->IsConstant()) {
	      // yes there is
	      a->GetNode(i)->SetValue(a->GetNode(i)->GetSimpleValue() + 1);
	      addflag = true;
	      break;
	    }
	  }
	  if (!addflag) {
	    // no there wasn't, add it as a symbolic node
	    Expression one(1.0);
	    a->AddNode(one);
	  }
	  // check that there is more than just one node
	  if (a->GetSize() == 1) {
	    // only one node, shift everything one rank level up
	    a = a->GetNode(0);
	  }
	}
      }
    }
    if (sinpossimple != -1 && cospossimple != -1) {
      if (a->GetNode(sinpossimple)->GetNode(0)->IsEqualTo
	  (a->GetNode(cospossimple)->GetNode(0))) {
	ret++;
	bret = true;
	// arguments are equal, can simplify
	Int f = sinpossimple < cospossimple ? sinpossimple : cospossimple; 
	Int l = sinpossimple > cospossimple ? sinpossimple : cospossimple; 
	a->DeleteNode(l); 
	a->DeleteNode(f); 
	// verify that there are still some nodes left
	if (a->GetSize() == 0) {
	  // there aren't any, set a to one
	  a->One();
	} else {
	  // there are some, check whether there is a constant part
	  // we can add the 1 to
	  bool addflag = false;
	  for (i = 0; i < a->GetSize(); i++) {
	    if (a->GetNode(i)->IsConstant()) {
	      // yes there is
	      a->GetNode(i)->SetValue(a->GetNode(i)->GetSimpleValue() + 1);
	      addflag = true;
	      break;
	    }
	  }
	  if (!addflag) {
	    // no there wasn't, add it as a symbolic node
	    Expression one(1.0);
	    a->AddNode(one);
	  }
	  // check that there is more than just one node
	  if (a->GetSize() == 1) {
	    // only one node, shift everything one rank level up
	    a = a->GetNode(0);
	  }
	}
      }
    }
  } 
  if (ret > 0)
    bret = true;
  return bret;
}

// generic simplification with modification of the expression
bool Simplify(Expression* a) {
  bool changedflag = false;
  bool ret = false;
  bool goonflag = true;
  while(goonflag) {
    goonflag = false;
    (*a)->ConsolidateProductCoeffs();
    (*a)->DistributeCoeffOverSum();
    ret = DifferenceToSum(a);
    if (ret) {
      changedflag = true;
      goonflag = true;
    }
    ret = SimplifyConstant(a);
    if (ret) {
      changedflag = true;
      goonflag = true;
    }
    ret = CompactProducts(a);
    if (ret) {
      changedflag = true;  
      goonflag = true;
    }
    ret = CompactLinearPart(a);
    if (ret) {
      changedflag = true;  
      goonflag = true;
    }    
    ret = SimplifyRecursive(a);
    if (ret) {
      changedflag = true;
      goonflag = true;
    }
    ret = TrigSimp(*a);
    if (ret) {
      changedflag = true;
      goonflag = true;
    }
  }
  return changedflag;
}

// call after DifferenceToSum
bool SimplifyConstant(Expression* a) {
  bool ret = false;
  Expression one(1.0);
  Expression zero(0.0);
  if ((*a)->GetExponent() == 0) {
    RecursiveDestroy(a);
    a->SetTo(one);
    ret = true;
  } else if ((*a)->GetCoeff() == 0) {
    RecursiveDestroy(a);
    a->SetTo(zero);
    ret = true;
  } else {
    int i = 0;
    int op, ops;
    op = (*a)->GetOpType();
    bool ischanged = false;
    int sz = (*a)->GetSize();
    while (i < sz) {
      // simplify each of the terms
      ischanged = SimplifyConstant((*a)->GetNodePtr(i));
      if (!ret && ischanged) {
	ret = true;
      }
      i++;
    }
    i = 0;
    while (i < sz) {
      // simplify this operand as a whole
      ops = (*a)->GetNode(i)->GetOpType();
      switch(op) {
      case SUM:
	if (ops == CONST) {
	  if ((*a)->GetNode(i)->GetValue() == 0) {
	    (*a)->DeleteNode(i);
	    ret = true;
	    sz--;
	    if (sz == 1) {
	      a->SetTo((*a)->GetNode(0));
	      i = 0;
	      sz = (*a)->GetSize();
	    }
	  } else {
	    i++;
	  }
	} else {
	  i++;
	}
	break;
      case PRODUCT:
	if (ops == CONST) {
	  if ((*a)->GetNode(i)->GetValue() == 1) {
	    (*a)->DeleteNode(i);
	    ret = true;
	    sz--;
	    if (sz == 1) {
	      a->SetTo((*a)->GetNode(0));
	      i = 0;
	      sz = (*a)->GetSize();
	    }
	  } else if ((*a)->GetNode(i)->GetValue() == 0) {
	    RecursiveDestroy(a);
	    ret = true;
	    a->SetTo(zero);
	    sz = 0;
	  } else {
	    i++;
	  }
	} else {
	  i++;
	}
	break;
      case FRACTION:
	if (ops == CONST && i == 1) {
	  a->SetTo((*a)->GetNode(0));
	  ret = true;
	  sz--;
	} else {
	  i++;
	}
	if (sz >= 2 && (*a)->GetNode(0)->IsConstant() && 
	    (*a)->GetNode(1)->IsConstant()) {
	  double d = (*a)->GetNode(1)->GetValue();
	  if (d == 0) {
	    throw ErrDivideByZero(23, "Expression", "SimplifyConstant", 
				  "d==0", 
				  "cannot divide by zero", HELPURL, 
				  (*a)->ToString());
	  } 
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue((*a)->GetValue() / d);
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	}
	break;
      case POWER:
	if (sz >= 2 && (*a)->GetNode(0)->IsConstant() && 
	    (*a)->GetNode(1)->IsConstant()) {
	  double d = (*a)->GetNode(1)->GetValue();
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(pow((*a)->GetValue(), d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case LOG:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  if (d <= 0) {
	    throw ErrNotPermitted(24, "Expression", "SimplifyConstant", 
				  "d<=0", 
				  "log of nonpositive not allowed", HELPURL);
	  }
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(log(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case EXP:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(exp(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case SIN:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(sin(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case COS:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(cos(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case TAN:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(tan(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case COT:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(1/tan(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case SINH:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(sinh(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case COSH:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(cosh(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case TANH:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(tanh(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case COTH:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(1/tanh(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      case SQRT:
	if ((*a)->GetNode(0)->IsConstant()) {
	  double d = (*a)->GetNode(0)->GetValue();
	  if (d <= 0) {
	    throw ErrNotPermitted(25, "Expression", "SimplifyConstant", 
				  "d<=0", 
				  "sqrt of nonpositive not allowed", HELPURL);
	  }
	  ret = true;
	  a->SetTo((*a)->GetNode(0));
	  (*a)->SetValue(sqrt(d));
	  (*a)->SetCoeff(1);
	  (*a)->SetExponent(1);
	  sz = 0;
	} else {
	  i++;
	}
	break;
      default:
	i++;
	break;
      }
    }
  }
  return ret;
}

bool SimplifyRecursive(Expression* a) {
  bool ret = false;
  bool ischanged = false;
  // signals whether we're dealing with 
  // -1 : nothing
  // 0 : constants
  // 1 : linear variables
  // 2 : a variable raised to an exponent different from 1
  // 3 : any other more complex node
  if (!(*a)->IsLeaf()) {
    int op = (*a)->GetOpType();
    Expression t1, t2;
    int i, j;
    for(i = 0; i < (*a)->GetSize(); i++) {
      ischanged = SimplifyRecursive((*a)->GetNodePtr(i));
      if (!ret && ischanged)
	ret = true;
    }
    int status = -1;
    int prestatus = -1;
    double consolidated[4] = {0, 0, 0, 0};
    double expon = 0;
    double preexpon = 0;
    double c = 0;
    int prevarindex = -1;
    int varindex = -1;
    int firstvarindex = -1;
    int firstconstindex = -1;
    int sz = (*a)->GetSize();
    Expression one(1.0);
    switch(op) {
    case SUM:
      i = 0;
      while(i < sz) {
	// work out which status we're in
	if ((*a)->GetNode(i)->IsConstant()) {
	  if (status == -1 || firstconstindex == -1) {
	    firstconstindex = i;
	  }
	  // constant
	  status = 0;
	} else if ((*a)->GetNode(i)->IsVariable() && 
		   (*a)->GetNode(i)->GetExponent() == 1) {
	  // variable
	  status = 1;
	} else if ((*a)->GetNode(i)->IsVariable() && 
		   (*a)->GetNode(i)->GetExponent() != 1) {
	  // variable raised to power
	  status = 2;
	} else {
	  // other term
	  status = 3;
	}
	if (status == 0) {
	  // constant
	  consolidated[status] += (*a)->GetNode(i)->GetValue();
	  (*a)->GetNode(firstconstindex)->SetValue(consolidated[status]);
	  (*a)->GetNode(firstconstindex)->SetCoeff(1);
	  (*a)->GetNode(firstconstindex)->SetExponent(1);
	  if (prestatus == 0) {
	    (*a)->DeleteNode(i);
	    ret = true;
	    sz--;
	    if (sz == 1) {
	      a->SetTo((*a)->GetNode(0));
	      i = 0;
	      sz = (*a)->GetSize();
	    }
	  } else {
	    i++;
	  }
	} else if (status == 1) {
	  // variable
	  varindex = (*a)->GetNode(i)->GetVarIndex();
	  c = (*a)->GetNode(i)->GetCoeff();
	  if (varindex != prevarindex) {
	    firstvarindex = i;
	    consolidated[status] = c;
	    i++;
	  } else {
	    consolidated[status] += c;
	    (*a)->GetNode(firstvarindex)->SetCoeff(consolidated[status]);
	    ret = true;
	    (*a)->DeleteNode(i);
	    sz--;
	    if (sz == 1) {
	      a->SetTo((*a)->GetNode(0));
	      i = 0;
	      sz = (*a)->GetSize();
	    }
	  }
	  prevarindex = varindex;
	} else if (status == 2) {
	  // variable raised to power
	  varindex = (*a)->GetNode(i)->GetVarIndex();
	  expon = (*a)->GetNode(i)->GetExponent();
	  c = (*a)->GetNode(i)->GetCoeff();
	  if (expon != preexpon || varindex != prevarindex) {
	    firstvarindex = i;
	    consolidated[status] = c;
	    i++;
	  } else {
	    consolidated[status] += c;
	    (*a)->GetNode(firstvarindex)->SetCoeff(consolidated[status]);
	    ret = true;
	    (*a)->DeleteNode(i);
	    sz--;
	    if (sz == 1) {
	      a->SetTo((*a)->GetNode(0));
	      i = 0;
	      sz = (*a)->GetSize();
	    }
	  }
	  preexpon = expon;
	  prevarindex = varindex;
	} else if (status == 3) {
	  // other term
	  c = (*a)->GetNode(i)->GetCoeff();
	  firstvarindex = i;
	  consolidated[status] = c;
	  j = i + 1;
	  while(j < sz) {
	    if ((*a)->GetNode(i)->IsEqualTo((*a)->GetNode(j))) {
	      consolidated[status] += c;
	      ret = true;
	      (*a)->GetNode(firstvarindex)->SetCoeff(consolidated[status]);
	      RecursiveDestroy((*a)->GetNodePtr(j));
	      sz--;
	      if (sz == 1) {
		a->SetTo((*a)->GetNode(0));
		j = 0;
		sz = (*a)->GetSize();
	      }
	    } else {
	      j++;
	    }
	  }
	  i++;
	} else {
	  // should never happen, but anyway...
	  i++;
	}
	// update status of last iteration
	prestatus = status;
      }
      break;
    case PRODUCT:
      i = 0;
      prevarindex = -1;
      consolidated[0] = 1;
      expon = 0;
      while(i < sz) {
	if ((*a)->GetNode(i)->IsVariable()) {
	  varindex = (*a)->GetNode(i)->GetVarIndex();
	  if (varindex != prevarindex) {
	    firstvarindex = i;
	    consolidated[0] = (*a)->GetNode(i)->GetCoeff();
	    expon = (*a)->GetNode(i)->GetExponent();
	    i++;
	  } else {
	    consolidated[0] *= (*a)->GetNode(i)->GetCoeff();
	    expon += (*a)->GetNode(i)->GetExponent();
	    (*a)->GetNode(firstvarindex)->SetCoeff(consolidated[0]);
	    (*a)->GetNode(firstvarindex)->SetExponent(expon);
	    (*a)->DeleteNode(i);
	    ret = true;
	    sz--;
	    if (sz == 1) {
	      a->SetTo((*a)->GetNode(0));
	      i = 0;
	      sz = (*a)->GetSize();
	    }
	  }
	} else if (!(*a)->GetNode(i)->IsLeaf()) {
	  // WARNING: work to be done
	  // not going to do the same as in sum just yet, maybe future
	  // work - transform expr * expr in expr^2 when expr not a variable
	  i++;
	}
      }
      break;
    case FRACTION:
      if ((*a)->GetNode(0)->IsEqualTo((*a)->GetNode(1))) {
	// f(x)/f(x)
	RecursiveDestroy(a);
	a->SetTo(one);
	ret = true;
	sz = 0;
      } else {
	// try to simplify denominator by one of numerator factors
	if ((*a)->GetNode(0)->GetOpType() == PRODUCT) {
	  for(j = 0; j < (*a)->GetNode(0)->GetSize(); j++) {
	    if ((*a)->GetNode(1)->IsEqualTo((*a)->GetNode(0)->GetNode(j))) {
	      a->SetTo((*a)->GetNode(0));
	      (*a)->DeleteNode(j);	      
	      ret = true;
	      sz = 0;
	      break;
	    } 
	  }
	}
	// do the opposite
	if (sz > 0 && (*a)->GetNode(1)->GetOpType() == PRODUCT) {
	  for(j = 0; j < (*a)->GetNode(1)->GetSize(); j++) {
	    if ((*a)->GetNode(0)->IsEqualTo((*a)->GetNode(1)->GetNode(j))) {
	      (*a)->GetNode(0).SetTo(one);
	      (*a)->GetNode(1)->DeleteNode(j);
	      ret = true;
	      sz = 0;
	      break;
	    } 
	  }
	} 
	if (sz > 0 && (*a)->GetNode(0)->GetOpType() == PRODUCT &&
	    (*a)->GetNode(1)->GetOpType() == PRODUCT) {
	  // both num and denom. are products, try and find common factors
	  int k = 0;
	  int sz1, sz2;
	  j = 0;
	  sz1 = (*a)->GetNode(0)->GetSize();
	  sz2 = (*a)->GetNode(1)->GetSize();	
	  while (j < sz1) {
	    k = 0;
	    while (k < sz2) {
	      if ((*a)->GetNode(0)->GetNode(j)->IsEqualTo
		  ((*a)->GetNode(1)->GetNode(k))) {
		(*a)->GetNode(0)->DeleteNode(j);
		(*a)->GetNode(1)->DeleteNode(k);
		ret = true;
		sz1--;
		if (sz1 == 0) {
		  // numerator empty, replace with 1
		  (*a)->GetNode(0)->One();
		}
		sz2--;
		if (sz2 == 0) {
		  // denominator empty, node becomes num.
		  a->SetTo((*a)->GetNode(0));
		}
		if (sz1 == 0 && sz2 == 0) {
		  // 1/1, simplify
		  (*a)->One();
		}
		if (sz1 == 0 || sz2 == 0) {
		  // either num. or den. 1, exit loop
		  sz1 = 0;
		  sz2 = 0;
		  break;
		}
		j--;
	      } else {
		k++;
	      }
	    }
	    j++;
	  }
	}
      }
      sz = 0;
      break;
    case POWER:
      if (sz == 2 && 
	  (*a)->GetNode(0)->IsVariable() &&
	  (*a)->GetNode(1)->IsConstant()) {
	// case var^const, transform in variable with an exponent
	(*a)->GetNode(0)->SetCoeff((*a)->GetNode(1)->GetValue());
	(*a)->DeleteNode(1);
	a->SetTo((*a)->GetNode(0));
      }
      break;
    default:
      break;
    }
  }
  return ret;
}

bool DifferenceToSum(Expression* a) {
  bool ret = false;
  double d, e;
  if (!(*a)->IsLeaf()) {
    if (((*a)->GetOpType() == SUM || (*a)->GetOpType() == DIFFERENCE) && 
	(*a)->GetSize() == 1) {
      DifferenceToSum((*a)->GetNodePtr(0));
      // replace a with its child
      a->SetTo((*a)->GetNode(0));
      ret = true;
    }
    if ((*a)->GetOpType() == DIFFERENCE) {
      int i;
      (*a)->SetOpType(SUM);
      for(i = 1; i < (*a)->GetSize(); i++) { 
	// start from node 1 not 0 because a difference is +op0 -op1 -op2 ...
	(*a)->GetNode(i)->SetCoeff(- (*a)->GetNode(i)->GetCoeff());
      }
    } else if ((*a)->GetOpType() == MINUS) {
      d = (*a)->GetCoeff();
      e = (*a)->GetExponent();
      if (is_even(e)) {
	// replace a with its child and adjust coeff
	a->SetTo((*a)->GetNode(0));
	(*a)->SetCoeff((*a)->GetCoeff() * d); // since exponent is even, +
	(*a)->SetExponent((*a)->GetExponent() * e);
	ret = true;
      } else if (is_odd(e)) {
	// replace a with its child and adjust coeff
	a->SetTo((*a)->GetNode(0));
	(*a)->SetCoeff(- (*a)->GetCoeff() * d); // since exponent is odd, -
	(*a)->SetExponent((*a)->GetExponent() * e);
	ret = true;
      }
    } else if ((*a)->GetOpType() == PLUS) {
      // replace a with its child
      a->SetTo((*a)->GetNode(0));      
      (*a)->SetCoeff((*a)->GetCoeff() * d); // since exponent is even, +
      (*a)->SetExponent((*a)->GetExponent() * e);
      ret = true;
    }
  } 
  return ret;
}    

// standard order for a set of subnodes of a sum is:
// constants + linear variables + vars^{rising powers} + complex operands
class NodeOrderSum {
public:
  int operator() (const Expression& a, const Expression& b) {
    if (a->IsConstant() && !b->IsConstant()) {
      return true;
    } else if (a->IsVariable() && b->IsVariable()) {
      if (a->GetExponent() == 1 && b->GetExponent() != 1) {
	return true;
      } else if (a->GetExponent() < b->GetExponent()) {
	return true;
      } else if (a->GetExponent() > b->GetExponent()) {
	return false;
      } else {
	if (a->GetVarIndex() < b->GetVarIndex()) {
	  return true;
	} else {
	  return false;
	}
      }
    } else if (a->IsLeaf() && !b->IsLeaf()) {
      return true;
    } else {
      return false;
    }
  }
};

// standard order for a set of subnodes is:
// constants + vars^{rising powers} + complex operands
class NodeOrder {
public:
  int operator() (const Expression& a, const Expression& b) {
    if (a->IsConstant() && !b->IsConstant()) {
      return true;
    } else if (a->IsVariable() && b->IsVariable()) {
      if (a->GetExponent() < b->GetExponent()) {
	return true;
      } else if (a->GetExponent() > b->GetExponent()) {
	return false;
      } else {
	if (a->GetVarIndex() < b->GetVarIndex()) {
	  return true;
	} else {
	  return false;
	}
      }
    } else if (a->IsLeaf() && !b->IsLeaf()) {
      return true;
    } else {
      return false;
    }
  }
};

bool ReorderNodes(Expression* a) {
  bool ret = false;
  if (!(*a)->IsLeaf() && (*a)->GetSize() > 1 &&
      ((*a)->GetOpType() == SUM || (*a)->GetOpType() == PRODUCT)) {
    int i;
    for(i = 0; i < (*a)->GetSize(); i++) {
      ReorderNodes((*a)->GetNodePtr(i));
    }
    // how do I get this sort to tell me whether it did anything or not?
    // at the moment this function returns false by definition, incorrect
    if ((*a)->GetOpType() == SUM) {
      sort(((*a)->GetNodeVectorPtr())->begin(), 
	   ((*a)->GetNodeVectorPtr())->end(), NodeOrderSum());
    } else {
      sort(((*a)->GetNodeVectorPtr())->begin(), 
	   ((*a)->GetNodeVectorPtr())->end(), NodeOrder());
    }
  }
  return ret;
}

bool CompactLinearPart(Expression* a) {
  bool ret = false;
  bool ischanged = false;
  ischanged = SimplifyConstant(a);
  if (!ret && ischanged)
    ret = true;
  ischanged = DifferenceToSum(a);
  if (!ret && ischanged)
    ret = true;
  ischanged = CompactLinearPartRecursive(a);
  if (!ret && ischanged) 
    ret = true;
  ischanged = ReorderNodes(a);
  return ret;
}

bool CompactLinearPartRecursive(Expression* a) {
  bool ret = false;
  bool ischanged = false;
  int i, j;
  i = 0;
  int sz = (*a)->GetSize();
  if ((*a)->GetOpType() == SUM) {
    while(i < sz) {
      ischanged = CompactLinearPartRecursive((*a)->GetNodePtr(i));
      if (!ret && ischanged)
	ret = true;
      if ((*a)->GetNode(i)->GetOpType() == SUM) {
	ret = true;
	for(j = 0; j < (*a)->GetNode(i)->GetSize(); j++) {
	  (*a)->AddNode((*a)->GetNode(i)->GetNode(j));
	}	
	(*a)->DeleteNode(i);
	sz--; // we have deleted a node
	// we don't need to increase i, since we have deleted the i-th node
	// the next node is still the i-th node
	if (sz == 1) {
	  a->SetTo((*a)->GetNode(0));
	  i = 0;
	  sz = (*a)->GetSize();
	}
      } else {
	i++; 
      }
    }
  }
  return ret;
}


// deals with cases like *(*(x,y), z) --> *(x,y,z)
bool CompactProducts(Expression* a) {
  bool ret = false;
  bool ischanged = false;
  int i, j;
  if ((*a)->GetOpType() == PRODUCT) {
    for(i = 0; i < (*a)->GetSize(); i++) {
      ischanged = CompactProducts((*a)->GetNodePtr(i));
      if (!ret && ischanged)
	ret = true;
      if ((*a)->GetNode(i)->GetOpType() == PRODUCT) {
	ret = true;
	for(j = 0; j < (*a)->GetNode(i)->GetSize(); j++) {
	  (*a)->AddNode((*a)->GetNode(i)->GetNode(j));
	}
	(*a)->DeleteNode(i);
      }
    }
    if ((*a)->GetSize() == 1) {
      // product with just one operand, up a level
      a->SetTo((*a)->GetNode(0));
      ret = true;
    }
  }
  (*a)->ConsolidateProductCoeffs();
  return ret;
}

// generic simplification on a copy of the expression
Expression SimplifyCopy(Expression* a, bool& ischanged) {
  Expression b;
  b = (*a).Copy();
  ischanged = Simplify(&b);
  return b;
}

// recursive destroy - deleted all the tree and nodes in a tree - use
// with caution
void RecursiveDestroy(Expression* a) {
  int i;
  for(i = 0; i < (*a)->GetSize(); i++) {
    RecursiveDestroy((*a)->GetNodePtr(i));
  }
  a->Destroy();
}
