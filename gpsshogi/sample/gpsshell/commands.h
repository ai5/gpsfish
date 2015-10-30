#ifndef _GPSSHOGI_GPSSHELL_COMMANDS_H
#define _GPSSHOGI_GPSSHELL_COMMANDS_H

#ifndef MINIMAL_GPSSHELL
#ifdef ENABLE_REDIS
#  include "gpsshogi/redis/redis.h"
#  include <hiredis/hiredis.h>
#endif
#endif
#include <list>
#include <vector>
#include <string>

#include <boost/function.hpp>
#include <boost/functional.hpp>

namespace gpsshell
{
  class MySession;
}

/**
 * Regist all commands.
 */
void setCommandsMain(gpsshell::MySession *s);

namespace gpsshell
{
  class MySession
  {
  public:
    struct MyElement;
    typedef boost::function<void (gpsshell::MySession*, const std::vector<std::string>&)> MemberFunc;
    typedef std::pair<std::string, MemberFunc> Element;
    typedef std::list<MyElement> MyContainer;

    struct MyElement : public Element
    {
      MyElement(const std::string& arg1, MemberFunc arg2)
        : Element(arg1, arg2)
      {}
      operator std::string() const { return first; }
    };

  private:
    MyContainer advanced_completers; /**< contain command names and functions to be called */
    int verbose;

    void pushBoard();
    void popBoard();
    /**
     * Repease the first character "~" of a path with "$HOME".
     * @return false if an error happend; true otherwise.
     */
    bool expandTildaInPath(const std::string& in,
                           std::string& out);

#ifndef MINIMAL_GPSSHELL
#ifdef ENABLE_REDIS
    redisContext *redis_context;
#endif
#endif
  public:
    MySession();
    virtual ~MySession();

    const MyContainer& getContainer() const { return advanced_completers; }
    /**
     * Add a command.
     * @param arg1 a command name string
     * @param arg2 a member function to be called
     */
    void addCommand(const std::string& arg1, MemberFunc arg2);
    /**
     * Execute a command by its name
     * @param tokens its first element is a command name string
     * @return true if a command was really called; false otherwise 
     */
    bool executeCommand(const std::vector<std::string>& tokens);

    void help(const std::vector<std::string>& /*params*/);
    void rollBack(const std::vector<std::string>& /*params*/);
    void open(const std::vector<std::string>& params);
    void openstdin();
    void usiposition(const std::vector<std::string>& params);
    void openurl(const std::vector<std::string>& params);
    void next(const std::vector<std::string>& params);
    void prev(const std::vector<std::string>& params);
    void first(const std::vector<std::string>& /*params*/);
    void last(const std::vector<std::string>& /*params*/);
    void history(const std::vector<std::string>& /*params*/);
    void move(const std::vector<std::string>& params);

    void eval(const std::vector<std::string>& params);
    void recorddb(const std::vector<std::string>& params);
    void search(const std::vector<std::string>& params);
    void qsearch(const std::vector<std::string>& params);
    void search4(const std::vector<std::string>& params);
    void search_usi(const std::vector<std::string>& params);
    void checkmate_attack(const std::vector<std::string>& params);
    void checkmate_escape(const std::vector<std::string>& params);
    void threatmate(const std::vector<std::string>& params);
    void generateMoves(const std::vector<std::string>& params);
    void generateLegalMoves(const std::vector<std::string>& params);
    void generateNotLosingMoves(const std::vector<std::string>& params);
    void rating(const std::vector<std::string>& params);
    void rating2(const std::vector<std::string>& params);
    void quiescemoves(const std::vector<std::string>& params);
    void annotate(const std::vector<std::string>& params);

    void showStates(const std::vector<std::string>& /*params*/);
    void csaShow(const std::vector<std::string>& params);
    void usiShow(const std::vector<std::string>& params);
    void usiHistory(const std::vector<std::string>& /*params*/);
    void myshogi(const std::vector<std::string>& params);

    void setBlackColor(const std::vector<std::string>& params);
    void setWhiteColor(const std::vector<std::string>& params);
    void setLastMoveColor(const std::vector<std::string>& params);
    void setVerbose(const std::vector<std::string>& params);

    void setOpeningBook(const std::vector<std::string>& params);
    void openingShow(const std::vector<std::string>& params);
    void openingNext(const std::vector<std::string>& params);
    /**
     * usage: onext_random_level
     */
    void openingNextRandomLevel(const std::vector<std::string>& params);
    /**
     * usage: onext_random_weight [coef]
     */
    void openingNextRandomWeight(const std::vector<std::string>& params);
    /**
     * oinclude <file_name> <n-th move>
     */
    void openingInclude(const std::vector<std::string>& params);
    /**
     * Show the N shortest lines in a book.
     * command: oshow_shortest_lines [top_n]
     */
    void openingShowShortestLines(const std::vector<std::string>& params);
    /** show contents in osl::book::BookInMemory */
    void showBookInMemory(const std::vector<std::string>& params);

    void setIgnoreList(const std::vector<std::string>& params);
    void ignoreListShow(const std::vector<std::string>& params);
    void ignoreListNext(const std::vector<std::string>& params);
    void ignoreListPrev(const std::vector<std::string>& params);
    void ignoreListFirst(const std::vector<std::string>& params);
    void ignoreListLast(const std::vector<std::string>& params);
    void ignoreListShowAll(const std::vector<std::string>& params);
#ifdef OSL_SMP
    void setNumCPUs(const std::vector<std::string>& params);
#endif
    void version(const std::vector<std::string>& params);
  };
}


#endif /* _GPSSHOGI_GPSSHELL_COMMANDS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
