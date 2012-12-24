
#pragma once
#include <string>
#include <vector>
#include <functional>

class Content {
public:
	Content(const std::string &filename);

	const std::string filename;
	const std::string mime_type;

	size_t size() const;
	ssize_t write(std::function<ssize_t (const void *, size_t)> writer, size_t offset) const;

private:
	std::string getMimeType() const;

	std::vector<uint8_t> content;
};
