////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#include "QueryString.h"
#include "Basics/fasthash.h"

using namespace arangodb::aql;
  
void QueryString::append(std::string& out) const {
  if (empty()) {
    return;
  }
  out.append(_data, _length);
}

uint64_t QueryString::hash() {
  if (!_hashed) {
    if (_data == nullptr) {
      return 0;
    }

    _hash =  fasthash64(_data, _length, 0x3123456789abcdef);
    _hashed = true;
  }

  return _hash;
}
    
std::string QueryString::extract(size_t maxLength) const {
  if (_length <= maxLength) {
    // no truncation
    return std::string(_data, _length);
  }

  // query string needs truncation
  size_t length = maxLength;
    
  // do not create invalid UTF-8 sequences
  while (length > 0) {
    uint8_t c = _data[length - 1];
    if ((c & 128) == 0) {
      // single-byte character
      break;
    }
    --length;

    // start of a multi-byte sequence
    if ((c & 192) == 192) {
      // decrease length by one more, so we the string contains the
      // last part of the previous (multi-byte?) sequence
      break;
    }
  }

  std::string result;
  result.reserve(length + 3);
  result.append(_data, length);
  result.append("...", 3);
  return result;
}

/// @brief extract a region from the query
std::string QueryString::extractRegion(int line, int column) const {
  TRI_ASSERT(_data != nullptr);

  // note: line numbers reported by bison/flex start at 1, columns start at 0
  int currentLine = 1;
  int currentColumn = 0;

  char c;
  char const* p = _data;

  while ((static_cast<size_t>(p - _data) < _length) && (c = *p)) {
    if (currentLine > line ||
        (currentLine >= line && currentColumn >= column)) {
      break;
    }

    if (c == '\n') {
      ++p;
      ++currentLine;
      currentColumn = 0;
    } else if (c == '\r') {
      ++p;
      ++currentLine;
      currentColumn = 0;

      // eat a following newline
      if (*p == '\n') {
        ++p;
      }
    } else {
      ++currentColumn;
      ++p;
    }
  }

  // p is pointing at the position in the query the parse error occurred at
  TRI_ASSERT(p >= _data);

  size_t offset = static_cast<size_t>(p - _data);

  static int const SNIPPET_LENGTH = 32;

  if (_length < offset + SNIPPET_LENGTH) {
    // return a copy of the region
    return std::string(_data + offset, _length - offset);
  }

  // copy query part
  std::string result;
  result.reserve(SNIPPET_LENGTH + strlen("..."));
  result.append(_data + offset, SNIPPET_LENGTH);
  result.append("...");

  return result;
}

namespace arangodb {
namespace aql {

std::ostream& operator<<(std::ostream& stream, QueryString const& queryString) {
  if (queryString.empty()) {
    stream << "(empty query)";
  } else {
    stream.write(queryString.data(), queryString.length());
  }
   
  return stream;
}

}
}
