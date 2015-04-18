#总体思路

# Introduction #

由于vim是多模式编辑器。所以，addin里应该提供“**陷入**”机制，每一个“陷阱”模拟一种模式，直到用户选择关闭addin模拟功能，则跳出所有陷阱。



# Details #

当然，这一切的前提是捕获用户的编辑器按键消息。

原型：
  * 捕获用户在代码编辑器里的按键消息
  * 用2个函数分别模拟命令模式和可视化模式（先做2个试试）。这2个函数都是陷阱，就是死循环，直到触发机关跳出陷阱
  * 提供一个表，表项为：| 陷阱名字 | vim按键命令 | 动作的名字 |
|:-------------|:----------------|:----------------|
> 这个表的作用是：在当前陷阱里，按下一串按键后，能触发按键命令对应的动作。<br>动作当然是要在陷阱里具体实现的。<br>
</li></ul><ul><li>按键命令解析器。（现在只支持单键命令即可。将来可扩充到支持将命令分解为基本动作流）