#pragma once
#include "http_fields.hpp"
#include "http_verb.hpp"
#include "http_status.hpp"
namespace bingo {
template <bool isRequest, class Fields = fields> class header;

template <class Fields>
class header<true, Fields> : public Fields

{
public:
  //   static_assert(is_fields<Fields>::value, "Fields type requirements not
  //   met");

  using is_request = std::true_type;

  /// The type representing the fields.
  using fields_type = Fields;

  /// Constructor
  header() = default;

  /// Constructor
  header(header &&) = default;

  /// Constructor
  header(header const &) = default;

  /// Assignment
  header &operator=(header &&) = default;

  /// Assignment
  header &operator=(header const &) = default;

  /** Return the HTTP-version.

      This holds both the major and minor version numbers,
      using these formulas:
      @code
          unsigned major = version / 10;
          unsigned minor = version % 10;
      @endcode

      Newly constructed headers will use HTTP/1.1 by default.
  */
  unsigned version() const noexcept { return version_; }

  /** Set the HTTP-version.

      This holds both the major and minor version numbers,
      using these formulas:
      @code
          unsigned major = version / 10;
          unsigned minor = version % 10;
      @endcode

      Newly constructed headers will use HTTP/1.1 by default.

      @param value The version number to use
  */
  void version(unsigned value) noexcept { version_ = value; }

  /** Return the request-method verb.

      If the request-method is not one of the recognized verbs,
      @ref verb::unknown is returned. Callers may use @ref method_string
      to retrieve the exact text.

      @note This function is only available when `isRequest == true`.

      @see method_string
  */
  verb method() const;

  /** Set the request-method.

      This function will set the method for requests to a known verb.

      @param v The request method verb to set.
      This may not be @ref verb::unknown.

      @throws std::invalid_argument when `v == verb::unknown`.

      @note This function is only available when `isRequest == true`.
  */
  void method(verb v);

  /** Return the request-method as a string.

      @note This function is only available when `isRequest == true`.

      @see method
  */
  std::string_view method_string() const;

  /** Set the request-method.

      This function will set the request-method a known verb
      if the string matches, otherwise it will store a copy of
      the passed string.

      @param s A string representing the request-method.

      @note This function is only available when `isRequest == true`.
  */
  void method_string(std::string_view s);

  /** Returns the request-target string.

      The request target string returned is the same string which
      was received from the network or stored. In particular, it will
      contain url-encoded characters and should follow the syntax
      rules for URIs used with HTTP.

      @note This function is only available when `isRequest == true`.
  */
  std::string_view target() const;

  /** Set the request-target string.

      It is the caller's responsibility to ensure that the request
      target string follows the syntax rules for URIs used with
      HTTP. In particular, reserved or special characters must be
      url-encoded. The implementation does not perform syntax checking
      on the passed string.

      @param s A string representing the request-target.

      @note This function is only available when `isRequest == true`.
  */
  void target(std::string_view s);

  // VFALCO Don't rearrange these declarations or
  //        ifdefs, or else the documentation will break.

  /** Constructor

      @param args Arguments forwarded to the `Fields`
      base class constructor.

      @note This constructor participates in overload
      resolution if and only if the first parameter is
      not convertible to @ref header, @ref verb, or
      @ref status.
  */
  template <
      class Arg1, class... ArgN,
      class = typename std::enable_if<
          !std::is_convertible<typename std::decay<Arg1>::type,
                               header>::value &&
          !std::is_convertible<typename std::decay<Arg1>::type, verb>::value &&
          !std::is_convertible<typename std::decay<Arg1>::type,
                               status>::value>::type>
  explicit header(Arg1 &&arg1, ArgN &&... argn);

private:
  template <bool, class, class> friend class message;

  template <class T> friend void swap(header<true, T> &m1, header<true, T> &m2);

  template <class... FieldsArgs>
  header(verb method, std::string_view target_, unsigned version_value,
         FieldsArgs &&... fields_args)
      : Fields(std::forward<FieldsArgs>(fields_args)...), method_(method) {
    version(version_value);
    target(target_);
  }

  unsigned version_ = 11;
  verb method_ = verb::unknown;
};

/** A container for an HTTP request or response header.

    A `header` includes the start-line and header-fields.
*/
template <class Fields> class header<false, Fields> : public Fields {
public:
  //   static_assert(is_fields<Fields>::value, "Fields type requirements not
  //   met");

  /// Indicates if the header is a request or response.
  using is_request = std::false_type;

  /// The type representing the fields.
  using fields_type = Fields;

  /// Constructor.
  header() = default;

  /// Constructor
  header(header &&) = default;

  /// Constructor
  header(header const &) = default;

  /// Assignment
  header &operator=(header &&) = default;

  /// Assignment
  header &operator=(header const &) = default;

  /** Constructor

      @param args Arguments forwarded to the `Fields`
      base class constructor.

      @note This constructor participates in overload
      resolution if and only if the first parameter is
      not convertible to @ref header, @ref verb, or
      @ref status.
  */
  template <
      class Arg1, class... ArgN,
      class = typename std::enable_if<
          !std::is_convertible<typename std::decay<Arg1>::type,
                               header>::value &&
          !std::is_convertible<typename std::decay<Arg1>::type, verb>::value &&
          !std::is_convertible<typename std::decay<Arg1>::type,
                               status>::value>::type>
  explicit header(Arg1 &&arg1, ArgN &&... argn);

  /** Return the HTTP-version.

      This holds both the major and minor version numbers,
      using these formulas:
      @code
          unsigned major = version / 10;
          unsigned minor = version % 10;
      @endcode

      Newly constructed headers will use HTTP/1.1 by default.
  */
  unsigned version() const noexcept { return version_; }

  /** Set the HTTP-version.

      This holds both the major and minor version numbers,
      using these formulas:
      @code
          unsigned major = version / 10;
          unsigned minor = version % 10;
      @endcode

      Newly constructed headers will use HTTP/1.1 by default.

      @param value The version number to use
  */
  void version(unsigned value) noexcept { version_ = value; }

  /** The response status-code result.

      If the actual status code is not a known code, this
      function returns @ref status::unknown. Use @ref result_int
      to return the raw status code as a number.

      @note This member is only available when `isRequest == false`.
  */
  status result() const;

  /** Set the response status-code.

      @param v The code to set.

      @note This member is only available when `isRequest == false`.
  */
  void result(status v);

  /** Set the response status-code as an integer.

      This sets the status code to the exact number passed in.
      If the number does not correspond to one of the known
      status codes, the function @ref result will return
      @ref status::unknown. Use @ref result_int to obtain the
      original raw status-code.

      @param v The status-code integer to set.

      @throws std::invalid_argument if `v > 999`.
  */
  void result(unsigned v);

  /** The response status-code expressed as an integer.

      This returns the raw status code as an integer, even
      when that code is not in the list of known status codes.

      @note This member is only available when `isRequest == false`.
  */
  unsigned result_int() const;

  /** Return the response reason-phrase.

      The reason-phrase is obsolete as of rfc7230.

      @note This function is only available when `isRequest == false`.
  */
  std::string_view reason() const;

  /** Set the response reason-phrase (deprecated)

      This function sets a custom reason-phrase to a copy of
      the string passed in. Normally it is not necessary to set
      the reason phrase on an outgoing response object; the
      implementation will automatically use the standard reason
      text for the corresponding status code.

      To clear a previously set custom phrase, pass an empty
      string. This will restore the default standard reason text
      based on the status code used when serializing.

      The reason-phrase is obsolete as of rfc7230.

      @param s The string to use for the reason-phrase.

      @note This function is only available when `isRequest == false`.
  */
  void reason(std::string_view s);

private:
  template <bool, class, class> friend class message;

  template <class T>
  friend void swap(header<false, T> &m1, header<false, T> &m2);

  template <class... FieldsArgs>
  header(status result, unsigned version_value, FieldsArgs &&... fields_args)
      : Fields(std::forward<FieldsArgs>(fields_args)...), result_(result) {
    version(version_value);
  }

  unsigned version_ = 11;
  status result_ = status::ok;
};

/// A typical HTTP request header
template <class Fields = fields> using request_header = header<true, Fields>;

/// A typical HTTP response header
template <class Fields = fields> using response_header = header<false, Fields>;
} // namespace bingo
