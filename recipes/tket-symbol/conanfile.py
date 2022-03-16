# Copyright 2019-2022 Cambridge Quantum Computing
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from conans import ConanFile, CMake, tools
import os

required_conan_version = ">=1.45.0"


class TketSymbolConan(ConanFile):
    name = "tket-symbol"
    version = "0.1.0"
    description = "symengine wrapper library"
    license = "Apache 2"
    topics = "symbolic", "algebra"
    homepage = "https://github.com/CQCL/tket"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "integer_class": ["boostmp", "gmp"],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "integer_class": "boostmp",
    }
    generators = "cmake"
    exports_sources = ["../../tket/libs/symbol/*", "!*/build/*"]
    # symengine from remote: https://tket.jfrog.io/artifactory/api/conan/tket-conan
    requires = "symengine/0.9.0@tket/stable", "nlohmann_json/3.10.5"
    short_paths = True

    _cmake = None

    def _configure_cmake(self):
        if self._cmake is None:
            self._cmake = CMake(self)
            self._cmake.configure()
        return self._cmake

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        self.options["symengine"].integer_class = self.options.integer_class

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        self.copy("LICENSE", dst="licenses", src="../..")
        self.copy("include/*.hpp", dst="include/symbol", keep_path=False)
        self.copy("*.dll", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
