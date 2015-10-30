#include "osl/record/record.h"
#include "osl/record/kanjiCode.h"
#include "osl/misc/eucToLang.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <stack>
#include <iostream>

osl::record::Record::Record() : result(Unknown)
{
}
osl::record::Record::~Record()
{
}

void osl::record::Record::setMoveComment(const std::string& msg)
{
  if (moves().empty()) {
    addWithNewLine(initial_comment, msg);
    return;
  }
  comments.resize(moves().size());
  comments.back() = msg;
}
void osl::record::Record::setMoveInfo(const SearchInfo& info)
{
  move_info.resize(moves().size());
  if (!move_info.empty()) {
    move_info.back() = info;
  }
}
void osl::record::Record::setMoveTime(int s)
{
  times.resize(moves().size());
  times.back() = s;
}

void osl::record::Record::setDate(const std::string& date_str)
{
  std::vector<std::string> values;
  boost::algorithm::split(values, date_str, boost::algorithm::is_any_of("/"));
  if (values.size() < 3) {
    std::cerr << "ERROR: Invalid date format found: "
#ifndef MINIMAL
	      << misc::eucToLang(date_str)
#endif
	      << "\n";
    return;
  } else if (values.size() > 3) {
    std::cerr << "WARNING: Invalid date format found: "
#ifndef MINIMAL
	      << misc::eucToLang(date_str)
#endif
	      << "\n";
    // go through
  }
  for (std::string& value: values) {
    static const CArray<const char *,9> kanji = {{
	K_R1, K_R2, K_R3, K_R4, K_R5, K_R6, K_R7, K_R8, K_R9, 
      }};
    for (size_t i=0; i<kanji.size(); ++i)
      boost::algorithm::replace_all(value, kanji[i], std::string(1, char('0'+i)));
  }
  int year  = 0;
  int month = 0;
  int day   = 0;
  try {
    year  = stoi(values[0]);
    month = stoi(values[1]);
    if (month == 0) month = 1;
    if ("??" == values[2]) {
      std::cerr << "WARNING: Invalid date format found: "
#ifndef MINIMAL
		<< misc::eucToLang(values[2])
#endif
		<< "\n";
      // go through
      day = 1;
    } else if (values[2].size() > 2) {
      std::cerr << "WARNING: Invalid date format found: "
#ifndef MINIMAL
		<< misc::eucToLang(values[2])
#endif
		<< "\n";
      // go through
      day = stoi(values[2].substr(0,2));
    } else {
      day = stoi(values[2]);
    }
    if (day == 0) day = 1;
    start_date = boost::gregorian::date(year, month, day);
    assert(!start_date.is_special());
  }
  catch (boost::gregorian::bad_day_of_month& ebdm) {
    std::cerr << "Bad day of month: "
#ifndef MINIMAL
	      << misc::eucToLang(date_str)
#endif
	      << "\n"
	      << ebdm.what() << std::endl;
  }
  catch (std::exception& e) {
    std::cerr << "Invalid date format found: "
#ifndef MINIMAL
	      << misc::eucToLang(date_str)
#endif
	      << "\n"
	      << e.what() << std::endl;
  } 
  return;
}

#ifndef MINIMAL  
std::ostream& osl::record::operator<<(std::ostream& os, const Record & r){
  os << "Record(";
  os << "version=" << r.version
     << ",BLACK=" << r.player[BLACK]
     << ",WHITE=" << r.player[WHITE];
  os << ",initial=" << std:: endl << r.record.initial_state << std::endl;
  NumEffectState state(r.record.initial_state);
  for (auto& m: r.record.moves) os << m;
  return os << ')';
}
#endif  


osl::record::RecordFile::~RecordFile()
{
}
  
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
