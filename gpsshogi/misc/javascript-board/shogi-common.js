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

var HIRATE_CSA = "\
P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n\
P2 * -HI *  *  *  *  * -KA * \n\
P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n\
P8 * +KA *  *  *  *  * +HI * \n\
P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n\
+\n";

var ALL_PTYPES = ["OU", "HI", "KA", "KI", "GI", "KE", "KY", "FU"];
var NUM_PTYPES = [2, 2, 2, 4, 4, 4, 4, 18];

function pieceCountTable() {
  var pieces = new Array();
  for (var i = 0; i < ALL_PTYPES.length; i++) {
    pieces[ALL_PTYPES[i]] = NUM_PTYPES[i];
  }
  return pieces;
}



function unpromote(ptype) {
  if (ptype == "TO") {
    return "FU";
  } else if (ptype == "NY") {
    return "KY";
  } else if (ptype == "NK") {
    return "KE";
  } else if (ptype == "NG") {
    return "GI";
  } else if (ptype == "UM") {
    return "KA";
  } else if (ptype == "RY") {
    return "HI";
  }
  return ptype;
}

function promote(ptype) {
  if (ptype == "FU") {
    return "TO";
  } else if (ptype == "KY") {
    return "NY";
  } else if (ptype == "KE") {
    return "NK";
  } else if (ptype == "GI") {
    return "NG";
  } else if (ptype == "KA") {
    return "UM";
  } else if (ptype == "HI") {
    return "RY";
  }
  return ptype;
}

function getImage(name) {
  var base = "/shogi/view/images/";
  return base + name + ".png";
}

function getPieceImage(piece) {
  return getImage(piece ? piece : "EMPTY");
}

function createTable(rows, columns, func) {
  var table = document.createElement("table");
  var tbody = document.createElement("tbody");
  for (var i = 0; i < rows; i++) {
    var tr = document.createElement("tr");
    for (var j = 0; j < columns; j++) {
      var td = document.createElement("td");
      if (func) {
	func(j, i, td);
      }
      tr.appendChild(td);
    }
    tbody.appendChild(tr);
  }
  table.appendChild(tbody);
  return table;
}

function boardTable(func) {
  var table = createTable(9, 9,
			  function(x, y, td) {
			    var image = document.createElement("img");
			    image.src = getPieceImage();
			    if (func) {
			      func(x, y, td);
			    }
			    td.appendChild(image);
			  });
  table.border = 1;
  return table;
}

var STAND_PIECES = ["HI", "KA", "KI", "GI", "KE", "KY", "FU"];
var PIECE_STAND_MAP = new Array;
for (var i = 0; i < STAND_PIECES.length; i++) {
  PIECE_STAND_MAP[STAND_PIECES[i]] = i;
}

function pieceStandRow(turn, ptype) {
  var y = PIECE_STAND_MAP[ptype] * 2;
  if (turn) {
    return y;
  } else {
    return STAND_PIECES.length * 2 + 1 - y;
  }
}

function rowToPtype(turn, row) {
  var index;
  if (turn) {
    index = row / 2;
  } else {
    index = (STAND_PIECES.length * 2 + 1 - row) / 2;
  }
  if (index < STAND_PIECES.length) {
    return STAND_PIECES[index];
  }
  return false;
}

function standTable(turn, func, fill_piece) {
  var table = createTable(STAND_PIECES.length * 2 + 2, 1,
			  function(x, y, td) {
			    if (turn && y == STAND_PIECES.length * 2 + 1
				|| (!turn && y == 0)
				|| (fill_piece && y % 2 == 0)) {
			      var image = document.createElement("img");
			      image.src = getPieceImage();
			      td.appendChild(image);
			    }
			    if (func) {
			      func(x, y, td);
			    }
			  });
  return table;
}

function TableOperationModule() {
}

TableOperationModule.prototype.setPieceOnBoard = function(x, y, piece) {
  var cell = this.board.rows[y - 1].cells[9 - x];
  if (!cell.firstChild) {
    var image = document.createElement("img");
    cell.appendChild(image);
  }
  cell.firstChild.src = getPieceImage(piece);
}

TableOperationModule.prototype.setPieceOnStand = function(turn, ptype, count) {
  ptype = unpromote(ptype);
  var table = turn ? this.black_stand : this.white_stand;
  var turn_symbol = (turn ? "+" : "-");
  var add = turn ? 1 : -1;
  var y = pieceStandRow(turn, ptype);
  var cell = table.rows[y].cells[0];
  var num_cell = table.rows[y + add].cells[0];
  var next_cell = table.rows[y + add + add].cells[0];
  if (count == 0){
    if (cell.firstChild) {
      cell.removeChild(cell.firstChild);
    }
    if (num_cell.firstChild) {
      num_cell.removeChild(num_cell.firstChild);
    }
  } else {
    if (!cell.firstChild) {
      var image = document.createElement("img");
      cell.appendChild(image);
    }
    cell.firstChild.src = getPieceImage(turn_symbol + ptype);

    if (count == 1) {
      if (num_cell.firstChild) {
	num_cell.removeChild(num_cell.firstChild);
      }
    } else {
      var image = document.createElement("img");
      image.src = getImage(turn_symbol + (count > 10 ? 10 : count));
      if (num_cell.firstChild) {
	num_cell.replaceChild(image, num_cell.firstChild);
      } else {
	num_cell.appendChild(image);
      }
      if (count > 10) {
	if (next_cell.firstChild) {
	  next_cell.removeChild(next_cell.firstChild);
	}
	var image = document.createElement("img");
	image.src = getImage(turn_symbol + count % 10);
	next_cell.appendChild(image);
      }
    }
  }
  if (ptype == "FU" && count < 10) {
    if (next_cell.firstChild) {
      next_cell.removeChild(next_cell.firstChild);
    }
  }
}

// Class Board
function Board() {
  this.board = new Array();
  for (var i = 1; i < 10; i++) {
    this.board[i] = new Array()
    for (var j = 1; j < 10; j++) {
      this.board[i][j] = false;
    }
  }
  this.black_stand = new Array();
  this.white_stand = new Array();
  var index = 0
  for (var i = 0; i < STAND_PIECES.length; i++) {
    this.black_stand[i] = 0;
    this.white_stand[i] = 0;
  }
  this.turn = true;
  this.players = new Array();
  this.players[0] = "";
  this.players[1] = "";
  this.eventname = "";
  this.move_index = 0;
}

Board.prototype.incPieceOnStand = function(turn, ptype) {
  ptype = unpromote(ptype);
  if (turn) {
    this.black_stand[PIECE_STAND_MAP[ptype]]++;
  } else {
    this.white_stand[PIECE_STAND_MAP[ptype]]++;
  }
}

Board.prototype.decPieceOnStand = function(turn, ptype) {
  ptype = unpromote(ptype);
  if (turn) {
    this.black_stand[PIECE_STAND_MAP[ptype]]--;
  } else {
    this.white_stand[PIECE_STAND_MAP[ptype]]--;
  }
}

Board.prototype.setPieceOnStand = function(turn, ptype, count) {
  ptype = unpromote(ptype);
  if (turn) {
    this.black_stand[PIECE_STAND_MAP[ptype]] = count;
  } else {
    this.white_stand[PIECE_STAND_MAP[ptype]] = count;
  }
}

Board.prototype.getPieceOnStand = function(turn, ptype) {
  ptype = unpromote(ptype);
  if (turn) {
    return this.black_stand[PIECE_STAND_MAP[ptype]];
  } else {
    return this.white_stand[PIECE_STAND_MAP[ptype]];
  }
}

Board.prototype.setPiece = function(x, y, piece) {
  if (0 < x && x < 10) {
    this.board[x][y] = piece;
  } else {
    alert("x out of range: " + x);
  }
}

Board.prototype.getPiece = function(x, y) {
  return this.board[x][y];
}

Board.prototype.setTurn = function(turn) {
  this.turn = turn;
}

Board.prototype.getTurn = function() {
  return this.turn;
}

Board.prototype.setMoves = function(moves) {
  this.moves = moves;
}

Board.prototype.getMoves = function() {
  return this.moves;
}

Board.prototype.setTimes = function(times) {
  this.times = times;
}

Board.prototype.getTimes = function() {
  return this.times;
}


Board.prototype.setComments = function(comments) {
  this.comments = comments;
}

Board.prototype.getComments = function() {
  return this.comments;
}

Board.prototype.setMoveComments = function(move_comments) {
  this.move_comments = move_comments;
}

Board.prototype.getMoveComments = function() {
  return this.move_comments;
}

Board.prototype.setPlayer = function(turn, player) {
  this.players[turn ? 0 : 1] = player;
}

Board.prototype.getPlayer = function(turn) {
  return this.players[turn ? 0 : 1];
}

Board.prototype.setEvent = function(event) {
  this.eventname = event;
}

Board.prototype.getEvent = function() {
  return this.eventname;
}

Board.prototype.setMoveIndex = function(index) {
  this.move_index = index;
}

Board.prototype.getMoveIndex = function() {
  return this.move_index;
}

Board.prototype.toCsa = function() {
  var str = "";
  str += "N+" + this.getPlayer(true) + "\n";
  str += "N-" + this.getPlayer(false) + "\n";
  if (this.getEvent())
    str += "$EVENT:" + this.getEvent() + "\n";
  for (var y = 1; y < 10; y++) {
    str += "P" + y;
    for (var x = 9; x > 0; x--) {
      var piece = this.getPiece(x, y);
      str += (piece ? piece : " * ");
    }
    str += "\n";
  }
  var piece_in_black_stand = false;
  for (var i = 0; i < STAND_PIECES.length; i++) {
    var ptype = STAND_PIECES[i];
    var count = this.getPieceOnStand(true, ptype);
    if (count > 0) {
      if (!piece_in_black_stand) {
	str += "P+"
	piece_in_black_stand = true;
      }
      for (var j = 0; j < count; j++) {
	str += "00" + ptype;
      }
    }
  }
  if (piece_in_black_stand) {
    str += "\n";
  }
  var piece_in_white_stand = false;
  for (var i = 0; i < STAND_PIECES.length; i++) {
    var ptype = STAND_PIECES[i];
    var count = this.getPieceOnStand(false, ptype);
    if (count > 0) {
      if (!piece_in_white_stand) {
	str += "P-"
	piece_in_white_stand = true;
      }
      for (var j = 0; j < count; j++) {
	str += "00" + ptype;
      }
    }
  }
  if (piece_in_white_stand) {
    str += "\n";
  }

  str += (this.turn ? "+" : "-") + "\n";
  return str;
}

Board.prototype.toKi2CsaStyle = function() {
  var str = "";

  var piece_in_white_stand = false;
  str += "P-"
  for (var i = 0; i < STAND_PIECES.length; i++) {
    var ptype = STAND_PIECES[i];
    var count = this.getPieceOnStand(false, ptype);
    if (count > 0) {
      piece_in_white_stand = true;
      str += "00" + ptype;
      if (count > 1)
	str += count;
    }
  }
  if (! piece_in_white_stand)
    str += "none";
  str += "\n";

  for (var y = 1; y < 10; y++) {
    str += "P" + y;
    for (var x = 9; x > 0; x--) {
      var piece = this.getPiece(x, y);
      str += (piece ? piece : " * ");
    }
    str += "\n";
  }
  var piece_in_black_stand = false;
  str += "P+"
  for (var i = 0; i < STAND_PIECES.length; i++) {
    var ptype = STAND_PIECES[i];
    var count = this.getPieceOnStand(true, ptype);
    if (count > 0) {
      piece_in_black_stand = true;
      str += "00" + ptype;
      if (count > 1)
	str += count;
    }
  }
  if (! piece_in_black_stand)
    str += "none";
  str += "\n";

  return str;
}

Board.prototype.toUsiCsaStyle = function() {
  var str = "";

  for (var y = 1; y < 10; y++) {
    for (var x = 9; x > 0; x--) {
      var piece = this.getPiece(x, y);
      str += (piece ? piece : " * ");
    }
    if (y < 9)
      str += "/";
  }

  str += (this.turn ? " b " : " w ") + "\n";

  var piece_in_black_stand = false;
  str += ""
  for (var i = 0; i < STAND_PIECES.length; i++) {
    var ptype = STAND_PIECES[i];
    var count = this.getPieceOnStand(true, ptype);
    if (count > 0) {
      piece_in_black_stand = true;
      if (count > 1)
	str += count;
      ptype = ptype.replace(/FU/g, "P").replace(/KY/g, "L").replace(/KE/g, "N")
	  .replace(/GI/g, "S").replace(/KI/g, "G").replace(/KA/g, "B")
	  .replace(/HI/g, "R");
      str += ptype;
    }
  }
  var piece_in_white_stand = false;
  str += ""
  for (var i = 0; i < STAND_PIECES.length; i++) {
    var ptype = STAND_PIECES[i];
    var count = this.getPieceOnStand(false, ptype);
    if (count > 0) {
      piece_in_white_stand = true;
      ptype = ptype.replace(/FU/g, "p").replace(/KY/g, "l").replace(/KE/g, "n")
	  .replace(/GI/g, "s").replace(/KI/g, "g").replace(/KA/g, "b")
	  .replace(/HI/g, "r");
      if (count > 1)
	str += count;
      str += ptype;
    }
  }
  if (! piece_in_black_stand && ! piece_in_white_stand)
    str += "-";
  str += "\n";


  return str;
}

// End Class Board

function parseCsaBoard(body) {
  var piece_count = pieceCountTable();
  var board = new Board();
  var lines = body.split("\r\n");
  if (lines.length == 1) {
    lines = body.split("\n");
    if (lines.length == 1) {
      lines = body.split("\r");
    }
  }
  var moves = new Array();
  var times = new Array();
  var comments = new Array();
  var move_comments = new Array();
  for (var i in lines) {
   var elements = lines[i].split(",");
   for (var j in elements) {
   var line = elements[j];
    if (line.charAt(0) == 'P') {
      var code = line.charAt(1) - '0';
      if (0 < code && code < 10) {
	for (var j = 0; j < 9; j++) {
	  var piece = lines[i].substr(2 + 3 * j, 3);
	  if (piece != " * ") {
	    board.setPiece(9 - j, code, piece);
	    piece_count[unpromote(pieceToPtype(piece))]--;
	  }
	}
      } else if (line.charAt(1) == "+" || line.charAt(1) == "-") {
	var turn = line.charAt(1) == "+";
	for (var j = 0; j < (lines[i].length - 2) / 4; j++) {
	  var str = line.substr(2 + 4 * j, 4);
	  var position = str.substr(0, 2);
	  var ptype = str.substr(2, 2);
	  if (position == "00") {
	    if (ptype == "AL") {
	      // 0 is OU.  Skip it.
	      for (var k = 1; k < ALL_PTYPES.length; k++) {
		var pptype = ALL_PTYPES[k];
		var count = piece_count[pptype];
		if (count > 0) {
		  board.setPieceOnStand(turn, pptype,
					board.getPieceOnStand(turn, pptype) + count);
		  piece_count[pptype] = 0;
		}
	      }
	    } else {
	      board.incPieceOnStand(turn, ptype);
	    }
	  } else {
	    board.setPiece(position[0], position[1], line.charAt(1) + ptype);
	  }
	  if (ptype != "AL") {
	    piece_count[ptype]--;
	  }
	}
      }
    } else if (line == "+") {
      board.setTurn(true);
    } else if (line == "-") {
      board.setTurn(false);
    } else if (line.charAt(0) == '+' || line.charAt(0) == '-') {
      moves.push(line);
    } else if (line.charAt(0) == "N") {
      board.setPlayer(line.charAt(1) == "+",
		      line.substring(2, line.length));
    } else if (line.charAt(0) == 'T') {
      var length = times.length;
      for (var i = 0; i < moves.length - length - 1; i++) {
	times.push(0);
      }
      times.push(parseInt(line.substring(1, line.length)));
    } else if (line.substr(0, 4) == "'** ") {
      var length = move_comments.length;
      for (var i = 0; i < moves.length - length; i++) {
	move_comments.push("");
      }
      var data = line.substring(3, line.length);
      data=String(data).replace(/</g,"&lt;").replace(/>/g,"&gt;");
      move_comments.push(data);
    } else if (line.substr(0, 8) == "'summary") {
      var length = comments.length;
      for (var i = 0; i < moves.length - length; i++) {
	comments.push("");
      }
      var data = line.substring(1, line.length);
      data=String(data).replace(/</g,"&lt;").replace(/>/g,"&gt;");
      comments.push(data);
    } else if (line.substr(0, 3) == "'* ") {
      var length = comments.length;
      for (var i = 0; i < moves.length - length; i++) {
	comments.push("");
      }
      comments.push(line.substring(3, line.length));
    } else if (line.substr(0, 7) == "$EVENT:") {
      board.setEvent(line.substring(7, line.length));
    }
   }
  }
  board.setMoves(moves);
  board.setTimes(times);
  board.setComments(comments);
  board.setMoveComments(move_comments);
  return board;
}

function pieceToPtype(piece) {
  return piece.substr(1, 2);
}
