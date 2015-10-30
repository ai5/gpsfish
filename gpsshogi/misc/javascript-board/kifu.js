// Copyright (c) 2005 Yoshiki Hayashi

// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

var kanjiNumber = ["零", "一", "二", "三", "四", "五", "六", "七", "八", "九"];
var pieceKanjiMap = new Array();
pieceKanjiMap['FU'] = "歩";
pieceKanjiMap['KY'] = "香";
pieceKanjiMap['KE'] = "桂";
pieceKanjiMap['GI'] = "銀";
pieceKanjiMap['KI'] = "金";
pieceKanjiMap['KA'] = "角";
pieceKanjiMap['HI'] = "飛";
pieceKanjiMap['OU'] = "玉";
pieceKanjiMap['TO'] = "と";
pieceKanjiMap['NY'] = "成香";
pieceKanjiMap['NK'] = "成桂";
pieceKanjiMap['NG'] = "成銀";
pieceKanjiMap['UM'] = "馬";
pieceKanjiMap['RY'] = "竜";

function makeFormInputHandler(display_id, csa_id, move_length, input) {
  return function() {
    var value = parseInt(input.value);
    if (0 <= value && value <= move_length) {
      makeBoard(display_id, csa_id, value);
    }
    return false;
  };
}

function DisplayTable(display_id, csa_id, move_length, board) {
  this.display_id = display_id;
  this.csa_id = csa_id;
  this.board = boardTable();
  this.state = board;
  this.black_stand = standTable(true);
  this.white_stand = standTable(false);
  this.side_table = createTable(4, 2);
  this.side_table.rows[0].cells[0].innerHTML = "先手: ";
  this.side_table.rows[0].cells[1].style.textAlign = "right";
  this.side_table.rows[1].cells[0].innerHTML = "後手: ";
  this.side_table.rows[1].cells[1].style.textAlign = "right";
  this.side_table.rows[2].cells[0].colspan = 2;
  this.side_table.rows[3].cells[0].colspan = 2;

  var table = document.createElement("table");
  var tbody = document.createElement("tbody");

  for (var i = 0; i < 3; i++) {
    var tr = document.createElement("tr");
    for (var j = 0; j < 5; j++) {
      var td = document.createElement("td");
      if (i == 2 && j == 0) {
        td.innerHTML = "棋譜";
      } else if (i == 2 && j == 4) {
        td.innerHTML = "消費時間";
      }
      tr.appendChild(td);
    }
    tbody.appendChild(tr);
  }

  this.moves_div = document.createElement("div");
  var array = [this.moves_div, this.white_stand, this.board, this.black_stand, this.side_table];
  var tr = document.createElement("tr");
  for (var i = 0; i < array.length; i++) {
    var td = document.createElement("td");
    td.appendChild(array[i]);
    if (i == array.length - 1) {
      var div = document.createElement("div");
      td.style.verticalAlign = "top";
      var p = document.createElement("p");
      p.innerHTML = "コメント";
      div.appendChild(p);
      this.comment = document.createElement("div");
      div.appendChild(this.comment);
      td.appendChild(div);
    }
    tr.appendChild(td);
  }
  tbody.appendChild(tr);

  var south_tr = document.createElement("tr");
  var csa_body = document.createElement("td");
  csa_body.setAttribute("colspan", array.length - 1);
  csa_body.style.fontFamily = "monospace";
  for (var i = 0; i < array.length; i++) {
    var td = document.createElement("td");
    if (i == 0) {
      td.style.textAlign = "right";
      var form = document.createElement("form");
      form.action = "POST";
      var input = document.createElement("input");
      var closure = makeFormInputHandler(display_id, csa_id,
      				   move_length, input);
      form.onsubmit = closure;
      input.type = "text";
      input.name = "move_form_input";
      input.style.width = "2EM";
      var submit = document.createElement("input");
      submit.type = "button";
      submit.value = "手目へ";
      submit.onclick = closure;
      form.appendChild(input);
      form.appendChild(submit);
      td.appendChild(form);
    }
    if (i == 2) {
      td.style.textAlign = "center";
    }
    if (i == array.length - 1) {
      var exp = document.createElement("p");
      exp.innerHTML = "CSAにエクスポート";
      exp.setAttribute("class", "clickable");
      var exp2 = document.createElement("p");
      exp2.innerHTML = "KI2にエクスポート";
      exp2.setAttribute("class", "clickable");
      var exp3 = document.createElement("p");
      exp3.innerHTML = "USIにエクスポート";
      exp3.setAttribute("class", "clickable");
      var close = document.createElement("p");
      close.innerHTML = "閉じる";
      close.setAttribute("class", "hidden");
      var state = this.state;
      exp.onclick = function() {
	  exp.setAttribute("class", "hidden");
	  exp2.setAttribute("class", "hidden");
	  exp3.setAttribute("class", "hidden");
	  close.setAttribute("class", "clickable");
	  csa_body.innerHTML = "<pre>"+state.toCsa().replace(/\n/g, "\r\n")+"</pre>";
      };
      exp2.onclick = function() {
	  exp.setAttribute("class", "hidden");
	  exp2.setAttribute("class", "hidden");
	  exp3.setAttribute("class", "hidden");
	  close.setAttribute("class", "clickable");
	  csa_body.innerHTML = "<pre>"
	+"後手："+state.getPlayer(false)+"\r\n"
	+state.toKi2CsaStyle()
	.replace(/P\+/g, "先手の持駒：").replace(/P-/g, "後手の持駒：")
	.replace(/\* /g, "・").replace(/\+/g, " ").replace(/-/g, "v")
	.replace(/FU/g, "歩").replace(/KY/g, "香").replace(/KE/g, "桂")
	.replace(/GI/g, "銀").replace(/KI/g, "金").replace(/KA/g, "角")
	.replace(/HI/g, "飛").replace(/TO/g, "と").replace(/NY/g, "杏")
	.replace(/NK/g, "圭").replace(/NG/g, "全").replace(/UM/g, "馬")
	.replace(/RY/g, "龍").replace(/OU/g, "玉")
	.replace(/:00/g, ":").replace(/00/g, "　").replace(/none/g, "なし")
	.replace(/P1(.*)\n/g, 
	   function (all,s1) { return "  ９ ８ ７ ６ ５ ４ ３ ２ １\n"
	   +"+---------------------------+\n"
	   +"|"+s1+"|"+kanjiNumber[1]+"\n"; })
	.replace(/P9(.*)\n/g, 
	   function (all,s1) { return "|"+s1+"|"+kanjiNumber[9]+"\n"
	   +"+---------------------------+\n"; })
	.replace(/P([0-9])(.*)\n/g, 
	   function (all,s1,s2) { return "|"+s2+"|"+kanjiNumber[s1]+"\n"; })
	.replace(/([0-9])/g, function (all,s1) { return kanjiNumber[s1]; })
	.replace(/\n/g, "\r\n")
	+"先手："+state.getPlayer(true)+"\r\n"
	+"*" + state.getEvent() + "\r\n"
	+"手数=" + state.getMoveIndex() + "\r\n"
	+(state.getTurn() ? "" : "後手番\r\n")
	+"</pre>";
      };
      exp3.onclick = function() {
	  exp.setAttribute("class", "hidden");
	  exp2.setAttribute("class", "hidden");
	  exp3.setAttribute("class", "hidden");
	  close.setAttribute("class", "clickable");
	  csa_body.innerHTML = "<pre>sfen "
	+state.toUsiCsaStyle()
        .replace(/\+NG/g, "+S") // N+G=NG
	.replace(/\+FU/g, "P").replace(/\+KY/g, "L").replace(/\+KE/g, "N")
	.replace(/\+GI/g, "S").replace(/\+KI/g, "G").replace(/\+KA/g, "B")
	.replace(/\+HI/g, "R").replace(/\+TO/g, "+P").replace(/\+NY/g, "+L")
	.replace(/\+NK/g, "+N").replace(/\+UM/g, "+B")
	.replace(/\+RY/g, "+R").replace(/\+OU/g, "K")
	.replace(/-NG/g, "+s")
	.replace(/-FU/g, "p").replace(/-KY/g, "l").replace(/-KE/g, "n")
	.replace(/-GI/g, "s").replace(/-KI/g, "g").replace(/-KA/g, "b")
	.replace(/-HI/g, "r").replace(/-TO/g, "+p").replace(/-NY/g, "+l")
	.replace(/-NK/g, "+n").replace(/-UM/g, "+b")
	.replace(/-RY/g, "+r").replace(/-OU/g, "k")
	.replace(/ \*  \*  \*  \*  \*  \*  \*  \*  \* /g, "9")
	.replace(/ \*  \*  \*  \*  \*  \*  \*  \* /g, "8")
	.replace(/ \*  \*  \*  \*  \*  \*  \* /g, "7")
	.replace(/ \*  \*  \*  \*  \*  \* /g, "6")
	.replace(/ \*  \*  \*  \*  \* /g, "5")
	.replace(/ \*  \*  \*  \* /g, "4")
	.replace(/ \*  \*  \* /g, "3")
	.replace(/ \*  \* /g, "2")
	.replace(/ \* /g, "1")
	.replace(/:/g, " ").replace(/00/g, "").replace(/none/g, "-")
	.replace(/\n/g, "")
	+" 1</pre>";
      };
      close.onclick = function() {
	  exp.setAttribute("class", "clickable");
	  exp2.setAttribute("class", "clickable");
	  exp3.setAttribute("class", "clickable");
	  close.setAttribute("class", "hidden");
	  csa_body.innerHTML = "";
      }
      td.appendChild(exp);
      td.appendChild(exp2);
      td.appendChild(exp3);
      td.appendChild(close);
    }
    south_tr.appendChild(td);
  }
  tbody.appendChild(south_tr);
  var csa = document.createElement("tr");
  csa.appendChild(csa_body);
  tbody.appendChild(csa);

  table.appendChild(tbody);
  this.table = table;

  this.updateMoveNumber(0);
  this.updateTime(true, 0);
  this.updateTime(false, 0);
}

// Inherit all functions from TableOperationModule
for (var i in TableOperationModule.prototype) {
  DisplayTable.prototype[i] = TableOperationModule.prototype[i];
}

DisplayTable.prototype.setPieceOnBoardParent = DisplayTable.prototype.setPieceOnBoard;
DisplayTable.prototype.setPieceOnBoard = function(x, y, piece) {
    this.setPieceOnBoardParent(x, y, piece);
}

DisplayTable.prototype.setPieceOnStandParent = DisplayTable.prototype.setPieceOnStand;
DisplayTable.prototype.setPieceOnStand = function(turn, ptype, count) {
    this.setPieceOnStandParent(turn, ptype, count);
}

DisplayTable.prototype.getTable = function() {
  return this.table;
}

function secondsToString(time) {
  var str = time % 60 + "秒";
  if (time >= 60) {
    if (time % 60 < 10)
      str = "0" + str;
    var minutes = Math.floor(time / 60);
    str = minutes % 60 + "分" + str;
    if (minutes >= 60) {
      if (minutes % 60 < 10)
        str = "0" + str;
      var hours = Math.floor(minutes / 60);
      str = hours % 24 + "時間" + str;
      if (hours >= 24) {
	str = Math.floor(hours / 24) + "日" + str;
      }
    }
  }
  return str;
}

DisplayTable.prototype.updateTime = function(turn, time) {
  this.side_table.rows[turn ? 0 : 1].cells[1].innerHTML = secondsToString(time);
}

DisplayTable.prototype.updateComment = function(comment, move_comment) {
  content = "<textarea style=\"height:150px;\" readonly wrap=on>" + comment + "\n";
  content += String(move_comment).replace(/ *((%[^ ]*)|([+-][0-9]{4}[A-Z]{2}))/g, 
    function (all,s1) { 
      return "\n " + (s1.charAt(0) == '%' 
                      ? s1 
                      : csaMoveToKanji(s1) + " (" + s1 + ")"); 
  });
  content += "</textarea>";
  this.comment.innerHTML = content;
}

DisplayTable.prototype.updateCommentButton = function(index, comments) {
  var display_id = this.display_id;
  var csa_id = this.csa_id;
  var cell = this.side_table.rows[2].cells[1];
  if (index + 2 < comments.length) {
    if (!cell.firstChild) {
      var button = document.createElement("input");
      button.type = "button";
      button.value = "次のコメント";
      cell.appendChild(button);
    }
    cell.firstChild.onclick = function() {
      for (var i = index + 1 + 1; index < comments.length; i++) {
	if (comments[i]) {
	  makeBoard(display_id, csa_id, i);
	  return;
	}
      };
    }
  } else {
    if (cell.firstChild) {
      cell.removeChild(cell.firstChild);
    }
  }
}

function csaMoveToKanji(move) {
  var turn = move.substr(0, 1);
  return ((turn == "+") ? "▲" : "△") + move.substr(3, 1) + kanjiNumber[move.substr(4, 1)] + pieceKanjiMap[move.substr(5, 2)]
  // + (move.substr(1,2) == "00" ? "(打)" : "")
  ;
}
function csaMoveToKanji2(move, prev) {
  var pos = move.substr(3,2);
  var ppos = prev.substr(3,2);
  if (pos == ppos) {
    var turn = move.substr(0, 1);
    return ((turn == "+") ? "▲" : "△") 
	 + move.substr(3, 1) + kanjiNumber[move.substr(4, 1)]
	 + "同"
	 + pieceKanjiMap[move.substr(5, 2)];
  }
  return csaMoveToKanji(move);
}

DisplayTable.prototype.setPlayers = function(black, white) {
  var cell_black = this.table.rows[0].cells[2];
  var cell_white = this.table.rows[1].cells[2];
  black=String(black).replace(/</g, "&lt;").replace(/>/g, "&gt;");
  white=String(white).replace(/</g, "&lt;").replace(/>/g, "&gt;");
  cell_black.innerHTML = "先手: " + black;
  cell_white.innerHTML = "後手: " + white;
}

DisplayTable.prototype.updateMoveNumber = function(n, last_move, consumed, last2_move) {
  this.table.rows[2].cells[2].innerHTML = n + "手目" 
    + (last_move 
     ? (last2_move ? csaMoveToKanji2(last_move, last2_move) : csaMoveToKanji(last_move))
       +"まで"
      : "");
}

DisplayTable.prototype.getSouthCell = function() {
  return this.table.rows[4].cells[2];
}
DisplayTable.prototype.getMovesDiv = function() {
  return this.moves_div;
}

function makeSetIndexClosure(display_id, csa_id, index, csa_text, board_manager) {
  return function() {
    if (board_manager.currentIndex() < index) {
      board_manager.forward(index - board_manager.currentIndex());
    } else {
      makeBoardHandler(display_id, csa_id, index, csa_text);
    }
  };
}

function BoardManager(table, board, display_id, csa_id, csa_text) {
  this.index = 0;
  this.black_time = 0;
  this.white_time = 0;
  this.table = table;
  this.board = board;

  var moves = board.getMoves();
  var times = board.getTimes();
  var moves_div = table.getMovesDiv();
  moves_div.style.height = "250px";
  moves_div.style.width = "11em";
  moves_div.style.overflow = "scroll";
  moves_div.style.fontFamily = "monospace";
  moves_div.setAttribute("class", "moves");
  var moves_table = createTable(moves.length, 4);
  moves_table.style.width = "13em";
  for (var i=0; i<moves.length; i++) {
    var row = moves_table.rows[i];
    var closure = makeSetIndexClosure(display_id, csa_id, i+1, csa_text, this); 
    row.cells[0].innerHTML = i+1;
    row.cells[0].onclick = closure;
    row.cells[0].style.textAlign = "right";
    row.cells[1].innerHTML = i ? csaMoveToKanji2(moves[i], moves[i-1]) : csaMoveToKanji(moves[i]);
//    row.cells[1].innerHTML = String(moves[i]).replace(/([+-][0-9]{4}[A-Z]{2})/g, 
//      function (all,s1) { 
//        return csaMoveToKanji(s1);
//    });
    row.cells[1].setAttribute("nowrap", "true");
    row.cells[1].setAttribute("class", "clickable");
    row.cells[1].onclick = closure;
    row.cells[2].innerHTML = (i < times.length) ? (times[i] + "秒") : "";
    row.cells[2].style.textAlign = "right";
    row.cells[2].setAttribute("nowrap", "true");
    row.cells[3].innerHTML = moves[i];
    row.cells[3].style.textAlign = "right";
  }
  if (i>0) {
    var row = moves_table.rows[0];
    row.cells[0].setAttribute("nowrap", "true");
    row.cells[1].setAttribute("nowrap", "true");
    row.cells[3].setAttribute("nowrap", "true");
  }
  moves_div.appendChild(moves_table);
  this.moves_div = moves_div;
}

BoardManager.prototype.forward = function(n) {
  if (n == undefined) {
    n = 1;
  }
  n = Math.min(n, this.board.getMoves().length);
  for (var i = 0; i < n; i++) {
    this.index++;
    this.update(i == n-1);
   }
  this.moves_div.scrollTop = this.moves_div.scrollHeight * Math.max(this.index - 1, 0) / this.board.getMoves().length;
}

BoardManager.prototype.currentIndex = function() {
  return this.index;
}

BoardManager.prototype.update = function(update_display) {
  var board = this.board;
  var table = this.table;
  var moves = board.getMoves();
  var times = board.getTimes();
  var comments = board.getComments();
  var move_comments = board.getMoveComments();

  board.setMoveIndex(this.index);
  if (this.index == 0)
    return;

  var turn_symbol = moves[this.index - 1].charAt(0);
  var old_x = moves[this.index - 1].charAt(1);
  var old_y = moves[this.index - 1].charAt(2);
  var x = moves[this.index - 1].charAt(3);
  var y = moves[this.index - 1].charAt(4);
  var ptype = moves[this.index - 1].substr(5, 2);
  var current_consumed = 0;
  if (this.index - 1 < times.length) {
    current_consumed = times[this.index - 1];
    if (board.getTurn()) {
      this.black_time += times[this.index - 1];
      table.updateTime(true, this.black_time);
    } else {
      this.white_time += times[this.index - 1];
      table.updateTime(false, this.white_time);
    }
  }
  if (update_display) {
    var comment =  (this.index< comments.length) 
    ? comments[this.index]
    : "";
    var move_comment = (this.index < move_comments.length) 
    ? move_comments[this.index]
    : "";
    table.updateComment(comment, move_comment);
    table.updateCommentButton(this.index - 1, comments);
    table.updateMoveNumber(this.index, moves[this.index - 1],
                           current_consumed,
	this.index-1 ? moves[this.index - 2] : moves[this.index - 1]);
  }
  if (old_x == 0 && old_y == 0) {
    var turn = turn_symbol == "+";
    board.decPieceOnStand(turn, ptype);
    board.setPiece(x, y, turn_symbol + ptype);
    table.setPieceOnBoard(x, y, turn_symbol + ptype);
    table.setPieceOnStand(turn, ptype,
			  board.getPieceOnStand(turn, ptype));
  } else {
    var piece = board.getPiece(old_x, old_y);
    if (piece) {
      var victim = board.getPiece(x, y);
      if (victim) {
	if (victim.charAt(0) == piece.charAt(0)) {
	  alert("味方の駒は取れません");
	  return;
	} else {
	  var victim_ptype = victim.substr(1, 2);
	  board.incPieceOnStand(board.getTurn(), victim_ptype);
	  var n = board.getPieceOnStand(board.getTurn(), victim_ptype);
	  table.setPieceOnStand(board.getTurn(), victim_ptype, n);
	}
      }
    }
    // update display
    table.setPieceOnBoard(old_x, old_y, false);
    table.setPieceOnBoard(x, y, turn_symbol + ptype);
    // update internal state
    board.setPiece(old_x, old_y, false);
    // board.setPiece(x, y, piece);
    board.setPiece(x, y, turn_symbol + ptype);
  }
  board.setTurn(!board.getTurn());
  return false;
}

function makeBoardHandler(display_id, csa_id, forward_moves, csa_text) {
  var board = parseCsaBoard(csa_text);
  if (forward_moves == 'last') {
    forward_moves = board.getMoves().length;
  }
  forward_moves = Math.min(forward_moves, board.getMoves().length);
  var top_element = document.getElementById(display_id);
  if (!csa_text) {
    csa_text = top_element.csa_text;
  } else {
    top_element.csa_text = csa_text;
  }
  // buttons
  var beginning = document.createElement("img");
  beginning.src = getImage("beginning");
  beginning.onclick = function() {
    if (top_element.firstChild) {
      top_element.removeChild(top_element.firstChild);
    }
    makeBoardHandler(display_id, csa_id, false, csa_text);
  }

  // table
  var table = new DisplayTable(display_id, csa_id, board.getMoves().length,
                               board);
  for (var y = 1; y < 10; y++) {
    for (var x = 1; x < 10; x++) {
      var piece = board.getPiece(x, y);
      if (piece) {
	table.setPieceOnBoard(x, y, piece);
      }
    }
  }
  for (var i = 0; i < STAND_PIECES.length; i++) {
    var ptype = STAND_PIECES[i];
    table.setPieceOnStand(true, ptype,
			  board.getPieceOnStand(true, ptype));
    table.setPieceOnStand(false, ptype,
			  board.getPieceOnStand(false, ptype));
  }
  table.setPlayers(board.getPlayer(true), board.getPlayer(false));
  if (board.getComments().length > 0) {
    table.updateComment(board.getComments()[0]);
  }
  table.updateCommentButton(0, board.getComments());
  var board_element = document.getElementById(display_id);
  if (board_element.firstChild) {
    board_element.removeChild(board_element.firstChild);
  }
  board_element.appendChild(table.getTable());

  var board_manager = new BoardManager(table, board, display_id, csa_id, csa_text);
  var forward10 = document.createElement("img");
  forward10.src = getImage("forward10");
  forward10.onclick = function() {
    board_manager.forward(10);
  };
  var forward = document.createElement("img");
  forward.src = getImage("forward");
  forward.onclick = function() { board_manager.forward(1) };
  var backward = document.createElement("img");
  backward.src = getImage("backward");
  backward.onclick = function() {
    makeBoardHandler(display_id, csa_id, board_manager.currentIndex() - 1,
		     csa_text);
  };
  var backward10 = document.createElement("img");
  backward10.src = getImage("backward10");
  backward10.onclick = function() {
    makeBoardHandler(display_id, csa_id, board_manager.currentIndex() - 10,
		     csa_text);
  };
  board_manager.forward(forward_moves);
  var cell = table.getSouthCell();
  cell.appendChild(beginning);
  cell.appendChild(backward10);
  cell.appendChild(backward);
  cell.appendChild(forward);
  cell.appendChild(forward10);
}

function makeBoard(display_id, csa_id, forward_moves) {
  if (!document.getElementById(csa_id)) {
    var disp = document.getElementById(display_id);
    if (disp.csa_data) {
      makeBoardHandler(display_id, csa_id, forward_moves, disp.csa_data);
      return;
    }
    var req = false;
    if (window.XMLHttpRequest) {
      req = new XMLHttpRequest();
    } else if (window.ActiveXObject) {
      try {
	req = new ActiveXObject("Msxml2.XMLHTTP");
      } catch (e) {
	try {
	  req = new ActiveXObject("Microsoft.XMLHTTP");
	} catch (ex) {
	  req = false;
	}
      }
    }
    if (req) {
      req.onreadystatechange = function() {
	if (req.readyState == 4) {
	  makeBoardHandler(display_id, csa_id, forward_moves, req.responseText);
	}
      };
      req.open("GET", csa_id);
      req.send("");
    }
  } else {
    makeBoardHandler(display_id, csa_id, forward_moves,
		     document.getElementById(csa_id).innerHTML);
  }
}
