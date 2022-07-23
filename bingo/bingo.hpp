#include "bingosock.hpp"
#include <unifex/static_thread_pool.hpp>
#include <unifex/then.hpp>
#include <unifex/let_value.hpp>
#include <unifex/on.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/just.hpp>
#include <unifex/async_scope.hpp>
#include <unifex/let_error.hpp>
#include <unifex/just.hpp>

namespace bingo{
    auto make_sender = [](auto&& func) {
        return  unifex::just([](){return 0;})|unifex::then((decltype(func))func);
    };

    auto make_listener(auto address, auto port){
    return unifex::just_from([=]() { 
            sock_stream listener;
            listener.bind({address,port});
            listen(listener); 
            return listener;});
    }
    template<typename Context ,typename Handler>
    struct handle_clients{
    Context& context;
    Handler handler;
    handle_clients(Context& C,Handler h):context(C),handler(std::move(h)){}
    auto get()const{
        return unifex::let_value([&](sock_stream& listener) {
            return make_sender([&,handler=std::move(handler)](auto){
            unifex::async_scope scope;
            while(true){
                    auto newsock=accept(listener);
                    scope.spawn(unifex::on(context.get_scheduler(),handler(std::move(newsock))));     
            }  
                return 0;
            });
        });
    }
    template<typename Sender,typename C,typename H>
    friend auto operator |(Sender&& sender,handle_clients<C,H>&& clihandler ){
        return sender | clihandler.get();
    }
    
    };
    template<typename Context,typename H>
    auto process_clients(Context& where,H handler){
    return handle_clients(where,std::move(handler));
    }
    inline auto error_to_response(std::exception_ptr err) {
        try {
            std::rethrow_exception(err);
        }catch (const std::exception& e) {
            return unifex::just(std::string(e.what()));
        }
    }
    template<typename Func>
    struct upon_error{
        Func callback; 
        upon_error(Func func):callback(std::move(func)){}
        template<typename Sender>
        friend auto operator |(Sender sender, upon_error onerror){
            return sender|unifex::let_error(error_to_response)|unifex::then([onerror=std::move(onerror)](auto v) {return onerror.callback(v);});
        }
    };
    template<typename Handler>
    struct acceptor{
    using Request_Handler = Handler;
    Request_Handler request_handler;
    acceptor(Request_Handler handler):request_handler(std::move(handler)){}
    auto operator()(auto newsock)const{
            auto newclient=unifex::just(new sock_stream(std::move(newsock)));
            auto responder=unifex::let_value([=](auto& newsock){
            return make_sender([&](auto){
                std::unique_ptr<sock_stream> sock(newsock);
                try{
                while(true){
                    std::vector<char> vec;vec.reserve(1024);
                    Buffer buffer{vec.data(),vec.capacity()};
                    read(*sock,buffer);
                    send(*sock,request_handler(buffer));
                }   
                }
                catch(std::exception& e){
                    printf("%s",e.what());
                }
            
            });   
            });
            auto session=newclient|responder;
            return session;
    }
    
    
    };
    template<typename T>
    acceptor<T> make_request_handler(T handler){
        return acceptor<T>(handler);
    }
    struct EchoHandler
    {
    Buffer operator()(Buffer buff)const{
        return buff;
    }
    };
}