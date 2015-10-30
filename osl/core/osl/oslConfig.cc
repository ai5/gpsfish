/* oslConfig.cc
 */
#include "osl/oslConfig.h"
#include "osl/config.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include <boost/filesystem/operations.hpp>
#include <map>
#include <limits>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <thread>
#ifndef _MSC_VER
#  include <unistd.h>
#endif
#ifdef _WIN32
#  include <windows.h>
#  include <psapi.h>
#else
#include <sys/resource.h>
#ifdef __FreeBSD__
#  include <kvm.h>
#  include <sys/param.h>
#  include <sys/sysctl.h>
#  include <sys/user.h>
#  include <paths.h>
#  include <fcntl.h>
#endif
#ifdef __APPLE__
#  include <sys/types.h>
#  include <sys/sysctl.h>
#  include <mach/task.h>
#  include <mach/mach_init.h>
#endif
#endif

const int osl::OslConfig::MaxThreads; // define
unsigned int osl::OslConfig::eval_random = 0;

bool osl::OslConfig::is_verbose = false;
#ifndef OSL_NCPUS
const int osl::OslConfig::default_ncpus = std::thread::hardware_concurrency();
#else
static_assert(OSL_NCPUS <= osl::OslConfig::MaxThreads, "#threads");
const int osl::OslConfig::default_ncpus = OSL_NCPUS;
#endif
int osl::OslConfig::num_cpu = default_ncpus;
volatile osl::OslConfig::UsiMode osl::OslConfig::usi_mode = osl::OslConfig::NoUSI;
volatile bool osl::OslConfig::usi_mode_silent = false, 
  osl::OslConfig::force_root_window = false;
int osl::OslConfig::usi_output_pawn_value = 100; 
volatile int osl::OslConfig::root_window_alpha = 0; 
volatile int osl::OslConfig::root_window_beta = 0; 
volatile int osl::OslConfig::in_unit_test = 0;
int osl::OslConfig::dfpn_max_depth = 256;
bool osl::OslConfig::search_exact_value_in_one_reply = false;
bool osl::OslConfig::has_byoyomi = false;
std::mutex osl::OslConfig::lock_io;

namespace
{
  size_t system_memory_use_limit() 
  {
#ifdef _WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return statex.ullTotalPhys; // in bytes
#else
    size_t limit_by_rlimit = std::numeric_limits<size_t>::max();
    {
      rlimit rlp;
      if (getrlimit(RLIMIT_AS, &rlp) == 0
	  && rlp.rlim_cur != std::numeric_limits<rlim_t>::max()) {
	limit_by_rlimit = rlp.rlim_cur;
#ifdef __APPLE__
	limit_by_rlimit *= 1024;
#endif
	std::cerr << "rlimit " << limit_by_rlimit << "\n";
      }
    }
#ifdef __APPLE__
    {
      int mib[2];
      unsigned int usermem;
      size_t len=sizeof(usermem);
      mib[0] = CTL_HW;
      mib[1] = HW_USERMEM;
      if (sysctl(mib, 2, &usermem, &len, NULL, 0) == 0
	  && len == sizeof(usermem)) {
	std::cerr << "usermem " << usermem << std::endl;
	return std::min((size_t)usermem, limit_by_rlimit);
      }
    }
#endif
    {
      std::string name, unit;
      size_t value;
      std::ifstream is("/proc/meminfo");
      if (is >> name >> value >> unit
	  && name == "MemTotal:" && unit == "kB")
	return std::min(value * 1024, limit_by_rlimit);
    }
#if (defined __FreeBSD__)
    const long mem = sysconf(_SC_PHYS_PAGES);
    if (mem != -1) 
      return std::min(mem * getpagesize(), limit_by_rlimit);
#endif
    return std::min((rlim_t)limit_by_rlimit, std::numeric_limits<rlim_t>::max());
#endif
  }
}

size_t osl::OslConfig::memory_use_limit = system_memory_use_limit();
const size_t osl::OslConfig::memory_use_limit_system_max = 
#ifdef _WIN32
  3000000000; // 3GB
#else
  std::numeric_limits<rlim_t>::max();
#endif
double osl::OslConfig::memory_use_percent = 100.0;

void osl::OslConfig::setNumCPUs(int ncpu)
{
  if (ncpu > MaxThreads) {
    std::cerr << "ncpu " << ncpu << " > " << "MaxThreads " << MaxThreads << "\n";
    ncpu = MaxThreads;
  }
  num_cpu = ncpu;
}

int osl::OslConfig::concurrency()
{
  if (num_cpu > MaxThreads)
    std::cerr << "ncpu " << num_cpu << " > " << "MaxThreads " << MaxThreads << "\n";
  return std::min(num_cpu, MaxThreads);
}

void osl::OslConfig::setVerbose(bool v)
{
  is_verbose = v;
}

bool osl::OslConfig::verbose()
{
  return is_verbose;
}

osl::OslConfig::UsiMode osl::OslConfig::usiMode()
{
  return usi_mode;
}
void osl::OslConfig::setUsiMode(UsiMode enable)
{
  usi_mode = enable;
}
bool osl::OslConfig::usiModeInSilent()
{
  return usi_mode_silent;
}
void osl::OslConfig::setUsiSilent(bool enable)
{
  usi_mode_silent = enable;
}
bool osl::OslConfig::searchExactValueInOneReply()
{
  return search_exact_value_in_one_reply;
}
void osl::OslConfig::setSearchExactValueInOneReply(bool enable)
{
  search_exact_value_in_one_reply = enable;
}

bool osl::OslConfig::hasByoyomi()
{
  return has_byoyomi;
}

void osl::OslConfig::setHasByoyomi(bool value)
{
  has_byoyomi = value;
}

void osl::OslConfig::showOslHome(const std::string& home)
{
  std::cerr << "using " << home << " as OSL_HOME, word size "
	    << OSL_WORDSIZE << std::endl;
}

void osl::OslConfig::showOslHome()
{
  showOslHome(home());
}

bool osl::OslConfig::isGoodDir(const std::string& dir)
{
  return boost::filesystem::exists(dir)
    && boost::filesystem::is_directory(dir);
}

void osl::OslConfig::trySetDir(std::string& dir, const std::string& candidate)
{
  if (isGoodDir(candidate))
  {
    dir = candidate;
    return;
  }
  if (verbose())
    std::cerr << "skipping " << candidate << std::endl;
}

const std::string osl::OslConfig::makeHome(const std::string& first_try)
{
  std::string result;
  if (first_try != "")
    trySetDir(result, first_try);

  if (const char *env = getenv("GPSSHOGI_HOME"))
    trySetDir(result, env);
  
#if defined GPSSHOGI_HOME
  if (result.empty())
    trySetDir(result, GPSSHOGI_HOME);
#endif

  if (result.empty())
    if (const char *env = getenv("OSL_HOME"))
      trySetDir(result, env);

  if (result.empty())
    result = OSL_HOME;

  if (verbose())
    showOslHome(result);
  return result;
}

const std::string& osl::OslConfig::home(const std::string& init)
{
  static const std::string home_directory = makeHome(init);
  return home_directory;
}

const char * osl::OslConfig::home_c_str()
{
  return home().c_str();
}

const std::string osl::OslConfig::gpsusiConf()
{
  // issue:
  // - 開発者には $HOME ではなく OSL_HOME の方が使い分ける都合が良い
  // - 一方、配布版では OSL_HOME は共有ディレクトリで書き込めないかもしれない
#ifdef OSL_PUBLIC_RELEASE
  // for personal users
  if (const char *env = getenv("HOME"))
    return std::string(env) + "/gpsusi.conf";
  if (const char *env = getenv("USERPROFILE"))
    return std::string(env) + "/gpsusi.conf";
#endif
  // for developpers
  static const std::string home_directory = makeHome();
  return home_directory + "/gpsusi.conf";
}

int osl::OslConfig::resignThreshold()
{
  static const int value = getenv("OSL_RESIGN_VALUE") 
    ? atoi(getenv("OSL_RESIGN_VALUE")) : 0;
  return (value > 0) ? value : 10000;
}

const std::string osl::OslConfig::makeTest()
{
  std::string result;
  if (const char *env = getenv("OSL_TEST"))
    trySetDir(result, env);

  if (result.empty())
    result = home() + "/data";	// 今はdata内に混在

  std::cerr << "using " << result << " as OSL_TEST" << std::endl;
  return result;
}

const std::string osl::OslConfig::makeTestPublic()
{
  std::string result;
  if (const char *env = getenv("OSL_TEST_PUBLIC"))
    trySetDir(result, env);

  if (result.empty())
    result = home() + "/public-data";

  std::cerr << "using " << result << " as OSL_TEST_PUBLIC" << std::endl;
  return result;
}

const std::string osl::OslConfig::testPrivate()
{
  static const std::string test_directory = makeTest();
  return test_directory;
}

const std::string osl::OslConfig::testPublic()
{
  static const std::string test_directory = makeTestPublic();
  return test_directory;
}

namespace 
{
  struct NameHolder : std::map<std::string,std::string>
  {
    std::string directory;

    NameHolder(const std::string& d) : directory(d)
    {
      directory += "/";
    }

    iterator add(const std::string& key, const std::string& value)
    {
      return insert(std::make_pair(key, value)).first;
    }
    iterator addRelative(const std::string& key, const std::string& filename)
    {
      std::string value = directory + filename;
      return add(key, value);
    }
    iterator addRelative(const std::string& filename)
    {
      return addRelative(filename, filename);
    }
  };
}

const char * osl::OslConfig::testPrivateFile(const std::string& filename)
{
  static NameHolder table(testPrivate());
  NameHolder::iterator p=table.find(filename);
  if (p == table.end()) {
    p = table.addRelative(filename);
  }
  return p->second.c_str();
}

const char * osl::OslConfig::testPublicFile(const std::string& filename)
{
  static NameHolder table(testPublic());
  NameHolder::iterator p=table.find(filename);
  if (p == table.end()) {
    p = table.addRelative(filename);
  }
  return p->second.c_str();
}

const char * osl::OslConfig::testCsaFile(const std::string& filename)
{
  static NameHolder table(testPublic()+"/floodgate2010");
  NameHolder::iterator p=table.find(filename);
  if (p == table.end()) {
    p = table.addRelative(filename);
  }
  return p->second.c_str();
}

const char *osl::OslConfig::openingBook(const std::string& filename)
{
  static NameHolder table(home()+"/data");
  NameHolder::iterator p=table.find(filename);
  if (p == table.end()) {
    if (! filename.empty() && filename[0] == '/') {
      // absolute path
      p = table.add(filename, filename);
    }
    else {
      // relative path
      p = table.addRelative(filename, 
			    (filename == "" ? "joseki.dat" : filename));
    }
  }
  return p->second.c_str();
}


size_t osl::OslConfig::residentMemoryUse()
{
#if defined(_WIN32)
  static const DWORD process_id = GetCurrentProcessId();
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                FALSE, process_id);
  if (NULL == hProcess)
  {
    std::cerr << "Failed to get residentMemoryUse()\n";
    return 0;
  }

  size_t working_set = 0;
  PROCESS_MEMORY_COUNTERS pmc;
  if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
    working_set = pmc.WorkingSetSize; // in bytes
  }
  CloseHandle(hProcess);
  return working_set;
#else
  // see proc(5)
  // note: < 40000 cycles @macpro2
  std::ifstream is("/proc/self/statm");
  size_t total, resident;
  if (is >> total >> resident)
    return resident*getpagesize();
#ifdef __APPLE__
  mach_msg_type_number_t count = TASK_BASIC_INFO_64_COUNT;
  task_basic_info_64 ti;
  if (task_info(current_task(), TASK_BASIC_INFO_64, (task_info_t)&ti, &count)
      == KERN_SUCCESS)
    return ti.resident_size;
#endif
#ifdef __FreeBSD__
  static kvm_t *kd = kvm_open(NULL, _PATH_DEVNULL, NULL, O_RDONLY, "osl kvm_open");
  int nproc;
  kinfo_proc *pp = kvm_getprocs(kd, KERN_PROC_PID, getpid(), &nproc);
  if (pp)
    return pp->ki_rssize * getpagesize();
#endif  
#endif
  return 0;
}

static std::vector<std::function<void()>>& function_vector() 
{
  static std::vector<std::function<void()>> initialize_functions;
  return initialize_functions;
}

void osl::OslConfig::setUp()
{
  for (auto f:function_vector())
    f();
  function_vector().clear();
#ifndef DFPNSTATONE
  eval::OpenMidEndingEval::setUp();
  progress::NewProgress::setUp();
#endif
}

void osl::OslConfig::registerInitializer(std::function<void()> f) {
  function_vector().push_back(f);
}

bool osl::OslConfig::healthCheck()
{
  bool old_verbose = verbose();
  setVerbose(true);
  std::cerr << "health check\n";
  showOslHome(home());
#ifndef DFPNSTATONE
  {
    //std::string filename = eval::ml::OpenMidEndingEval::defaultFilename();
    std::string filename = "eval.bin";
    std::cerr << "loading " << filename << ' ';
    //bool success = eval::ml::OpenMidEndingEval::setUp(filename.c_str());
    bool success = true;
    std::cerr << (success ? "success" : "failed\a") << "\n";
    if (! success) {
      std::cerr << "exists?  " << boost::filesystem::exists(filename.c_str()) << "\n";
      std::cerr << "regular? " << boost::filesystem::is_regular_file(filename.c_str()) << "\n";
      return false;
    }
  }
#if 0
  {
    std::string filename = progress::ml::NewProgress::defaultFilename();
    std::cerr << "loading " << filename << ' ';
    bool success = progress::ml::NewProgress::setUp(filename.c_str());
    std::cerr << (success ? "success" : "failed\a") << "\n";
    if (! success) {
      std::cerr << "exists?  " << boost::filesystem::exists(filename.c_str()) << "\n";
      std::cerr << "regular? " << boost::filesystem::is_regular_file(filename.c_str()) << "\n";
      return false;
    }
  }
#endif
#endif
  setVerbose(old_verbose);
  return true;
}

int osl::OslConfig::dfpnMaxDepth()
{
  return dfpn_max_depth;
}
void osl::OslConfig::setDfpnMaxDepth(int new_depth)
{
  dfpn_max_depth = new_depth;
}

std::string osl::OslConfig::configuration()
{
  return
    "wordsize " +std::to_string(OSL_WORDSIZE)+""
# ifdef __GNUC__
  " gcc " __VERSION__
# endif
#ifndef OSL_USE_SSE 
    " nosse"
#endif
# ifndef NDEBUG
    " (debug)"
# endif  
    ;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
