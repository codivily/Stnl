#include "stnl/utils.hpp"

#include <algorithm>  // For std::transform
#include <cctype>
#include <string>

namespace STNL {
  
  std::string Utils::StringToLower(std::string& s) {
    std::string tmp = s;
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c) { return std::tolower(c); });
    return tmp;
  }

  std::string Utils::StringToUpper(std::string& s) {
    std::string tmp = s;
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c) { return std::toupper(c); });
    return tmp;
  }
}