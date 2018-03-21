#! /bin/bash

if [ -f input_$1.tar.gz ]; then
    rm ./package.tar.gz
fi

tar -hcf package.tar --exclude='.git' --exclude='my*.root' --exclude='*.tar*' --exclude='batchsubmit*' ../../../../../CMSSW_8_0_28
gzip ./package.tar
