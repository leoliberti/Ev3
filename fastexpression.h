//                                               -*- C++ -*-
/**
 *  @brief simplified non-C++ expression trees for fast eval/diff
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

#ifndef __EV3FASTEXPRESSIONH__
#define __EV3FASTEXPRESSIONH__

#define RCS13 "$Id: fastexpression.h,v 1.2 2006/07/30 05:36:45 liberti Exp liberti $"

#include "common.h"

struct fevaltree {
  int optype;
  Int varindex;
  double coeff;
  double* depcoeff;
  double exponent;
  double* depexponent;
  double value;
  double* depvalue;
  struct fevaltree* nodes;
  Int nodesize;
};
typedef struct fevaltree FastEvalTree;

// derivative of fet w.r.t. variable index varindex in (separate) tree
FastEvalTree* Diff(FastEvalTree* fet, int varindex);
// deletion (use nonrecursive version)
void DeleteFastEvalTree(FastEvalTree* fet);
void DeleteFastEvalTreeRecursive(FastEvalTree* fet);
// Fast evaluators (for repeated evaluations -- use nonrecursive version)
double FastEval(FastEvalTree* fet, double* varvalues, Int vsize);
double FastEval(FastEvalTree* fet, double* varvalues, 
		std::map<int,int>& varmap, Int vsize);  
double FastEvalRecursive(FastEvalTree* fet, double* varvalues, 
			 std::map<int,int>* varmap, Int vsize);


#endif
