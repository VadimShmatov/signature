#include "data/FileBlock.h"
#include "data/BlockHash.h"
#include "BlockingQueue.hpp"
#include "Worker.h"

/*
	Calculates MD5 hashes for file blocks from input_queue and writes them into output_queue
*/
class FileBlockHasherMD5 : public Worker
{
	std::shared_ptr<BlockingQueue<FileBlock>> input_queue;
	std::shared_ptr<BlockingQueue<BlockHash>> output_queue;
	FileBlock input_block;
	static std::string md5_hash(const FileBlock& block);

public:
	FileBlockHasherMD5(const std::shared_ptr<BlockingQueue<FileBlock>>& input_queue,
		const std::shared_ptr<BlockingQueue<BlockHash>>& output_queue);
	void on_start() override;
	bool do_work() override;
	void on_stop() override;
	~FileBlockHasherMD5() override;
};