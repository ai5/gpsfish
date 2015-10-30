/* csajapanize.cc
 */
#include "osl/record/csaRecord.h"
#include "osl/record/record.h"
#include "osl/record/ki2.h"
#include "osl/misc/iconvConvert.h"
#include <string>
#include <iostream>
#include <sstream>
using namespace osl;
using record::SearchInfo;
void process(int move_number, const NumEffectState& src,
	     const std::vector<Move>& history,
	     const SearchInfo& info) 
{
    auto moves = info.moves;
    if (moves.empty())
	return;
    std::ostringstream ss;
    NumEffectState state = src;
    for (int i=0; i<move_number; ++i)
	state.makeMove(history[i]);
    ss << "[(" << move_number+1 << ") "
       << ki2::show(history[move_number], state)
       << "] " << info.value << ' ';
    state.makeMove(history[move_number]);
    for (size_t i=0; i<moves.size(); ++i) {
	ss << ki2::show(moves[i],
				  state, i ? moves[i-1] : history[move_number]);
	state.makeMove(moves[i]);
    }
    std::string utf8 = misc::IconvConvert::convert("EUC-JP", "UTF-8", ss.str());
    std::cout << utf8 << std::endl;

}
int main() {
    std::string line;
    std::string all;
    int last_output = 0;
    while (getline(std::cin, line)) {
	all += line + "\n";
	if (line[0] == '#') break;
	try {
	    std::istringstream is(all);
	    CsaFile csa(is);
	    record::Record record = csa.load();
	    std::vector<Move> moves;
	    std::vector<int> times;
	    std::vector<std::string> comments;
	    std::vector<SearchInfo> info;
	    record.load(moves, times, comments, info);
	    if (info.empty() || info.back().moves.empty())
		continue;
	    while (last_output < (int)info.size()) {
		if (last_output > 0)
		    process(last_output, csa.initialState(),
			    moves, info[last_output]);
		++last_output;
	    }
	}
	catch (CsaIOError& e) {
	    if (last_output) {
		std::cerr << "oops " << e.what() << ' ' << last_output << '\n';
		std::cerr << all;
	    }
	}
    }
}
