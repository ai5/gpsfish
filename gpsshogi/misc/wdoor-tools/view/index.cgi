#!/usr/bin/ruby
# -*- coding: euc-jp -*-
# $Id$
#
# Author:: Team GPS
#
#--
# Copyright (C) 2006-2009 Team GPS
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#++
#
#
$:.unshift(File.expand_path(File.dirname(__FILE__)))
require 'cgi'
require 'erb'
require 'stringio'
require 'digest/sha1'

require 'download_manager'
require 'eval_graph'
require 'kifu2csa'

include ERB::Util

FILE_NAME=File.basename(__FILE__)

def first_attack(csa)
  require 'shogi-server'
  board = Board.new
  board.initial
  csa.each_line do |line|
    board.handle_one_move(line.strip, nil)
    break unless board.sente_hands.empty? && board.gote_hands.empty?
  end
  board.move_count - 1
end  

def first_checkmate(csa)
  require 'shogi-server'
  board = Board.new
  board.initial
  csa.each_line do |line|
    board.handle_one_move(line.strip, nil)
    break if board.checkmated?(true) || board.checkmated?(false)
  end
  board.move_count
end

def parse_move(move, csa=nil)
  return nil unless move

  case move.strip
  when /\d+/
    ", #{$&}"
  when /^first$/
    ", 1"
  when /^last$/
    ", 'last'"
  when /^first[_\-]attack$/
    i = first_attack(csa)
    i > 0 ? ", #{i}" : nil
  when /^first[_\-]checkmate$/
    i = first_checkmate(csa)
    i > 0 ? ", #{i}" : nil
  else     
    nil
  end
end

@cgi = CGI.new()
@cgi.header("type" => "text/html")
@title = "CSA Viewer"
@my_uri = "http://"+ENV["HTTP_HOST"]+ENV["REQUEST_URI"]

ENV['PATH'] = "/usr/bin"

unless @cgi["csa"].empty?
  url  = @cgi["csa"]
  dm = DownloadManager.new(url)
  kifu = ""
  begin
    kifu = dm.kifu_download
    if dm.is_kifu?
      flag = (FILE_NAME == "kifu.cgi")
      kifu = Kifu2Csa.new.kifu_to_csa(kifu, flag)
    end
  rescue DownloadManager::ForbiddenError
    cgi = CGI.new("html4")
    cgi.header("type" => "text/html", "status" => "FORBIDDEN")
    cgi.out({"content-type" => "text/html"}) do
      cgi.html do
        cgi.head {cgi.title{"403 FORBIDDEN"}} +
        cgi.body do
          cgi.h1{"403 FORBIDDEN"} +
          cgi.p{"You don't have permission to access the page you request on this server."}
        end
      end
    end
    exit
  end
  go_last = @cgi["go_last"].empty? ? nil : "last"
  move   = @cgi["move_to"].empty? ? nil : @cgi["move_to"]
  move ||= @cgi["move"].empty? ? nil : @cgi["move"]
  move ||= go_last
  
  # Generate an eval graph
  if ! dm.is_kifu?
    file = "cache/" + Digest::SHA1.hexdigest(@cgi["csa"])
    original_file_name = nil
    if %r!/(.*?\.csa$)! =~ @cgi["csa"]
      original_file_name= $1
    end
    file_svg = "#{file}.svg"
    read(StringIO.new(kifu).readlines, file, original_file_name)
    str = reformat_svg( open(file_svg){|f| f.read})
    open(file_svg,"w+") {|f| f << str}
  end
  # try to parse filename
  if %r!\+([A-Za-z0-9_@.-]+)\+([A-Za-z0-9_@.-]+)\+([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})\.csa$! =~ url
    @title = $1+" vs. "+$2+" ("+$3+"-"+$4+"-"+$5+" "+$6+":"+$7+")"
  end
end

erb = ERB.new( DATA.read, nil, "%<>" )
body = erb.result(binding)
@cgi.out({"content-type" => "text/html; charset=euc-jp"}){body}

__END__
<?xml version="1.0" encoding="EUC-JP"?>
<!DOCTYPE html
	PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
	 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="ja" xml:lang="ja">
<head>
  <title><%= @title %></title>
  <meta http-equiv="Content-Type" content="text/html; charset=EUC-JP" />
  <meta http-equiv="Content-Script-Type" content="text/javascript" />
  <meta http-equiv="Content-Style-Type" content="text/css" />
  <link rel="StyleSheet" type="text/css" href="/shogi/shogi.css">
</head>
<body>
<h1><%= @title %></h1>
<% if kifu %>
<pre id="csa" class="hidden">
<%= h kifu %></pre>

<div id="board" class="board">
</div>

<script type="text/javascript" src="/shogi/view/shogi-common.js"></script>
<script type="text/javascript" src="/shogi/view/kifu.js"></script>
<script type="text/javascript">makeBoard('board', 'csa' <%= parse_move(move, kifu) %>)</script>

<div>
<% if %r!index.cgi! =~ ENV["REQUEST_URI"] %>
<form method="get" action="<%= ENV["REQUEST_URI"] %>" enctype="application/x-www-form-urlencoded">
<input type="hidden" name="go_last" value="on" />
<input type="hidden" name="csa" value="<%=@cgi["csa"]%>" />
<input type="submit" value="最新の棋譜に更新" />
</form>
<% end %>
<% if %r!index.cgi! !~ ENV["REQUEST_URI"] %>
<p><a href="<%= ENV["REQUEST_URI"].sub(/\/[0-9]+$/,"") %>">最新の棋譜に更新</a></p>
<p>この対局にコメントする: 
<a href="http://twitter.com/share" class="twitter-share-button" data-count="horizontal">Tweet</a><script type="text/javascript" src="http://platform.twitter.com/widgets.js"></script>
<script type="text/javascript" src="https://apis.google.com/js/plusone.js"></script>
<g:plusone></g:plusone>
<script src="http://connect.facebook.net/en_US/all.js#xfbml=1"></script><fb:like href="<%=@my_uri%>" send="false" layout="button_count" width="450" show_faces="false" font=""></fb:like>
</p>

<% end %>
</div>

<% if ! dm.is_kifu? %>
<p>評価値グラフ</p>
<div style="text-align: center" class="graph">
<object width="800" height="500" data="<%="/shogi/view/"+file_svg%>" type="image/svg+xml">
  <param name="src" value="<%="/shogi/view/"+file_svg%>" />
<a href="<%="/shogi/view/"+file_svg%>">Evaluatin value graph</a>
</object>
</div>
<% end %>

URL: <a href="<%= dm.h_url %>"><%= dm.h_url %></a>
<% end %>

<h2>Input URL</h2>
<p><a href="http://www.sodan.org/~penny/shogi/kifu.html">棋譜再生用 Javascript 将棋盤</a>を利用して棋譜を表示します。</p>

<div>
<form method="get" action="<%="/shogi/view/"+FILE_NAME%>" enctype="application/x-www-form-urlencoded">
URL for csa file: <input type="text" name="csa" size="80" value="<%=@cgi["csa"]%>" /><br />
<input type="checkbox" name="go_last" value="on" <%=@cgi["go_last"] == "on" ? 'checked="checked"' : ""%> />show the last state<br />
<fieldset>
<input type="text"  name="move_to" size="3" />手目<br />
<input type="radio" name="move" value="first" />初手<br />
<input type="radio" name="move" value="first_attack" />最初に駒が取られる<br />
<input type="radio" name="move" value="first_checkmate" />最初の王手<br />
<input type="radio" name="move" value="last" />投了図<br />
<legend>move to:</legend>
</fieldset>
<input type="submit" value="show/reload" />
</form>
</div>

</body>
</html>      

