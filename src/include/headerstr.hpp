#pragma once

#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <functional>

namespace godgun {
  namespace headerstr {
    struct ci_compare {
      bool operator() (const unsigned char& c1, const unsigned char& c2) const {
        return tolower(c1) < tolower(c2);
      }
    };

    inline bool ci_equal(const std::string& str1, const std::string& str2) {
      return std::lexicographical_compare(str1.begin(), str1.end(),
                                          str2.begin(), str2.end(),
                                          ci_compare());
    }

    struct ci_less {
      bool operator() (const std::string& str1, const std::string& str2) const {
        return ci_equal(str1, str2);
      }
    };

    using HeaderMap = std::map<std::string, std::string, ci_less> ;
  }
}
