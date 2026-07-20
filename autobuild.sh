```bash
########################################################################
# File Name: autobuild.sh
# Author: Feng nan
# mail: 2631594479@qq.com
# Created Time:2026年07月20日
########################################################################
#!/bin/bash

set -x

rm -rf `pwd`/build/*
cd `pwd`/build &&
    cmake .. &&
    make
```
