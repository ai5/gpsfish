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
    '��'=>0, '��'=>0,
    '��'=>1, '��'=>1,
    '��'=>2, '��'=>2,
    '��'=>3, '��'=>3,
    '��'=>4, '��'=>4,
    '��'=>5, '��'=>5,
    'ϻ'=>6, '��'=>6,
    '��'=>7, '��'=>7,
    'Ȭ'=>8, '��'=>8,
    '��'=>9, '��'=>9,
  }
  @@piece = { 
    '��'=>"FU"   , '��'=>"KY"   , '��'=>'KE'   , '��'=>"GI" , '��'=>"KI" , 
    '��'=>"KA"   , '��'=>"HI"   , '��'=>'OU'   , '��'=>"OU" , '��'=>"TO" , 
    "����"=>"NY" , "����"=>"NK" , "����"=>"NG" , 
    "��"=>"UM"   , "ζ"=>"RY"   , "ε"=>"RY"   , 
    '����'=>"TO" , "����"=>"NY" , "����"=>"NK" , 
    "����"=>"NG" , "����"=>"UM" , "����"=>"RY" , 
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
        break if /��λ/ =~ move
        move.sub!(/Ʊ/,"#{prev}")
        move.sub!(/��/,"")
        move.sub!(/��/,"(00)")
        if /^(.)(.)(.+)\((\d+)\)/ =~ move
          csa << "'* %s" % [comment.join("��")] if enable_comment
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
      when /��ꡧ(.+)/
        csa << "N+#$1"
      when /��ꡧ(.+)/
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
