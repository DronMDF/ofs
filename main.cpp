
#include <iostream>
#include <signal.h>
#include <sys/resource.h>
#include <unistd.h>
#include "Listener.h"
#include "Content.h"

using namespace std;

int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);

	if (getuid() == 0) {
		rlimit nofile;
		nofile.rlim_cur = 100000;
		nofile.rlim_max = 100000;
		setrlimit(RLIMIT_NOFILE, &nofile);
	}

	if (argc < 2) {
		cout << "usage: httpserver <content file>" << endl;
	}

	Content content(argv[1]);
	Listener listener(&content);
	listener.loop();
	return 0;
}

