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
#include <vector>

#include "../utils/utils.h"

const size_t k_max_msg = 4096;

static int32_t read_full(int fd, char* buf, size_t n) {
	while (n > 0) {
		ssize_t bytes_read = read(fd, buf, n);
		if (bytes_read <= 0) {
			return -1;
		}
		assert((size_t) bytes_read <= n);
		n -= (size_t) bytes_read;
		buf += bytes_read;
	}
	return 0;
}

static int32_t write_all(int fd, char* buf, size_t n) {
	while (n > 0) {
		int bytes_sent = write(fd, buf, n);
		if (bytes_sent <= 0) {
			return -1;
		}
		assert((size_t) bytes_sent <= n);
		n -= (size_t) bytes_sent;
		buf += bytes_sent;
	}
	return 0;
}

static int32_t send_request(int connection_fd, const std::vector<std::string> &command) {
    uint32_t length = 4;
    for (const std::string &word : command) {
        length += 4 + word.size();
    }
    if (length > k_max_msg) {
        return -1;
    }

    char write_buffer[4 + k_max_msg];
    memcpy(&write_buffer[0], &length, 4);
    uint32_t command_size = command.size();
    memcpy(&write_buffer[4], &command_size, 4);
    size_t cur = 8;
    for (const std::string &word : command) {
        uint32_t word_size = (uint32_t)word.size();
        memcpy(&write_buffer[cur], &word_size, 4);
        memcpy(&write_buffer[cur + 4], word.data(), word.size());
        cur += 4 + word.size();
    }
    return write_all(connection_fd, write_buffer, 4 + length);
}

static int32_t read_respond(int connection_fd) {
    // 4 bytes header
    char read_buffer[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(connection_fd, read_buffer, 4);
    if (err) {
        if (errno == 0) {
            msg("EOF");
        } else {
            msg("read() error");
        }
        return err;
    }

    uint32_t length = 0;
    memcpy(&length, read_buffer, 4);
    if (length > k_max_msg) {
        msg("too long");
        return -1;
    }

    // reply body
    err = read_full(connection_fd, &read_buffer[4], length);
    if (err) {
        msg("read() error");
        return err;
    }

    // print the result
    uint32_t respond_code = 0;
    if (length < 4) {
        msg("bad response");
        return -1;
    }
    memcpy(&respond_code, &read_buffer[4], 4);
    printf("server says: [%u] %.*s\n", respond_code, length - 4, &read_buffer[8]);
    return 0;
}

int main(int argc, char **argv) {
	int connection_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connection_fd < 0) {
		die("socket()");
	}

	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); //localhost
	
	int error = connect(connection_fd, (const struct sockaddr *) &addr, sizeof(addr));
	if (error) {
		die("connect()");
	}

    std::vector<std::string> command;
    for (int i = 1; i < argc; ++i) {
        command.push_back(argv[i]);
    }
    int32_t err = send_request(connection_fd, command);
	if (err) {
		close(connection_fd);
		return 0;
	}
    read_respond(connection_fd);

	close(connection_fd);
	return 0;
}
