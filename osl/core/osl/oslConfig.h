/* oslConfig.h
 */
#ifndef OSL_OSLCONFIG_H
#define OSL_OSLCONFIG_H

#include "osl/config.h"
#include <stdexcept>
#include <string>
#include <utility>
#include <mutex>
#include <functional>
#include <cassert>
#include <vector>

namespace osl
{
  /** osl の実行環境に関する指定 */
  struct OslConfig
  {
    static const int MaxThreads=64;
    /** compile時に指定されたディレクトリを返す. 実行時の指定は環境変数が基本 */
    static const std::string& home(const std::string& initialize_if_first_invocation="");
    static const char * home_c_str();
    static const std::string gpsusiConf();

    /** テストケースのデータ */
    static const std::string testPrivate();
    static const std::string testPublic();
    static const char *testPrivateFile(const std::string& filename);
    static const char *testPublicFile(const std::string& filename);
    static const char *testCsaFile(const std::string& filename);

    /** 標準の定跡ファイルを返す
     * @param filename specify to use non-standard file
     * (absolute path, or home()/data/filename otherwise)
     */
    static const char *openingBook(const std::string& filenamme="");

    static void setVerbose(bool verbose);
    static bool verbose();

    static void showOslHome();
    static void setNumCPUs(int ncpu);
    static int concurrency();

    static int dfpnMaxDepth();
    static void setDfpnMaxDepth(int);

    enum UsiMode { NoUSI, PortableUSI, ExtendedUSI };
    static UsiMode usiMode();
    static void setUsiMode(UsiMode new_mode=PortableUSI);
    static bool usiModeInSilent();
    static void setUsiSilent(bool silent=true);

    static bool searchExactValueInOneReply();
    static void setSearchExactValueInOneReply(bool new_value);

    static size_t residentMemoryUse();
    static size_t memoryUseLimit()
    {
      return static_cast<size_t>(memory_use_limit * memory_use_percent / 100.0); 
    }
    static void setMemoryUseLimit(size_t limit) { memory_use_limit = limit; }
    static double memoryUseRatio() 
    {
      return residentMemoryUse() * 1.0 / memoryUseLimit();
    }
    static bool isMemoryLimitEffective() 
    {
      return memory_use_limit != memory_use_limit_system_max
	&& residentMemoryUse() > 0;
    }
    static void setMemoryUsePercent(double limit) 
    { 
      assert(limit > 0.0 && limit <= 100.0);
      limit = std::max(0.01, limit);
      limit = std::min(100.0, limit);
      memory_use_percent = limit; 
    }
    /** @return standard deviation of normal distribution */
    static unsigned int evalRandom() { return eval_random; }
    static void setEvalRandom(unsigned int sigma) { eval_random = sigma; }

    static void setUsiOutputPawnValue(int new_value) { usi_output_pawn_value = new_value; }
    static int usiOutputPawnValue() { return usi_output_pawn_value; }
    /** @return 0 not testing, 1 short test, 2 long test */
    static int inUnitTest() { return in_unit_test; }
    static bool inUnitTestShort() { return in_unit_test == 1; }
    static bool inUnitTestLong() { return in_unit_test == 2; }
    static void setInUnitTest(int new_value) { in_unit_test = new_value; }

    /** 評価関数等を初期化. mainの中で一度呼ぶ必要がある */
    static void setUp();
    static bool hasByoyomi();
    static void setHasByoyomi(bool);

    static bool healthCheck();
    static int resignThreshold();
    static std::string configuration();
  private:
    static const std::string makeHome(const std::string& first_try="");
    static const std::string makeTest();
    static const std::string makeTestPublic();
    static bool isGoodDir(const std::string&);
    static void trySetDir(std::string&, const std::string&);
    static void showOslHome(const std::string&);
    static size_t memory_use_limit;
    static double memory_use_percent;
    static const size_t memory_use_limit_system_max;
    static unsigned int eval_random;
    static bool is_verbose;
    static const int default_ncpus;
    static int num_cpu;
    static volatile UsiMode usi_mode;
    static volatile bool usi_mode_silent;
    static int usi_output_pawn_value;
    static bool search_exact_value_in_one_reply, has_byoyomi;
    static volatile bool force_root_window;
    static volatile int root_window_alpha, root_window_beta;
    static volatile int in_unit_test;
    static int dfpn_max_depth;
  public:
    static std::mutex lock_io;
    // designed to be called before main
    static void registerInitializer(std::function<void()>);
  };
  struct SetUpRegister 
  {
    SetUpRegister(std::function<void()> f) {
      OslConfig::registerInitializer(f);
    }
  };
  struct NoMoreMemory : std::runtime_error
  {
    NoMoreMemory() : std::runtime_error("memory exhausted")
    {
    }
  };
}

#endif /* OSL_OSLCONFIG_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
