class Point
  def initialize(x, y)
    @x = x
    @y = y
  end

  def to_s
    return "(#{x}, #{y})"
  end

  def ==(a)
    return a.x == x && a.y == y
  end

  attr_reader :x, :y
  attr_writer :x, :y
end

class Line
  def initialize(start_point, end_point)
    @start_point = start_point
    @end_point = end_point
  end

  def to_s
    return "#{@start_point}---#{@end_point}"
  end

  attr_reader :start_point, :end_point
end

