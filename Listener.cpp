
#include "Listener.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>
#include <vector>
#include <fcntl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "Worker.h"

using namespace std;
using namespace std::placeholders;

Listener::Listener(const Content *content)
	: content(content)
{
}

void Listener::setSocketNonblock(int sock) const
{
	const int flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

bool Listener::addSocketToEpoll(int epollfd, int sock) const
{
	epoll_event ev = { EPOLLIN | EPOLLET | EPOLLOUT, { } };
	ev.data.fd = sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1) {
		return false;
	}
	return true;
}

int Listener::createListenSocket() const
{
	int lsock = socket(AF_INET, SOCK_STREAM, 0);
	if (lsock == -1) {
		throw runtime_error(string("Cannot create socket: ") + strerror(errno));
	}

	setSocketNonblock(lsock);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(7777);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (::bind(lsock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
		close(lsock);
		throw runtime_error(string("Cannot bind socket") + strerror(errno));
	}

	if (listen(lsock, 1000) == -1) {
		close(lsock);
		throw runtime_error(string("Cannot listen socket: ") + strerror(errno));
	}

	cout << "Server listen at port 7777, welcome." << endl;
	return lsock;
}

void Listener::loop()
{
	int epollfd = epoll_create(1000);
	if (epollfd == -1) {
		throw runtime_error(string("epoll_create failed: ") + strerror(errno));
	}

	int lsock = createListenSocket();
	if (!addSocketToEpoll(epollfd, lsock)) {
		throw runtime_error(string("epoll_add failed") + strerror(errno));
	}

	vector<epoll_event> events(1000);
	map<int, Worker> workers;
	while (true) {
		int event_count = epoll_wait(epollfd, &events[0], events.size(), -1);
		if (event_count == -1) {
			throw runtime_error(string("epoll  failed: ") + strerror(errno));
		}

		for (int i = 0; i < event_count; i++) {
			const auto &e = events[i];

			if ((e.events & (EPOLLERR | EPOLLHUP)) != 0) {
	      			close(e.data.fd);
				workers.erase(e.data.fd);
			      	continue;
			}

			if (e.data.fd == lsock) {
				while (true) {
					const int nfd = accept(lsock, 0, 0);
					if (nfd == -1) {
						if (errno != EAGAIN) {
							cout << "accept was failed: " << strerror(errno) << endl;
						}
						break;
					}
	
					setSocketNonblock(nfd);
					if (addSocketToEpoll(epollfd, nfd)) {
						workers.insert(make_pair(nfd, Worker(content)));
					} else {
						close(nfd);
					}
				}
				continue;
			}

			auto wi = workers.find(e.data.fd);
			if (wi == workers.end()) {
				continue;
			}

			if ((e.events & EPOLLIN) != 0) {
				if (!wi->second.read(bind(read, e.data.fd, _1, _2))) {
					close(e.data.fd);
					workers.erase(e.data.fd);
					continue;
				}
			}

			if ((e.events & EPOLLOUT) != 0) {
				if (!wi->second.write(bind(write, e.data.fd, _1, _2))) {
					close(e.data.fd);
					workers.erase(e.data.fd);
				}
			}
		}
	}
}

