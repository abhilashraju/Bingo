#pragma once
#include "http_utilities.hpp"
namespace bingo{

/** HTTP request method verbs

    Each verb corresponds to a particular method string
    used in HTTP request messages.
*/
enum class verb
{
    /** An unknown method.

        This value indicates that the request method string is not
        one of the recognized verbs. Callers interested in the method
        should use an interface which returns the original string.
    */
    unknown = 0,

    /// The DELETE method deletes the specified resource
    delete_,

    /** The GET method requests a representation of the specified resource.

        Requests using GET should only retrieve data and should have no other effect.
    */
    get,

    /** The HEAD method asks for a response identical to that of a GET request, but without the response body.
    
        This is useful for retrieving meta-information written in response
        headers, without having to transport the entire content.
    */
    head,

    /** The POST method requests that the server accept the entity enclosed in the request as a new subordinate of the web resource identified by the URI.

        The data POSTed might be, for example, an annotation for existing
        resources; a message for a bulletin board, newsgroup, mailing list,
        or comment thread; a block of data that is the result of submitting
        a web form to a data-handling process; or an item to add to a database
    */
    post,

    /** The PUT method requests that the enclosed entity be stored under the supplied URI.

        If the URI refers to an already existing resource, it is modified;
        if the URI does not point to an existing resource, then the server
        can create the resource with that URI.
    */
    put,

    /** The CONNECT method converts the request connection to a transparent TCP/IP tunnel.

        This is usually to facilitate SSL-encrypted communication (HTTPS)
        through an unencrypted HTTP proxy.
    */
    connect,

    /** The OPTIONS method returns the HTTP methods that the server supports for the specified URL.
    
        This can be used to check the functionality of a web server by requesting
        '*' instead of a specific resource.
    */
    options,

    /** The TRACE method echoes the received request so that a client can see what (if any) changes or additions have been made by intermediate servers.
    */
    trace,

    // WebDAV

    copy,
    lock,
    mkcol,
    move,
    propfind,
    proppatch,
    search,
    unlock,
    bind,
    rebind,
    unbind,
    acl,

    // subversion

    report,
    mkactivity,
    checkout,
    merge,

    // upnp

    msearch,
    notify,
    subscribe,
    unsubscribe,

    // RFC-5789

    patch,
    purge,

    // CalDAV

    mkcalendar,

    // RFC-2068, section 19.6.1.2

    link,
    unlink
};

/** Converts a string to the request method verb.

    If the string does not match a known request method,
    @ref verb::unknown is returned.
*/
inline
verb
string_to_verb(std::string_view v){
    /*
    ACL
    BIND
    CHECKOUT
    CONNECT
    COPY
    DELETE
    GET
    HEAD
    LINK
    LOCK
    M-SEARCH
    MERGE
    MKACTIVITY
    MKCALENDAR
    MKCOL
    MOVE
    NOTIFY
    OPTIONS
    PATCH
    POST
    PROPFIND
    PROPPATCH
    PURGE
    PUT
    REBIND
    REPORT
    SEARCH
    SUBSCRIBE
    TRACE
    UNBIND
    UNLINK
    UNLOCK
    UNSUBSCRIBE
*/

    if(v.size() < 3)
        return verb::unknown;
    auto c = v[0];
    v.remove_prefix(1);
    switch(c)
    {
    case 'A':
        if(v == "CL"_sv)
            return verb::acl;
        break;

    case 'B':
        if(v == "IND"_sv)
            return verb::bind;
        break;

    case 'C':
        c = v[0];
        v.remove_prefix(1);
        switch(c)
        {
        case 'H':
            if(v == "ECKOUT"_sv)
                return verb::checkout;
            break;

        case 'O':
            if(v == "NNECT"_sv)
                return verb::connect;
            if(v == "PY"_sv)
                return verb::copy;
            

        default:
            break;
        }
        break;

    case 'D':
        if(v == "ELETE"_sv)
            return verb::delete_;
        break;

    case 'G':
        if(v == "ET"_sv)
            return verb::get;
        break;

    case 'H':
        if(v == "EAD"_sv)
            return verb::head;
        break;

    case 'L':
        if(v == "INK"_sv)
            return verb::link;
        if(v == "OCK"_sv)
            return verb::lock;
        break;

    case 'M':
        c = v[0];
        v.remove_prefix(1);
        switch(c)
        {
        case '-':
            if(v == "SEARCH"_sv)
                return verb::msearch;
            break;

        case 'E':
            if(v == "RGE"_sv)
                return verb::merge;
            break;

        case 'K':
            if(v == "ACTIVITY"_sv)
                return verb::mkactivity;
            if(v[0] == 'C')
            {
                v.remove_prefix(1);
                if(v == "ALENDAR"_sv)
                    return verb::mkcalendar;
                if(v == "OL"_sv)
                    return verb::mkcol;
                break;
            }
            break;

        case 'O':
            if(v == "VE"_sv)
                return verb::move;
            

        default:
            break;
        }
        break;

    case 'N':
        if(v == "OTIFY"_sv)
            return verb::notify;
        break;

    case 'O':
        if(v == "PTIONS"_sv)
            return verb::options;
        break;

    case 'P':
        c = v[0];
        v.remove_prefix(1);
        switch(c)
        {
        case 'A':
            if(v == "TCH"_sv)
                return verb::patch;
            break;

        case 'O':
            if(v == "ST"_sv)
                return verb::post;
            break;

        case 'R':
            if(v == "OPFIND"_sv)
                return verb::propfind;
            if(v == "OPPATCH"_sv)
                return verb::proppatch;
            break;

        case 'U':
            if(v == "RGE"_sv)
                return verb::purge;
            if(v == "T"_sv)
                return verb::put;
            

        default:
            break;
        }
        break;

    case 'R':
        if(v[0] != 'E')
            break;
        v.remove_prefix(1);
        if(v == "BIND"_sv)
            return verb::rebind;
        if(v == "PORT"_sv)
            return verb::report;
        break;

    case 'S':
        if(v == "EARCH"_sv)
            return verb::search;
        if(v == "UBSCRIBE"_sv)
            return verb::subscribe;
        break;

    case 'T':
        if(v == "RACE"_sv)
            return verb::trace;
        break;

    case 'U':
        if(v[0] != 'N')
            break;
        v.remove_prefix(1);
        if(v == "BIND"_sv)
            return verb::unbind;
        if(v == "LINK"_sv)
            return verb::unlink;
        if(v == "LOCK"_sv)
            return verb::unlock;
        if(v == "SUBSCRIBE"_sv)
            return verb::unsubscribe;
        break;

    default:
        break;
    }

    return verb::unknown;
}

/// Returns the text representation of a request method verb.
inline
std::string_view
to_string(verb v){
   
    switch(v)
    {
    case verb::delete_:       return "DELETE"_sv;
    case verb::get:           return "GET"_sv;
    case verb::head:          return "HEAD"_sv;
    case verb::post:          return "POST"_sv;
    case verb::put:           return "PUT"_sv;
    case verb::connect:       return "CONNECT"_sv;
    case verb::options:       return "OPTIONS"_sv;
    case verb::trace:         return "TRACE"_sv;

    case verb::copy:          return "COPY"_sv;
    case verb::lock:          return "LOCK"_sv;
    case verb::mkcol:         return "MKCOL"_sv;
    case verb::move:          return "MOVE"_sv;
    case verb::propfind:      return "PROPFIND"_sv;
    case verb::proppatch:     return "PROPPATCH"_sv;
    case verb::search:        return "SEARCH"_sv;
    case verb::unlock:        return "UNLOCK"_sv;
    case verb::bind:          return "BIND"_sv;
    case verb::rebind:        return "REBIND"_sv;
    case verb::unbind:        return "UNBIND"_sv;
    case verb::acl:           return "ACL"_sv;

    case verb::report:        return "REPORT"_sv;
    case verb::mkactivity:    return "MKACTIVITY"_sv;
    case verb::checkout:      return "CHECKOUT"_sv;
    case verb::merge:         return "MERGE"_sv;

    case verb::msearch:       return "M-SEARCH"_sv;
    case verb::notify:        return "NOTIFY"_sv;
    case verb::subscribe:     return "SUBSCRIBE"_sv;
    case verb::unsubscribe:   return "UNSUBSCRIBE"_sv;

    case verb::patch:         return "PATCH"_sv;
    case verb::purge:         return "PURGE"_sv;

    case verb::mkcalendar:    return "MKCALENDAR"_sv;

    case verb::link:          return "LINK"_sv;
    case verb::unlink:        return "UNLINK"_sv;

    case verb::unknown:
        return "<unknown>"_sv;
    }

    throw std::invalid_argument{"unknown verb"};
}

/// Write the text for a request method verb to an output stream.
inline
std::ostream&
operator<<(std::ostream& os, verb v)
{
    return os << to_string(v);
}

} //bingo

