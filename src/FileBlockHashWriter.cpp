#include "FileBlockHashWriter.h"
#include <filesystem>
#include <boost/log/trivial.hpp>

FileBlockHashWriter::FileBlockHashWriter(const std::shared_ptr<BlockingQueue<BlockHash>>& input_queue, const std::string& file_name, const size_t seek_reduction_factor)
	: input_queue(input_queue), output_file(file_name), io_buffer(io_buffer_size_bytes), seek_reduction_factor(seek_reduction_factor)
{
	std::ios::sync_with_stdio(false);
	if (std::filesystem::exists(output_file)) {
		throw std::runtime_error("Output file " + output_file + " already exists");
	}
	file.open(output_file, std::ios::binary);
	if (!file) {
		throw std::runtime_error("Error opening output file " + output_file);
	}
	file.rdbuf()->pubsetbuf(io_buffer.data(), io_buffer_size_bytes);
}

void FileBlockHashWriter::on_start()
{
	BOOST_LOG_TRIVIAL(debug) << "Starting FileBlockHashWriter";
}

void FileBlockHashWriter::write_last_buffer()
{
	if (hash_buffers.empty()) {
		return;
	}
	if (hash_buffers.size() > 1) {
		throw std::runtime_error("Work is done but some hashes to write to signature file are missing");
	}
	size_t buffer_index = hash_buffers.begin()->first;
	FileBlockHashBuffer& buffer = hash_buffers.begin()->second;
	file.seekp(buffer_index * buffer.get_max_size());
	file.write(buffer.get_data(), buffer.get_size());
	hash_buffers.clear();
}

bool FileBlockHashWriter::do_work()
{
	bool block_read = input_queue->pop(block_hash);
	if (!block_read) {
		write_last_buffer();
		return false;
	}

	// Writing hashes to file seems to be a choke point in many cases
	// Minimize file seeks by preparing a big buffer to write first
	size_t buffer_index = block_hash.position / seek_reduction_factor;
	auto it = hash_buffers.emplace(std::piecewise_construct,
		                           std::forward_as_tuple(buffer_index),
		                           std::forward_as_tuple(block_hash.hash_hex.size(), seek_reduction_factor));
	FileBlockHashBuffer& buffer = it.first->second;
	buffer.add_hash(block_hash);
	if (buffer.get_remaining_hashes() == 0) {
		size_t buffer_index = it.first->first;
		file.seekp(buffer_index * buffer.get_max_size());
		file.write(buffer.get_data(), buffer.get_size());
		hash_buffers.erase(it.first);
	}
	return true;
}

void FileBlockHashWriter::on_stop()
{
	BOOST_LOG_TRIVIAL(debug) << "Stopping FileBlockHashWriter";
	file.close();
	BOOST_LOG_TRIVIAL(debug) << "Stopped FileBlockHashWriter";
}

FileBlockHashWriter::~FileBlockHashWriter()
{

}