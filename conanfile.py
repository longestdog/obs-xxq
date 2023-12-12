import os
from conans import ConanFile, MSBuild


class ObsXXQConan(ConanFile):
    license = "MIT"
    name = "obs-xxq_x64"
    url = "git@git.yupaopao.com:pc-media/obs-xxq.git"
    settings = "os", "compiler", "build_type", "arch"
    generators = "visual_studio"

    def package(self):
    
        self.copy("*.h", dst="include/libobs", src="libobs", keep_path=True)
        self.copy("*.hpp", dst="include/libobs", src="libobs", keep_path=True)
        
        if self.settings.build_type=="Debug":
            self.copy("*.dll", dst="bin", src="xxq-obs-deps/bin", keep_path=False)
            self.copy("*.lib", dst="lib", src="build64/libobs/Debug", keep_path=True)
        else:
            self.copy("*.dll", dst="bin", src="xxq-obs-deps/bin", keep_path=False, excludes="debug")
            self.copy("*.lib", dst="lib", src="build64/libobs/RelWithDebInfo", keep_path=True)
            
            
        if self.settings.build_type=="Debug":
            self.copy("*.dll", dst="bin", src="xxq-obs-deps/bin/debug", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/libobs/Debug", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/libobs/Debug", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/libobs-d3d11/Debug", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/libobs-d3d11/Debug", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/libobs-opengl/Debug", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/libobs-opengl/Debug", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/libobs-winrt/Debug", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/libobs-winrt/Debug", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/deps/obs-scripting/Debug", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/deps/obs-scripting/Debug", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/deps/w32-pthreads/Debug", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/deps/w32-pthreads/Debug", keep_path=False)
           
            self.copy("*.dll", dst="plugins/obs-plugins/64bit", src="build64/plugins/obs-ffmpeg/Debug", keep_path=False)
            self.copy("*.pdb", dst="plugins/obs-plugins/64bit", src="build64/plugins/obs-ffmpeg/Debug", keep_path=False)
            self.copy("*.dll", dst="plugins/obs-plugins/64bit", src="build64/plugins/obs-outputs/Debug", keep_path=False)
            self.copy("*.pdb", dst="plugins/obs-plugins/64bit", src="build64/plugins/obs-outputs/Debug", keep_path=False)
            self.copy("*.dll", dst="plugins/obs-plugins/64bit", src="build64/plugins/win-capture/Debug", keep_path=False)
            self.copy("*.pdb", dst="plugins/obs-plugins/64bit", src="build64/plugins/win-capture/Debug", keep_path=False)
            self.copy("*.dll", dst="plugins/data/obs-plugins/win-capture", src="build64/plugins/win-capture/graphics-hook/Debug", keep_path=False)
            self.copy("*.pdb*", dst="plugins/data/obs-plugins/win-capture", src="build64/plugins/win-capture/graphics-hook/Debug", keep_path=False)
            self.copy("*.exe", dst="plugins/data/obs-plugins/win-capture", src="build64/plugins/win-capture/get-graphics-offsets/Debug", keep_path=False)
            self.copy("*.pdb*", dst="plugins/data/obs-plugins/win-capture", src="build64/plugins/win-capture/get-graphics-offsets/Debug", keep_path=False)
            self.copy("*.dll", dst="plugins/data/obs-plugins/win-capture", src="build32/plugins/win-capture/graphics-hook/Debug", keep_path=False)
            self.copy("*.pdb*", dst="plugins/data/obs-plugins/win-capture", src="build32/plugins/win-capture/graphics-hook/Debug", keep_path=False)
            self.copy("*.exe", dst="plugins/data/obs-plugins/win-capture", src="build32/plugins/win-capture/get-graphics-offsets/Debug", keep_path=False)
            self.copy("*.pdb*", dst="plugins/data/obs-plugins/win-capture", src="build32/plugins/win-capture/get-graphics-offsets/Debug", keep_path=False)


        else:
            self.copy("*.dll", dst="bin", src="build64/libobs/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/libobs/RelWithDebInfo", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/libobs-d3d11/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/libobs-d3d11/RelWithDebInfo", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/libobs-opengl/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/libobs-opengl/RelWithDebInfo", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/libobs-winrt/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/libobs-winrt/RelWithDebInfo", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/deps/obs-scripting/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/deps/obs-scripting/RelWithDebInfo", keep_path=False)
            self.copy("*.dll", dst="bin", src="build64/deps/w32-pthreads/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="bin", src="build64/deps/w32-pthreads/RelWithDebInfo", keep_path=False)
           
            self.copy("*.dll", dst="plugins/obs-plugins/64bit", src="build64/plugins/obs-ffmpeg/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="plugins/obs-plugins/64bit", src="build64/plugins/obs-ffmpeg/RelWithDebInfo", keep_path=False)
            self.copy("*.dll", dst="plugins/obs-plugins/64bit", src="build64/plugins/obs-outputs/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="plugins/obs-plugins/64bit", src="build64/plugins/obs-outputs/RelWithDebInfo", keep_path=False)
            self.copy("*.dll", dst="plugins/obs-plugins/64bit", src="build64/plugins/win-capture/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="plugins/obs-plugins/64bit", src="build64/plugins/win-capture/RelWithDebInfo", keep_path=False)
            self.copy("*.dll", dst="plugins/data/obs-plugins/win-capture", src="build64/plugins/win-capture/graphics-hook/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="plugins/data/obs-plugins/win-capture", src="build64/plugins/win-capture/graphics-hook/RelWithDebInfo", keep_path=False)
            self.copy("*.exe", dst="plugins/data/obs-plugins/win-capture", src="build64/plugins/win-capture/get-graphics-offsets/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb*", dst="plugins/data/obs-plugins/win-capture", src="build64/plugins/win-capture/get-graphics-offsets/RelWithDebInfo", keep_path=False)
            self.copy("*.dll", dst="plugins/data/obs-plugins/win-capture", src="build32/plugins/win-capture/graphics-hook/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb", dst="plugins/data/obs-plugins/win-capture", src="build32/plugins/win-capture/graphics-hook/RelWithDebInfo", keep_path=False)
            self.copy("*.exe", dst="plugins/data/obs-plugins/win-capture", src="build32/plugins/win-capture/get-graphics-offsets/RelWithDebInfo", keep_path=False)
            self.copy("*.pdb*", dst="plugins/data/obs-plugins/win-capture", src="build32/plugins/win-capture/get-graphics-offsets/RelWithDebInfo", keep_path=False)
            
            
            
        self.copy("*.*", dst="plugins/data/libobs", src="libobs/data", keep_path=True)
        
        
        self.copy("*.*", dst="plugins/data/obs-plugins/obs-ffmpeg", src="plugins/obs-ffmpeg/data", keep_path=True)
        self.copy("*.*", dst="plugins/data/obs-plugins/obs-outputs", src="plugins/obs-outputs/data", keep_path=True)
        self.copy("*.*", dst="plugins/data/obs-plugins/win-capture", src="plugins/win-capture/data", keep_path=True)
        
        if self.settings.build_type!="Debug":
            os.system("cd signtool && python sign_x64.py " + self.package_folder)

    def package_info(self):
        self.cpp_info.libs = ["obs"]
