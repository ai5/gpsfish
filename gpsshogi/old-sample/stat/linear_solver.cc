/* linear_solver.cc
 */
#include "gpsshogi/stat/weightRecorder.h"
#include "osl/stat/iterativeLinearSolver.h"
#include "osl/stat/sparseRegressionMultiplier.h"
#include "osl/stat/twoDimensionalStatistics.h"
#include "osl/stat/diagonalPreconditioner.h"
#include <boost/program_options.hpp>
#include <valarray>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace po = boost::program_options;

// compute w s.t. x'x w = x'y  (least squares)
// x and y are given from stdin.
// program reads instances repeatedly, and
// an instance is a line of y_i x_i.
// x_i consists of (index value) pairs of its non-zero elements.

// e.g.
// x         w     y
// 8 3 4     1     26
// 1 5 9  *  2  =  38
// 6 7 2     3     26
// 0 1 1            5

/*
% ./linear_solver -f 3 -e 4
# repeatedly type the following lines
26  0 8  1 3  2 4
38  0 1  1 5  2 9
26  0 6  1 7  2 2
 5  1 1  2 1
% cat w.txt
1 2 3
*/

void solve(size_t num_features, size_t num_instances, size_t skip_head,
	   double lambda, size_t max_loop,
	   const char *output_filename, const char *tmp_filename);
bool binary_mode = false;

int main(int argc, char *argv[])
{
  size_t num_features = 0;
  size_t num_elements = 0;
  size_t skip_head = 0;
  std::string output_filename;
  std::string tmp_filename;
  double lambda = 0.0;
  size_t loop;

  po::options_description options;
  options.add_options()
    ("num-features,f", po::value<size_t>(&num_features)->default_value(0),
     "number of features")
    ("num-elements,e", po::value<size_t>(&num_elements)->default_value(0),
     "number of elements")
    ("skip-head,s", po::value<size_t>(&skip_head)->default_value(0),
     "number of elements separated for cross validation")
    ("lambda,L", po::value<double>(&lambda)->default_value(0.0),
     "regularization term")
    ("loop,l", po::value<size_t>(&loop)->default_value(10),
     "maximum number of iterations")
    ("output-filename,o", 
     po::value<std::string>(&output_filename)->default_value("w.txt"),
     "filename for weights")
    ("tmp-filename,t", 
     po::value<std::string>(&tmp_filename)->default_value("tmp-w.txt"),
     "filename for interim weights")
    ("binary,b", "binary input from stdin")
    ("help,h", "produce this message");
    ;
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }
  if (vm.count("help") || num_features == 0 || num_elements == 0) {
    std::cerr << options << std::endl;
    return 0;
  }
  binary_mode = vm.count("binary");
  solve(num_features, num_elements, skip_head, lambda, loop,
	output_filename.c_str(), tmp_filename.c_str());
}

using gpsshogi::stat::WeightRecorder;
class StreamMultiplier : public osl::stat::SparseRegressionMultiplier
{
  size_t m_num_instances;
  size_t m_skip_head;
  mutable size_t iteration;
  const double *weights;
  WeightRecorder recorder;
  mutable double next_y;		// state dependent
  mutable size_t cur_instance;	// state dependent
  mutable double initial_error;
public:
  StreamMultiplier(size_t num_features, size_t num_instances,
		   size_t skip_head, double lambda,
		   const double *w, const char *tmp_out)
    : SparseRegressionMultiplier(num_features, lambda),
      m_num_instances(num_instances), m_skip_head(skip_head),
      iteration(0), weights(w), recorder(tmp_out),
      next_y(0.0), cur_instance(0), initial_error(-1.0)
  {
    assert(m_skip_head*2 <= m_num_instances);
    assert(num_features <= m_num_instances);
  }
  ~StreamMultiplier();

  bool getVectorX(unsigned int& num_elements,
		  unsigned int *non_zero_indices, 
		  double *non_zero_values) const;
  void newIteration() const;
  void computeXtY(double *xty, double *diag_inv);

  static double dotProduct(const unsigned int a_non_zeros, 
			   const unsigned int *a_indices, 
			   const double *a_values,
			   const double *b)
  {
    double result = 0.0;
    for (size_t i=0; i<a_non_zeros; ++i)
    {
      result += a_values[i]*b[a_indices[i]];
    }
    return result;
  }
};

StreamMultiplier::~StreamMultiplier()
{
}

void StreamMultiplier::computeXtY(double *xty, double *diag_inv)
{
  // SparseRegressionMultiplier::computeXtY で y.read() が getVectorX 
  // の後に呼ばれることに依存
  assert(cur_instance == 0);
  if (m_skip_head) {
    boost::scoped_array<unsigned int> indices_dummy(new unsigned int[dim()]);
    boost::scoped_array<double> values_dummy(new double[dim()]);
    unsigned int num_elements_dummy;
    while (cur_instance < m_skip_head) {
#ifndef NDEBUG
      const bool go_next = 
#endif
	getVectorX(num_elements_dummy, &indices_dummy[0], &values_dummy[0]);
      assert(go_next);
    }
  }
    
  osl::stat::DoubleReferenceReader y(next_y);
  SparseRegressionMultiplier::computeXtY(y, xty, diag_inv);
}

int readInt(std::istream& is) 
{
  const size_t buf_size = 1024 * 2; // PIPE_SIZE/2 if portable
  static boost::scoped_array<char> buf(new char[buf_size]);
  static size_t cur = buf_size;
  if (cur == buf_size) {
    is.read(buf.get(), buf_size);
    cur = 0;
  }

  int32_t value = *reinterpret_cast<int32_t*>(&buf[cur]);
  cur += sizeof(int32_t);
  return value;
}

bool StreamMultiplier::getVectorX(unsigned int& num_elements,
				  unsigned int *non_zero_indices, 
				  double *non_zero_values) const
{
  // file からbinary で読む方法もほしい。
  num_elements = 0;
  if (binary_mode) {
    next_y = readInt(std::cin);
    num_elements = readInt(std::cin);
    for (size_t i=0; i<num_elements; ++i) {
      non_zero_indices[i] = readInt(std::cin);
      non_zero_values[i] = readInt(std::cin);
    }
  }
  else {
    std::string line;
    if (! std::getline(std::cin, line))
      throw std::runtime_error("read_error");
    std::istringstream is(line);
    is >> next_y;
  
    int index;
    int value;
    while (is >> index >> value) {
      non_zero_indices[num_elements] = index;
      non_zero_values[num_elements] = value;
      ++num_elements;
    }
  }
  ++cur_instance;
  if (cur_instance >= m_num_instances)
    cur_instance = 0;
  return cur_instance;
}

void StreamMultiplier::newIteration() const
{
  assert(cur_instance == 0);
  recorder.write(iteration++, dim(), weights);

  if (m_skip_head == 0)
    return;

  osl::stat::TwoDimensionalStatistics stat;
  boost::scoped_array<unsigned int> indices(new unsigned int[dim()]);
  boost::scoped_array<double> values(new double[dim()]);

  while (cur_instance < m_skip_head)
  {
    unsigned int non_zeros;
#ifndef NDEBUG
    const bool go_next = 
#endif
      getVectorX(non_zeros, &indices[0], &values[0]);
    assert(go_next);
    const double prediction 
      = dotProduct(non_zeros, &indices[0], &values[0], weights);
    stat.addInstance(prediction, next_y);
  }

  const double mse = stat.meanSquaredErrorsAdjustConstant();
  std::cerr << "At " << iteration << " iteration\n";
  std::cerr << "Cross Validation: " << sqrt(mse) << "\n" << std::flush;
  if (iteration == 1)
    initial_error = mse;
  else if (mse > initial_error) {
    throw std::runtime_error("convergence failed");
  }
}

void solve(size_t num_features, size_t num_instances, size_t skip_head,
	   double lambda, size_t max_loop,
	   const char *output_filename, const char *tmp_filename)
{
  const double eps = 0.001;
  std::valarray<double> result(0.0, num_features);
  int iter;
  double tol;

  StreamMultiplier prod_A(num_features, num_instances, skip_head, lambda,
			  &result[0], tmp_filename);
  std::valarray<double> b(num_features);
  std::valarray<double> diag_inv(num_features);
  std::cerr << "computing x^t y\n";
  prod_A.computeXtY(&b[0], &diag_inv[0]);
  osl::stat::DiagonalPreconditioner preconditioner(num_features);
  preconditioner.setInverseDiagonals(&diag_inv[0]);
  std::cerr << "preconditioner\n";

  osl::stat::IterativeLinearSolver solver(prod_A, &preconditioner, max_loop, eps);
  std::cerr << "solver started ";
  int err = 0;

  std::cerr << "using cg\n";
  try {
    err = solver.solve_by_CG(b, result, &iter, &tol);
  }
  catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    err = 1;
  }
  if (err) {
    std::cerr << "solver failed " << err << std::endl;
    return;
  }

  WeightRecorder::write(output_filename, num_features, &result[0]);  
  std::cerr << "success" << std::endl;
  if (skip_head)
    prod_A.newIteration();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
