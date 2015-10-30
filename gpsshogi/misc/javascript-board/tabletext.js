//
// tabletext.js - display Japanese text as a table using DHTML.
//
// Copyright (C) 2005 Satoru Takabayashi <satoru@namazu.org> 
//     All rights reserved.
//     This is free software with ABSOLUTELY NO WARRANTY.
//
// You can redistribute it and/or modify it under the terms of 
// the GNU General Public License version 2.
//

function makeChar(bitmap) {
    var table = document.createElement("table");
    table.className = "char";
    var tbody = document.createElement("tbody");
    for (var i = 0; i < 16; ++i) {
        var tr = document.createElement("tr");
        var row = bitmap[i];
        for (var j = 15; j >= 0; --j) {
            var td = document.createElement("td");
            if (row & (1 << j)) {
                td.className = "black";
            } else {
                td.className = "white";
            }
            td.appendChild(document.createTextNode(""));
            tr.appendChild(td);
        }
        tbody.appendChild(tr);
    }
    table.appendChild(tbody);
    return table;
}

function getBitmap(code) {
    return font[code];
}

function rotate180(bitmap) {
    var rotated = bitmap.concat().reverse(); // concat == copy
    for (var i = 0; i < 16; ++i) {
        var value = rotated[i];
        rotated[i] = 
            ((value & 0x0001) << 15) |
            ((value & 0x0002) << 13) |
            ((value & 0x0004) << 11) |
            ((value & 0x0008) <<  9) |
            ((value & 0x0010) <<  7) |
            ((value & 0x0020) <<  5) |
            ((value & 0x0040) <<  3) |
            ((value & 0x0080) <<  1) |
            ((value & 0x0100) >>  1) |
            ((value & 0x0200) >>  3) |
            ((value & 0x0400) >>  5) |
            ((value & 0x0800) >>  7) |
            ((value & 0x1000) >>  9) |
            ((value & 0x2000) >> 11) |
            ((value & 0x4000) >> 13) |
            ((value & 0x8000) >> 15);
    }
    return rotated;
}

function printTextCommon(index, filter) {
    var table = document.createElement("table");
    table.className = "text";
    var tbody = document.createElement("tbody");
    var tr = document.createElement("tr");
    var bitmap = font[index];
    if (bitmap) {
      if (filter) {
	bitmap = filter(bitmap);
      }
      var chr = makeChar(bitmap);
      var td = document.createElement("td");
      td.appendChild(chr);
      tr.appendChild(td);
    }
    tbody.appendChild(tr);
    table.appendChild(tbody);
    return table;
}

function printText(text) {
  var table = printTextCommon(text);
  var div = document.getElementById("tabletext");
  if (div) {
    div.appendChild(table);
  }
}

function printUpsideDownText(text) {
  var table = printTextCommon(text, rotate180);
  var div = document.getElementById("tabletext");
  if (div) {
    div.appendChild(table);
  }
}
