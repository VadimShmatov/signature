#include "BlockingQueue.hpp"
#include "FileBlockReader.h"
#include "FileBlockHasherMD5.h"
#include "FileBlockHashWriter.h"
#include "Task.h"

#include <boost/log/utility/setup.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include <filesystem>
#include <algorithm>
#include <thread>

class SignatureApp
{
    // Memory consumption restrictions. Total memory consumption must not exceed 1Gb
    static const size_t max_file_data_memory_consumption_bytes = 100 * 1024 * 1024;
    static const size_t max_hash_data_memory_consumption_bytes = 100 * 1024 * 1024;
    static const size_t max_write_data_memory_consumption_bytes = 128 * 1024;
    static const size_t max_queue_elements_per_thread = 1024;
    static const size_t max_write_grouping = 128;

    // Other restrictions and constants
    static const size_t hash_size_bytes = 32;
    static const uint64_t max_input_file_size_bytes = 128ULL * 1024ULL * 1024ULL * 1024ULL;
    static const size_t min_block_size_bytes = 512;
    static const size_t max_block_size_bytes = 10 * 1024 * 1024;
    static const size_t default_block_size_bytes = 1024 * 1024;


    /*static const size_t max_hasher_memory_consumption = 512 * 1024 * 1024;
    static const size_t max_queue_size_per_thread = 64 * 1024;
    static const size_t max_queue_size_writer = 1024 * 1024;*/

    bool validate_inputs(const std::string& input_file, const std::string& output_file, const size_t block_size) const
    {
        bool result = true;
        if (!std::filesystem::exists(input_file)) {
            BOOST_LOG_TRIVIAL(error) << "Input file " << input_file << " does not exist";
            result = false;
        }
        else if (std::filesystem::file_size(input_file) > max_input_file_size_bytes) {
            BOOST_LOG_TRIVIAL(error) << "Input file size " << std::filesystem::file_size(input_file)
                << " exceeds " << max_input_file_size_bytes << " bytes";
            result = false;
        }
        if (std::filesystem::exists(output_file)) {
            BOOST_LOG_TRIVIAL(error) << "Input file " << input_file << " already exists";
            result = false;
        }
        if (block_size < 512 || block_size > 10 * 1024 * 1024) {
            BOOST_LOG_TRIVIAL(error) << "Block size " << block_size << " is outside of allowed range: "
                << min_block_size_bytes << " - " << max_block_size_bytes << " bytes";
            result = false;
        }
        return result;
    }

public:
	SignatureApp()
	{
        // Logging setup
        static const std::string COMMON_FMT("[%TimeStamp%][%Severity%]:  %Message%");
        boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
        boost::log::add_console_log(
            std::cout,
            boost::log::keywords::format = COMMON_FMT,
            boost::log::keywords::auto_flush = true
        );
        boost::log::add_common_attributes();
#ifndef _DEBUG
        boost::log::core::get()->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::info
        );
#endif
	}

	void run(int argc, char* argv[])
	{
        // Parse and validate input arguments
        if (argc < 3 || argc > 4) {
            BOOST_LOG_TRIVIAL(info) << "Usage: " << argv[0] << " input_file output_file [block_size_bytes]";
            return;
        }
        std::string input_file = argv[1];
        std::string output_file = argv[2];
        size_t block_size = default_block_size_bytes;
        if (argc == 4) {
            try {
                block_size = std::stoi(argv[3]);
            }
            catch (const std::exception& ex) {
                BOOST_LOG_TRIVIAL(error) << "Cannot parse block size " << argv[3] << ": " << ex.what();
                return;
            }
        }
        if (!validate_inputs(input_file, output_file, block_size)) {
            return;
        }

        // Set up queues
        size_t hasher_number = std::max(1U, 2 * std::thread::hardware_concurrency());
        size_t max_block_number = std::min(max_file_data_memory_consumption_bytes / (sizeof(FileBlock) + block_size), max_queue_elements_per_thread * hasher_number);
        size_t max_hash_number = std::min(max_hash_data_memory_consumption_bytes / (sizeof(BlockHash) + hash_size_bytes), max_queue_elements_per_thread * hasher_number);
        size_t write_grouping = std::min(max_write_data_memory_consumption_bytes / ((sizeof(FileBlockHashBuffer) + hash_size_bytes + 1) * hasher_number), max_write_grouping);
        std::shared_ptr<BlockingQueue<FileBlock>> file_block_queue = std::make_shared<BlockingQueue<FileBlock>>(max_block_number);
        std::shared_ptr<BlockingQueue<BlockHash>> block_hash_queue = std::make_shared<BlockingQueue<BlockHash>>(max_hash_number);

        // Print parameters
        BOOST_LOG_TRIVIAL(info) << "Generating file signature...";
        BOOST_LOG_TRIVIAL(info) << "Input file: " << input_file;
        BOOST_LOG_TRIVIAL(info) << "Output file: " << output_file;
        BOOST_LOG_TRIVIAL(info) << "Block size: " << block_size << " bytes";
        BOOST_LOG_TRIVIAL(debug) << "File block queue size: " << max_block_number;
        BOOST_LOG_TRIVIAL(debug) << "File hash queue size: " << max_hash_number;
        BOOST_LOG_TRIVIAL(debug) << "Write seek reduction factor: " << write_grouping;

        // Start tasks: FileBlockReader -> FileBlockHasherMD5 -> FileBlockHashWriter
        // thread_pool is used for convenience only, threads match tasks one to one
        boost::asio::thread_pool pool(hasher_number + 2);
        boost::asio::post(pool, Task("Input file reader", std::make_unique<FileBlockReader>(file_block_queue, input_file, block_size)));
        for (size_t i = 0; i < hasher_number; i++) {
            boost::asio::post(pool, Task("Hasher #" + std::to_string(i), std::make_unique<FileBlockHasherMD5>(file_block_queue, block_hash_queue)));
        }
        boost::asio::post(pool, Task("Output file writer", std::make_unique<FileBlockHashWriter>(block_hash_queue, output_file, write_grouping)));
        pool.join();
        BOOST_LOG_TRIVIAL(info) << "Signature generated!";
	}
};

int main(int argc, char* argv[])
{
	SignatureApp app;
	app.run(argc, argv);
}