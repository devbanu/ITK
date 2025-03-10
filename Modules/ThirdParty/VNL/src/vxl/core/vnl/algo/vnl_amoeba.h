// This is core/vnl/algo/vnl_amoeba.h
#ifndef vnl_amoeba_h_
#define vnl_amoeba_h_
//:
// \file
// \brief Nelder-Meade downhill simplex.
// \author Andrew W. Fitzgibbon, Oxford RRG
// \date   23 Oct 97
//
// \verbatim
//  Modifications
//   971023 AWF Initial version
//   dac (Manchester) 26/03/2001: tidied up documentation
//   Tim Cootes 7-Jan-02: Added documentation and additional methods
//   Feb.2002 - Peter Vanroose - brief doxygen comment placed on single line
// \endverbatim

//-----------------------------------------------------------------------------

#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_algo_export.h>

class vnl_cost_function;
class vnl_least_squares_function;

//: Nelder-Meade downhill simplex.
//  vnl_amoeba is an implementation of the Nelder-Meade downhill simplex
//  algorithm.  For most problems, it's a few times slower than
//  vnl_levenberg_marquardt, but it can perform much better on noisy error
//  functions.
//
//  It works by creating a simplex (n+1 points in n-D space) which then
//  crawls about the space searching for the solution.
//
//  By default the set of (n+1) starting points are generated by applying
//  a scaling (relative_diameter) to each element of the supplied starting
//  vector,  with a small offset used instead if the value is zero.
//
//  Alternatively, if one uses minimize(x,dx), then the starting points
//  are obtained by adding each dx[i] to the elements of x, one at a time.
//  This is useful if you know roughly the scale of your space.

class VNL_ALGO_EXPORT vnl_amoeba
{
public:
  int verbose;
  int maxiter;
  double X_tolerance;
  double F_tolerance;

  //: Define maximum number of iterations to use
  void
  set_max_iterations(int n)
  {
    maxiter = n;
  }

  //: Define tolerance on elements of x
  void
  set_x_tolerance(double tol)
  {
    X_tolerance = tol;
  }

  //: Define tolerance on function evaluation
  void
  set_f_tolerance(double tol)
  {
    F_tolerance = tol;
  }

  //: Define scaling used to select starting vertices relative to initial x0.
  //  I.e. the i'th vertex has x[i] = x0[i]*(1+relative_diameter)
  void
  set_relative_diameter(double r)
  {
    relative_diameter = r;
  }

  void
  set_zero_term_delta(double d)
  {
    zero_term_delta = d;
  }
  //: Scaling used to select starting vertices relative to initial x0.
  //  I.e. the i'th vertex has x[i] = x0[i]*(1+relative_diameter)
  double relative_diameter;
  double zero_term_delta;
  //: Construct and supply function to be minimized
  vnl_amoeba(vnl_cost_function & f);

  //: Modify x to minimise function supplied in constructor
  //  Start simplex defined by scaling elements of x
  void
  minimize(vnl_vector<double> & x);

  //: Perform optimisation.
  //  Start simplex defined by adding dx[i] to each x[i]
  void
  minimize(vnl_vector<double> & x, const vnl_vector<double> & dx);

  double
  get_end_error() const
  {
    return end_error_;
  }

  //: Number of evaluations used in last call to minimize
  int
  get_num_evaluations() const
  {
    return num_evaluations_;
  }

public:
  //: Modify x so as to minimise f(x)
  static void
  minimize(vnl_cost_function & f, vnl_vector<double> & x);

  //: Modify x so as to minimise f(x)
  //  Start simplex defined by adding dx[i] to each x[i]
  static void
  minimize(vnl_cost_function & f, vnl_vector<double> & x, const vnl_vector<double> & dx);

  //: Modify x so as to minimise f(x)
  //  delta defines relative size of initial simplex
  //  ie the i'th vertex has xi[i] = x[i]*(1+delta)
  static void
  minimize(vnl_cost_function & f, vnl_vector<double> & x, double delta);

  //: Modify x so as to minimise f(x)
  static void
  minimize(vnl_least_squares_function & f, vnl_vector<double> & x);

  static bool default_verbose;

protected:
  vnl_cost_function * fptr;
  double end_error_;
  int num_evaluations_{ 0 };
};

// Private struct needs to be declared in the header file
// in order to instantiate STL container of it elsewhere.
struct vnl_amoeba_SimplexCorner
{
  vnl_vector<double> v;
  double fv;

  vnl_amoeba_SimplexCorner(int = 0);
  vnl_amoeba_SimplexCorner &
  operator=(const vnl_amoeba_SimplexCorner & that);
  static int
  compare(const vnl_amoeba_SimplexCorner & s1, const vnl_amoeba_SimplexCorner & s2);
};

#endif // vnl_amoeba_h_
