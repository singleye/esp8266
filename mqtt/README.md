# 简介
这个程序是将nodeMCU作为一个MQTT的客户端运行。

程序每次启动后先把将芯片设置到STA模式下，并连接指定的WI-FI路由器。连接建立好之后连接指定的MQTT服务器，并注册从服务器接收数据的topic，并定期将数据（uptime）发送到服务器端指定的topic上。

* 上传topic："MAC地址/uplink"
* 接收topic："MAC地址/downlink"

当收到downlink消息后把数据进行解析并执行，目前只支持把芯片上的LED灯按照指定的次数点亮的简单操作。

# 使用方法
## 编译
我使用Arduino IDE进行开发，这是一个蛮不错的开发环境。不熟悉的人可以参考我的一篇[“使用Arduino IDE进行nodeMCU开发”](http://www.singleye.net/2017/04/使用arduino-ide进行nodemcu开发/)的blog。

## 运行
### nodeMCU客户端
将程序烧入nodeMCU后每次只要通电程序就会自动运行。

### 数据接收端
简单运行可以使用'mosquitto_sub'。命令可以参照下面的写法，需要用'-h'指定你的MQTT服务器地址，用'-t'指定接收的topic，这个topic会在nodeMCU每次运行时在串口输出，nodeMCU的MAC地址也会从串口输出。

```
mosquitto_sub -h "your_mqtt_server" -t "MAC_ADDRESS/uplink"
```

### 远程控制端
可以使用'mosquitto_pub'进行远程数据发送实现对nodeMCU的控制。下面的例子可以把LED连续点亮3次。

```
mosquitto_pub -h "your_mqtt_server" -t "MAC_ADDRESS/downlink" -m "blink#3"
```