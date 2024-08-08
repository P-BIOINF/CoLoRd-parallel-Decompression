#include "D:\\C++\\BioInformatyka\\CoLoRd-parallel-Decompression\\api\\colord_api.h" // scieszka do colord_api.h
#include "Parallel.h"
#include "Timer.h"
#include <map>
#include <string>
#include <thread>
#include <vector>
#include <ranges>
#include <fstream>
#include <numeric>
#include <iostream>
#include <filesystem>
#include <unordered_set>

Status Parallel::parseArguments(const int argc, char** argv)
{
	const std::unordered_set<std::string> setOfOneParam{ "-G", "--reference-genome", "-s","-v","--verbose","-h", "--help" };
	for (int i{ 0 }; i < argc; ++i)
	{
		if (std::string param{ argv[i] }; param == "--input")
		{
			m_input = std::string(argv[++i]);
			if (!std::filesystem::directory_entry(m_input).exists())
			{
				m_status = Status::not_ready;

				return getStatus();
			}
		}
		else if (param == "--output")
		{
			m_output = std::string(argv[++i]);
			m_output.remove_filename();
		}
		else if (param == "--api")
		{
			m_api = true;
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
				if (std::thread::hardware_concurrency() < m_threads)
					m_threads = std::thread::hardware_concurrency();
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
	if (m_input.empty() || m_colordPath.empty() && m_api == false || m_output.empty() || m_extension.empty() || m_count == -1 || m_threads == -1)
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
	for (int i = 0; i < m_threads; i++)
		threads.emplace_back([this]() { this->handleDecompression(); });
	for (auto& thread : threads)
		thread.join();
}

void Parallel::handleDecompression()
{
	int index{};
	while (true)
	{
		index = m_pathIndex.fetch_add(1, std::memory_order_relaxed);
		if (index >= m_directories.size())
			break;
		if(m_api)
			API_colordDecompression(m_directories[index], index);
		else
			systemDecompression(m_directories[index], index);
	}
}

void Parallel::systemDecompression(const std::filesystem::path& path, int current)
{
	std::filesystem::path tempOutput(m_output);
	tempOutput.append("temp");
	tempOutput.append(std::to_string(current) + ".fastq");
	const std::string temp{ " " + m_colordPath.string() + " decompress " + path.string() + " " + tempOutput.string()};
	std::system(temp.c_str());
}

void Parallel::API_colordDecompression(const std::filesystem::path& path, int current)
{
	std::filesystem::path tempOutputPath(m_output);
	tempOutputPath.append("temp");
	tempOutputPath.append(std::to_string(current) + ".fastq");
	std::ofstream tempOutputFile(tempOutputPath);
	if (tempOutputFile)
	{
		try
		{
			colord::DecompressionStream stream(path.string());
			auto info = stream.GetInfo();
			while (auto x = stream.NextRecord()) {
				if (info.isFastq)
				{
					tempOutputFile << "@" << x.ReadHeader() << '\n';
					tempOutputFile << x.Read() << '\n';
					tempOutputFile << "+" << x.QualHeader() << '\n';
					tempOutputFile << x.Qual() << '\n';
				}
				else
				{
					tempOutputFile << ">" << x.ReadHeader() << '\n';
					tempOutputFile << x.Read() << '\n';
				}
			}
		}
		catch (const std::exception& ex)
		{
			std::cerr << "Error: " << ex.what() << '\n';
			return;
		}
	}
}

void Parallel::generateOutput()
{
	std::filesystem::path temp{ m_output };
	temp.append("temp");
	std::filesystem::path tempOutput{ m_output };
	std::filesystem::path outputFile{ tempOutput.append("DecompressedOutput.fastq") };
	std::ofstream output{ outputFile.string(), std::ios::binary };
	std::vector<std::ifstream> inputStreams{};
	std::map<int, std::filesystem::path > tempMap{};
	for (const auto& file : std::filesystem::directory_iterator(temp))
		tempMap.emplace(std::stoi(file.path().stem().stem().string()), file);
	for (const auto& file : tempMap | std::ranges::views::values)
		inputStreams.emplace_back(file, std::ios::binary);
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