BigPot��ˮ��������


ʹ�������¿����⣺
FFmpeg, SDL2, iconv, jsoncpp, libass, fribidi, freetype, fontconfig

FFmpeg�������

--enable-shared --disable-symver --disable-doc --disable-symver --disable-doc --disable-ffplay --disable-ffmpeg --disable-ffprobe --disable-ffserver --disable-avdevice --disable-avfilter --disable-encoders --disable-muxers --disable-filters --disable-devices --disable-everything --disable-iconv --enable-protocols --enable-parsers --enable-demuxers --enable-decoders --enable-bsfs --enable-network --enable-swscale --enable-swresample --disable-demuxer=sbg --disable-demuxer=dts --disable-parser=dca --disable-decoder=dca --disable-yasm --enable-version3

����yasmʹ�õı������
./configure --enable-shared --enable-version3 --disable-symver --disable-doc --disable-ffplay --disable-ffmpeg --disable-ffprobe --disable-ffserver --disable-sdl --disable-zlib --disable-bzlib --disable-lzma --enable-swresample --disable-avfilter --disable-swscale

��̬������ҪԤ��ָ��
export LDFLAGS="-static-libgcc -static-libstdc++ -Wl,-static -lstdc++ -lpthread -lwinpthread"



