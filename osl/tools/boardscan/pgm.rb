#! /usr/bin/env ruby

class Pgm
  def initialize(width, height, data)
    @width = width
    @height = height
    @data = data
  end

  def get(x, y)
    return @data[y * @width + x]
  end

  def to_s()
    return "P5\n" + "#{width} #{height}\n" + "255\n" + @data
  end

  def sub(x, y, width, height)
    data = ""
    y.upto(y + height - 1) {|j|
      x.upto(x + width - 1) {|i|
	data << get(i, j)
      }
    }
    return Pgm.new(width, height, data)
  end

  def Pgm.create(file)
    pnm = nil
    File.open(file) {|f|
      if (f.readline.chomp != 'P5')
	puts "Unsupported file"
	exit 1
      end
      width, height = f.readline.chomp.split
      if (f.readline.chomp != '255')
	puts "Unknown depth"
	exit 1
      end

      width = width.to_i
      height = height.to_i
      data = f.read(width * height)

      pnm = Pgm.new(width, height, data)
    }
    return pnm
  end

  def rotate()
    PgmRotated.new(@width, @height, @data)
  end

  attr_reader :width, :height
end

class PgmRotated < Pgm
  def initialize(width, height, data)
    super(width, height, data)
  end

  def get(x, y)
    super(@width - x - 1, @height - y - 1)
  end
end

# 1200 x 1500
# width = 1200
# height = 1500
# puts 'P5'
# puts "#{width} #{height}"
# puts 255
# 0.upto(height - 1){|y|
#   0.upto(width - 1) {|x|
#     printf "%c", pnm.get(x, y)
#   }
# }
