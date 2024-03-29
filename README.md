# PCS(ProcessesCommunicationSystem)
A DDS(Data Distribution Service) implementation

## 一、简介
### 1. 目的
本系统旨在开发一种可运行在linux操作系统环境下，能够部分替代ROS1的库，其本质是一种能够实现分布式的、多个进程间组网的、通信的方法的库。本系统部分采用了DDS(Data Distribution Service)标准，实现了以数据为中心的发布/订阅(Data-CentricPublish-Subscribe)机制，DDS将分布式网络中传输的数据定义为主题(Topic),将数据的产生对象定义为发布者(Publisher),数据的接收对象定义为订阅者(Subscriber),两者执行的动作即发布(Publish)和订阅(Subscribe)，并将每一个进程实体称之为节点(Node)。使用订阅/发布机制可有效实现分布式的多进程通信中的数据解耦合,即节点间数据的发布和订阅是分开的，互不关系对方是否存在，只需通过主题(topic)来进行关联即可。此外，本系统在UDP协议基础上实现了一个节点间发现协议,可实现分布式网络中的节点间的自组网工作，包括网络节点的增添和删除管理，可实现一对一，一对多和多对多的进程间通信方式，并且只需要使用简单的几个接口函数即可实现。
### 2. 特点
#### a. 取消了Master节点
ROS1最大的一个特征之一是在系统开始工作之前要使用roscore命令启动一个Master节点，此节点即节点管理器，其功能主要有四个：为ROS节点提供命名和注册服务；帮助ROS节点间进行相互的查找；相同topic的节点间通信的建立以及提供一个参数服务器帮助ROS管理全局参数。故每一个节点的接入，删除，topic的维护都是通过Master节点来完成的。因此ROS1中Master节点的重要性高于一般的节点，一旦Master节点由于意外或者各种因素而退出，整个网络将彻底崩溃，不能工作。<br>
本系统针对这个问题采用了一个自定义的节点间发现协议(Nodes Discovery Protocol)来消除掉Master节点，即在此PCS网络中每一个节点都是平等的，任何节点的接入或者删除都不会影响到其他的节点的正常工作，通过topic进行匹配和建立通信链接的过程都是节点自己完成的，不需要再像ROS1中需要借助于Master节点。此方法的实现要点是在每一个节点中都维护一张主题信息表(Topic Information Table)，每当有节点的删除或接入就在整个PCS网络中对每一个节点的表都进行更新，确保所有节点所掌握的网络信息都是一样的。
#### b. 高实时性
本系统未使用任何第三方库，避免了封装带来的效率问题，通过直接调用linux操作系统的系统接口来实现所有的操作。进程间的通信主要是通过面向连接的TCP协议和共享内存来实现。在主机与主机之间的不同进程间使用TCP协议来进行通信，而在主机内部的进程间通信则采用共享内存来实现，这样通信的时延大小也主要取决消息在于信道间传输所花费的时间的大小，而tcp协议中解包或者共享内存的一些内存读写操作所消耗的时间影响极小，可满足绝大多数的任务需求。
#### c. 实现了两种消息时间戳同步的方法
在分布式应用或者多进程间通信过程中，消息同步是一件很重要的事情，本系统采用了两种消息同步的策略。分别是最近时间戳(Approximate TimeStamp Synchronization)同步方法和插值同步(Interpolation Synchronization)方法。其原理可在后续详解。
#### d. 可自动生成自定义消息格式的头文件
仿造ROS的自定义消息，在PCS系统中也实现了一个自动生成自定义消息.h文件的工具，通过在.message文件中以类C语言结构体风格定义一个自定义消息，可通过工具auto_message_generate生成一个.h文件，要想使用自定义消息时，包含这个头文件即可，非常方便。
#### e. 使用方法简单
由于采用了pubish/subscribe的方式，数据之间实现了完全的解耦合，节点与节点之间相互独立，因此在进程间进行通信的时候，每个节点都无需关心对方是否存在，是否接收到数据等等问题，不需要额外编写复杂的应答机制，无需关心底层通信的实现方法。此外要完成基本的通信功能，只需要调用publish、subscribe两个函数即可，极其方便快捷。

#### f. 订阅的回调机制
在通信过程中，我们不使用while循环阻塞接收消息，而是使用IO复用模型。IO多路复用是一种同步IO模型，可以实现通过在操作系统层面监听文件句柄，一旦某个文件句柄就绪，即一个事件(Event)就绪，就能够通知应用程序进行相应的读写操作。<br>
linux下最常用的IO复用模型有select，poll和epoll，本框架采取epoll模型。<br>
我们在IO复用机制的基础上实现了一个消息回调(Callback)处理机制。对于每个订阅者，在收到发布者发布的消息之后，将立即调用注册的回调函数，实现对消息的实时并且及时处理。

## 二、使用方法
### 1. 运行所需环境
此pcs库运行运行所需的操作系统为linux，要求安装cmake3.5.1或者其更高版本。
cmake的安装方式如下：
``` shell
运行 cmake --version 查看当前环境下是否已经安装了cmake
若没有安装，可执行以下语句进行安装：
apt-get install cmake
```
### 2. pcs库的使用
从github上拉取代码：
``` shell
git clone https://github.com/softdream/ProcessesCommunicationSystem.git
```
在build文件夹下依次执行：
``` shell
cmake ..
make 
```
### 3. pcs库的基本接口
3.1 pcs节点初始化<br>
pcs节点的初始化只需要实例化一个PCS实例即可：<br>
``` cpp
pcs::PCS n; 
```
3.2 订阅接口<br>
``` cpp
n.subscribe<Data Type>( "topic name", callback function );
```
3.3 发布接口<br>
``` cpp
pcs::Publisher pub =  n.advertise ("topic name");
...
while(1){
  ...
  pub.publishMessage<Data Type>( Data, Send Buff );
  ...
}
```
