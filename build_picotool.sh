PICO_SDK_PATH=${PWD}/pico-sdk

mkdir picotool/build
cd picotool/build
cmake -DPICO_SDK_PATH=$PICO_SDK_PATH -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..
make