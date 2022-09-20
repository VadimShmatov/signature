#pragma once
#include "Worker.h"
#include "data/BlockHash.h"
#include "data/FileBlockHashBuffer.h"
#include "BlockingQueue.hpp"
#include <fstream>
#include <vector>
#include <map>

/*
	Writes hashes from input_queue into output_file
*/
class FileBlockHashWriter : public Worker
{
	static constexpr const size_t io_buffer_size_bytes = 1024 * 1024;
	const size_t seek_reduction_factor;
	const std::string output_file;
	std::shared_ptr<BlockingQueue<BlockHash>> input_queue;
	std::ofstream file;
	std::vector<char> io_buffer;
	BlockHash block_hash;
	std::map<size_t, FileBlockHashBuffer> hash_buffers;

	void write_last_buffer();

public:
	FileBlockHashWriter(const std::shared_ptr<BlockingQueue<BlockHash>>& input_queue, const std::string& file_name, const size_t seek_reduction_factor);
	void on_start() override;
	bool do_work() override;
	void on_stop() override;
	~FileBlockHashWriter() override;
};