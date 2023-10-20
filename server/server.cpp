#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include "utils.h"

static void do_something(int connection_fd) {
	char rbuf[64] = {};
	ssize_t n = read(connection_fd, rbuf, sizeof(rbuf) - 1);
	if (n < 0) {
		msg("read() erorr");
		return;
	}
	std::cout << "client says:" << rbuf << std::endl;

	char wbuf[] = "world";
	write(connection_fd, wbuf, strlen(wbuf));
}
int main() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);

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

	while (true) {
		//accept
		struct sockaddr_in client_addr = {};
		socklen_t socklen = sizeof(client_addr);
		
		int connection_fd = accept(fd, (struct sockaddr *) &client_addr, &socklen);
		if (connection_fd < 0) {
			continue;
		}

		//do_somethin(connection_fd)
		close(connection_fd);

	}
}
