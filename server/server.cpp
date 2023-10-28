#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <vector>

#include "../utils/utils.h"

const size_t k_max_msg = 4096;

enum {
	STATE_REQ = 0,
	STATE_RES = 1,
	STATE_END = 2,
};

struct Conn {
	int fd = -1;
	uint32_t state = 0;
	//read buffer
	size_t read_buffer_size = 0;
	uint8_t read_buffer[4 + k_max_msg];
	//write buffer
	size_t write_buffer_size = 0;
	size_t write_buffer_sent = 0;
	uint8_t write_buffer[4 + k_max_msg];
};

static void fd_set_non_blocking(int fd) {
	errno = 0;
	int flags = fcntl(fd, F_GETFL, 0);
	if (errno) {
		die("fcntl error");
		return;
	}

	flags |= O_NONBLOCK;

	errno = 0;
	(void) fcntl(fd, F_SETFL, flags);
	if (errno) {
		die("fcntl error flag");
	}
}

static void connection_put(std::vector<Conn *> &clients_fd, struct Conn *connection) {
	if (clients_fd.size() <= (size_t) connection->fd) {
		clients_fd.resize(connection->fd + 1);
	}
	clients_fd[connection->fd] = connection;
}

static int32_t accept_new_connection(std::vector<Conn *> &clients_fd, int server_fd) {
	//accept 
	struct sockaddr_in client_addr = {};
	socklen_t socklen = sizeof(client_addr);
	int connection_fd = accept(server_fd, (struct sockaddr *) &client_addr, &socklen);
	if (connection_fd < 0) {
		msg("accept() error");
		return -1;
	}

	//set the new connection fd to nonblocking mode
	fd_set_non_blocking(connection_fd);
	//creating the struct Conn
	struct Conn *connection = (struct Conn *)malloc(sizeof(struct Conn));
	if (!connection) {
		close(connection_fd);
		return -1;
	}

	connection->fd = connection_fd;
	connection->state = STATE_REQ;
	connection->read_buffer_size = 0;
	connection->write_buffer_size = 0;
	connection->write_buffer_sent = 0;
	connection_put(clients_fd, connection);
	return 0;
}

static int32_t do_request(
    const uint8_t *req, uint32_t reqlen,
    uint32_t *rescode, uint8_t *res, uint32_t *reslen)
{
	return 0;

}
static void read_request(Conn *connection) {
	if (connection->read_buffer_size) {
		// Not enough data to read, wait for the next recursion
		return;
	}

	uint32_t len = 0;
	memcpy(&len, connection->read_buffer, 4);
	if (len > k_max_msg) {
		msg("message too long");
		connection->state = STATE_END;
		return;
	}

	if (len > connection->read_buffer_size - 4) {
		// Not enough data to read, wait for the next recursion
		return;
	}

    uint32_t rescode = 0;
    uint32_t wlen = 0;
    int32_t err = do_request(
        &connection->read_buffer[4], len,
        &rescode, &connection->write_buffer[4 + 4], &wlen
    );
    if (err) {
        connection->state = STATE_END;
        return;
    }

	if (connection->state == STATE_REQ) read_request(connection); //there are multiple commands from one connection
}
static void handle_request(Conn *connection) {
	//try to fill buffer
	assert(connection->read_buffer_size < sizeof(connection->read_buffer));
	ssize_t bytes_read = 0;

	do {
		size_t capacity_remaining = sizeof(connection->read_buffer) - connection->read_buffer_size;
		bytes_read = read(connection->fd, connection->read_buffer, capacity_remaining);
	} while(bytes_read < 0 && bytes_read == EINTR); //try again if there is error and the error is interrupt

	if (bytes_read < 0 && bytes_read == EAGAIN) {
		return;
	}
	
	if (bytes_read < 0) {
		msg("read() error");
		connection->state = STATE_END;
		return;
	}

	if (bytes_read == 0) {
		if (connection->read_buffer_size > 0) {
			msg("EOF unexpected");
		} else {
			msg("EOF");
		}

		connection->state = STATE_END;
		return;
	}

	connection->read_buffer_size += (size_t) bytes_read;
	//parse
	read_request(connection);
	if (connection->state == STATE_REQ) handle_request(connection); //there are multiple request from one connection
}

static void connection_io(Conn *connection) {
	if (connection->state == STATE_REQ) {
		handle_request(connection);
	} else if (connection->state == STATE_RES) {
		handle_respond(connection);
	} else {
		assert(0);
	}
}
int main() {
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		die("socket()");
	}

	int val = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

	struct sockaddr_in server_info = {};
	server_info.sin_family = AF_INET; //TCP
	server_info.sin_port = ntohs(1234); //port number
	server_info.sin_addr.s_addr = ntohl(0); //localhost

	int err = bind(server_fd, (const sockaddr *) &server_info, sizeof(server_info));
	if (err) {
		die("bind()");
	}

	//listen to server
	err = listen(server_fd, SOMAXCONN);
	if (err) {
		die("listen()");
	}
	fd_set_non_blocking(server_fd);

	std::vector<Conn *> clients_fd;
	std::vector<struct pollfd> poll_args;

	while (true) {
		poll_args.clear();

		struct pollfd server_poll = {server_fd, POLLIN, 0};
		poll_args.push_back(server_poll);

		for (Conn *client: clients_fd) {
			if (!client) {
				continue;
			}
			struct pollfd client_poll = {};
			client_poll.fd = client->fd;
			client_poll.events = (client->state == STATE_REQ) ? POLLIN : POLLOUT;
			client_poll.events = client_poll.events | POLLERR;
			poll_args.push_back(client_poll);
		}

		int err = poll(poll_args.data(), (nfds_t) poll_args.size(), 1000);
		if (err < 0) {
			die("poll()");
		}

		for (size_t i = 1; i < poll_args.size(); ++i) {
			if (poll_args[i].revents) {
				Conn *client = clients_fd[poll_args[i].fd];
				connection_io(client);
				if (client->state == STATE_END) {
					clients_fd[client->fd] = NULL;
					(void) close(client->fd);
					free(client);
				}
			}
		}

		if (poll_args[0].revents) {
			(void)accept_new_connection(clients_fd, server_fd);
		}
	}

	return 0;
}
