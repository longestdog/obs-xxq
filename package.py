#!/usr/bin/env python
# -*- coding:utf-8 -*-
import os
import sys
import time
import platform
import shutil
import glob
import requests
import json
import zipfile

def log(*args, **kwargs):
    kwargs.setdefault('flush', True)
    print(*args, **kwargs)

def system(command, check_only=False):
    retcode = os.system(command)
    if not check_only and retcode != 0:
        raise Exception("Error while executing:\n\t %s" % command)
    return retcode

def execute_compile_bat():
    system('call win_x64_compile.bat ')

def update_ci_build_status(isOnline, pid, status):
    if pid == "0":
        return
    url = "https://test-app.yupaopao.com/api/ci/v3/publish/callback"
    if isOnline == "1":
        url = "https://app.yupaopao.com/api/ci/v3/publish/callback"
    payload = {
        "pid": pid,
        "status": status
    }
    response = requests.post(url, json=payload)
    log(f"paramList, url = {url},pid = {pid},status = {status},response={response.text}")
    response_data = response.json()
    if response_data["code"] != 8000:
        raise Exception(f"更新任务状态失败, pid = {pid}, status = {status}")
    else:
        log(response.text)

def main():
    version = "0.0.0"
    isOnline = "0"
    channel = "stable"
    nuwaId = "0"
    paramlen = len(sys.argv)
    if paramlen == 4:
        version = sys.argv[1]
        isOnline = sys.argv[2]
        nuwaId = sys.argv[3]
    elif paramlen == 3:
       version = sys.argv[1]
       isOnline = sys.argv[2]
    elif paramlen == 2:
       version = sys.argv[1]
    else:
        log("params error.");
        return;
    log(f"version: {version}, isOnline: {isOnline},nuwaId: {nuwaId}")
    
    if isOnline == "0":
       channel = "test"
    else:
        if nuwaId != "0":
            log("update summodule recursive")
            system("git submodule update --init --recursive")
        # 检查本地标签是否存在
        if system('git show-ref --tags %s' % version, check_only=True) == 0:
            system('git tag --delete %s' % version)
            try:
                system('git push origin --delete tag %s' % version)
            except Exception as e:
                log(f"Failed to delete remote tag, skipping: {e}")
    update_ci_build_status(isOnline, nuwaId, 1)
    try:
        #编译
        log("start execute compile...")
        execute_compile_bat()
        log("execute compile success!")
        #打包
        system('conan export-pkg . obs-xxq_x64/%s@bixin/%s -s compiler.version=15 -s arch=x86_64 -s build_type=Debug --force' % (version,channel))
        system('conan export-pkg . obs-xxq_x64/%s@bixin/%s -s compiler.version=15 -s arch=x86_64 -s build_type=Release --force' % (version,channel))
        if isOnline == "1":
            #上传
            system('conan upload obs-xxq_x64/%s@bixin/%s --all -r=pc' % (version,channel))
            system('git tag %s' % version)
            system('git push --tags')
        update_ci_build_status(isOnline, nuwaId, 2)
    except Exception as e:
        update_ci_build_status(isOnline, nuwaId, 3)
        log(f"Failed: {e}")
if __name__ == "__main__":
    main()