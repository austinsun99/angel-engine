#!/bin/sh


if [[ "$*" == *"release"* ]]; then
    build_dir="release"
    build_opt="RELEASE"
else
    build_dir="debug"
    build_opt="DEBUG"
fi


if [ "$1" = "rebuild" ]; then
    rm -rf build
    rm compile_commands.json
fi

clear
echo "----- BUILDING -----"
echo

echo "Building to ${build_opt}"

make -n VERBOSE=1 BUILD=${build_opt} | bear parse-sh
make BUILD=${build_opt}

echo "----- BUILD FINISHED -----"
echo

./build/${build_dir}/bin/sandbox
exit 1
