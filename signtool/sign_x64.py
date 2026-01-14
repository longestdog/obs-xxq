#!/usr/bin/env python
# -*- coding:utf-8 -*-
import os
import sys
import glob
import subprocess as sp
excludeSet = {'test.exe'}

def sign(file):
    sha1 = os.environ.get('SIGN_CERT_SHA1')
    if not sha1:
        print("Error: 环境变量'SIGN_CERT_SHA1'未设置, 无法完成签名操作")
        sys.exit(1)
    for i in range(0,3):
        sign_proc = subprocess.Popen("signtool sign /v /debug /fd sha256 /sha1 %s /tr http://rfc3161timestamp.globalsign.com/advanced /td sha256 %s" %(sha1, file))
        ret = sign_proc.wait()
        if ret == 0:
            return
    # 重试3次后仍然失败，抛出错误并退出
    print("Error: 签名失败, 文件: %s, 已重试3次" % file)
    sys.exit(1)

for file in glob.glob(os.path.join(sys.argv[1]+'/bin', '*.exe')):
    filepath,filename = os.path.split(file)
    if filename not in excludeSet:
        sign(file)
for file in glob.glob(os.path.join(sys.argv[1]+'/bin', '*.dll')):
    filepath,filename = os.path.split(file)
    if filename not in excludeSet:
        sign(file) 
        
for file in glob.glob(os.path.join(sys.argv[1]+'/plugins/obs-plugins/64bit', '*.exe')):
    filepath,filename = os.path.split(file)
    if filename not in excludeSet:
        sign(file)
for file in glob.glob(os.path.join(sys.argv[1]+'/plugins/obs-plugins/64bit', '*.dll')):
    filepath,filename = os.path.split(file)
    if filename not in excludeSet:
        sign(file) 

for file in glob.glob(os.path.join(sys.argv[1]+'/plugins/data/obs-plugins/enc-amf', '*.exe')):
    filepath,filename = os.path.split(file)
    if filename not in excludeSet:
        sign(file)
        
sign(sys.argv[1]+'/plugins/data/obs-plugins/win-capture/graphics-hook64.dll')
sign(sys.argv[1]+'/plugins/data/obs-plugins/win-capture/graphics-hook32.dll')
sign(sys.argv[1]+'/plugins/data/obs-plugins/win-capture/get-graphics-offsets64.exe')
sign(sys.argv[1]+'/plugins/data/obs-plugins/win-capture/get-graphics-offsets32.exe')
sign(sys.argv[1]+'/plugins/data/obs-plugins/wasapi-capture/wasapi-hook64.dll')
sign(sys.argv[1]+'/plugins/data/obs-plugins/wasapi-capture/wasapi-hook32.dll')
sign(sys.argv[1]+'/plugins/data/obs-plugins/wasapi-capture/get-wasapi-offsets64.exe')
sign(sys.argv[1]+'/plugins/data/obs-plugins/wasapi-capture/get-wasapi-offsets32.exe')