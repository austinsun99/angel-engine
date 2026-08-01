#!/bin/sh


if [[ "$*" == *"release"* ]]; then
    build_dir="release"
    build_opt="RELEASE"
else
    build_dir="debug"
    build_opt="DEBUG"
fi


if [ "$1" = "rebuild" ]; then
    make clean
    rm compile_commands.json
fi

clear
echo "----- BUILDING -----"
echo

echo "Building to ${build_opt}"
time bear -- make BUILD=${build_opt}

echo "----- BUILD FINISHED -----"
echo

./build/${build_dir}/bin/sandbox
exit 1
