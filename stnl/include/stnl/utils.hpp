#ifndef STNL_UTILS_HPP
#define STNL_UTILS_HPP

#include <string>

namespace STNL {
  class Utils {
    public:
      static std::string StringToLower(const std::string& s);
      static std::string StringToUpper(const std::string &s);

    private:
      Utils();
  };
}

#endif // STNL_UTILS_HPP