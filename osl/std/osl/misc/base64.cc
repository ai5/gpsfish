#include "osl/misc/base64.h"
#include "osl/book/compactBoard.h"
#include <sstream>
std::string osl::misc::
base64Encode(boost::dynamic_bitset<> src)
{
  if (src.empty())
    return "";

  const size_t bits_to_add = 6 - src.size()%6;
  if (bits_to_add < 6)
  {
    for (size_t i=0; i<bits_to_add; ++i)
    {
      src.push_back(0ul); // this appends to the most significant bit
      src <<= 1; // Instead, append to the least significant bit
    }
  }
  assert(src.size()%6 == 0);
  assert(src.size()/6 > 0);

  std::vector<char> dst(src.size()/6, 0);
  const boost::dynamic_bitset<> mask(src.size(), 63ul);
  for (size_t i=0; i<dst.size(); ++i)
  {
    const unsigned long c = ((src >> i*6) & mask).to_ulong();
    assert (c <= 63);
    if (/*0 <= c &&*/ c <= 25)   // A..Z
      dst[dst.size()-1-i] = static_cast<char>(c+65);
    else if (26 <= c && c <= 51) // a..z 
      dst[dst.size()-1-i] = static_cast<char>(c+97-26);
    else if (52 <= c && c <= 61) // 0..9
      dst[dst.size()-1-i] = static_cast<char>(c+48-52);
    else if (c == 62)
      dst[dst.size()-1-i] = '-'; // for URL instread of '+'
    else if (c == 63)
      dst[dst.size()-1-i] = '_'; // for URL instread of '/'
    else
    {
      assert(false);
      return "";
    }
  }

  const size_t char_to_add = 4 - dst.size()%4;
  if (char_to_add < 4)
  {
    for (size_t i=0; i<char_to_add; ++i)
      dst.push_back('=');
  }

  return std::string(dst.begin(), dst.end());
}

boost::dynamic_bitset<> osl::misc::
base64Decode(std::string src)
{
  if (src.empty() || src.size()%4 != 0)
    return boost::dynamic_bitset<>(0);

  {
    int count = 0;
    while (src[src.size()-1] == '=')
    {
      src.erase(src.end()-1);
      ++count;
    }
    if (count >= 4)
      return boost::dynamic_bitset<>(0);
  }

  const size_t dst_size = src.size()*6;
  const size_t redundant = dst_size%8;
  boost::dynamic_bitset<> dst(dst_size, 0ul);
  for (char c: src)
  {
    unsigned long tmp = 0;
    if (48 <= c && c <= 48+9)       // 0..9
      tmp = c -48+52;
    else if (65 <= c && c <= 65+25) // A..Z
      tmp = c - 65;
    else if (97 <= c && c <= 97+25) // a..z
      tmp = c -97+26;
    else if (c == '-')
      tmp = 62;
    else if (c == '_')
      tmp = 63;
    else
    {
      assert(false);
      return boost::dynamic_bitset<>(0);
    }
    assert(/*0 <= tmp &&*/ tmp <= 63);
    const boost::dynamic_bitset<> mask(dst_size, tmp);
    dst = (dst << 6) | mask;
  }
  if (redundant > 0)
  {
    dst >>= redundant;
    dst.resize(dst.size()-redundant);
  }
  return dst;
}


std::string osl::misc::
toBase64(const book::CompactBoard& board) 
{
  const static size_t ninteger = 41;
  const static size_t integer_size = 32;
  const static size_t size = ninteger*integer_size;

  std::stringstream ss;
  ss << board;

  ss.clear();
  ss.seekg(0, std::ios::beg);

  boost::dynamic_bitset<> bits(size);

  for (size_t i = 0; i < ninteger; ++i)
  {
    const unsigned int tmp = static_cast<unsigned int>(book::readInt(ss));
    const boost::dynamic_bitset<> mask(size, static_cast<unsigned long>(tmp));
    bits = (bits << integer_size) | mask;
  }

  return misc::base64Encode(bits);
}

osl::book::CompactBoard osl::misc::
toCompactBoard(const std::string& str)
{
  const boost::dynamic_bitset<> bits = misc::base64Decode(str);
  std::stringstream ss;
  assert(bits.size()%32 == 0);
  const boost::dynamic_bitset<> mask(bits.size(), 4294967295ul);
  for (size_t i=0; i<bits.size()/32; ++i)
  {
    const unsigned long tmp = ((bits >> ((bits.size()/32-1-i)*32)) & mask).to_ulong();
    book::writeInt(ss, static_cast<int>(tmp));
  }
      
  ss.clear();
  ss.seekg(0, std::ios::beg);

  book::CompactBoard cb;
  ss >> cb;
  return cb;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
