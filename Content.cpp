
#include "Content.h"

#include <iostream>
#include <iterator>
#include <fstream>

using namespace std;

Content::Content(const string &filename)
	: filename(filename), mime_type(getMimeType()), content()
{
	ifstream content_stream(filename, ios::binary);
	content_stream.unsetf(ios::skipws);
	content.assign(istream_iterator<uint8_t>(content_stream), istream_iterator<uint8_t>());

	cout << "uri: / or /" << filename << endl;
	cout << "Content lenght: " << content.size() << endl;
	cout << "Content-Type: " << mime_type << endl;
}

string Content::getMimeType() const
{
	string type;
	const string cmd = "file -i " + filename;
	FILE *pf = popen(cmd.c_str(), "r");
	if (pf != NULL) {
		while (!feof(pf)) {
			char buffer[80];
			const ssize_t readed = fread(buffer, 1, 80, pf);
			type += string(begin(buffer), begin(buffer) + readed);
		}
		pclose(pf);
	}
	const size_t cut = type.find(": ");
	const size_t ecut = type.find('\n');
	if (cut != string::npos && ecut != string::npos) {
		return type.substr(cut + 2, ecut - (cut + 2));
	}
	return "text/html; charset=utf-8";
}

size_t Content::size() const
{
	return content.size();
}

ssize_t Content::write(function<ssize_t (const void *, size_t)> writer, size_t offset) const
{
	return writer(&content[offset], content.size() - offset);
}

