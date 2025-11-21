from conan import ConanFile
from conan.tools.cmake import cmake_layout

class NetworkMonitorConan(ConanFile):
    name = "network-monitor"
    version = "0.1.0"

    requires = "boost/1.83.0", "openssl/1.1.1w", "libcurl/8.6.0", "nlohmann_json/3.12.0"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"

    default_options = {
        "boost/*:shared": False,
    }

    def layout(self):
        cmake_layout(self)
