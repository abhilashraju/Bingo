
template<class Fields>
template<class Arg1, class... ArgN, class>
header<true, Fields>::
header(Arg1&& arg1, ArgN&&... argn)
    : Fields(std::forward<Arg1>(arg1),
        std::forward<ArgN>(argn)...)
{
}

template<class Fields>
verb
header<true, Fields>::
method() const
{
    return method_;
}

template<class Fields>
void
header<true, Fields>::
method(verb v)
{
    if(v == verb::unknown)
        throw(
            std::invalid_argument{"unknown method"});
    method_ = v;
    this->set_method_impl({});
}

template<class Fields>
std::string_view
header<true, Fields>::
method_string() const
{
    if(method_ != verb::unknown)
        return to_string(method_);
    return this->get_method_impl();
}

template<class Fields>
void
header<true, Fields>::
method_string(std::string_view s)
{
    method_ = string_to_verb(s);
    if(method_ != verb::unknown)
        this->set_method_impl({});
    else
        this->set_method_impl(s);
}

template<class Fields>
std::string_view
header<true, Fields>::
target() const
{
    return this->get_target_impl();
}

template<class Fields>
void
header<true, Fields>::
target(std::string_view s)
{
    this->set_target_impl(s);
}

template<class Fields>
void
swap(
    header<true, Fields>& h1,
    header<true, Fields>& h2)
{
    using std::swap;
    swap(
        static_cast<Fields&>(h1),
        static_cast<Fields&>(h2));
    swap(h1.version_, h2.version_);
    swap(h1.method_, h2.method_);
}

//------------------------------------------------------------------------------

template<class Fields>
template<class Arg1, class... ArgN, class>
header<false, Fields>::
header(Arg1&& arg1, ArgN&&... argn)
    : Fields(std::forward<Arg1>(arg1),
        std::forward<ArgN>(argn)...)
{
}

template<class Fields>
status
header<false, Fields>::
result() const
{
    return int_to_status(
        static_cast<int>(result_));
}

template<class Fields>
void
header<false, Fields>::
result(status v)
{
    result_ = v;
}

template<class Fields>
void
header<false, Fields>::
result(unsigned v)
{
    if(v > 999)
        throw(
            std::invalid_argument{
                "invalid status-code"});
    result_ = static_cast<status>(v);
}

template<class Fields>
unsigned
header<false, Fields>::
result_int() const
{
    return static_cast<unsigned>(result_);
}


template<class Fields>
std::string_view
header<false, Fields>::
reason() const
{
    // auto const s = this->get_reason_impl();
    // if(! s.empty())
    //     return s;
    return obsolete_reason(result_);
}

template<class Fields>
void
header<false, Fields>::
reason(std::string_view s)
{
    this->set_reason_impl(s);
}

template<class Fields>
void
swap(
    header<false, Fields>& h1,
    header<false, Fields>& h2)
{
    using std::swap;
    swap(
        static_cast<Fields&>(h1),
        static_cast<Fields&>(h2));
    swap(h1.version_, h2.version_);
    swap(h1.result_, h2.result_);
}
