#ifndef _RECORD_H
#define _RECORD_H
#include "osl/record/searchInfo.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <iosfwd>

namespace osl
{
  namespace record
  {
    struct Record
    {
      RecordMinimal record;
      enum ResultType {
	Unknown=0,
	BlackWin=1,
	WhiteWin=2,
	Sennnichite=3,
	JiShogi=4,
      };
      std::vector<int> times;
      std::vector<std::string> comments;
      std::vector<SearchInfo> move_info;

      std::string version, initial_comment, tournament_name;
      CArray<std::string,2> player;
      ResultType result;
      boost::gregorian::date start_date; // default : not_a_date_time

      Record();
      ~Record();
      static void addWithNewLine(std::string& a, const std::string& b) {
	if (! a.empty())
	  a += "\n";
	a += b;
      }
      NumEffectState initialState() const { return record.initial_state; }
      std::vector<Move> moves() const { return record.moves; }

      void setDate(const std::string& date_str);
      void setMoveComment(const std::string&);
      void setMoveInfo(const SearchInfo&);
      void setMoveTime(int);
      Move lastMove() const { return moves().empty() ? Move() : moves().back(); }

      void load(std::vector<Move>& moves, std::vector<int>& times) const {
	moves = record.moves;
	times = this->times;
      }
      void load(std::vector<Move>& moves, std::vector<int>& times,
		std::vector<std::string>& comments,
		std::vector<SearchInfo>& move_info) const {
	load(moves, times);
	comments = this->comments;
	move_info = this->move_info;
      }
    };
    std::ostream& operator<<(std::ostream& os, const Record & r);

    class RecordFile
    {
    protected:
      Record record;
    public:
      virtual ~RecordFile();
      Record load() const { return record; }
      const NumEffectState initialState() const { return record.initialState(); }
      const std::vector<Move> moves() const { return record.moves(); }
    };
  }
  using record::SearchInfo;
  using record::Record;
  using record::RecordFile;
} // namespace osl
#endif /* _RECORD_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
