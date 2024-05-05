#include "Parallel.h"
#include "Timer.h"
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <numeric>

#include <iostream>
#include <ranges>

Status Parallel::parseArguments(const int argc, char** argv)
{
	const std::unordered_set<std::string> setOfOneParam{ "-G", "--reference-genome", "-s","-v","--verbose","-h", "--help" };
	for (int i{ 0 }; i < argc; ++i)
	{
		if (std::string param{ argv[i] }; param == "--input")
		{
			m_input = std::string(argv[++i]) + "\\";
			if (!std::filesystem::directory_entry(m_input).exists())
			{
				m_status = Status::not_ready;

				return getStatus();
			}
		}
		else if (param == "--output")
		{
			m_output = std::string(argv[++i]) + "\\";
			m_output.remove_filename();
		}
		else if (param == "--count")
		{
			try
			{
				m_count = std::stoi(argv[++i]);
			}
			catch (const std::invalid_argument& ex)
			{
				std::cerr << ex.what() << '\n';
				m_count = -1;
			}
		}
		else if(param == "--extension")
		{
			m_extension = argv[++i];
		}
		else if (param == "--lpthread")
		{
			try
			{
				m_threads = std::stoi(argv[++i]);
			}
			catch (const std::invalid_argument& ex)
			{
				std::cerr << ex.what() << '\n';
				m_threads = -1;
			}
		}
		else if (param == "--colord")
		{
			m_colordPath = argv[++i];
		}
		else if (setOfOneParam.contains(param))
		{
			m_arguments.append(' ' + param);
		}
	}
	m_arguments.append(" ");
	if (m_input.empty() || m_colordPath.empty() || m_output.empty() || m_extension.empty() || m_count == -1 || m_threads == -1)
	{
		m_status = Status::not_ready;
		return getStatus();
	}
	m_status = Status::ready;
	return getStatus();
}

void Parallel::getFilesToDecomp()
{
	std::map<int, std::filesystem::path > temp{};
	for(const auto& entry: std::filesystem::directory_iterator(m_input))
		if(entry.path().extension() == m_extension)
			temp.emplace(std::stoi(entry.path().stem().string()), entry);

	for (const auto& file : temp | std::ranges::views::values)
		m_directories.emplace_back(file);
}

void Parallel::decompress()
{
	std::filesystem::path temp{ m_output };
	std::filesystem::create_directory(temp);
	temp.append("temp");
	std::filesystem::create_directory(temp);
	std::vector<std::thread> threads{};
	threads.reserve(m_threads);
	for (int i = 0; i < m_threads; i++)
		threads.emplace_back([this]() { this->handleDecompression(); });
	for (auto& thread : threads)
		thread.join();
}

void Parallel::handleDecompression()
{
	static std::atomic<int> pathIndex{ 0 };
	int index{};
	while ((index = pathIndex++) < m_directories.size())
		systemDecompression(m_directories[index]);
}

void Parallel::systemDecompression(const std::filesystem::path& path)
{
	static int current{0};
	std::filesystem::path tempOutput(m_output);
	tempOutput.append("temp");
	tempOutput.append(std::to_string(++current) + ".fastq");
	const std::string temp{ " " + m_colordPath.string() + " decompress " + path.string() + " " + tempOutput.string()};
	std::system(temp.c_str());
}

void Parallel::generateOutput()
{
	std::filesystem::path temp{ m_output };
	temp.append("temp");
	std::filesystem::path tempOutput{ m_output };
	std::filesystem::path outputFile{ tempOutput.append("DecompressedOutput.fastq") };
	std::ofstream output{ outputFile.string() };

	std::vector<std::ifstream> inputStreams{};
	std::map<int, std::filesystem::path > tempMap{};

	for (const auto& file : std::filesystem::directory_iterator(temp))
		tempMap.emplace(std::stoi(file.path().stem().stem().string()), file);

	for (const auto& file : tempMap | std::ranges::views::values)
		inputStreams.emplace_back(file);

	std::string identifier{};
	std::string sequence{};
	std::string signAndIdentifier{};
	std::string qualityScores{};

	std::uint64_t currentSequence{ 0 };

	int currentFile{ 0 };
	std::vector<int> end(inputStreams.size(), 1);

	while (!inputStreams.empty())
	{
		if (std::getline(inputStreams[currentFile], identifier))
		{
			std::getline(inputStreams[currentFile], sequence);
			std::getline(inputStreams[currentFile], signAndIdentifier);
			std::getline(inputStreams[currentFile], qualityScores);
			output << identifier << '\n' << sequence << '\n' << signAndIdentifier << '\n' << qualityScores << '\n';
		}
		else
			end[currentFile] = 0;

		if (std::reduce(end.begin(), end.end(), 0) == 0)
			break;

		if (++currentSequence == m_count)
		{
			currentSequence = 0;

			if (++currentFile >= std::ssize(inputStreams))
				currentFile = 0;
			else if (end[currentFile] == 0)
				++currentFile;
		}

	}
	for (auto& temporary : inputStreams)
		temporary.close();
	std::filesystem::remove_all(temp);
	output.close();
}

