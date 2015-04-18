#Subclassing the TextWindow

# Introduction #

介绍接下来的一些任务。


# Details #

~~插件起来的第一件事，就是获取所有的TextDocument，以及这些TextDocument所有的TextWindow，~~
我想多了，仅凭窗口HWND句柄，不能反向获取这个窗口相关联的逻辑对象ITextDocument与ITextWindow之类等等。因此，咱们这个插件只能随vc6启动的时候就运行。

插件起来第一件事，就是subclassing MDIClient：用来获取每个文本窗口的新建和销毁信息，以及每个文本窗口的HWND。在句柄中绑定这些属性：caret、输入模式、ITextWindow、命令历史列表、undo/redo相关信息等。考虑Subclassing这个子窗口类，这样，就能截获按键消息了。达到这一步，界面方面的工作应该就差不多了。


