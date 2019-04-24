/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#ifndef __ROO_OTHER_INSANE_BIND_H__
#define __ROO_OTHER_INSANE_BIND_H__

// 对于一些服务来说，即使不需要绑定端口，但是为了同ZooKeeper合作提供自己的实例
// 名字，还是需要使用到一个端口的

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

namespace roo {

class InsaneBind {
public:

    InsaneBind() :
        port_(0),
        socketfd_(0) {
        do_internal_bind();
    }

    explicit InsaneBind(uint16_t port) :
        port_(port),
        socketfd_(0) {
        do_internal_bind();
    }

    ~InsaneBind() {
        if(socketfd_ > 0) {
            close(socketfd_);
            socketfd_ = 0;
        }
    }

    uint16_t port() {
        return port_;
    }
    
    // 禁止拷贝
    InsaneBind(const InsaneBind&) = delete;
    InsaneBind& operator=(const InsaneBind&) = delete;

private:

    void do_internal_bind() {

        if( (socketfd_ = ::socket(AF_INET, SOCK_STREAM, 0)) < 0)
            return;
        
        int flag = 1;
        ::setsockopt(socketfd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

        struct sockaddr_in srvaddr {};
        srvaddr.sin_family = AF_INET;
        srvaddr.sin_port = port_ > 0 ? htons(port_) : 0;
        srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if(::bind(socketfd_, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr_in)) < 0)
            return;

        struct sockaddr_in r_addr {};
        socklen_t len = sizeof(r_addr);
        if(::getsockname(socketfd_, (struct sockaddr *)&r_addr, &len) < 0)
            return;

        // return port check here
        if(port_ != 0 && port_ != ntohs(r_addr.sin_port)) {
            port_ = 0;
        } else {
            port_ = ntohs(r_addr.sin_port);
        }
    }

    uint16_t port_;
    int socketfd_;
};

} // end namespace roo


#endif // __ROO_OTHER_INSANE_BIND_H__