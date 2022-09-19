#define BOOST_UUID_COMPAT_PRE_1_71_MD5
#include "FileBlockHasherMD5.h"
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/log/trivial.hpp>

using boost::uuids::detail::md5;

FileBlockHasherMD5::FileBlockHasherMD5(const std::shared_ptr<BlockingQueue<FileBlock>>& input_queue,
	const std::shared_ptr<BlockingQueue<BlockHash>>& output_queue)
	: input_queue(input_queue), output_queue(output_queue)
{
	output_queue->start_writing();
}

std::string FileBlockHasherMD5::md5_hash(const FileBlock& block)
{
	md5 hash;
	md5::digest_type digest;
	const char* char_digest;
	std::string result;

	hash.process_bytes(block.data.get(), block.size);
	hash.get_digest(digest);
	char_digest = reinterpret_cast<const char*>(&digest);
	boost::algorithm::hex(char_digest, char_digest + sizeof(md5::digest_type), std::back_inserter(result));
	return result;
}

void FileBlockHasherMD5::on_start()
{
	BOOST_LOG_TRIVIAL(debug) << "Starting FileBlockHasherMD5";
}

bool FileBlockHasherMD5::do_work()
{
	bool block_read = input_queue->pop(input_block);
	if (!block_read) {
		return false;
	}
	BlockHash block_hash(input_block.position, md5_hash(input_block));
	output_queue->push(std::move(block_hash));
	return true;
}

void FileBlockHasherMD5::on_stop()
{
	BOOST_LOG_TRIVIAL(debug) << "Stopping FileBlockHasherMD5";
	output_queue->stop_writing();
	output_queue.reset();
	BOOST_LOG_TRIVIAL(debug) << "Stopped FileBlockHasherMD5";
}

FileBlockHasherMD5::~FileBlockHasherMD5()
{
	if (output_queue) {
		output_queue->stop_writing();
		output_queue.reset();
	}
}