#pragma once
#include <vector>
#include <memory>

struct FileBlock
{
	size_t position;
	size_t size;
	std::unique_ptr<char[]> data;

	FileBlock() = default;
	FileBlock(size_t position, size_t size)
		: position(position), size(size)
	{
		data = std::make_unique<char[]>(size);
	}
};