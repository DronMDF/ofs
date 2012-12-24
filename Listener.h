
#pragma once

#include <string>

class Content;

class Listener {
public:
	Listener(const Content *content);

	void loop();

private:
	void setSocketNonblock(int sock) const;
	bool addSocketToEpoll(int epollfd, int sock) const;
	int createListenSocket() const;

	const Content *content;
};

