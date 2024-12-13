from conan import ConanFile
from conan.tools.cmake import cmake_layout

class Recipe(ConanFile):
    settings   = ["os", "compiler", "build_type", "arch"]
    generators = ["CMakeToolchain", "CMakeDeps"]
    requires   = ["fmt/11.0.2", "scnlib/4.0.1", "cli11/2.4.2", "nameof/0.10.4"]

    def layout(self):
        cmake_layout(self)
