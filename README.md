# Redis Clone from Scratch in C++

## Features

This Redis-inspired project, implemented from scratch in C++, offers the following key features:

- **Basic Redis Commands:** Supports essential Redis commands such as `GET`, `SET`, and `DEL` for key-value operations.

- **Networking:** Utilizes sockets for establishing server-client communication, with a focus on non-blocking I/O.

- **Protocol Parsing:** Implements a Redis-like protocol for parsing and formatting commands and responses.

- **Event Loop:** Utilizes an event loop for managing multiple client connections and non-blocking I/O.

- **Data Storage:** Utilizes hashtables for efficient key-value data storage.

- **Data Serialization:** Implements data serialization for storing and retrieving data in a serialized format.

- **Sorted Sets:** Incorporates an AVL tree for managing sorted sets efficiently.

- **Timers:** Enhances the event loop with timers for time-based operations.

- **Time-to-Live (TTL):** Manages key expiration using a heap data structure to handle TTL.

- **Thread Pool:** Utilizes a thread pool for asynchronous task handling and improved server performance.

---

This project aims to provide a basic Redis-like key-value store with key features while also serving as a learning resource for building such systems from the ground up.

