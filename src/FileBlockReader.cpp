#include "FileBlockReader.h"
#include <boost/log/trivial.hpp>

FileBlockReader::FileBlockReader(const std::shared_ptr<BlockingQueue<FileBlock>>& output_queue, const std::string& file_name, const size_t block_size)
	: output_queue(output_queue), block_size(block_size), input_file(file_name), io_buffer(io_buffer_size_bytes)
{
	std::ios::sync_with_stdio(false);
	output_queue->start_writing();
	file.open(input_file, std::ios::binary);
	if (!file) {
		output_queue->stop_writing();
		throw std::runtime_error("Error opening input file " + input_file);
	}
	file.rdbuf()->pubsetbuf(io_buffer.data(), io_buffer_size_bytes);
}

void FileBlockReader::on_start()
{
	BOOST_LOG_TRIVIAL(debug) << "Starting FileBlockReader";
}

bool FileBlockReader::do_work()
{
	FileBlock block(current_pos++, block_size);
	file.read(block.data.get(), block_size);
	size_t bytes_read = file.gcount();
	if (bytes_read == 0) {
		return false;
	}
	if (bytes_read < block_size) {
		std::fill_n(block.data.get() + bytes_read, block_size - bytes_read, 0);
	}
	output_queue->push(std::move(block));
	return !file.eof();
}

void FileBlockReader::on_stop()
{
	BOOST_LOG_TRIVIAL(debug) << "Stopping FileBlockReader";
	file.close();
	output_queue->stop_writing();
	output_queue.reset();
	BOOST_LOG_TRIVIAL(debug) << "Stopped FileBlockReader";
}

FileBlockReader::~FileBlockReader()
{
	if (output_queue) {
		output_queue->stop_writing();
		output_queue.reset();
	}
}