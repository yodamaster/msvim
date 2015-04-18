#Objects and the Object Hierarchy

# Introduction #

The following table shows the parent of each object:

| **Object** | **Parent object** |
|:-----------|:------------------|
| Application | Application |
| Breakpoint | Breakpoints |
| Breakpoints | Application |
| Configuration | Project |
| Configurations | Project |
| Debugger | Application |
| Document | Application |
| Documents | Application |
| Project | Application |
| Projects | Application |
| TextDocument | Application |
| TextEditor | Application |
| TextSelection | TextDocument |
| TextWindow | TextDocument |
| Window | Document (for "Generic" window types) <br />TextDocument (for "Text" window types) |
| Windows | Application |

Application
> Application (Application is its own parent)
> > Projects
> > > Project
> > > > Configurations
> > > > > Configuration
> > > > > > Configurations (circular)

> > Documents
> > > Document
> > > > Window (for "Generic" window types)

> > > TextDocument
> > > > TextSelection
> > > > TextWindow
> > > > Window (for "Text" window types)

> > TextEditor
> > Windows
> > Debugger
> > Breakpoints
> > > Breakpoint



# Details #

TextEditor是Application对象的一个属性。用来代表Develop Studio text editor。这玩意就是设置菜单项里面的那个Text Editor，可以设置Studio所用的字体、颜色、背景色之类的东西。很明显，一个Applicationd对象只有一个TextEditor对象。

Develop Studio里有很多窗口，比如Output窗口、Debug窗口、Workspace窗口、Watch窗口、代码或文本窗口等等，所有的窗口的集合称为Windows对象，可以通过Application对象的Windows属性来获得。代表所有文档窗口集合的Windows对象，可以通过Document的Windows属性获得。我说的是文档，不是文本。图像资源也是文档。文本资源在Studio里由特别的对象来建模，叫TextDocument，相应的，文本窗口就是TextWindow。



---
