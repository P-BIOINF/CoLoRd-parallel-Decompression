#include "Parallel.h"

int main(const int argc, char** argv)
{
	std::chrono::high_resolution_clock::time_point start{ std::chrono::high_resolution_clock::now() };
	std::ios_base::sync_with_stdio(false);

	Parallel parallel{};

	if (parallel.parseArguments(argc, argv) != Status::ready)
	{
		std::cerr << "Status code: " << static_cast<int>(parallel.getStatus()) << '\n';
		std::cerr << "Something went wrong! Please make sure that you have included:\n"
			"--output <output directory> --input <input directory> --colord <colord directory> --extension <file extension to decompress> --lpthread <maximum number of threads> --count <number of sequences after which the next file is moved>\n";
		return -1;
	}

	parallel.getFilesToDecomp();
	parallel.decompress();
	parallel.generateOutput();
	std::chrono::high_resolution_clock::time_point end{ std::chrono::high_resolution_clock::now() };
	displayTime("Elapsed time during the entire program: ", start, end);
	return 0;
}