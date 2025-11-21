#include <network-monitor/file-downloader.h>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>

using NetworkMonitor::DownloadFile;
using NetworkMonitor::ParseJsonFile;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(file_downloader)
{
    const std::string fileUrl {
        "https://ltnm.learncppthroughprojects.com/network-layout.json"
    };
    const auto destination {
        std::filesystem::temp_directory_path() / "network-layout.json"
    };

    // Download the file.
    bool downloaded {DownloadFile(fileUrl, destination, TESTS_CACERT_PEM)};
    BOOST_CHECK(downloaded);
    BOOST_CHECK(std::filesystem::exists(destination));

    // Check the content of the file.
    // We cannot check the whole file content as it changes over time, but we
    // can at least check some expected file properties.
    {
        const std::string expectedString {"\"stations\": ["};
        std::ifstream file {destination};
        std::string line {};
        bool foundExpectedString {false};
        while (std::getline(file, line)) {
            if (line.find(expectedString) != std::string::npos) {
                foundExpectedString = true;
                break;
            }
        }
        BOOST_CHECK(foundExpectedString);
    }

    // Clean up
    std::filesystem::remove(destination);
}

BOOST_AUTO_TEST_CASE(parse_json) {
    const std::filesystem::path source {TESTS_NETWORK_LAYOUT};
    auto data = ParseJsonFile(source);

    BOOST_CHECK(data.contains("lines"));
    BOOST_CHECK(!data["lines"].empty());

    BOOST_CHECK(data.contains("stations"));
    BOOST_CHECK(!data["stations"].empty());

    BOOST_CHECK(data.contains("travel_times"));
    BOOST_CHECK(!data["travel_times"].empty());
}

BOOST_AUTO_TEST_SUITE_END();