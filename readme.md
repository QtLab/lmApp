# 项目说明:
[学习项目](https://github.com/lheric/GitlHEVCAnalyzer)；
以上述项目为模板，学习**QT-GUI**和**设计模式**，实现H.265码流分析;
## 配置环境:
[**Visual studio 2015 com**](https://www.visualstudio.com/)；
[**QT5.7.0**](https://www.qt.io/qt5-7/);
~~Maybe [**Opencv2.4.12**]();~~
## LongmanEvt项目
模仿学习项目，实现了以QT库为基础的**Event发送总线**功能；在实现过程中，实践了学过的**单例模式**，**克隆模式**；了解*std::function*、*std::bind*和*std::map*等用法；在利用*union*实现*Variant*数据格式时，了解到*union*的一些特点，比如，其成员不能是带有**默认构造函数**的对象，但可以是对象指针。
 在本模块与目标模块实现耦合的过程中发现有两种耦合方式，不太理解原因:
**1.在本模块头文件中直接包含目标模块的头文件。**
**2.在本模块源文件包含头文件，在本模块头文件中声明对象；**
学习了<<efficient C++>>后，上述方式2的耦合方式应该是所谓的前置声明...这个问题有待研究理解；
*model*与*EventBus*之间关系有点像**观察者模式**，*model*扮演观察者，*EventBus*扮演被观察者。*model*在构造函数中，创建了**单例***EventBus*实例，并在该实例中注册*model*，其注册过程为:将*EventBus*的触发信号与*model*的处理函数，通过QT的*signal/slot*机制相连接，传递参数为*Event*的指针。在这里，*model*模块中的处理函数实现**update**功能，*EventBus*的触发信号实现**notify**功能
## LongmanMVC项目
继承**LongmanEvt项目**的*lmmodel基类*的两个派生类分别是*lmView派生类*和*lmController派生类*，本项目中M和C结合在一起，由*lmController派生类*实现。
**lmController派生类:**
 使用单例模式，实现用户命令的接受、排队和执行，并通知View模块。
所有命令交由其成员*cmdProcessThread类*处理，(计划:目前，许多重要的通知View过程都放在*cmdProcessThread类*，~~(计划将所有关于*LongmanEvt*构建发送功能放到*lmController派生类*中)~~
在*cmdProcessThread类*中构建简单的事件排队机制。
正在思考如何进一步优化*lmController派生类*，该模块与V模块的依赖性过强，且多线程结构存在问题。