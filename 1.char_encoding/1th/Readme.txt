你可以用sourceinsight4打开ansi.txt和utf-8.txt来看看，ansi.txt中的“中”字在si4里显示是乱码，而utf-8.txt正常显示，为什么呢？因为si4默认以unicode编码方式打开一个文件，而ansi.txt是以ansi的编码(对于中文，是以GBK编码)来保存文件的，所以看起来是乱码。

utf-16le可以正常显示
utf-16be显示乱码

所以，以后最好以utf-8来保存文件。