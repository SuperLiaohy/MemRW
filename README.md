# MemRW

> MemRW(memory read and write) 是本人从2025年8月1日历时开始的一个学习性项目，在同年9月12日主体首位（后期可能更改小部分）。本项目中的ui布局设计借鉴于一众实时数据采集软件，如[klonyyy/MCUViewer: Real-time embedded variable & trace viewer](https://github.com/klonyyy/MCUViewer), [RigoLigoRLC/ProbeScope: (UNDER CONSTRUCTION) Real time variable acquisition, data plotting via debug probe (ST-Link, CMSIS-DAP, and more extensible).](https://github.com/RigoLigoRLC/ProbeScope), [STM32CubeMonitor | Software - STMicroelectronics](https://www.st.com/en/development-tools/stm32cubemonitor.html), [LinkScope: 非侵入式硬件芯片调试软件，有变量示波、在线改值、数据导出和日志输出等功能](https://gitee.com/skythinker/link-scope)等。因在学习嵌软的MCU和进行自控中经常需要实时查看数据结果，经常使用上述软件，但是发现这些软件对于本人的环境适配的非常有限，本人喜欢使用较高的CPP标准并搭配`arm-none-eabi-gcc`或者`armclang`使用`cmake`构建。比如MCUViewer对cpp的dwarf支持实在太差，且没有CMSIS-DAP的实现，笔者实在无法忍受，这也是本人写这个项目的很大一部分原因，不过MCUViewer的颜值和使用确实不错，本项目很多设计理念都参考的MCUViewer。用过MCUViewer的应该可以很快适应。此外在变量的布局方面主要借鉴了ProbeScope，ProbeScope为按照树状的变量布局显示，比其他大部分用的表格布局显示极大地提升了便利性。

MemRW 目前只支持ARM MCU得支持ADIv5协议接口，并且调试接口要为SWD，使用CMSIS-DAPv2来通信。目前不支持CMSIS-DAPv1通信，因为v1使用的为hid协议，usb传输延时实在太大了故我在测了v1的尝试后之后便没有使用。而v2使用的usb bulk延时低，且可以大数据通信，故本项目使用的为CMSIS-DAPv2。之所以没有实现STLINK和JLINK的协议，是因为这两都是闭源的，想要实现得逆向。不过好在他们似乎都有自己的库可以用，不过我太懒了，懒得看了，没去实现，也许以后可以支持，至于为什么一定要是SWD，因为JTAG接口的寄存器我懒得配了，只配了SWD情况下的寄存器。

在数据采集方面：SWD速率默认10Mhz(没有留ui接口改,懒得写)， MemRW的设计充分利用了CMSIS-DAPv2的协议，实现了高效的数据采集。

在dwarf调试信息解析方面：在常规模式下直接对变量的die进行递归大部分都可以得到变量的完整信息，但是在写的时候发现有写代码是能正常过编译运行的，但是其类型的完整信息却无法直接通过对变量die直接递归获得，递归只能获得类型的声明die而非定义die，但是若直接查看dwarf文件会发现定义die却又存在，针对这种情况，MemRW设计了一个`DeepSearch`模式，此模式会提前遍历所有die，获取其中的类型die，使用其做为递归的补充，但是这种模式速度慢，并且使用情况非常少，故设计为可选。

在代码方面：只能说特别生草，一是这是本人第一次在上位机写较大的cpp项目，之前一直在mcu上写。其次Qt同样也是边用边学。再者就是本人在写的后面，写ui实在太无聊了，实在没有耐心了，就比较潦草首尾了。