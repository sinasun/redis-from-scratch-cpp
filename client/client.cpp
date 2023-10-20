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

static int32_t query(int connection_fd, const char* body) {
	uint32_t len = (uint32_t) strlen(body);
	char write_buffer[len + 4];
	memcpy(write_buffer, &len, 4);
	memcpy(write_buffer + 4, body, len);

	int32_t err = write_all(connection_fd, write_buffer, len + 4);
	if (err) {
		return err;
	}

	//read
	errno = 0;

	char read_header_buffer[4];
	char read_body_buffer[k_max_msg + 1];
	err = read_full(connection_fd, read_header_buffer, 4);
	if (err) {
		if (errno == 0) {
			msg("EOF");
		} else {
			msg("read() error");
		}
		return err;
	}

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
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		die("socket()");
	}

	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); //localhost
	
	int rv = connect(fd, (const struct sockaddr *) &addr, sizeof(addr));
	if (rv) {
		die("connect()");
	}

	query(fd, "hello1");
	query(fd, "hello2");
	close(fd);
}
