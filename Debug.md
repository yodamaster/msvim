#关于程序的调试

# Introduction #

因为visual studio add-in是运行在msdev.exe进程内的，所以，调试的时候，应该调试msdev.exe，并将add-in加载到msdev.exe进程内。


# Details #

Project->Settings...->Debug->Executable for debug session:<br>
C:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin\msdev.exe<br>
<br>
这样设置之后，按F5启动Debug调试。<br>
这时，一个新的visual studio被会启动，在这个新的visual studio的Tools->Customize...->Add-ins and Macro Files->Browser...选择我们编译出来的msvim.dll确定即可.