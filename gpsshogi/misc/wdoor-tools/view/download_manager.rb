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
require 'erb'
require 'open-uri'

class DownloadManager
  class ForbiddenError < Exception; end

  WHITE_LIST = %w(
    ^http://localhost/
    ^http://.*\.ac\.jp/
    ^http://gps\.tanaka\.ecc\.u-tokyo\.ac\.jp/
    ^http://wdoor\.c\.u-tokyo\.ac\.jp/
    ^http://.*\.shogi\.or\.jp/
    ^http://.*\.hokkaido-np\.co\.jp/
    ^http://mynavi-open\.jp/
    ^http://.*\.kobe-np\.co\.jp/
    ^http://event\.nishinippon\.co\.jp/
    ^http://.*.*topics.*or.*jp/
    ^http://www.*chunichi.*co.*jp/
  )
#    ^http://www.geocities.jp/

  def initialize(url)
    @url = url.lines.first.gsub("<", "&lt;").gsub(">", "&gt;");
  end

  def h_url
    return ERB::Util::h @url
  end

  def white_listed?
    return WHITE_LIST.any? do |str|
      Regexp.new(str).match(@url)
    end
  end

  def is_csa?
    return /\.csa$/ =~ @url || /viewvc\.cgi\/.*\.csa\?/ =~ @url
  end

  def is_kifu?
    return /\.kif$/ =~ @url 
  end

  def download_contents
    # Optimize for local files
    if %r!^http://wdoor\.c\.u-tokyo\.ac\.jp/(shogi/LATEST/\d{4}/\d{2}/\d{2}/[\w@\.\+\-]+\.csa)$! =~ @url
      file = File.join("/var/www", $1)
      kifu = open(file) {|f| f.read} 
      return kifu
    else
      kifu = open(@url, "Accept" => "text/plain") do |f|
        f.read
      end
      return kifu
    end
  end

  def kifu_download
    unless white_listed?
      raise ForbiddenError
    end

    kifu = ""
    if is_csa? || is_kifu?
      kifu = download_contents
    else
      raise ForbiddenError
    end
    return kifu
  rescue
    raise ForbiddenError
  end
end
