/*******************************************************************************
 
 CoLoRd 
 Copyright (C) 2021, M. Kokot, S. Deorowicz, and A. Gudys
 https://github.com/refresh-bio/CoLoRd

 This program is free software: you can redistribute it and/or modify it under 
 the terms of the GNU General Public License as published by the Free Software 
 Foundation; either version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY 
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR 
 A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this 
 program. If not, see https://www.gnu.org/licenses/.

******************************************************************************/
#pragma once

#include "utils.h"
#include "parallel_queue.h"
#include "queues_data.h"
#include "stats_collector.h"
#include "zlib.h"
#include <string>
#include <vector>
#include <cassert>

class CKmerWalker
{
	const read_t& read;
	uint32_t cur_pos;
	uint64_t mask;
	kmer_type& kmer, str, rev;
	uint32_t rev_offset;

public:
	CKmerWalker(const read_t& read, uint32_t kmer_len, kmer_type& kmer) :
		read(read),
		cur_pos(0),
		mask((1ull << (2 * kmer_len)) - 1),
		kmer(kmer)
	{
		kmer = 0;
		str = 0;
		rev = 0;
		rev_offset = 2 * (kmer_len - 1);
		for (; cur_pos < kmer_len - 1; ++cur_pos)
		{
			str <<= 2;
			str += read[cur_pos];

			rev >>= 2;
			rev += ((kmer_type)(3 - read[cur_pos])) << rev_offset;
		}
	}

	bool NextKmer()
	{
		if (cur_pos >= read_len(read))
			return false;

		str <<= 2;
		str += read[cur_pos];
		str &= mask;

		rev >>= 2;
		rev += ((uint64_t)(3 - read[cur_pos])) << rev_offset;
		++cur_pos;
		kmer = str < rev ? str : rev;
		return true;
	}
};


class CInputReads
{
	CStatsCollector stats;
	std::vector<std::pair<bool, read_t>> reads;	
	std::vector<std::pair<read_t, qual_t>> quals;
	std::vector<std::pair<std::string, qual_header_type>> headers;

	uint64_t current_reads_bytes{}, current_header_bytes{};
	
	CParallelQueue<read_pack_t>& reads_queue;
	CParallelQueue<qual_pack_t>& quals_queue;
	CParallelQueue<header_pack_t>& headers_queue;
	
	read_t to_read_t(const std::string& str, bool& hasN);


	std::string currentLine;
	read_t last_read;
	std::string last_read_header;

	bool is_fastq;

	uint64_t total_bytes{};
	uint64_t total_bases{};
	uint64_t total_symb_header{};

	void addReadHeader();
	void addRead();
	void addQualHeader();
	void addQual();

	void porcessFastaOrMultiFasta(std::vector<uint8_t>& buff, uint64_t readed, uint32_t buf_size, gzFile gzfile);
	void processFastq(std::vector<uint8_t>& buff, uint64_t readed, uint32_t buf_size, gzFile gzfile);
public:
	explicit CInputReads(bool verbose, const std::string& path, CParallelQueue<read_pack_t>& reads_queue, CParallelQueue<qual_pack_t>& quals_queue, CParallelQueue<header_pack_t>& headers_queue);
	void GetStats(uint64_t& total_bytes, uint64_t& total_bases, uint64_t& total_symb_header)
	{
		total_bytes = this->total_bytes;
		total_bases = this->total_bases;
		total_symb_header = this->total_symb_header;
	}
};

