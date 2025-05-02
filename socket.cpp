#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <stdint.h>
#include <errno.h>
#include <vector>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string>

#include "socket.hpp"

using std::string;

Socket::Socket(int domain, int type, int protocol) {
    if (domain == AF_INET6)
    {
        this->isIpv6 = true;
        this->info6.sin6_family = AF_INET6;
        this->sockfd = socket(AF_INET6, type, protocol);
    } else {
        this->isIpv6 = false;
        this->info.sin_family = domain;
        this->sockfd = socket(domain, type, protocol);
    }
}

bool Socket::connect(const char* addr, uint16_t port) {
    int status;
    string myaddr = addr;
    if ((isdigit(addr[0])) == 0) // hostname to IP
    {
        string IP = IPfromHostName(myaddr);
        myaddr = IP;
    }
    if (!this->isIpv6)
    {
        this->info.sin_port = htons(port);
        this->info.sin_addr.s_addr = inet_addr(myaddr.c_str());
        status = ::connect(this->sockfd, (struct sockaddr *)&this->info, sizeof(this->info));
    } else if (this->isIpv6) {
        this->info6.sin6_port = htons(port);
        memcpy(this->info6.sin6_addr.s6_addr, (void*)inet_addr(addr), sizeof(this->info6.sin6_addr.s6_addr));
        status = ::connect(this->sockfd, (struct sockaddr *)&this->info6, sizeof(this->info6));
    }
    if (status < 0)
    {
        return false;
    }
    return true;
}

bool Socket::bind(const uint16_t port) {
    int status;
    std::string portString = std::to_string(port);

    if (!this->isIpv6)
    {
        this->info.sin_port = htons(port);
        this->info.sin_addr.s_addr = INADDR_ANY;
        status = ::bind(this->sockfd, (struct sockaddr *) &this->info, sizeof(this->info));
    } else if (this->isIpv6) {
        this->info6.sin6_port = htons(port);
        status = ::bind(this->sockfd, (struct sockaddr *) &this->info6, sizeof(this->info6));
    }
    if (status < 0)
    {
        return false;
    }

    struct addrinfo *res;
    addr_info.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo("127.0.0.1", portString.c_str(), &addr_info, &res)) != 0)
    {
        return false;
    }
    addr_info.ai_addrlen = res->ai_addrlen;
    addr_info.ai_addr = res->ai_addr;
    freeaddrinfo(res);
    return true;
}

bool Socket::listen(const uint64_t numOfClients) {
    int status = ::listen(this->sockfd, numOfClients);
    if (status < 0)
    {
        return false;
    }
    return true;
}

Socket* Socket::accept() {
    struct sockaddr_in their_addr;
    socklen_t addrlen = sizeof(their_addr);
    int client = ::accept(this->sockfd, (struct sockaddr *)&their_addr, &addrlen);
    
    if (client < 0)
    {
        return NULL;
    }
    Socket* newSocket = new Socket(addr_info.ai_family, addr_info.ai_socktype, addr_info.ai_protocol);
    newSocket->sockfd = client;
    char host[NI_MAXHOST];
    memset(&host, '\0', sizeof(host));
    getnameinfo((struct sockaddr *)&their_addr, sizeof(their_addr), host, sizeof(host), NULL, 0, NI_NUMERICHOST);
    newSocket->address = host;
    return newSocket;
}

bool Socket::setsockopt(const int level, 
                        const int optname, 
                        const void* optval) {
    if (::setsockopt(this->sockfd,level,optname,&optval,sizeof(optval)) == -1) {
        return false;
    }
    return true;
}
bool Socket::send(const std::string msg) {
    int status = ::send(this->sockfd, msg.c_str(), strlen(msg.c_str()), 0);
    if (status == -1)
    {
        return false;
    }
    return true;
}
bool Socket::sendto(const std::string msg,
                    const char* addr,
                    const uint16_t port)
{   
    string myaddr = addr;
    if (isdigit(addr[0]) == 0) // hostname to IP
    {
        string IP = IPfromHostName(myaddr);
        myaddr = IP;
    }
    struct sockaddr_in to;
    to.sin_family = info.sin_family;
    to.sin_port = htons(port);
    to.sin_addr.s_addr = inet_addr(myaddr.c_str());
    
    int status = ::sendto(this->sockfd, msg.c_str(), strlen(msg.c_str()), 0, (struct sockaddr *)&to, sizeof(to));
    if (status == -1) {
        return false;
    }
    return true;
}
uint64_t Socket::recv(char *buffer, size_t size) {
    int status = ::recv(this->sockfd, buffer, size, 0);
    return status;
}
uint64_t Socket::recvfrom(char* buffer,
                        size_t size,
                        const char* addr,
                        const uint16_t port) {
    string myaddr = addr;
    if (!isdigit(addr[0])) // hostname to IP
    {
        string IP = IPfromHostName(myaddr);
        myaddr = IP;
    }
    struct sockaddr_in from;
    from.sin_family = info.sin_family;
    from.sin_port = htons(port);
    from.sin_addr.s_addr = inet_addr(myaddr.c_str());

    int status = ::recvfrom(this->sockfd, buffer, size, MSG_WAITALL, (struct sockaddr *)&from, (socklen_t*)sizeof(from));
    return status;
}

int Socket::GetSockfd(void) {
    return this->sockfd;
}

bool Socket::close(void) {
    int status = ::close(this->sockfd);
    if (status != 0)
    {
        return false;
    }
    return true;
}

bool Socket::setBlocking(void)
{
    int_fast32_t status = fcntl(this->sockfd, F_GETFL, NULL);
    if (status < 0)
    {
        return false;
    }
    status &= (~O_NONBLOCK);
    status = fcntl(this->sockfd, F_SETFL, status);
    if (status < 0)
    {
        return false;
    }
    return true;
}

bool Socket::setNonBlocking(void)
{
    int_fast32_t status = fcntl(this->sockfd, F_GETFL, NULL);
    if (status < 0)
    {
        return false;
    }
    status |= O_NONBLOCK;
    status = fcntl(this->sockfd, F_SETFL, status);
    if (status < 0)
    {
        return false;
    }
    return true;
}
bool Socket::shutdown(void)
{
    int status = ::shutdown(this->sockfd, SHUT_RDWR);
    if(status <0)
    {
        return false;
    }
    return true;
}

int Socket::select(std::vector<Socket> *reads,
                   std::vector<Socket> *writes,
                   std::vector<Socket> *exceptions,
                   int timeout)
{
    // Got from: https://github.com/KMakowsky/Socket.cpp/blob/master/Socket.cpp
    //int id = reads->at(0).sockfd;
    struct timeval tv;
    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&writefds);

    int maxSock = 0;
    if (reads != NULL)
    {
        for (uint64_t i=0; i < reads->size(); i++)
        {
            int sockInt = reads->at(i).sockfd;
            if (sockInt > maxSock)
            {
                maxSock = sockInt;
            }
            FD_SET(sockInt, &readfds);
        }
    }
    if (writes != NULL)
    {
        for (uint64_t i=0; i < writes->size(); i++)
        {
            int sockInt = writes->at(i).sockfd;
            if (sockInt > maxSock)
            {
                maxSock = sockInt;
            }
            FD_SET(sockInt, &writefds);
        }
    }
    if (exceptions != NULL)
    {
        for (uint64_t i=0; i < exceptions->size(); i++)
        {
            int sockInt = exceptions->at(i).sockfd;
            if (sockInt > maxSock)
            {
                maxSock = sockInt;
            }
            FD_SET(sockInt, &exceptfds);
        }
    }
    int result = ::select(maxSock+1, &readfds, &writefds, &exceptfds, &tv);

    // if (result < 0)
    // {
    //     std::cerr << "select error: " << gai_strerror(errno) << std::endl;
    // }
    if (reads != NULL)
    {
        for(auto i = (int)reads->size() - 1; i >= 0;i--)
        {
            if (!FD_ISSET(reads->at(i).sockfd, &readfds))
            {
                reads->erase(reads->begin() +i);
            }
        }
    }
    if (writes != NULL)
    {
        for(auto i = (int)writes->size() - 1; i >= 0;i--)
        {
            if (!FD_ISSET(writes->at(i).sockfd, &writefds))
            {
                writes->erase(writes->begin() +i);
            }
        }
    }
    if (exceptions != NULL)
    {
        for(auto i = (int)exceptions->size() - 1; i >= 0;i--)
        {
            if (!FD_ISSET(exceptions->at(i).sockfd, &exceptfds))
            {
                exceptions->erase(exceptions->begin() +i);
            }
        }
    }
    return result;
}

Socket::~Socket()
{   
    shutdown();
    close();
}

string IPfromHostName(const string host) {
    hostent* hostname = gethostbyname(host.c_str());

    if (hostname)
    {
        return string(inet_ntoa(**(in_addr**)hostname->h_addr_list));
    }
    return NULL;
}
