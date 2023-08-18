#ifndef HYDROGEN_ERROR
#define HYDROGEN_ERROR

#include <iostream>
#include <optional>
#include <sstream>

namespace err 
{

inline std::string caret_display(size_t pos, size_t width = 1,
                                 size_t offset = 0) {
  std::stringstream output;
  if (width == 1) {
    output << std::string(pos + offset + 1, ' ') << '^';
  } else {
    output << std::string(pos - width + offset + 1, ' ')
           << std::string(width, '^');
  }
  return output.str();
};

inline std::string caret_to_line(const std::string &str, size_t pos,
                                 size_t line = 1, size_t width = 1) {
  std::stringstream output;
  size_t line_start = 0;
  for (size_t i = 0; i < pos; i++) {
    if (str.at(i) == '\n') {
      line++;
      line_start = i + 1;
    }
  }
  std::string line_str = std::to_string(line) + ": ";
  output << line_str
         << str.substr(line_start, str.find('\n', line_start) - line_start)
         << std::endl;
  output << caret_display(pos - line_start - 1, width, line_str.size());
  return output.str();
};

}


#endif
