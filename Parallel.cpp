#include "Parallel.h"

#include "Timer.h"
#include <string>
#include <thread>
#include <vector>
#include <fstream>
#include <filesystem>
#include <unordered_set>

#include <iostream>

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
		else if(param == "--extension")
		{
			m_extension = argv[++i];
		}
		else if (param == "--colord")
		{
			m_path = argv[++i];
		}
		else if (setOfOneParam.contains(param))
		{
			m_arguments.append(" " + param);
		}
	}
	m_arguments.append(" ");
	if (m_path.empty() || m_extension.empty())
	{
		m_status = Status::not_ready;

		return getStatus();
	}
	/**getInputStream().open(getInput());

	std::filesystem::create_directory(getOutput());
	if (!getInputStream())
	{
		m_status = Status::failed;

		return getStatus();
	}
	*/
	m_status = Status::ready;
	return getStatus();
}

void Parallel::getFilesToDecomp()
{
	for(const auto& entry: std::filesystem::directory_iterator(m_input))
	{
		if(entry.path().extension() == m_extension)
		{
			m_directories.emplace_back(entry);
		}
	}
}

void Parallel::decompress() 
{
	std::filesystem::path temp{m_output};
	std::filesystem::create_directory(temp);
	temp.append("temp");
	std::filesystem::create_directory(temp);
	std::vector<std::thread> threads;
	for (const auto& path : m_directories)
		threads.emplace_back([this, path] { this->handleDecompression(path); });
	for (auto& thread : threads)
		thread.join();
}

void Parallel::handleDecompression(const std::filesystem::path& path)
{
	static int current{0};
	std::filesystem::path tempOutput(m_output);
	tempOutput.append("temp");
	tempOutput.append(std::to_string(++current) + ".fastq");
	const std::string temp{ " " + m_path.string() + " decompress " + path.string() + " " + tempOutput.string()};
	std::system(temp.c_str());
}

void Parallel::generateOutput()
{
	std::filesystem::path temp{m_output};
	temp.append("temp");
	std::filesystem::path tempOutput{m_output};
	std::filesystem::path outputFile{tempOutput.append("DecompressOutput.fastq")};
	std::ofstream output{ outputFile.string() };
	for (auto& file : std::filesystem::directory_iterator(temp))
	{
		std::ifstream tempStream{ file.path() };
		output << tempStream.rdbuf();
		tempStream.close();
	}
	std::filesystem::remove_all(temp);
	output.close();
}