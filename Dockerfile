FROM ubuntu:18.04

RUN apt-get update && \
    apt-get -y install g++ cmake make python3 gcc-aarch64-linux-gnu qemu git && \
    apt-get clean

RUN git clone https://github.com/dfszabo/miniCC.git

RUN mkdir build && cd build && cmake ../miniCC && make

RUN git clone https://github.com/dfszabo/tetris-c.git

RUN cd tetris-c && \
    git checkout dev/compiler-test-version && \
    cd -


# compiling tetris.c using miniCC and using the resulting assembly file to
# compile the Tetris app
RUN /bin/bash -c 'source $HOME/.bashrc; \
aarch64-linux-gnu-gcc tetris-c/src/main.c -c -static -g -O0; \
build/miniCC tetris-c/src/tetris.c > tetris_mcc.s; \
aarch64-linux-gnu-gcc tetris_mcc.s -c -static; \
aarch64-linux-gnu-gcc main.o tetris_mcc.o -o Tetris -fno-pie -static'

# run the resulting Tetris app with qemu, expected output is 419
RUN qemu-aarch64 Tetris

# compile it with ubuntu default gcc and run, expected output is 419
RUN cd tetris-c && make && ./Tetris
