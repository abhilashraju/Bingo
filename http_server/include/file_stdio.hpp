#pragma once
#include <stdio.h>
#include "file_base.hpp"
namespace bingo {
class file_stdio {
  FILE *f_ = nullptr;

public:
  /** The type of the underlying file handle.

      This is platform-specific.
  */
  using native_handle_type = FILE *;

  /** Destructor

      If the file is open it is first closed.
  */
  inline ~file_stdio();

  /** Constructor

      There is no open file initially.
  */
  file_stdio() = default;

  /** Constructor

      The moved-from object behaves as if default constructed.
  */
  inline file_stdio(file_stdio &&other);

  /** Assignment

      The moved-from object behaves as if default constructed.
  */
  inline file_stdio &operator=(file_stdio &&other);

  /// Returns the native handle associated with the file.
  FILE *native_handle() const { return f_; }

  /** Set the native handle associated with the file.

      If the file is open it is first closed.

      @param f The native file handle to assign.
  */
  inline void native_handle(FILE *f);

  /// Returns `true` if the file is open
  bool is_open() const { return f_ != nullptr; }

  /** Close the file if open

      @param ec Set to the error, if any occurred.
  */
  inline void close();

  /** Open a file at the given path with the specified mode

      @param path The utf-8 encoded path to the file

      @param mode The file mode to use

      @param ec Set to the error, if any occurred
  */
  inline void open(char const *path, file_mode mode);

  /** Return the size of the open file

      @param ec Set to the error, if any occurred

      @return The size in bytes
  */
  inline std::uint64_t size() const;

  /** Return the current position in the open file

      @param ec Set to the error, if any occurred

      @return The offset in bytes from the beginning of the file
  */
  inline std::uint64_t pos() const;

  /** Adjust the current position in the open file

      @param offset The offset in bytes from the beginning of the file

      @param ec Set to the error, if any occurred
  */
  inline void seek(std::uint64_t offset);

  /** Read from the open file

      @param buffer The buffer for storing the result of the read

      @param n The number of bytes to read

      @param ec Set to the error, if any occurred
  */
  inline std::size_t read(void *buffer, std::size_t n) const;

  /** Write to the open file

      @param buffer The buffer holding the data to write

      @param n The number of bytes to write

      @param ec Set to the error, if any occurred
  */
  inline std::size_t write(void const *buffer, std::size_t n);
};
#include "file_stdio.ipp"
} // namespace bingo