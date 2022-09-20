#pragma once
#include "Worker.h"
#include "data/FileBlock.h"
#include "BlockingQueue.hpp"
#include <string>
#include <fstream>
#include <vector>

/*
	Reads input_file and puts its blocks into output_queue
*/
class FileBlockReader : public Worker
{
	static constexpr const size_t io_buffer_size_bytes = 1024 * 1024;
	size_t current_pos = 0;
	const size_t block_size;
	const std::string input_file;
	std::shared_ptr<BlockingQueue<FileBlock>> output_queue;
	std::ifstream file;
	std::vector<char> io_buffer;

public:
	FileBlockReader(const std::shared_ptr<BlockingQueue<FileBlock>>& output_queue, const std::string& file_name, const size_t block_size);
	void on_start() override;
	bool do_work() override;
	void on_stop() override;
	~FileBlockReader() override;
};