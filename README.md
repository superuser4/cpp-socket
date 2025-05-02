# Socket Abstraction C++
This is a socket abstraction library for C++

# Installation
Just clone the repo and add it to your project
```bash
git clone https://github.com/supercat7/cpp-socket.git
```

# Usage
```C++
// Create a socket
Socket *sock = new Socket(AF_INET, SOCK_STREAM, 0);
if (!sock) die();

```

# Examples
```C++
// Bind the socket
sock->bind(8080);

// Connect the socket
sock->connect("127.0.0.1", 8080);

// Listen
sock->listen(1000);

// Accept a client
Socket *cli = sock->accept()

// Print client address info
std::cout << cli->address << std::endl;

// setsockopt
sock->setsockopt(SOL_SOCKET, SO_REUSEADDR, (void*)1);

// Send a message (TCP)
sock->send("Hey\n");

// Send a message (UDP)
sock->sendto("Hey\n", "127.0.0.1", 8080);

// Receive a message (TCP)
char* buf = malloc(4096);
sock->recv(buf, 4096);

// Receive a message (UDP)
char* buf = malloc(4096);
sock->recvfrom(buf, 4096, "127.0.0.1", 8080);

// Set non-blocking
sock->setNonBlocking();

// Set blocking
sock->setBlocking();

// Select on sockets
vector<Socket> reads;
vector<Socket> writes;
vector<Socket> exceptions;
int timeout = 2;

if (sock->select(reads, writes, exceptions, timeout) < 1) {
   continue;
} else {
   // handle socket data
}

// Shutdown the socket
sock->shutdown();

// Close the socket
sock->close();

// Hostname to IP translation
string ip = IPfromhostname("www.google.com");
```
