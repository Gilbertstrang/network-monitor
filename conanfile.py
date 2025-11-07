from conans import ConanFile

class ConanPackage(ConanFile):
    name = 'network-monitor'
    version = "2.22.1"

    generators = 'cmake_find_package'

    requires = [
    ]