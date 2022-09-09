#include <iostream>
#include <cppmisc/files.h>


int main(int argc, char const* argv[])
{
	char mask[] = "/usr/lib/*.so";
	auto files = get_files(mask);
	std::cout << "files matching " << mask << ":" << std::endl;
	for (auto f : files)
		std::cout << f << std::endl;
	return 0;
}