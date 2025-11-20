#ifndef STNL_UTILS_HPP
#define STNL_UTILS_HPP

#include <string>

namespace STNL {
  class Utils {
    public:
      static std::string StringToLower(std::string& s);
      static std::string StringToUpper(std::string &s);

    private:
      Utils();
  };
}

#endif // STNL_UTILS_HPP