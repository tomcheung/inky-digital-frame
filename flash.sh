if which ninja >/dev/null; then
    cmake -B build -G Ninja && \
    ninja -C build $1
else
    cmake -B build && \
    make -j $(getconf _NPROCESSORS_ONLN) -C build $1 && \
    echo "done. P.S.: Consider installing ninja - it's faster"
fi

PICO_TOOL_PATH=./picotool/build

if [[ $? -eq 0 ]]; then
# cd -
$PICO_TOOL_PATH/picotool load ./build/inky-digital-frame.uf2 -f
$PICO_TOOL_PATH/picotool reboot
# else
# cd -
fi