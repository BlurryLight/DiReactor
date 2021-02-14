//
// Created by BlurryLight on 2021/2/11.
//

#pragma once
#include <cstddef>
#include <cstdint>
struct sockaddr_in;
namespace PD {
class Socket;
// from little endian to big endian
uint64_t host_to_network64(uint64_t host64);
uint32_t host_to_network32(uint32_t host32);
uint16_t host_to_network16(uint16_t host16);

uint64_t net_to_host64(uint64_t net64);
uint32_t net_to_host32(uint64_t net32);
uint16_t net_to_host16(uint64_t net16);

// abort when error
Socket createNonblocking();
void bindOrDie(int sockfd, const struct sockaddr_in &addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in *addr);
void close(int sockfd);

void toHostPort(char *buf, size_t size, const struct sockaddr_in &addr);
void fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr);

struct sockaddr_in getLocalAddr(int sockfd);
} // namespace PD