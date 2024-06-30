if which ninja >/dev/null; then
    cmake -B build -G Ninja && \
    ninja -C build $1
else
    cmake -B build && \
    make -j $(getconf _NPROCESSORS_ONLN) -C build $1 && \
    echo "done. P.S.: Consider installing ninja - it's faster"
fi

if [[ $? -eq 0 ]]; then
# cd -
picotool load ./build/inky-digital-frame.uf2 -f
picotool reboot
# else
# cd -
fi