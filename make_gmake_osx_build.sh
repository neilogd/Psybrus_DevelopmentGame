#!/bin/bash

python Psybrus/reflection_parse.py DevelopmentGame

Psybrus/Tools/genie/genie-osx --platform=x64 --toolchain=linux-clang --boostpath=$BOOST_ROOT gmake
