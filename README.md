# webserv

Developed by **[Afonso Coutinho](https://github.com/afonsopc)** and **[Paulo Cordeiro](https://github.com/pvcordeiro)**.

## About the Project

Webserv is a HTTP server implementation written in C++ 98, developed as part of the [42](https://www.42lisboa.com) curriculum.

The project explores the fundamentals of the HTTP protocol, socket programming, and non-blocking I/O operations. It is designed to be resilient, non-blocking, and compatible with standard web browsers.

### Features

*   **HTTP/1.1 Compliant:** Supports common methods like GET, POST, and DELETE.
*   **Configuration:** Fully configurable via a JSON-like configuration file (ports, hosts, error pages, routes).
*   **CGI Support:** Executes scripts in Python, PHP, and Ruby.
*   **Static File Serving:** Serves HTML, CSS, images, and other static assets.
*   **Custom Error Pages:** Handles HTTP errors with customizable HTML pages.
*   **Non-blocking I/O:** Uses `epoll` for efficient connection handling.

## How to Compile

To compile the server, ensure you have a C++ compiler and `make` installed. Run:

```bash
make
```

## Usage

To start the server, provide a configuration file as an argument:

```bash
./webserv [config_file]
```

If no argument is provided, it will try to load `server.json` by default.

```bash
./webserv server.json
```
