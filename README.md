# tinypot
<img src="https://raw.githubusercontent.com/scarsty/bigpot/master/logo.png" width = "20%" />

一个轻量级播放器。

## 编译
### Windows
#### VS2017
已经包含了Windows版本所需的工程文件，以及导入库和dll。其中大部分是作者自行编译。
#### mingw
因为各个库的依赖很复杂，不建议使用。

### macosx
需先安装SDL2以及相关扩展的framework，并安装libass、FFmpeg、iconv等，通常homebrew可以做。jsoncpp是可选的。脚本a.sh可以自动做编译和处理库依赖。

### linux
跟上面方法类似，但是一般不需要解决依赖库了。

## 遗留问题
因为是单线程架构，所以在一些文件跳转时会出现马赛克。一般来说这个可以通过清除解码器状态来解决，但是单线程架构下这个操作会导致后面一帧的解码卡顿，故没有这么做。或者谁有更好的办法可以指教一下。
srt支持不完善，只能使用没有bom的utf8文件，而且时间不能出负值。等ka大神有空解决。

## 功能键
方向左右是跳过几秒，上下是音量，空格是暂停，回车是全屏，退格是返回文件开始，del删除播放记录（打开曾经播放过的文件会自动跳转到上次退出的位置）。
1是切换音频流，2是切换字幕流，3是内部字幕显示/隐藏，4是外部字幕显示/隐藏。
鼠标点右上是暂停。

## 预编译版下载
windows版：
<http://www.dawuxia.net/tinypot/tinypot-win64.zip>

Mac版：
<http://www.dawuxia.net/tinypot/tinypot.app.zip>


## 常见问题

#### Q：使用什么开发？
A：程序语言是C++，使用FFmpeg进行解码，SDL2硬件输出，还有SDL_image、SDL_ttf等库。字幕部分使用libass，该库又依赖Fontconfig、freetype和fribidi。配置文件使用的是tinyxml2和jsoncpp（二选一），hash是sha3。

#### Q：播放器采用了什么架构？
A：该播放器的架构并未参考其他主流播放器，而是重新设计的单线程预解，原理如下图。在跳转的时候可能会稍慢于其他的主流播放器，但是相差并不明显。

<img src="https://raw.githubusercontent.com/scarsty/bigpot/master/pic/ac.png" width = "50%" />

#### Q：为什么叫大水壶？
A：大水壶播放器的前身是金庸水浒传的片头动画播放子程。在整个游戏过程中该子程仅被调用了一次，但是为了做好这个部分，开发组使用了FFmpeg进行解码，BASS进行播放，SDL2进行输出，并成功将其移植到了其他平台。因此，金庸水浒传的片头实际支持相当多的格式。而大水壶播放器在设计阶段，也是使用类似的架构，但是在开发阶段发现音频难以控制，因此改为了使用SDL2播放。
而水浒的谐音是水壶，同时新论坛叫大武侠，有个“大”字，所以该播放器起名“大水壶”，英文BigPot。至于跟著名的播放器PotPlayer有没有关系，答案是一点都没有，而且PotPlayer的功能远远强于大水壶，名字有点像只是巧合。

另外现在改叫小水壶了……

#### Q：可以播放什么格式？
A：FFmpeg能解什么格式它就能放什么格式，FFmpeg不能解的，它也放不出来。而且也不考虑调用其他的解码器，因为作者不会。
特别地，不能播放WAV，以及WAV为音频流的视频文件，因为WAV是没有压缩的，谈不上解码。也不推荐用它播放纯音频，因为它的音频没有经过处理，只是把解码的结果原样放出来，远不及专门的播放器。

#### Q：怎样打开文件？
A：因为没有制作配置的图形界面，所以仅能将文件拖到图标或者窗口上进行播放。
打开文件的时候，会先判断有没有字幕，有的话会自动载入。或者播放的时候拖一个字幕进去也会载入字幕，而字幕的扩展名必须是ass、ssa、srt、txt其中之一。其他文件都会当成媒体文件处理，能否播放看解码器的。

#### Q：音量怎样改变？
A：鼠标滚轮或者上下方向，没有制作拖动音量条。

#### Q：能否跨平台？
A：可以。

#### Q：会不会开源？
A：以lgpl协议开源。

#### Q：还有什么其他功能？

A：可以嵌入其他程序的窗口播放。特别是基于SDL2的游戏，用法非常简单。

## 播放效果
<img src="https://raw.githubusercontent.com/scarsty/bigpot/master/pic/1.png" width = "80%" />

<img src="https://raw.githubusercontent.com/scarsty/bigpot/master/pic/2.png" width = "80%" />
