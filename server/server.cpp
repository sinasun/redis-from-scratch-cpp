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
	size_t rbuf_size = 0;
	//read buffer
	uint8_t read_header_buffer[4];
	uint8_t read_body_buffer[k_max_msg];
	//write buffer
	size_t write_buffer_size = 0;
	size_t write_buffer_sent = 0;
	uint8_t write_buffer[4 + k_max_msg];
};

static int32_t read_full(int fd, char *buf, size_t n) {
	while (n > 0) {
		ssize_t rv = read(fd, buf, n);
		if (rv <= 0) {
			return -1;
		}
		assert((size_t) rv <= n);
		n -= (size_t) rv;
		buf += rv;
	}
	return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
	while (n > 0) {
		ssize_t rv = write(fd, buf, n);
		if (rv <= 0) {
			return -1;
		}
		assert((size_t) rv <= n);
		n -= (size_t) rv;
		buf += rv;
	}

	return 0;
}

static int32_t one_request(int connection_fd) {
	char read_header_buffer[4];
	char read_body_buffer[k_max_msg + 1];
	errno = 0;

	int32_t err = read_full(connection_fd, read_header_buffer, 4);
	if (err) {
		if (errno == 0) {
			msg("EOF");		
		} else {
			msg("read() error");
		}
		return err;
	}
	
	uint32_t len = 0;
	memcpy(&len, read_header_buffer, 4);
	if (len >= k_max_msg) {
		msg("Message too long");
		return -1;
	}

	err = read_full(connection_fd, read_body_buffer, len);
	if (err) {
		msg("read() message");
		return err;
	}

	read_body_buffer[len] = '\0';
	std::cout << "client says: " << read_body_buffer << std::endl;


	//write
	const char* reply = "world";
	char write_buffer[4 + strlen(reply)];
	
	len = (uint32_t) strlen(reply);
	memcpy(write_buffer, &len, 4);
	memcpy(write_buffer + 4, reply, len);

	return write_all(connection_fd, write_buffer, len + 4);
}

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

int main() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		die("socket()");
	}

	int val = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(0);

	int rv = bind(fd, (const sockaddr *) &addr, sizeof(addr));
	if (rv) {
		die("bind()");
	}

	//listen
	rv = listen(fd, SOMAXCONN);
	if (rv) {
		die("listen()");
	}
	fd_set_non_blocking(fd);

	std::vector<Conn *> clients_fd;
	std::vector<struct pollfd> poll_args;

	while (true) {
		poll_args.clear();

		struct pollfd pfd = {fd, POLLIN, 0};
		poll_args.push_back(pfd);

		for (Conn *client: clients_fd) {
			if (!client) {
				continue;
			}
			struct pollfd pfd = {};
			pfd.fd = client -> fd;
			pfd.events = (client -> state == STATE_REQ) ? POLLIN : POLLOUT;
			pfd.events = pfd.events | POLLERR;
			poll_args.push_back(pfd);
		}

		int rv = poll(poll_args.data(), (nfds_t) poll_args.size(), 1000);
		if (rv < 0) {
			die("poll()");
		}

		for (size_t i = 1; i < poll_args.size(); ++i) {
			if (poll_args[i].revents) {
				Conn *client = clients_fd[poll_args[i].fd];
				connection_io(client);
				if (client -> state == STATE_END) {
					clients_fd[client->fd] = NULL;
					(void) close(client->fd);
					free(client);
				}
			}
		}

		if (poll_args[0].revents) {
			(void)accept_new_connection(clients_fd, fd);
		}
	}

	return 0;
}
