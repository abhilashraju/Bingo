#pragma once
namespace bingo
{
enum class status : unsigned
{
    /** An unknown status-code.

        This value indicates that the value for the status code
        is not in the list of commonly recognized status codes.
        Callers interested in the exactly value should use the
        interface which provides the raw integer.
    */
    unknown = 0,

    continue_ = 100,

    /** Switching Protocols

        This status indicates that a request to switch to a new
        protocol was accepted and applied by the server. A successful
        response to a WebSocket Upgrade HTTP request will have this
        code.
    */
    switching_protocols = 101,

    processing = 102,

    ok = 200,
    created = 201,
    accepted = 202,
    non_authoritative_information = 203,
    no_content = 204,
    reset_content = 205,
    partial_content = 206,
    multi_status = 207,
    already_reported = 208,
    im_used = 226,

    multiple_choices = 300,
    moved_permanently = 301,
    found = 302,
    see_other = 303,
    not_modified = 304,
    use_proxy = 305,
    temporary_redirect = 307,
    permanent_redirect = 308,

    bad_request = 400,
    unauthorized = 401,
    payment_required = 402,
    forbidden = 403,
    not_found = 404,
    method_not_allowed = 405,
    not_acceptable = 406,
    proxy_authentication_required = 407,
    request_timeout = 408,
    conflict = 409,
    gone = 410,
    length_required = 411,
    precondition_failed = 412,
    payload_too_large = 413,
    uri_too_long = 414,
    unsupported_media_type = 415,
    range_not_satisfiable = 416,
    expectation_failed = 417,
    misdirected_request = 421,
    unprocessable_entity = 422,
    locked = 423,
    failed_dependency = 424,
    upgrade_required = 426,
    precondition_required = 428,
    too_many_requests = 429,
    request_header_fields_too_large = 431,
    connection_closed_without_response = 444,
    unavailable_for_legal_reasons = 451,
    client_closed_request = 499,

    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503,
    gateway_timeout = 504,
    http_version_not_supported = 505,
    variant_also_negotiates = 506,
    insufficient_storage = 507,
    loop_detected = 508,
    not_extended = 510,
    network_authentication_required = 511,
    network_connect_timeout_error = 599
};

/** Represents the class of a status-code.
 */
enum class status_class : unsigned
{
    /// Unknown status-class
    unknown = 0,

    /// The request was received, continuing processing.
    informational = 1,

    /// The request was successfully received, understood, and accepted.
    successful = 2,

    /// Further action needs to be taken in order to complete the request.
    redirection = 3,

    /// The request contains bad syntax or cannot be fulfilled.
    client_error = 4,

    /// The server failed to fulfill an apparently valid request.
    server_error = 5,
};

/** Converts the integer to a known status-code.

    If the integer does not match a known status code,
    @ref status::unknown is returned.
*/
inline status int_to_status(unsigned v)
{
    switch (static_cast<status>(v))
    {
    // 1xx
    case status::continue_:
    case status::switching_protocols:
    case status::processing:
        __attribute__((fallthrough));

    // 2xx
    case status::ok:
    case status::created:
    case status::accepted:
    case status::non_authoritative_information:
    case status::no_content:
    case status::reset_content:
    case status::partial_content:
    case status::multi_status:
    case status::already_reported:
    case status::im_used:
        __attribute__((fallthrough));

    // 3xx
    case status::multiple_choices:
    case status::moved_permanently:
    case status::found:
    case status::see_other:
    case status::not_modified:
    case status::use_proxy:
    case status::temporary_redirect:
    case status::permanent_redirect:
        __attribute__((fallthrough));

    // 4xx
    case status::bad_request:
    case status::unauthorized:
    case status::payment_required:
    case status::forbidden:
    case status::not_found:
    case status::method_not_allowed:
    case status::not_acceptable:
    case status::proxy_authentication_required:
    case status::request_timeout:
    case status::conflict:
    case status::gone:
    case status::length_required:
    case status::precondition_failed:
    case status::payload_too_large:
    case status::uri_too_long:
    case status::unsupported_media_type:
    case status::range_not_satisfiable:
    case status::expectation_failed:
    case status::misdirected_request:
    case status::unprocessable_entity:
    case status::locked:
    case status::failed_dependency:
    case status::upgrade_required:
    case status::precondition_required:
    case status::too_many_requests:
    case status::request_header_fields_too_large:
    case status::connection_closed_without_response:
    case status::unavailable_for_legal_reasons:
    case status::client_closed_request:
        __attribute__((fallthrough));

    // 5xx
    case status::internal_server_error:
    case status::not_implemented:
    case status::bad_gateway:
    case status::service_unavailable:
    case status::gateway_timeout:
    case status::http_version_not_supported:
    case status::variant_also_negotiates:
    case status::insufficient_storage:
    case status::loop_detected:
    case status::not_extended:
    case status::network_authentication_required:
    case status::network_connect_timeout_error:
        return static_cast<status>(v);

    default:
        break;
    }
    return status::unknown;
}

/** Convert an integer to a status_class.

    @param v The integer representing a status code.

    @return The status class. If the integer does not match
    a known status class, @ref status_class::unknown is returned.
*/
inline status_class to_status_class(unsigned v)
{
    switch (v / 100)
    {
    case 1:
        return status_class::informational;
    case 2:
        return status_class::successful;
    case 3:
        return status_class::redirection;
    case 4:
        return status_class::client_error;
    case 5:
        return status_class::server_error;
    default:
        break;
    }
    return status_class::unknown;
}

/** Convert a status_code to a status_class.

    @param v The status code to convert.

    @return The status class.
*/
inline status_class to_status_class(status v)
{
    return to_status_class(static_cast<int>(v));
}

/** Returns the obsolete reason-phrase text for a status code.

    @param v The status code to use.
*/
inline std::string_view obsolete_reason(status v)
{
    switch (static_cast<status>(v))
    {
    // 1xx
    case status::continue_:
        return "Continue";
    case status::switching_protocols:
        return "Switching Protocols";
    case status::processing:
        return "Processing";

    // 2xx
    case status::ok:
        return "OK";
    case status::created:
        return "Created";
    case status::accepted:
        return "Accepted";
    case status::non_authoritative_information:
        return "Non-Authoritative Information";
    case status::no_content:
        return "No Content";
    case status::reset_content:
        return "Reset Content";
    case status::partial_content:
        return "Partial Content";
    case status::multi_status:
        return "Multi-Status";
    case status::already_reported:
        return "Already Reported";
    case status::im_used:
        return "IM Used";

    // 3xx
    case status::multiple_choices:
        return "Multiple Choices";
    case status::moved_permanently:
        return "Moved Permanently";
    case status::found:
        return "Found";
    case status::see_other:
        return "See Other";
    case status::not_modified:
        return "Not Modified";
    case status::use_proxy:
        return "Use Proxy";
    case status::temporary_redirect:
        return "Temporary Redirect";
    case status::permanent_redirect:
        return "Permanent Redirect";

    // 4xx
    case status::bad_request:
        return "Bad Request";
    case status::unauthorized:
        return "Unauthorized";
    case status::payment_required:
        return "Payment Required";
    case status::forbidden:
        return "Forbidden";
    case status::not_found:
        return "Not Found";
    case status::method_not_allowed:
        return "Method Not Allowed";
    case status::not_acceptable:
        return "Not Acceptable";
    case status::proxy_authentication_required:
        return "Proxy Authentication Required";
    case status::request_timeout:
        return "Request Timeout";
    case status::conflict:
        return "Conflict";
    case status::gone:
        return "Gone";
    case status::length_required:
        return "Length Required";
    case status::precondition_failed:
        return "Precondition Failed";
    case status::payload_too_large:
        return "Payload Too Large";
    case status::uri_too_long:
        return "URI Too Long";
    case status::unsupported_media_type:
        return "Unsupported Media Type";
    case status::range_not_satisfiable:
        return "Range Not Satisfiable";
    case status::expectation_failed:
        return "Expectation Failed";
    case status::misdirected_request:
        return "Misdirected Request";
    case status::unprocessable_entity:
        return "Unprocessable Entity";
    case status::locked:
        return "Locked";
    case status::failed_dependency:
        return "Failed Dependency";
    case status::upgrade_required:
        return "Upgrade Required";
    case status::precondition_required:
        return "Precondition Required";
    case status::too_many_requests:
        return "Too Many Requests";
    case status::request_header_fields_too_large:
        return "Request Header Fields Too Large";
    case status::connection_closed_without_response:
        return "Connection Closed Without Response";
    case status::unavailable_for_legal_reasons:
        return "Unavailable For Legal Reasons";
    case status::client_closed_request:
        return "Client Closed Request";
    // 5xx
    case status::internal_server_error:
        return "Internal Server Error";
    case status::not_implemented:
        return "Not Implemented";
    case status::bad_gateway:
        return "Bad Gateway";
    case status::service_unavailable:
        return "Service Unavailable";
    case status::gateway_timeout:
        return "Gateway Timeout";
    case status::http_version_not_supported:
        return "HTTP Version Not Supported";
    case status::variant_also_negotiates:
        return "Variant Also Negotiates";
    case status::insufficient_storage:
        return "Insufficient Storage";
    case status::loop_detected:
        return "Loop Detected";
    case status::not_extended:
        return "Not Extended";
    case status::network_authentication_required:
        return "Network Authentication Required";
    case status::network_connect_timeout_error:
        return "Network Connect Timeout Error";

    default:
        break;
    }
    return "<unknown-status>";
}

/// Outputs the standard reason phrase of a status code to a stream.
inline std::ostream &operator<<(std::ostream &os, status v)
{
    return os << obsolete_reason(v);
}
} // namespace bingo
