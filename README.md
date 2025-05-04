# IM 即时通讯系统

一个基于C++和GO开发的分布式即时通讯系统，采用客户端-服务器架构，提供实时消息传递、用户管理等功能。

## ✨ 功能特性

- 🔐 用户认证
  - 用户注册
  - 用户登录
- 💬 即时通讯
  - 私聊消息
  - 群组消息
  - 消息历史记录
- 👥 用户关系管理
  - 好友添加/删除
  - 好友列表管理
  - 用户在线状态显示
- ⚙️ 系统设置
  - 个性化设置
  - 消息通知设置
  - 界面主题设置

## 🛠️ 技术栈

### 客户端
- C++ 17
- Qt 6.7.3
- Protocol Buffers v5.29.3
- CMake 构建系统

### 服务器端
- Golang 1.23.3
- Consul
- Mysql
- Redis
- Protocol Buffers v5.29.1
- protoc-gen-go v1.36.0

## 📦 安装说明

### 环境要求
- C++ 17+
- CMake 3.10+
- Qt 6.7.3+
- Golang
- Mysql
- Redis
- Consul

### 编译步骤

1. 编译客户端
- 在CmakeLists中指定Qt安装的路径
- 安装Protobuf和spdlog
- 执行编译
```bash
cd Client
mkdir build && cd build
cmake ..
make
```

2. 编译服务器
- 依次进入Gate、User和Message目录
- 安装模块依赖 `go mod tidy`
- 执行编译 `go build`


## 🚀 使用说明
1. 启动Conusl、Mysql和Redis
2. 执行init.sql，导入数据库
3. 依次进入后端目录，在`config.yaml`中提供数据库连接等信息
4. 在`User/config.yaml`中设置邮箱SMTP服务
5. 启动所有服务
6. 开启客户端

## 📝 项目结构

```
IM/
├── Client/               # 客户端代码
│   ├── include/          # 头文件
│   ├── model/            # 数据模型
│   ├── proto/            # 协议定义
│   └── resource/         # 资源文件
├── Server/               # 服务器代码
│   ├── Gate/             # 网关服务器
│   ├── User/             # 用户服务器
│   └── Message/          # 消息服务器
```