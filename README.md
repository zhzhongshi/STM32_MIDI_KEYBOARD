 用STM32F103C8T6 整了个midi键盘（bushi

37键，8是C5
|PIN|B4|B5|B6|B7|B8|
|-|-|-|-|-|-|
|B9|1|2|3|4|5|
|A10|6|7|8|9|10|
|A9|11|12|13|14|15|
|A8|16|17|18|19|20|
|B15|21|22|23|24|25|
|B14|26|27|28|29|30|
|B13|31|32|33|34|35|
|B12|36|37|  |  |  |

编译

你需要下载arm-none-eabi-gcc工具链，make，以及openocd stlink工具。
将这些加入path环境变量中。

然后在项目目录下打开终端，输入
```
make clean
make
```
`make clean`是清理之前编译的结果，`make`是编译程序。


烧写

将编译好的程序烧写到stm32f103c8t6芯片中。

首先需要安装openocd工具，我默认已经配置好stlink以及相关驱动
```
openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -c "program build/midi_keyboard.hex verify reset exit"
```
将stm32f103c8t6用usb线缆插在电脑上，会识别为stm32 midi设备。  
至此，midi键盘已经完成了 xD
