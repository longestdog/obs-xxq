#!/usr/bin/env python
# -*- coding:utf-8 -*-
import os
import sys
import time
import platform
import shutil
import glob
import json
import zipfile
import requests
  
def system(command):
    retcode = os.system(command)
    if retcode != 0:
        raise Exception("Error while executing:\n\t %s" % command)

def execute_compile_bat():
    system('call win_x64_compile.bat ')

def update_ci_build_status(pid, status):
    url = "https://test-app.yupaopao.com/api/ci/v3/publish/callback"
    payload = {
       "pid": pid,
       "status": status
    }
    response = requests.post(url, json=payload)
    response_data = response.json()
    if response_data["code"] != 8000:
       raise Exception("更新任务状态失败, pid = {pid}, status = {status}")

def main():
    version = "0.0.0"
    isOnline = "0"
    channel = "stable"
    nuwaId = "0"
    paramlen = len(sys.argv)
    if paramlen == 5:
       version = sys.argv[1]
       isOnline = sys.argv[2]
       nuwaId = sys.argv[4]
       print("version:"+str(version)+" isOnline:"+str(isOnline) + " nuwaId:"+str(nuwaId))
    elif paramlen == 4:
       version = sys.argv[1]
       isOnline = sys.argv[2]
       nuwaId = sys.argv[3]
       print("version:"+str(version)+" isOnline:"+str(isOnline) + " nuwaId:"+str(nuwaId))
    elif paramlen == 3:
       version = sys.argv[1]
       isOnline = sys.argv[2]
       print("version:"+str(version)+" isOnline:"+str(isOnline) + " nuwaId:"+str(nuwaId))
    elif paramlen == 2:
       version = sys.argv[1]
       print("version:"+str(version))
    elif paramlen == 1:
       print("version:"+str(version)+" isOnline:"+str(isOnline))
    else:
       print("params error.");
       return;
    if isOnline == "0":
       channel = "test"
    if os.path.exists("conanfile.py") :
        os.remove("conanfile.py")
    shutil.copyfile("conanfile_x64.py", "conanfile.py")
    
    if nuwaId != "0":
      update_ci_build_status(nuwaId, 1)
    try:
      if nuwaId != "0":
         system("git submodule update -init --recursive")
         execute_compile_bat()
      system('conan export-pkg . obs-xxq_x64/%s@bixin/%s -s compiler.version=15 -s arch=x86_64 -s build_type=Debug --force' % (version,channel))
      system('conan export-pkg . obs-xxq_x64/%s@bixin/%s -s compiler.version=15 -s arch=x86_64 -s build_type=Release --force' % (version,channel))
      if isOnline == "1":
         system('conan upload obs-xxq_x64/%s@bixin/%s --all -r=pc' % (version,channel))
         system('git tag %s' % version)
         system('git push --tags')
         if nuwaId != "0":
            update_ci_build_status(nuwaId, 2)
    except Exception as e:
         if nuwaId!= "0":
            update_ci_build_status(nuwaId, 3)
         print(str(e))
    if os.path.exists("conanfile.py") :
       os.remove("conanfile.py")
if __name__ == "__main__":
    main()
