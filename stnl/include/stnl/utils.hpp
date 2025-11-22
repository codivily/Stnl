#ifndef STNL_UTILS_HPP
#define STNL_UTILS_HPP

#include <string>

namespace STNL {
  class Utils {
    public:
      static std::string StringToLower(const std::string_view s);
      static std::string StringToUpper(const std::string_view s);
      static std::string TrimLeft(const std::string_view s);
      static std::string TrimRight(const std::string_view s);
      static std::string Trim(const std::string_view s);
      static std::string FixIndent(const std::string_view s);
    private:
      Utils();
  };
}

#endif // STNL_UTILS_HPP