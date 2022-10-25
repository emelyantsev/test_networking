#include "coro_tools.h"

namespace nwkit {

	std::map<SOCKET, Task::promise_type::Handle> global_read_sockets;
	std::map<SOCKET, Task::promise_type::Handle> global_write_sockets;
}