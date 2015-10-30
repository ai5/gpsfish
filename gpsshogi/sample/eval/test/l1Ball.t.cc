#include "l1Ball.h"
#include "loss.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

extern bool long_test;
using namespace gpsshogi;

/* ------------------------------------------------------------------------- */
static int num_f = 10, num_e = 10;
static double mean_abs_value = 0.5;
typedef std::valarray<double> valarray_t;
static std::vector<valarray_t> x;
static valarray_t y, w;

class L1BallLogreg : public L1Ball
{
public:
    void makeGradient(const valarray_t& w, valarray_t& gradient, double& error);
    void iterationHead(int i, const valarray_t& w, double prev_error);
};

void L1BallLogreg::makeGradient(const valarray_t& w, valarray_t& gradient, double& error)
{
    gradient.resize(w.size());
    gradient = 0.0;
    error = 0.0;

    for (int i=0; i<num_e; ++i)
	error += LogLoss::addGradient(w, x[i], y[i], gradient);
    error /= num_e;
}

void L1BallLogreg::iterationHead(int i, const valarray_t& w, double prev_error)
{
    if (long_test)
	std::cerr << "iter " << i << " " << prev_error << "\n";
}
/* ------------------------------------------------------------------------- */

class L1BallSMDLogreg : public L1BallSMD
{
public:
    void makeGradient(const valarray_t& w, valarray_t& gradient, double& error,
		      const valarray_t& v, valarray_t& Hv);
    void iterationHead(int i, const valarray_t& w, double prev_error);
};

void L1BallSMDLogreg::makeGradient(const valarray_t& w, valarray_t& gradient, double& error,
				   const valarray_t& v, valarray_t& Hv)
{
    gradient.resize(w.size());
    gradient = 0.0;
    Hv.resize(w.size());
    Hv = 0.0;
    error = 0.0;

    for (int i=0; i<num_e; ++i)
	error += LogLoss::addGradient(w, x[i], y[i], gradient, v, Hv);
    error /= num_e;
}

void L1BallSMDLogreg::iterationHead(int i, const valarray_t& w, double prev_error)
{
    if (long_test)
	std::cerr << "iter " << i << " " << prev_error << "\n";
}

/* ------------------------------------------------------------------------- */

BOOST_AUTO_TEST_CASE(L1BallTestConstruct)
{
    L1BallLogreg solver;
}

BOOST_AUTO_TEST_CASE(L1BallTestSolve)
{
    L1BallLogreg solver;
    x = std::vector<valarray_t>(num_e, valarray_t(num_f));
    y.resize(num_e);
    w.resize(num_f); w = 0.0;

    std::ifstream X("x.txt"), Y("y.txt");
    for (int i=0; i<num_e; ++i) {
	for (int j=0; j<num_f; ++j) {
	    X >> x[i][j];
	}
	Y >> y[i];
    }

    solver.setMeanAbsWeight(mean_abs_value);
    solver.setEta0(0.1);
    valarray_t w(num_f);
    w = 0.0;
    // solver.setFixWidth(true);
    solver.solve(w);
    BOOST_CHECK(solver.lastIteration() > 0);
    BOOST_CHECK(solver.lastError() < 0.1);
}

BOOST_AUTO_TEST_CASE(L1BallTestSolveSMD)
{
    L1BallSMDLogreg solver;
    x = std::vector<valarray_t>(num_e, valarray_t(num_f));
    y.resize(num_e);
    w.resize(num_f); w = 0.0;

    std::ifstream X("x.txt"), Y("y.txt");
    for (int i=0; i<num_e; ++i) {
	for (int j=0; j<num_f; ++j) {
	    X >> x[i][j];
	}
	Y >> y[i];
    }

    solver.setMeanAbsWeight(mean_abs_value);
    solver.setEta0(0.1);
    valarray_t w(num_f);
    w = 0.0;
    solver.solve(w);
    BOOST_CHECK(solver.lastIteration() > 0);
    BOOST_CHECK(solver.lastError() < 0.1);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
