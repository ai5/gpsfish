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

function PiecesTable(board) {
  var self = this;
  this.table = createTable(2, 9,
			   function(x, y, td) {
			     var image = document.createElement("img");
			     image.src = getPieceImage();
			     if (y == 0) {
			       td.onmousedown = function() {
				 if (self.getPieceCount(ALL_PTYPES[x]) == 0)
				   return;
				 if (this.firstChild) {
				   this.removeChild(this.firstChild);
				 }
				 board.selectBox(x);
			       };
			       td.onmouseup = function() {
				 var selected = board.getSelected();
				 if (selected) {
				   var piece = selected[0];
				   var ptype = pieceToPtype(piece);
				   self.setPiece(ptype, self.getPieceCount(ptype) + 1);
				   board.removeSelection();
				 }
			       }
			     }
			     td.appendChild(image);
			   });
  this.piece_map = new Array();
  for (var i = 0; i < ALL_PTYPES.length; i++) {
    this.piece_map[ALL_PTYPES[i]] = i;
  }
  this.pieces = new Array();
  for (var i = 0; i < ALL_PTYPES.length; i++) {
    this.pieces[ALL_PTYPES[i]] = NUM_PTYPES[i];
    this.setPiece(ALL_PTYPES[i], NUM_PTYPES[i]);
  }
}

PiecesTable.prototype.reset = function() {
  for (var i = 0; i < ALL_PTYPES.length; i++) {
    this.pieces[ALL_PTYPES[i]] = NUM_PTYPES[i];
    this.setPiece(ALL_PTYPES[i], NUM_PTYPES[i]);
  }
}

PiecesTable.prototype.getTable = function() {
  return this.table;
}

PiecesTable.prototype.getPieceCount = function(ptype) {
  return this.pieces[ptype];
}

PiecesTable.prototype.setPiece = function(ptype, number) {
  var piece = "+" + ptype;
  var piece_cell = this.table.rows[0].cells[this.piece_map[ptype]];
  var num_cell = this.table.rows[1].cells[this.piece_map[ptype]];
  this.pieces[ptype] = number;
  if (!piece_cell.firstChild) {
    var image = document.createElement("img");
    image.src = getPieceImage(piece);
    piece_cell.appendChild(image);
  }

  if (number == 0) {
    piece_cell.firstChild.src = getPieceImage("");
    num_cell.firstChild.src = getImage("EMPTY");
  } else {
    piece_cell.firstChild.src = getPieceImage(piece);
    if (number == 1) {
      num_cell.firstChild.src = getImage("EMPTY");
    } else if (number > 10) {	// FU only
      var num_cell2 = this.table.rows[1].cells[this.piece_map[ptype] + 1];
      num_cell.firstChild.src = getImage("+" + 10);
      num_cell2.firstChild.src = getImage("+" + number % 10);
    } else {
      num_cell.firstChild.src = getImage("+" + number);
      }
    }
  if (ptype == "FU" && number <= 10) {
    var num_cell2 = this.table.rows[1].cells[this.piece_map[ptype] + 1];
    num_cell2.firstChild.src = getImage("EMPTY");
  }
}

function makeSelectHandler(x, y, board) {
  var closure = function() {
    if (board.getPiece(x, y)) {
      board.selectBoard(x, y);
      if (this.firstChild) {
	this.removeChild(this.firstChild);
      }
    }
  }
  return closure;
}

function makeSetHandler(x, y, board) {
  return function() {
    var selected = board.getSelected();
    if (selected) {
      if (selected[1] == x && selected[2] == y) {
	promotePiece(x, y, board);
	board.undoSelection();
      } else if (board.getPiece(x, y)) {
	board.undoSelection();
      } else {
	board.setPieceOnBoard(x, y, selected[0]);
	board.removeSelection();
      }
    }
  }
}

function changeTurnSymbol(turn) {
  if (turn == "+") {
    return "-";
  } else {
    return "+";
  }
}

function promotePiece(x, y, board) {
  var piece = board.getPiece(x, y, board);
  if (piece) {
    var ptype = pieceToPtype(piece);
    var turn = piece.substr(0, 1);
    if (ptype == promote(ptype)) {
      turn = changeTurnSymbol(turn);
      ptype = unpromote(ptype);
    } else {
      ptype = promote(ptype);
    }
    var new_piece = turn + ptype;
    board.setPieceOnBoard(x, y, new_piece);
  }
}

function standMouseUpClosure(self, turn) {
  return function() {
    var selected = self.getSelected();
    if (selected) {
      self.incPieceOnStand(turn, pieceToPtype(selected[0]));
      self.removeSelection();
    }
  };
}

function standMouseDownClosure(self, turn, y) {
  return function() {
    var ptype = rowToPtype(turn, y);
    var count = self.getPieceOnStand(turn, ptype);
    if (count > 0) {
      if (this.firstChild) {
	this.removeChild(this.firstChild);
      }
      self.selectStand(turn, ptype);
    }
  };
}

// Class EditBoard
function EditBoard() {
  var pieces = new PiecesTable(this);
  var self = this;
  this.state = new Board();
  this.board = boardTable(function(x, y, td) {
			    td.onmousedown = makeSelectHandler(x, y, self);
			    td.onmouseup = makeSetHandler(x, y, self);
			  });
  this.black_stand = standTable(true,
				function(x, y, td) {
				  if (y % 2 == 0) {
				    td.onmousedown = standMouseDownClosure(self,
									   true,
									   y);
				  }
				}, true);
  this.black_stand.onmouseup = standMouseUpClosure(self, true);
  this.white_stand = standTable(false,
				function(x, y, td) {
				  if (y % 2 == 1) {
				    td.onmousedown = standMouseDownClosure(self,
									   false,
									   y);
				  }
				}, true);
  this.white_stand.onmouseup = standMouseUpClosure(self, false);
  this.pieces = pieces;
  var board = [this.white_stand, this.board, this.black_stand];
  this.table = createTable(2, 3,
			   function(x, y, td) {
			     if (y == 1) {
			       td.appendChild(board[x]);
			     } else if (x == 1 && y == 0) {
			       td.appendChild(pieces.getTable());
			     }
			   });
}

// Inherit all functions from TableOperationModule
for (var i in TableOperationModule.prototype) {
  EditBoard.prototype[i] = TableOperationModule.prototype[i];
}

EditBoard.prototype.getTable = function() {
  return this.table;
}

EditBoard.prototype.getPiece = function(x ,y) {
  return this.state.getPiece(9 - x, y + 1);
}

EditBoard.prototype.incPieceOnStand = function(turn, ptype) {
  this.state.incPieceOnStand(turn, ptype);
  var count = this.state.getPieceOnStand(turn, ptype);
  this.setPieceOnStand(turn, ptype, count);
}

EditBoard.prototype.decPieceOnStand = function(turn, ptype) {
  this.state.decPieceOnStand(turn, ptype);
  var count = this.state.getPieceOnStand(turn, ptype);
  this.setPieceOnStand(turn, ptype, count);
}

EditBoard.prototype.setPieceOnStandParent = EditBoard.prototype.setPieceOnStand;

EditBoard.prototype.setPieceOnStand = function(turn, ptype, count) {
  this.state.setPieceOnStand(turn, ptype, count);
  this.setPieceOnStandParent(turn, ptype, count);
}

EditBoard.prototype.setPieceOnBoardParent = EditBoard.prototype.setPieceOnBoard;

EditBoard.prototype.setPieceOnBoard = function(x, y, piece) {
  this.setPieceOnBoardParent(9 - x, y + 1, piece);
  this.state.setPiece(9 - x, y + 1, piece);
}

EditBoard.prototype.getPieceOnStand = function(turn, ptype) {
  return this.state.getPieceOnStand(turn, ptype);
}

EditBoard.prototype.selectBox = function(x) {
  this.selected = new BoxSelection(x, this.pieces);
}

EditBoard.prototype.selectBoard = function(x, y) {
  this.selected = new BoardSelection(x, y, this, this.state);
}

EditBoard.prototype.selectStand = function(turn, ptype) {
  this.selected = new StandSelection(turn, ptype, this);
}

EditBoard.prototype.getSelected = function() {
  if (this.selected) {
    return this.selected.getSelected();
  }
  return false;
}


EditBoard.prototype.removeSelection = function() {
  if (this.selected) {
    this.selected.removeSelection();
  }
  this.selected = false;
}

EditBoard.prototype.undoSelection = function() {
  if (this.selected) {
    this.selected.undoSelection();
  }
  this.selected = false;
}

EditBoard.prototype.toCsa = function() {
  return this.state.toCsa();
}
EditBoard.prototype.parseCsa = function(text) {
  var decreaseBox = function(piece_table, ptype, count) {
    piece_table.setPiece(ptype, piece_table.getPieceCount(ptype) - count);
  };

  var adjustPieceStand = function (edit_board, turn) {
    for (var i = 0; i < STAND_PIECES.length; i++) {
      var ptype = STAND_PIECES[i];
      var count = state.getPieceOnStand(turn, ptype);
      edit_board.setPieceOnStand(turn, ptype, count);
      decreaseBox(edit_board.pieces, ptype, count);
    }
  };

  var state = parseCsaBoard(text);
  this.pieces.reset();
  for (var y = 1; y < 10; y++) {
    for (var x = 1; x < 10; x++) {
      var piece = state.getPiece(x, y);
      this.setPieceOnBoard(9 - x , y - 1, piece);
      if (piece) {
	var ptype = unpromote(pieceToPtype(piece));
	decreaseBox(this.pieces, ptype, 1);
      }
    }
  }
  adjustPieceStand(this, true);
  adjustPieceStand(this, false);
}

EditBoard.prototype.boxPiecesToStand = function(turn) {
  // skip OU
  for (var i = 1; i < ALL_PTYPES.length; i++) {
    var ptype = ALL_PTYPES[i];
    var count = this.pieces.getPieceCount(ptype);
    this.setPieceOnStand(turn, ptype,
			 this.getPieceOnStand(turn, ptype) + count);
    this.pieces.setPiece(ptype, 0);
  }
}

// End Class EditBoard


// Class BoxSelection
function BoxSelection(x, piece_table) {
  this.x = x;
  this.piece_table = piece_table;
}

BoxSelection.prototype.undoSelection = function() {
  var ptype = ALL_PTYPES[this.x];
  this.piece_table.setPiece(ptype, this.piece_table.getPieceCount(ptype));
}

BoxSelection.prototype.removeSelection = function() {
  var ptype = ALL_PTYPES[this.x];
  this.piece_table.setPiece(ptype, this.piece_table.getPieceCount(ptype) - 1);
}

BoxSelection.prototype.getSelected = function() {
  return ["+" + ALL_PTYPES[this.x], -1, -1];
}

function BoardSelection(x, y, edit_board, state_table) {
  this.x = x;
  this.y = y;
  this.edit_board = edit_board;
  this.state_table = state_table;
}

BoardSelection.prototype.undoSelection = function() {
  var piece = this.state_table.getPiece(9 - this.x, this.y + 1);
  this.edit_board.setPieceOnBoard(this.x, this.y, piece);
}

BoardSelection.prototype.removeSelection = function() {
  this.edit_board.setPieceOnBoard(this.x, this.y, false);
  this.state_table.setPiece(9 - this.x, this.y + 1, false);
}

BoardSelection.prototype.getSelected = function() {
  return [this.state_table.getPiece(9 - this.x, this.y + 1), this.x, this.y];
}

function StandSelection(turn, ptype, edit_board) {
  this.turn = turn;
  this.ptype = ptype;
  this.edit_board = edit_board;
}

StandSelection.prototype.undoSelection = function() {
  this.edit_board.setPieceOnStand(this.turn, this.ptype,
				  this.edit_board.getPieceOnStand(this.turn,
								  this.ptype));
}

StandSelection.prototype.removeSelection = function() {
  this.edit_board.decPieceOnStand(this.turn, this.ptype);
}

StandSelection.prototype.getSelected = function() {
  return [(this.turn ? "+" : "-") + this.ptype, -1, -1];
}



function makeTable() {
  var table = createTable(2, 2);
  var board = new EditBoard();
  table.rows[0].cells[0].appendChild(board.getTable());
  var element = document.getElementById("board");
  var textarea = document.createElement("textarea");
  textarea.cols = 36;
  textarea.rows = 12;
  table.rows[0].cells[1].appendChild(textarea);
  table.rows[0].cells[1].style.textAlign = "center";
  table.rows[0].cells[1].style.verticalAlign = "center";
  var hirate_button = document.createElement("input");
  hirate_button.type = "button";
  hirate_button.value = "平手";
  hirate_button.onclick = function() {
    board.parseCsa(HIRATE_CSA);
  }
  table.rows[0].cells[1].appendChild(document.createElement("br"));
  table.rows[0].cells[1].appendChild(hirate_button);

  var clear_button = document.createElement("input");
  clear_button.type = "button";
  clear_button.value = "クリア";
  clear_button.onclick = function() {
    textarea.value = "";
  }
  table.rows[0].cells[1].appendChild(document.createElement("br"));
  table.rows[0].cells[1].appendChild(clear_button);

  var button = document.createElement("input");
  button.type = "button";
  button.value = "CSA へ";
  button.onclick = function() {
    textarea.value = board.toCsa();
  }
  var button2 = document.createElement("input");
  button2.type = "button";
  button2.value = "CSA 読み込み";
  button2.onclick = function() {
    board.parseCsa(textarea.value);
  }
  table.rows[1].cells[0].style.textAlign = "center";
  table.rows[1].cells[0].appendChild(button);
  table.rows[1].cells[0].appendChild(button2);

  var button3 = document.createElement("input");
  button3.type = "button";
  button3.value = "先手の持駒へ";
  button3.onclick = function() {
    board.boxPiecesToStand(true);
  }
  var button4 = document.createElement("input");
  button4.type = "button";
  button4.value = "後手の持駒へ";
  button4.onclick = function() {
    board.boxPiecesToStand(false);
  }

  table.rows[1].cells[1].style.textAlign = "center";
  table.rows[1].cells[1].appendChild(button3);
  table.rows[1].cells[1].appendChild(button4);

  element.appendChild(table);
  document.addEventListener('mouseup',
			    function(event) {
			      board.undoSelection();
			    }, false);
}
