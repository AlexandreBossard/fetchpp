from conans import ConanFile, CMake, tools


class FetchppConan(ConanFile):
    name = "fetchpp"
    version = "0.0.1"
    license = "Apache 2.0"
    author = "Alexandre Bossard"
    url = ""
    description = "the simplest http client"
    topics = ("http", "beast", "fetch", "client")
    settings = "os", "compiler", "build_type", "arch"
    requires = (
        ("boost/1.71.0"), ("openssl/1.1.1d"),
        ("fmt/6.0.0")
    )
    build_requires = "Catch2/2.10.0@catchorg/stable"
    options = {"shared": [True, False], "fPIC": [True, False], "warn_as_error": [True, False]}
    default_options = {"shared": False, "fPIC": False, "warn_as_error": False, "boost:system_no_deprecated": True, "boost:asio_no_deprecated": True, "boost:filesystem_no_deprecated": True, "boost:without_math": True, "boost:without_log": True}
    generators = "cmake", "ycm"
    exports_sources = "CMakeLists.txt", "libs/*"
    cmake = None


    @property
    def should_build_tests(self):
        return self.develop and not tools.cross_building(self.settings)


    def init_cmake(self):
        if self.cmake:
            return
        self.cmake = CMake(self)
        self.cmake.definitions["WARN_AS_ERROR"] = self.options.warn_as_error
        self.cmake.definitions["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC
        self.cmake.definitions["ENABLE_TESTING"] = self.should_build_tests


    def build(self):
        self.init_cmake()
        if self.should_configure:
            self.cmake.configure()
        if self.should_build:
            self.cmake.build()
        if self.should_test:
            self.cmake.test()


    def package(self):
        self.init_cmake()
        self.cmake.install()


    def package_id(self):
        del self.info.options.warn_as_error


    def package_info(self):
        self.cpp_info.libs = ["fecthpp"]
