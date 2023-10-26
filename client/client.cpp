#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include "../utils/utils.h"

const size_t k_max_msg = 4096;

static int32_t read_full(int fd, char* buf, size_t n) {
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

static int32_t write_all(int fd, char* buf, size_t n) {
	while (n > 0) {
		int rv = write(fd, buf, n);
		if (rv <= 0) {
			return -1;
		}
		assert((size_t) rv <= n);
		n -= (size_t) rv;
		buf += rv;
	}
	return 0;
}

static int32_t send_req(int connection_fd, const char *text) {
	uint32_t len = (uint32_t) strlen(text);
	char write_buffer[len + 4];
	memcpy(write_buffer, &len, 4);
	memcpy(write_buffer + 4, text, len);

	int32_t err = write_all(connection_fd, write_buffer, len + 4);
	if (err) {
		return err;
	}

	return 0;
}

static int32_t read_res(int connection_fd) {
	errno = 0;

	char read_header_buffer[4];
	char read_body_buffer[k_max_msg + 1];
	int32_t err = read_full(connection_fd, read_header_buffer, 4);
	if (err) {
		if (errno == 0) {
			msg("EOF");
		} else {
			msg("read() error");
		}
		return err;
	}
	
	uint32_t len;
	memcpy(&len, read_header_buffer, 4);
	if (len >= k_max_msg) {
		msg("Message too long");
		return -1;
	}

	err = read_full(connection_fd, read_body_buffer, len);
	if (err) {
		msg("read() error");
		return err;
	}
	read_body_buffer[len] = '\0';
	std::cout << "server says: " << read_body_buffer << std::endl;

	return 0;
}

int main() {
	int connection_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connection_fd < 0) {
		die("socket()");
	}

	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); //localhost
	
	int rv = connect(connection_fd, (const struct sockaddr *) &addr, sizeof(addr));
	if (rv) {
		die("connect()");
	}

	const char *query_list[3] = {"hello1", "hello2", "hello3"};
	for (size_t i = 0; i < 3; ++i) {
		int32_t err = send_req(connection_fd, query_list[i]);
		if (err) {
			close(connection_fd);
			return 0;
		}
	}

	for (size_t i = 0; i < 3; ++i) {
		int32_t err = read_res(connection_fd);
		if (err) {
			close(connection_fd);
			return 0;
		}
	}
	close(connection_fd);
}
