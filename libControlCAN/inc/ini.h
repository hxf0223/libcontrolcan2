/*
Feather INI Parser - 1.41
You are free to use this however you wish.

If you find a bug, please attept to debug the cause.
Post your environment details and the cause or fix in the issues section of
GitHub.

Written by Turbine.

Website:
https://github.com/Turbine1991/feather-ini-parser
http://code.google.com/p/feather-ini-parser/downloads

Help:
Bundled example & readme.
http://code.google.com/p/feather-ini-parser/wiki/Tutorials
*/

#pragma once

#define __STDC_WANT_LIB_EXT1__ 1 // needed by strncpy_s
#include <cstring>
#include <fstream>
#include <ostream>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#define FINI_SAFE
#define FINI_BUFFER_SIZE 256

#if __cplusplus >= 201103L
#  include <unordered_map>
#  define FINI_CPP11
#  define ALLOCATE_SECTIONS 100
#  define ALLOCATE_KEYS 5
#else
#  include <map>
#endif

#ifdef FINI_WIDE_SUPPORT
#  include <wchar.h>

typedef std::wstringstream fini_sstream_t;
typedef std::wstring fini_string_t;
typedef wchar_t fini_char_t;
typedef std::wifstream fini_ifstream_t;
typedef std::wofstream fini_ofstream_t;

#  define fini_strlen(a) wcslen(a)
#  define fini_strncpy(a, b) wcscpy(a, b)
#  define fini_strncpy(a, b, c) wcsncpy(a, b, c)
#  define fini_strtok(a, b) wcstok(a, b)

#  define INITEXT(x) L##x
#else
#  include <cstring>

typedef std::stringstream fini_sstream_t;
typedef std::string fini_string_t;
typedef char fini_char_t;
typedef std::ifstream fini_ifstream_t;
typedef std::ofstream fini_ofstream_t;

#  define fini_strlen(a) strlen(a)
#  define fini_strcpy(a, b) strcpy(a, b)
#  define fini_strncpy(a, b, c) strncpy(a, b, c)
#  define fini_strtok(a, b) strtok(a, b)

#  define INITEXT(x) x
#endif

#define CHAR_SIZE sizeof(fini_char_t)

/// Simple converter using templates and streams to effectively required for the
/// flexibility of handling native types
class converters {
public:
  template <typename T, typename U>
  static T convert(U value);
  template <typename T>
  static void GetLine(fini_sstream_t& out, T& value);
  static void GetLine(fini_sstream_t& out, fini_string_t& value);
  template <typename T>
  static size_t GetDataSize(T& value);
  static size_t GetDataSize(fini_string_t value);
};

///
template <typename T = fini_string_t, typename U = fini_string_t, typename V = fini_string_t>
class Ini {
public:
  typedef T section_t;
  typedef U key_t;
  typedef V value_t;
  typedef Ini<section_t, key_t, value_t> ini_t;

  /// Type definition declarations
#ifdef FINI_CPP11
  typedef typename std::unordered_map<key_t, value_t> keys_t;
  typedef typename std::unordered_map<section_t, keys_t*> sections_t;
#else
  typedef typename std::map<key_t, value_t> keys_t;
  typedef typename std::map<section_t, keys_t*> sections_t;
#endif

  typedef typename keys_t::iterator keysit_t;
  typedef typename sections_t::iterator sectionsit_t;

  typedef typename std::pair<key_t, value_t> keyspair_t;
  typedef typename std::pair<section_t, keys_t*> sectionspair_t;

  typedef char data_t;

  enum source_e { source_file, source_memory };

  /// Data members
  std::string filename;
  data_t* data;
  size_t dataSize;
  keys_t* current;
  sections_t sections;
  source_e source;

  /// Constuctor/Destructor
  // Specify the filename to associate and whether to parse immediately
  Ini(const std::string filename, const bool doParse)
      : filename(filename) {
    init(source_file, doParse);
  }

  // Used for loading INI from memory
  Ini(void* data, const size_t dataSize, bool doParse)
      : data((data_t*)data)
      , dataSize(dataSize) {
    init(source_memory, doParse);
  }

  ~Ini() {
    clear();
  }

  /// Access Content
  // Provide bracket access to section contents
  keys_t& operator[](section_t section) {
#ifdef FINI_SAFE
    if (!sections[section])
      sections[section] = new keys_t;
#endif

    return *sections[section];
  }

  // Create a new section and select it
  bool create(const section_t section) {
    if (select(section))
      return false;

    current = new keys_t;
    sections[section] = current;

    reserveKeys(current);
    return true;
  }

  // Select a section for performing operations
  bool select(const section_t section) {
    sectionsit_t sectionsit = sections.find(section);
    if (sectionsit == sections.end())
      return false;

    current = sectionsit->second;
    return true;
  }

  /// Debug
  friend std::ostream& operator<<(std::ostream& os, const ini_t& ini) {
#ifdef FINI_CPP11
    for (auto i = ini.sections.begin(); i != ini.sections.end(); i++) // typename ini_t::sectionsit_t
    {
      // Section name as ini_t::section_t
      os << '[' << i->first << ']' << std::endl;

      if (i->second->size() == 0) // No keys/values in section, skip to next
        continue;

      for (typename ini_t::keysit_t j = i->second->begin(); j != i->second->end(); j++) {
        // Name as ini_t::key_t & Value as ini_t::key_t
        os << "  " << j->first << "=" << j->second << std::endl;
      }
    }
#else
    std::cout << "Error: FINI requires CPP11 when outputting to stream." << std::endl;
#endif

    return os;
  }

  /// Set
  // Assign a value for key under the selected section
  bool set(const key_t key, const value_t value) {
    if (current == nullptr)
      return false;

    (*current)[key] = value;
    return true;
  }

  template <typename W, typename X>
  bool set(const W key, const X value) {
    return set(converters::convert<key_t>(key), converters::convert<value_t>(value));
  }

  /// Get
  value_t get(const key_t key, value_t def = value_t()) {
    keysit_t it = current->find(key);
    if (current == nullptr || it == current->end())
      return def;

    return it->second;
  }

  value_t get(const section_t section, const key_t key, value_t def) {
    if (!select(section))
      return def;

    return get(key, def);
  }

  template <typename W, typename X>
  X get(const W key, const X def = value_t()) {
    return converters::convert<X>(get(converters::convert<key_t>(key), converters::convert<value_t>(def)));
  }

  template <typename W>
  fini_string_t get(const W key,
                    const fini_char_t* def = INITEXT("")) // Handle C string default value without casting
  {
    return converters::convert<fini_string_t>(get(converters::convert<key_t>(key), converters::convert<value_t>(def)));
  }

  template <typename W, typename X, typename Y>
  Y get(const W section, const X key, const Y def) {
    return converters::convert<Y>(
        get(converters::convert<section_t>(section), converters::convert<key_t>(key), converters::convert<value_t>(def)));
  }

  template <typename W, typename X>
  fini_string_t get(const W section, const X key,
                    const fini_char_t* def) // Handle C string default value without casting
  {
    return converters::convert<fini_string_t>(converters::convert<section_t>(section),
                                              get(converters::convert<key_t>(key), converters::convert<value_t>(def)));
  }

  /// Functions
  void parse(std::istream& file) {
    fini_char_t line[FINI_BUFFER_SIZE];
    bool first = true;
    fini_sstream_t out;

    while (!file.eof()) {
      file.getline(line, FINI_BUFFER_SIZE);

      if (first) {
        first = false;
        if (line[0] == 0xEF) // Allows handling of UTF-16/32 documents
        {
          memmove(line, line + (CHAR_SIZE * 3), CHAR_SIZE * (FINI_BUFFER_SIZE - 3));
          return;
        }
      }

      nake(line);

      if (line[0]) {
        auto const len = fini_strlen(line);
        if (len > 0 &&
            !((len >= 2 && (line[0] == '/' && line[1] == '/')) || (len >= 1 && line[0] == '#'))) // Ignore comment and empty lines
        {
          if (line[0] == '[') // Section
          {
            section_t section;
            size_t length = fini_strlen(line) - 2; // Without section brackets
            while (isspace(line[length + 1]))      // Leave out any additional new
                                                   // line characters, not "spaces"
                                                   // as the name suggests
              --length;

            auto ssection = static_cast<fini_char_t*>(calloc(CHAR_SIZE, length + 1));
            std::strncpy(ssection, line + 1,
                         length); // Count after first bracket

            current = new keys_t;

            out << ssection;
            free(ssection);
            converters::GetLine(out, section);

            sections[section] = current;
          } else // Key
          {
            key_t key;
            value_t value;

            auto const skey = fini_strtok(line, INITEXT("="));
            auto const svalue = fini_strtok(NULL, INITEXT("\n"));
            auto const skey_len = std::char_traits<char>::length(skey);

            if (skey && svalue) {
              size_t index = 0;            // Without section brackets
              while (isspace(skey[index])) // Leave out any additional new line characters,
                                           // not "spaces" as the name suggests
                index++;

              if (index != 0) // Has preceeding white space
                std::strncpy(skey, skey + index, skey_len - index);

              out << skey;
              converters::GetLine(out, key);

              out.clear();
              out.str(fini_string_t());

              out << svalue;
              converters::GetLine(out, value);

              if (value != value_t())
                (*current)[key] = value;
            }
          }

          out.clear();
          out.str(fini_string_t()); // Clear existing stream;
        }
      }
    }
  }

  // Parse an INI's contents into memory from the filename given during
  // construction
  bool parse() {
    switch (source) {
    case source_file: {
      fini_ifstream_t file(filename.c_str());
      if (!file.is_open())
        return false;

      parse(file);
      file.close();
    } break;

    case source_memory: {
      std::stringstream sstream;
      sstream.rdbuf()->pubsetbuf(data, dataSize);
      parse(sstream);
    } break;
    }

    return true;
  }

  bool parseBinary() {
    fini_ifstream_t file(filename.c_str(), std::ios::binary);
    if (!file.is_open())
      return false;

    size_t sectionCount;
    size_t keyCount;
    key_t key;
    value_t value;
    section_t section;

    // file.read((fini_char_t*)&sectionCount, sizeof(sectionCount));
    file >> sectionCount;

    for (size_t i = 0; i < sectionCount; i++) {
      if (i > 0)
        file.seekg(1 + file.tellg());

      file.read(reinterpret_cast<fini_char_t*>(&keyCount), sizeof(keyCount));
      file >> section;

      create(section);

      for (size_t j = 0; j < keyCount; j++) {
        file >> key;
        file >> value;
        set(key, value);
      }
    }

    file.close();

    return true;
  }

  // Clear the contents from memory
  void clear() {
    clean();
    sections.clear();
  }

  /// Output
  // Save from memory into file
  bool save(const std::string filename = "") {
    if (!has_file_association(filename))
      return false;

    fini_ofstream_t file(((filename == "") ? this->filename : filename).c_str(), std::ios::trunc);
    if (!file.is_open())
      return false;

    // Loop through sections
    for (typename Ini::sectionsit_t i = sections.begin(); i != sections.end(); i++) {
      if (i->second->size() == 0) // No keys/values in section, skip to next
        continue;

      // Write section
      const fini_string_t temp = make_section(i->first);
      const fini_char_t* line = temp.c_str();
      file.write(line, fini_strlen(line));

      for (typename Ini::keysit_t j = i->second->begin(); j != i->second->end(); ++j) {
        // Write key and value
        const fini_string_t temp = make_key_value(j->first, j->second);
        auto line = temp.c_str();
        file.write(line, fini_strlen(line));
      }
    }

    file.close();
    return true;
  }

  // Saves it without any conventional INI formatting characters, however it
  // only uses string streams
  bool saveBinary(const std::string filename = "") {
    if (!has_file_association(filename))
      return false;

    fini_ofstream_t file(((filename == "") ? this->filename : filename).c_str(), std::ios::trunc | std::ios::binary);
    if (!file.is_open())
      return false;

    size_t sectionCount = sections.size();

    file.write((fini_char_t*)&sectionCount, sizeof(sectionCount));

    // Loop through sections
    for (typename Ini::sectionsit_t i = sections.begin(); i != sections.end(); i++) {
      size_t key_count = i->second->size();
      file.write((fini_char_t*)&key_count, sizeof(key_count));

      file << i->first << std::endl;

      for (typename Ini::keysit_t j = i->second->begin(); j != i->second->end(); j++) {
        file << j->first << std::endl;
        file << j->second << std::endl;
      }
    }

    file.close();

    return true;
  }

  // Saves it as a true binary file, intended to replace the existing one. Don't
  // bother using it with all strings.
  bool saveBinaryExperimental(const std::string filename = "") {
    if (!has_file_association(filename))
      return false;

    fini_ofstream_t file(((filename == "") ? this->filename : filename).c_str(), std::ios::trunc | std::ios::binary);
    if (!file.is_open())
      return false;

    size_t section_count = sections.size();
    file.write(reinterpret_cast<fini_char_t*>(&section_count), sizeof(section_count));

    // Loop through sections
    for (typename Ini::sectionsit_t i = sections.begin(); i != sections.end(); ++i) {
      size_t key_count = i->second->size();
      file.write(reinterpret_cast<fini_char_t*>(&key_count), sizeof(key_count));
      file.write(static_cast<fini_char_t*>(&i->first), converters::GetDataSize(i->first));

      for (typename Ini::keysit_t j = i->second->begin(); j != i->second->end(); ++j) {
        file.write(static_cast<fini_char_t*>(&j->first), converters::GetDataSize(j->first));
        file.write(static_cast<fini_char_t*>(&j->second), converters::GetDataSize(j->second));
      }
    }

    file.close();

    return true;
  }

  // Alows another INI's contents to be insert into another, with the ability to
  // retain the original values
  void merge(ini_t& other, const bool retainValues = true) {
    for (typename Ini::sectionsit_t i = other.sections.begin(); i != other.sections.end(); i++) {
      if (!select(i->first)) // Create and insert all key values into a missing
                             // section
      {
        keys_t* keys = new keys_t(*i->second);
        sections.insert(std::make_pair(i->first, keys));
      } else {
        for (typename Ini::keysit_t j = i->second->begin(); j != i->second->end(); j++) {
          keysit_t it = current->find(j->first);
          if (it == current->end())
            current->insert(std::make_pair(j->first, j->second));
          else if (!retainValues)
            it->second = j->second;
        }
      }
    }
  }

private:
  /// Functions
  // Init the INI in with values set by constructor
  void init(const source_e source, const bool doParse) {
    this->source = source;

    reserveSections();
    if (doParse)
      parse();
  }

  // Clean the contents for descruction
  void clean() {
    for (sectionsit_t i = sections.begin(); i != sections.end(); i++)
      delete i->second;

    current = NULL;
  }

  // Make any alterations to the raw line
  void nake(const fini_char_t*) // Strip the line of any non-interpretable characters
  {}

  void reserveSections() {
#ifdef FINI_CPP11
    sections.reserve(ALLOCATE_SECTIONS);
#endif
  }

  void reserveKeys(keys_t* current) {
#ifdef FINI_CPP11
    current->reserve(ALLOCATE_KEYS);
#endif
  }

  bool has_file_association(const std::string filename) const {
    if (source == source_memory && filename == "") // No association to a file
      return false;

    return true;
  }

  /// Output
  // Creates a section as a string
  static fini_string_t make_section(const section_t& section) {
    fini_sstream_t line;
    line << '[' << section << ']' << std::endl;

    return line.str();
  }

  // Creates a key and a value as a string
  static fini_string_t make_key_value(const key_t& key, const value_t& value) {
    fini_sstream_t line;
    line << key << '=' << value << std::endl;

    return line.str();
  }
};

/// Definitions
template <typename T, typename U>
inline T converters::convert(U value) {
  fini_sstream_t sout;
  T result;

  sout << value;
  sout >> result;

  sout.str(fini_string_t());
  return result;
}

template <>
inline fini_string_t converters::convert<fini_string_t, fini_string_t>(fini_string_t value) {
  return value;
}

template <>
inline fini_string_t converters::convert<fini_string_t>(const fini_char_t* value) {
  return value;
}

template <typename T>
inline void converters::GetLine(fini_sstream_t& out, T& value) {
  out >> value;
}

inline void converters::GetLine(fini_sstream_t& out, fini_string_t& value) {
  std::getline(out, value);
}

template <typename T>
inline size_t converters::GetDataSize(T& value) {
  return sizeof(value);
}

inline size_t converters::GetDataSize(fini_string_t value) {
  return value.size() + 1;
}
