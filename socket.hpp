#ifndef SOCKET_H
#define SOCKET_H

using std::string;

class Socket {
    private:
    protected:
        struct sockaddr_in info;
        struct sockaddr_in6 info6;
        struct addrinfo addr_info;
        bool isIpv6;
    public:
        int sockfd;
        std::string address;
        Socket(int domain, int type, int protocol);
        bool connect(const char* addr, uint16_t port);
        bool bind(const uint16_t port);
        bool listen(const uint64_t numOfClients);
        Socket* accept();
        bool setsockopt(const int level,const int optname,const void* optval);
        virtual bool send(const std::string msg);
        bool sendto(const std::string msg, const char* addr, const uint16_t port);
        virtual uint64_t recv(char* buffer, size_t size);
        uint64_t recvfrom(char* buffer, size_t size, const char* addr, const uint16_t port);

        // getter setters
        int GetSockfd(void);

        // other socket functions
        bool setBlocking(void);
        bool setNonBlocking(void);
        bool close(void);
        bool shutdown(void);
        int select(std::vector<Socket> *reads, std::vector<Socket> *writes, std::vector<Socket> *exceptions, int timeout);

        ~Socket();
};

string IPfromHostName(const string host);

#endif