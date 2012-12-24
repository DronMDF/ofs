
#pragma once
#include <cstdio>
#include <functional>
#include <string>

class Content;

class Worker {
public:
	Worker(const Content *content);
	Worker(const Worker &worker);

	Worker &operator = (const Worker &worker);

	bool read(std::function<ssize_t (void *, size_t)> reader);
	bool write(std::function<ssize_t (const void *, size_t)> writer);

private:

	enum { INITIAL, REPLY, DATA, END} state;
	std::string request;
	std::string reply;
	bool header_only;
	const Content *content;
	size_t offset;
};
