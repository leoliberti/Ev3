//                                               -*- C++ -*-
/**
 *  @brief prototypes for auxiliary functions in expression.cxx 
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

bool is_integer(double);
bool is_even(double);
bool is_odd(double);
double Ev3NearZero(void);
double Ev3Infinity(void);
double argmin(double, double, double, double);
double argmax(double, double, double, double);
void bilinearprodmkrange(double, double, double, double, double*, double*);
void fractionmkrange(double, double, double, double, double*, double*);
void constpowermkrange(double, double, double, double*, double*);
void powermkrange(double, double, double, double, double*, double*);

