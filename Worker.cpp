
#include "Worker.h"

#include <array>
#include <iostream>
#include <sstream>
#include <cstring>
#include "Content.h"

using namespace std;

Worker::Worker(const Content *content)
	: state(INITIAL), request(), reply(), header_only(false), content(content), offset(0)
{
}

Worker::Worker(const Worker &worker)
	: state(worker.state), request(worker.request), reply(worker.reply),
	  header_only(worker.header_only), content(worker.content), offset(worker.offset)
{
}

Worker &Worker::operator = (const Worker &worker)
{
	state = worker.state;
	request = worker.request;
	reply = worker.reply;
	header_only = worker.header_only;
	content = worker.content;
	offset = worker.offset;
	return *this;
}


bool Worker::read(function<ssize_t (void *, size_t)> reader)
{
	array<char, 512> buf;
	while(true) {
		const auto rb = reader(&buf[0], buf.size());
		if (rb < 0) {
			if (errno == EAGAIN) {
				break;
			}
			cout << "Ошибка при чтении: " << strerror(errno) << endl;
			return false;
		}

		if (rb == 0) {
			return false;
		}

		request.append(buf.begin(), buf.begin() + rb);
	}

	if (state != INITIAL) {
		// drop all input after GET line...
		request.clear();
		return true;
	}

	auto endl = request.find("\r\n");
	if (endl == string::npos) {
		return true;
	}

	state = REPLY;
	request.erase(endl);

	if (request.substr(0, 4) == "GET ") {
		const auto uri = request.substr(4, request.rfind(" ") - 4);
		if (uri == "/" || uri == "/" + content->filename) {
			ostringstream replystream;
			replystream << "HTTP/1.1 200 Ok\r\n"
				<< "Content-Type: " << content->mime_type << "\r\n"
				<< "Content-Length: " << content->size() << "\r\n"
				<< "Connection: close\r\n\r\n";
			reply = replystream.str();
			return true;
		}

		header_only = true;
		reply = "HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/plain; charset=latin\r\n"
			"Content-Length: 25\r\n"
			"Connection: close\r\n\r\n"
			"404 error: File not found";
		return true;
	}

	reply = "HTTP/1.1 501 Not Implemented\r\n"
		"Allow: GET\r\n\r\n";
	return true;
}

bool Worker::write(function<ssize_t (const void *, size_t)> writer)
{
	if (state == REPLY) {
		const auto wb = writer(reply.c_str(), reply.size());
		if (wb < 0) {
			cerr << "Write header was failed: " << strerror(errno) << endl;
			return errno == EAGAIN;
		}

		if (size_t(wb) < reply.size()) {
			reply.erase(0, wb);
			return true;
		}

		state = DATA;
	}

	if (state == DATA && !header_only) {
		const auto wb = content->write(writer, offset);
		if (wb < 0) {
			cout << "Write data was failed: " << strerror(errno) << endl;
			return errno == EAGAIN;
		}

		offset += wb;
		if (offset == content->size()) {
			return false;
		}
	}

	return true;
}

