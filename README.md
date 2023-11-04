# Redis Clone from Scratch in C++

This project is an implementation of a Redis-like in-memory data store using C++. It was created based on a tutorial, serving as a practical exercise to gain hands-on experience in building a key-value store with features inspired by Redis.

## Features

This Redis-inspired project, implemented from scratch in C++, offers the following key features:

- [x] **Basic Redis Commands:** Supports essential Redis commands such as `GET`, `SET`, and `DEL` for key-value operations.

- [x] **Networking:** Utilizes sockets for establishing server-client communication, with a focus on non-blocking I/O.

- [x] **Protocol Parsing:** Implements a Redis-like protocol for parsing and formatting commands and responses.

- [x] **Event Loop:** Utilizes an event loop for managing multiple client connections and non-blocking I/O.

- [x] **Data Storage:** Utilizes hashtables for efficient key-value data storage.

- [x] **Data Serialization:** Implements data serialization for storing and retrieving data in a serialized format.

- [x] **Sorted Sets:** Incorporates an AVL tree for managing sorted sets efficiently.

- [ ] **Timers:** Enhances the event loop with timers for time-based operations.

- [ ] **Time-to-Live (TTL):** Manages key expiration using a heap data structure to handle TTL.

- [ ] **Thread Pool:** Utilizes a thread pool for asynchronous task handling and improved server performance.

## Getting Started

To run and test this Redis implementation, follow these steps:

1. Clone the repository to your local machine:

   ```bash
   git clone https://github.com/sinasun/redis-from-scratch-cpp
   ```

   Build the Utils:

   ```bashe
   cd redis-from-scratch-cpp/utils
   make
   ```

   Build and run the server:

   ```bashe
   cd ../server
   make
   ./server
   ```

   Build and test client in another terminal:

   ```bashe
   cd redis-from-scratch-cpp/client
   make
   ./client SET mykey myvalue
   ./client GET mykey
   ```

## Contributing

Contributions are welcome! Feel free to open issues or submit pull requests to improve the project.

## License

This project is licensed under the MIT - see the [LICENSE.md](LICENSE.md) file for details.

---

## Acknowledgments

- The project is based on a book by [James Smith](https://www.amazon.com/dp/B0BT2CT8XY), which provided valuable insights into building a Redis-like system in C++.
