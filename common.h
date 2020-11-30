//                                               -*- C++ -*-
/**
 *  @brief common stuff
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

#ifndef __EV3COMMONH__
#define __EV3COMMONH__

#define RCS12 "$Id: common.h,v 1.2 2006/07/30 05:36:44 liberti Exp liberti $"

#define NOVARIABLE -1
#define OPERANDSTRBUF 64
#define EXPRESSIONSTRBUF 2048
#define LARGE 1E10

#include <cassert>

typedef int Int;

// various operator types
enum OperatorType {  
  SUM, DIFFERENCE, PRODUCT, FRACTION, POWER,
  PLUS, MINUS, LOG, EXP, SIN, 
  COS, TAN, COT, SINH, COSH, 
  TANH, COTH, SQRT, VAR, CONST,
  ERROR
};

// utility functions
extern double Ev3NearZero(void);
extern double Ev3Infinity(void);

#endif
