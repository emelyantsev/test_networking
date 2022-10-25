#pragma once

#include <coroutine>
#include <map>

#include "nwhelper.h"

namespace nwkit {

struct Task {

    struct promise_type {

        using Handle = std::coroutine_handle<promise_type>;

        Task get_return_object() { return Task{ Handle::from_promise( *this ) }; }

        std::suspend_never initial_suspend() { return {}; }

        std::suspend_always final_suspend() noexcept { return {}; } // To check done in final suspended state

        void return_void() {}

        void unhandled_exception() {}
    };



    Task() = delete;

    Task( promise_type::Handle coro ) : coro_( coro ) {}

    Task( const Task& ) = delete;

    Task& operator=( const Task& ) = delete;

    Task( Task&& other ) noexcept :
        coro_{ other.coro_ }
    {
        other.coro_ = {};
    }
    Task& operator=( Task&& other ) noexcept
    {
        if( this != &other ) {
            if( coro_ ) {
                coro_.destroy();
            }
            coro_ = other.coro_;
            other.coro_ = {};
        }
        return *this;
    }



    ~Task() { 
        if ( coro_.address() != nullptr && !coro_.done() )
            coro_.destroy(); 
    }

    void resume() { coro_.resume(); }
    bool done() { return coro_.done(); } 

private:

    promise_type::Handle coro_;
};




extern std::map<SOCKET, Task::promise_type::Handle> global_read_sockets;
extern std::map<SOCKET, Task::promise_type::Handle> global_write_sockets;


struct Reader {

    Reader( SOCKET sock_, char * buf_, int len_ ) : socket{ sock_ }, buf(buf_), len(len_) {}

    bool await_ready() { return false; }

    void await_suspend( Task::promise_type::Handle h ) {
        
        global_read_sockets[socket] = h;
    }

    int await_resume() {
    
        return recv( socket, buf, len, 0 );
    }

    SOCKET socket;
    char* buf;
    int len;
};


struct Writer {

    Writer( SOCKET sock_, char* buf_, int len_ ) : socket{ sock_ }, buf( buf_ ), len( len_ ) {}

    bool await_ready() { return false; }

    void await_suspend( Task::promise_type::Handle h )
    {
        global_write_sockets[socket] = h;
    }

    int await_resume()
    {
        return send( socket, buf, len, 0 );
    }

    SOCKET socket;
    char* buf;
    int len;
};


} // namespace nwkit