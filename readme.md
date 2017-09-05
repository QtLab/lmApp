# 项目说明:
 * [学习项目](https://github.com/lheric/GitlHEVCAnalyzer)；
 * 以上述项目为模板，学习**QT**和**设计模式**；
 * [项目备忘](doc/项目备忘.md)
## 配置环境:
[**Visual studio 2015 com**](https://www.visualstudio.com/);
[**QT5.7.0**](https://www.qt.io/qt5-7/)
~~Maybe [**Opencv2.4.12**]()~~
## 项目功能

- [x] yuv文件查看和播放
- [x] 帧保存为图片
- [x] SHVC码流解码
- [x] CU分割可视化
- [ ] 码流信息可视化
- [ ] 其他解码器如HEVC、3D-HEVC等

## 测试版本下载
[lmSHVCBitStreamPlayer_0.9.0下载](http://download.csdn.net/detail/li651138628/9926455)
## 功能演示
* 提示：处于开发状态，以下介绍仅供参考.
### 主界面
![](/doc/C0.png)

* 功能区，列表区，显示区和播放区共4个区域
### 打开yuv文件
![](/doc/C1.png)

* 目前只提供尺寸和采样格式选择
### yuv播放控制
![](/doc/C5.png)

* 播放、停止、步进、步退、复位、末尾以及拉杆控制
* 显示相关信息
### yuv像素查看
![](/doc/C2.png)

* yuv像素值显示，像素信息显示
### SHVC解码
![](/doc/C3.png)

* 主要实现编码层级选择显示
### 层级列表
![](/doc/C4.png)

* 选择相应的层级，会显示相应的层级

### CU分割可视化
![](/doc/C6.png)
* CU分割4叉树

### 比特可视化
![](/doc/C7.png)
* 比特热图，显示ctu的比特
### 其他功能等待开发...