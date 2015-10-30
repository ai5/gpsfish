/* sjis2euc.cc
 */

#include "osl/misc/sjis2euc.h"
#include <cctype>
#include <cassert>
#include <iostream>

std::string osl::misc::sjis2euc(const std::string& str)
{
  if (str.empty()) 
    return str;

  std::string result;
  result.reserve(str.size());
  size_t index = 0;
  while (index < str.size())
  {
    unsigned char c1 = str[index++];
    if (0xa1 <= c1 && c1 <= 0xdf) // ignore hankaku-kana
      continue;
    if (isascii(c1)) 
    {
      result.push_back(c1);
      continue;
    }
    
    assert(index < str.size());
    if (index >= str.size())
      break;
    unsigned char c2 = str[index++];
    sjis2euc(c1, c2);
    result.push_back(c1);
    result.push_back(c2);
  }
  return result;
}

/**
 * Reference: 
 * http://www.net.is.uec.ac.jp/~ueno/material/kanji/sjis2euc.html
 */
void osl::misc::sjis2euc(unsigned char& c1, unsigned char& c2)
{
  if( c2 < 0x9f )
  {
    if( c1 < 0xa0 )
    {
      c1 -= 0x81;
      c1 *= 2;
      c1 += 0xa1;
    }
    else
    {
      c1 -= 0xe0;
      c1 *= 2;
      c1 += 0xdf;
    }
    if( c2 > 0x7f )
      -- c2;
    c2 += 0x61;
  }
  else
  {
    if( c1 < 0xa0 )
    {
      c1 -= 0x81;
      c1 *= 2;
      c1 += 0xa2;
    }
    else
    {
      c1 -= 0xe0;
      c1 *= 2;
      c1 += 0xe0;
    }
    c2 += 2;
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
