import os

from conans import ConanFile, tools


class FIoCConan(ConanFile):
    scm = {
         "type": "git",
         "subfolder": "",
         "url": "https://github.com/forry/FIoC.git",
         "revision": "auto",
     }
    name = "FIoC"
    version = "1.0.2"
    license = "free to use"
    author = ""
    url = "https://github.com/forry/FIoC"
    description = "Lightweight C++ IoC implementation"
    topics = ("C++", "IoC")
    no_copy_source = True
    # No settings/options are necessary, this is header only

    def package(self):
        self.copy("*.h", "include")

    def package_id(self):
        self.info.header_only()