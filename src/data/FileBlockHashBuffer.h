#include "BlockHash.h"
#include <memory>

/*
	Structure for buffering output
*/
class FileBlockHashBuffer
{
	static const char EOL = '\n';
	const size_t hash_size;
	const size_t line_size;
	const size_t buffer_size;
	std::unique_ptr<char[]> data;
	size_t hashes_remaining;

public:
	FileBlockHashBuffer(size_t hash_size, size_t buffer_size);
	void add_hash(const BlockHash& block_hash);
	const size_t get_remaining_hashes() const;
	const char* get_data() const;
	const size_t get_max_size() const;
	const size_t get_size() const;
};