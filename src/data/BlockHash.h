#pragma once
#include <string>

struct BlockHash
{
	size_t position;
	std::string hash_hex;

	BlockHash() = default;
	BlockHash(size_t position, std::string hash_hex)
		: position(position), hash_hex(std::move(hash_hex))
	{}
};