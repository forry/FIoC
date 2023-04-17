import os
from conan import ConanFile
from conan.tools.scm import Git
from conan.tools.files import copy, load, update_conandata

class FIoCConan(ConanFile):
    name = "fioc"
    version = "1.0.2"
    license = "free to use"
    author = "Tomas Starka"
    url = "https://github.com/forry/FIoC.git"
    homepage = "https://github.com/forry/FIoC"
    description = "Lightweight C++ IoC implementation"
    topics = ("C++", "IoC", "headers only")
    no_copy_source = True
    
    def layout(self):
        #self.folders.source is defaulted to "" so we need just set include dirs for editable layout the same
        self.cpp.source.includedirs = [""]
    
    def export(self):
        git = Git(self, self.recipe_folder)
        # we ignore the URL but we want the checks that are done on the commit byt this function
        scm_url, scm_commit = git.get_url_and_commit()
        self.output.info(f"Obtained URL: {scm_url} and {scm_commit}")
        # we store the current url and commit in conandata.yml
        update_conandata(self, {"sources": {"commit": scm_commit, "url": self.url}})
        
    def source(self):
        # we recover the saved url and commit from conandata.yml and use them to get sources
        git = Git(self)
        sources = self.conan_data["sources"]
        self.output.info(f"Cloning sources from: {sources}")
        git.clone(url=sources["url"], target=".")
        git.checkout(commit=sources["commit"])

    def package(self):
        copy(self, "*.h", self.source_folder, os.path.join(self.package_folder, "include"))

    def package_id(self):
        self.info.clear()