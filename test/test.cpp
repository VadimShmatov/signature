#define BOOST_TEST_MODULE SignatureTests
#include <boost/test/unit_test.hpp>
#include <boost/asio.hpp>
#include <functional>
#include <filesystem>

#include "../src/BlockingQueue.hpp"
#include "../src/FileBlockReader.h"
#include "../src/FileBlockHasherMD5.h"
#include "../src/FileBlockHashWriter.h"
#include "../src/Task.h"

std::atomic<size_t> entries_read = 0;

void reader_task(BlockingQueue<int>& queue)
{
    while (true) {
        int tmp;
        bool queue_active = queue.pop(tmp);
        if (!queue_active) {
            return;
        }
        else {
            entries_read++;
        }
    }
}

void writer_task(BlockingQueue<int>& queue)
{
    queue.start_writing();
    for (size_t i = 0; i < 100000; i++) {
        queue.push(rand());
    }
    queue.stop_writing();
}

BOOST_AUTO_TEST_CASE(QueueTest, *boost::unit_test::timeout(5))
{
    BlockingQueue<int> queue(1000);
    boost::asio::thread_pool pool(10);
    for (size_t i = 0; i < 5; i++) {
        boost::asio::post(pool, std::bind(reader_task, std::ref(queue)));
        boost::asio::post(pool, std::bind(writer_task, std::ref(queue)));
    }
    pool.join();
    BOOST_CHECK_EQUAL(5 * 100000, entries_read);
}

BOOST_AUTO_TEST_CASE(FileReaderTest, *boost::unit_test::timeout(5))
{
    std::shared_ptr<BlockingQueue<FileBlock>> queue = std::make_shared<BlockingQueue<FileBlock>>(2);
    std::ofstream test_file_out("test.bin", std::ios::binary);
    test_file_out.write("qwe", 3);
    test_file_out.close();
    std::unique_ptr<Worker> file_reader = std::make_unique<FileBlockReader>(queue, "test.bin", 2);
    Task read_task("File reader", std::move(file_reader));
    read_task();
    std::filesystem::remove("test.bin");
    FileBlock block;
    BOOST_CHECK_EQUAL(2, queue->get_size());
    bool block_read = queue->pop(block);
    BOOST_CHECK_EQUAL(true, block_read);
    BOOST_CHECK_EQUAL('q', block.data[0]);
    BOOST_CHECK_EQUAL('w', block.data[1]);
    block_read = queue->pop(block);
    BOOST_CHECK_EQUAL(true, block_read);
    BOOST_CHECK_EQUAL('e', block.data[0]);
    BOOST_CHECK_EQUAL('\0', block.data[1]);
    BOOST_CHECK_EQUAL(true, queue->get_closed());
}

BOOST_AUTO_TEST_CASE(FileBlockHasherMD5Test, *boost::unit_test::timeout(5))
{
    std::shared_ptr<BlockingQueue<FileBlock>> input_queue = std::make_shared<BlockingQueue<FileBlock>>(2);
    std::shared_ptr<BlockingQueue<BlockHash>> output_queue = std::make_shared<BlockingQueue<BlockHash>>(2);
    input_queue->start_writing();
    FileBlock block1(0, 3);
    std::copy_n("qwe", 3, block1.data.get());
    input_queue->push(std::move(block1));
    FileBlock block2(1, 3);
    std::copy_n("rty", 3, block2.data.get());
    input_queue->push(std::move(block2));
    input_queue->stop_writing();
    std::unique_ptr<Worker> hasher = std::make_unique<FileBlockHasherMD5>(input_queue, output_queue);
    Task hash_task("Hasher", std::move(hasher));
    hash_task();
    BOOST_CHECK_EQUAL(true, input_queue->get_closed());
    BOOST_CHECK_EQUAL(2, output_queue->get_size());
    BlockHash hash;
    bool hash_read = output_queue->pop(hash);
    BOOST_CHECK_EQUAL(true, hash_read);
    BOOST_CHECK_EQUAL(0, hash.position);
    BOOST_CHECK_EQUAL("76D80224611FC919A5D54F0FF9FBA446", hash.hash_hex);
    hash_read = output_queue->pop(hash);
    BOOST_CHECK_EQUAL(true, hash_read);
    BOOST_CHECK_EQUAL(1, hash.position);
    BOOST_CHECK_EQUAL("24113791D2218CB84C9F0462E91596EF", hash.hash_hex);
    BOOST_CHECK_EQUAL(true, output_queue->get_closed());
}

BOOST_AUTO_TEST_CASE(FileBlockHashWriterTest, *boost::unit_test::timeout(5))
{
    std::shared_ptr<BlockingQueue<BlockHash>> input_queue = std::make_shared<BlockingQueue<BlockHash>>(2);
    input_queue->start_writing();
    BlockHash hash1(0, "DEADBEEF");
    input_queue->push(std::move(hash1));
    BlockHash hash2(1, "CAFEBABE");
    input_queue->push(std::move(hash2));
    input_queue->stop_writing();
    std::filesystem::remove("test.txt");
    std::unique_ptr<Worker> writer = std::make_unique<FileBlockHashWriter>(input_queue, "test.txt", 3);
    Task write_task("Hash writer", std::move(writer));
    write_task();
    BOOST_CHECK_EQUAL(true, input_queue->get_closed());
    BOOST_CHECK_EQUAL(0, input_queue->get_size());
    BOOST_CHECK_EQUAL(true, std::filesystem::exists("test.txt"));
    std::ifstream t("test.txt", std::ios::binary);
    std::string result((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    t.close();
    std::filesystem::remove("test.txt");
    BOOST_CHECK_EQUAL("DEADBEEF\nCAFEBABE\n", result);
}