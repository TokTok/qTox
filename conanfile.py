# pylint: disable=not-callable
from conan import ConanFile
from conan.tools.cmake import CMake
from conan.tools.cmake import cmake_layout
from conan.tools.cmake import CMakeDeps
from conan.tools.cmake import CMakeToolchain


class ToxConan(ConanFile):
    name = "qtox"
    version = "1.18.0"
    url = "https://qtox.github.io"
    description = "The future of online communications."
    license = "GPL-3.0-only"
    settings = "os", "compiler", "build_type", "arch"
    requires = (
        "ffmpeg/7.0.1",
        "libexif/0.6.24",
        "libqrencode/4.1.1",
        "libsodium/1.0.20",
        "libvpx/1.14.1",
        "openal-soft/1.23.1",
        "sqlcipher/4.6.1",
        "sqlite3/3.46.1",
        "toxcore/0.2.20",
    )
    scm = {"type": "git", "url": "auto", "revision": "auto"}

    def requirements(self):
        self.requires("qt/6.7.3", options={"gui": True, "qtsvg": False})

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        # tc.variables["AUTOTEST"] = self.options.with_tests
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
