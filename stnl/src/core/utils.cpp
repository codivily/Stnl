#include "stnl/core/utils.hpp"

#include <algorithm>  // For std::transform
#include <cctype>
#include <sstream>
#include <string>
#include <limits>
#include <vector>

namespace STNL
{

  // Helper function to trim whitespace from the left of a string
  static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  }

  // Helper function to trim whitespace from the right of a string
  static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
  }

  // Helper function to trim both ends
  static inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
  }

  std::string Utils::StringToLower(const std::string_view s) {
    std::string tmp{s};
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c) { return std::tolower(c); });
    return tmp;
  }

  std::string Utils::StringToUpper(const std::string_view s) {
    std::string tmp{s};
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c) { return std::toupper(c); });
    return tmp;
  }

  std::string Utils::TrimLeft(const std::string_view s) {
    std::string tmp{s};
    ltrim(tmp);
    return tmp;
  }
  
  std::string Utils::TrimRight(const std::string_view s) {
    std::string tmp{s};
    rtrim(tmp);
    return tmp;
  }

  std::string Utils::Trim(const std::string_view s) {
    std::string tmp{s};
    trim(tmp);
    return tmp;
  }

  std::string Utils::Join(std::vector<std::string> const& parts, std::string const& separator) {
    std::stringstream ss;
    bool bFirst = true;
    for(const auto& s : parts) {
      if (!bFirst) { ss << separator; }
      else { bFirst = false; }
      ss << s;
    }
    return ss.str();
  }

  std::string Utils::FixIndent(const std::string_view s) {
        std::stringstream ss{std::string(s)};
        std::string line;  
        // Determine the minimum common indentation
        size_t minIndent = std::numeric_limits<size_t>::max(); 
        while (std::getline(ss, line)) {
            size_t currentIndent = line.find_first_not_of(" \t");
            if (currentIndent != std::string::npos && currentIndent < minIndent) { minIndent = currentIndent; }
        }
        // Handle the case where the input was empty or only whitespace
        if (minIndent == std::numeric_limits<size_t>::max()) { return ""; }
        
        // Second Pass: Apply the fix and build the new string.
        // rewind stream for second pass
        ss.clear();
        ss.seekg(0);

        std::stringstream output;
        bool firstLine = true;
        while (std::getline(ss, line)) {
            size_t currentIndent = line.find_first_not_of(" \t");

            // Append newline separator before every line except the first
            if (!firstLine) { output << "\n"; }
            firstLine = false;

            if (currentIndent == std::string::npos) {
                // Line is empty or pure whitespace. We append an empty string 
                // to effectively keep the line count but remove any remaining
                // whitespace from that line.
                output << ""; 
            } else {
                // If the line is shorter than minIndent (shouldn't happen with correct logic, 
                // but substr handles it gracefully), or just remove the common indent.
                output << line.substr(minIndent);
            }
        }
        // Trim any leading or trailing newlines that might have resulted from 
        // empty lines at the start or end of the input (not strictly required by the prompt, 
        // but often desired for clean output).
        std::string finalOutput = output.str();
        trim(finalOutput);
        return finalOutput;
    }
}
