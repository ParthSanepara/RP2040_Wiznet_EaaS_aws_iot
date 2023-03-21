setx PICO_SDK_PATH "./libraries/pico-sdk"
cd build
cmake -G "NMake Makefiles" ..
cd ..