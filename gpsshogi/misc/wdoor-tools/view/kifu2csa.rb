# -*- coding: euc-jp -*-
# $Id$
#
# Author:: Team GPS
#
#--
# Copyright (C) 2009 Team GPS
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
require 'nkf'

class Kifu2Csa
  @@table = {
    '〇'=>0, '０'=>0,
    '一'=>1, '１'=>1,
    '二'=>2, '２'=>2,
    '三'=>3, '３'=>3,
    '四'=>4, '４'=>4,
    '五'=>5, '５'=>5,
    '六'=>6, '６'=>6,
    '七'=>7, '７'=>7,
    '八'=>8, '８'=>8,
    '九'=>9, '９'=>9,
  }
  @@piece = { 
    '歩'=>"FU"   , '香'=>"KY"   , '桂'=>'KE'   , '銀'=>"GI" , '金'=>"KI" , 
    '角'=>"KA"   , '飛'=>"HI"   , '王'=>'OU'   , '玉'=>"OU" , 'と'=>"TO" , 
    "成香"=>"NY" , "成桂"=>"NK" , "成銀"=>"NG" , 
    "馬"=>"UM"   , "龍"=>"RY"   , "竜"=>"RY"   , 
    '歩成'=>"TO" , "香成"=>"NY" , "桂成"=>"NK" , 
    "銀成"=>"NG" , "角成"=>"UM" , "飛成"=>"RY" , 
  }

  def kifu_to_csa(kifu, enable_comment=false)
    kifu = NKF.nkf("-Se", kifu)
    prev = ""
    csa = Array.new
    comment = Array.new
    kifu.each_line do |line|
      case line
      when /^ *(\d+) *([^ ]+)/
        number = $1
        move   = $2
        break if /投了/ =~ move
        move.sub!(/同/,"#{prev}")
        move.sub!(/　/,"")
        move.sub!(/打/,"(00)")
        if /^(.)(.)(.+)\((\d+)\)/ =~ move
          csa << "'* %s" % [comment.join("　")] if enable_comment
          comment.clear
          prev = $1 + $2
          black_or_white = number.to_i.odd? ? '+' : '-'
          from           = $4
          to             = "%s%s" % [@@table[$1], @@table[$2]]
          csa_piece      = @@piece[$3]
          csa << black_or_white + from + to + csa_piece
        else
          csa << move
        end
      when /^\*(.+)/
        comment << $1.strip
      when /先手：(.+)/
        csa << "N+#$1"
      when /後手：(.+)/
        csa << "N-#$1"
      end
    end
    header = <<EOM
P1-KY-KE-GI-KI-OU-KI-GI-KE-KY
P2 * -HI *  *  *  *  * -KA * 
P3-FU-FU-FU-FU-FU-FU-FU-FU-FU
P4 *  *  *  *  *  *  *  *  * 
P5 *  *  *  *  *  *  *  *  * 
P6 *  *  *  *  *  *  *  *  * 
P7+FU+FU+FU+FU+FU+FU+FU+FU+FU
P8 * +KA *  *  *  *  * +HI * 
P9+KY+KE+GI+KI+OU+KI+GI+KE+KY
+
EOM
    return header + csa.join("\n")
  end

end
