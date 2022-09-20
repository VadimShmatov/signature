#include "FileBlockHashBuffer.h"

FileBlockHashBuffer::FileBlockHashBuffer(size_t hash_size, size_t buffer_size)
	: hash_size(hash_size), line_size(hash_size + 1), buffer_size(buffer_size), hashes_remaining(buffer_size)
{
	data = std::make_unique<char[]>(line_size * buffer_size);
}

void FileBlockHashBuffer::add_hash(const BlockHash& block_hash)
{
	size_t position = block_hash.position % buffer_size;
	std::copy(block_hash.hash_hex.data(), block_hash.hash_hex.data() + hash_size, data.get() + line_size * position);
	data.get()[line_size * position + hash_size] = EOL;
	hashes_remaining--;
}

const size_t FileBlockHashBuffer::get_remaining_hashes() const
{
	return hashes_remaining;
}

const char* FileBlockHashBuffer::get_data() const
{
	return data.get();
}

const size_t FileBlockHashBuffer::get_max_size() const
{
	return line_size * buffer_size;
}

const size_t FileBlockHashBuffer::get_size() const
{
	return (buffer_size - hashes_remaining) * line_size;
}