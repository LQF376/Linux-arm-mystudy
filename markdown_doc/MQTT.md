# MQTT

## 1. MQTT  协议相关概念

- 应用层协议，构建于 TCP/IP 协议之上
- 使用 发布/订阅消息模式，提供一对多的消息发布（基于客户端-服务器架构的消息传输协议）
- 1个服务端、多个客户端之间围绕“主题”进行通讯

### 1.1 MQTT 通信基本原理

一种基于客户端-服务端架构的消息传输协议

> 服务端：一台服务器（broker），作为 MQTT 信息传输的枢纽
>
> 客户端：MQTT 客户端可以向服务端发布消息，也可以从服务端订阅消息

客户端发布消息时需要为消息指定一个“主题”，表示消息发布到该主题；而订阅消息的客户端，通过订阅“主题”来订阅消息，这样当其他客户端或者当前客户端向该主题发布消息时，MQTT 服务端就会向该主题的信息发送给该主题的订阅者（客户端）

![1691395630155](E:\typora\markdownImage\1691395630155.png)

### 1.2 客户端连接服务端的过程

1. 客户端向服务端发起连接请求；本质是一个 CONNECT 报文
2. 服务端收到客户端的连接请求后，向客户端发送连接确认；本质：CONNACK 报文

------

报文：就是一个数据包

> MQTT 报文组成分三个部分：
>
> 1. 固定头 Fixed header
>
>    存在于所有 MQTT 报文中，固定头中有报文类型标识符，决定是哪一种类型的报文
>
> 2. 可变头 Variable header
>
>    存在于部分报文类型中，报文的类型决定了可变头是否存在以及其具体信息
>
> 3. 消息体 Playload
>
>    存在于部分报文类型中，消息载体的意思

------

#### CONNECT 报文

![1691395988353](E:\typora\markdownImage\1691395988353.png)

> clientID：客户端标识，服务端用来区分不同的客户端
>
> keepAlive：心跳时间间隔，客户端向服务端发送心跳包的时间间隔（单位：秒）
>
> - 客户端在空闲时，会定时的向服务端发送一个心跳数据包（心跳请求），用来告知服务端，当前客户端依然在线；服务端在收到客户端的心跳请求后，会回复一条消息（心跳响应）
>
> cleanSession：控制客户端与服务端在连接和断开连接时的行为
>
> - cleanSession = 0，当 MQTT 客户端由离线再次上线时，离线期间发给客户端的所有 QoS > 0 的消息仍然可以接收到；服务端会保存、存储客户端所订阅的主题；表示创建一个持久性会话，在客户端断开连接时，会话仍然保持并保存离线消息，直到会话超时注销
> - cleanSession = 1，当 MQTT 客户端由离线再次上线时，离线期间发给客户端的所有消息一律收不到；表示此次连接将创建一个新的临时会话，在客户端断开后，会话将自动销毁

#### CONNACK 报文

![1691397097430](E:\typora\markdownImage\1691397097430.png)

> returnCode：服务端收到客户端的连接请求后，会向客户端发送一个 returnCode（连接返回码）；非零表示连接失败
>
> sessionPresent：服务端告知客户端有没有保存会话状态（客户端上一次连接时的会话状态信息），与 cleanSession 相关
>
> - sessionPresent = 1，服务端为客户端保存了上一次连接时的会话状态
> - sessionPresent = 0，没有保存会话状态

![1691398531777](E:\typora\markdownImage\1691398531777.png)

![1691398547263](E:\typora\markdownImage\1691398547263.png)

### 1.3 客户端与服务端断开连接

客户端向服务端发送一个 DISCONNECT 报文来断开连接

#### 1.4 发布消息

客户端向服务端发布消息，其实就是向服务端发送一个 PUBLISH 报文，服务端收到客户端的 PUBLISH 报文之后，会回复一个报文（根据 QoS 不同，回复的报文类型不同）

![1691398797637](E:\typora\markdownImage\1691398797637.png)

> packetId：报文标识符；用来识别管理不同的报文，只有 QoS > 0，报文标识符才是非零数值
>
> topicName：主题名字
>
> playload：有效载荷，待发送消息的实际内容
>
> qos：服务质量等级，0，1，2 三个等级
>
> retain：保留标志；一般情况下，只有客户端订阅该主题之后，服务端接收到该主题的新消息时，服务端才会将最新收到的该主题消息推送给客户端，retain 可以保留一条消息，使得客户端在订阅了某个主题后马上接收到一条该主题的信息
>
> dup：指示此消息是否重复；报文接收方没有及时回复确认收到报文时，发送方会重复发送报文，dup 置 true（QoS > 0）

#### 1.5 订阅主题

1. 客户端向服务端发送主题订阅请求，本质是向服务端发送 SUBSCRIBE 报文
2. 服务端收到 SUBSCRIBE 报文之后会向客户端回复一个 SUBACK 报文（订阅确认报文）

#### SUBSCRIBE 报文

> 包含：订阅主题名（可以一次订阅多个主题）、QoS、报文标识符

#### SUBACK 报文

> 包含：订阅返回码（是否订阅成功） 和 报文标识符

![1691399612946](E:\typora\markdownImage\1691399612946.png)

### 1.6 取消订阅

1. 客户端向服务端发送 UNSUBSCRIBE 报文来取消订阅主题
2. 服务端收到 UNSUBSCRIBE 报文后，回复一个 UNSUBACK 取消订阅确认报文

#### UNSUBSCRIBE 报文

> 包含：取消订阅的主题名称、报文标识符

#### UNSUBACK 报文

> 包含：UNSUBSCRIBE 报文 中的 报文标识符

## 2.  主题相关

### 2.1 单级通配符 +

单级通配符可以匹配任意一个主题级别 

```
"home/sensor/+/status"
```

### 2.2 多级通配符 #

多级通配符自然是可以匹配任意数量个主题级别 

```
"home/sensor/#"
```

### 2.3 系统保留特殊主题

以 $ 号开头的主题是 MQTT 服务端系统保留的特殊主题，客户端不可随意订阅或向其发布信息 

## 3. Qos （Quality of Service，服务质量等级）

> 1. QoS = 0：最多发一次 
>    - 客户端和服务端不会对消息传输是否成功进行确认和检查，只管发一次（完全依赖于 TCP 重传机制）
> 2. QoS = 1：最少发一次 
>    - 发送端在消息发送完成后，会检查接收端是否已经成功收到消息
> 3. QoS = 2：保证收一次 
>    - 保证接收端只接收一次消息；发送端需要接收端进行两次消息确认

#### QoS = 1

![1691400508644](E:\typora\markdownImage\1691400508644.png)

- 发送端向接收端发送 PUBLISH 报文，当接收端收到 PUBLISH 报文后会向发送端回复一个 PUBACK 报文，如果发送端收到 PUBACK 报文，则表示接收端已成功收到该消息
- 若一段时间内，发送端未接收到 PUBACK 报文，那么发送端将再次发送消息（PUBLISH 报文中 dup 置 true）
- 注意 接收端并不会去对 dup 做任何检查，不会进行去重处理

#### QoS = 2

1. 发送端向接收端发送 PUBLISH 报文；（发布报文）
2. 接收端接收到 PUBLISH 报文后，向发送端回复一个 PUBREC 报文（发布收到）
3. 发送端接收到 PUBREC 报文后，会再次向接收端发送 PUBREL 报文 （发布释放）
4. 接收端接收到 PUBREL 报文后，会再次向发送端回复一个 PUBCOMP 报文 （发布完成）

要想实现 QoS>0 的 MQTT 通信，客户端在连接服务端时务必要将 cleanSession 设置为 false。如果这一步没有实现，那么客户端是无法实现 QoS>0 的 MQTT 通信， 

## 4. 保留消息

客户端向服务端发布消息时retain 标志设置为 true，以告诉服务端接收到此消息之后需要保留这个消息，这样服务端就会将该消息进行存储、保留， 无论客户端在任何时间订阅室温主题， 订阅之后都会马上收到该主题中的“保留消息” 

每一个主题只能有一个“保留消息” 

## 5. 心跳机制

1. 让客户端在没有向服务端发送消息的这个空闲时间里，定时向服务端发送一个心跳包， 这个心跳包被称为心跳请求（PINGREQ 报文）
2. 当服务端收到 PINGREQ 报文后就知道该客户端依然在线，然后向客户端回复一个 PINGRESP 报文，称为心跳响应 

 这个心跳机制不仅可以用于服务端判断客户端是否在线，客户端也可使用心跳机制来判断自己与服务端是否保持连接 

## 6. 遗嘱机制

客户端在“连接”的时候就写好遗嘱，这样一旦客户端意外断线，服务端就可以将客户端的遗嘱公之于众

![1691401451564](E:\typora\markdownImage\1691401451564.png)

> willTopic：遗嘱主题；告知服务端，遗嘱主题是什么，只有订阅了遗嘱主题的客户端才会收到消息
>
> willMessage：遗嘱消息；即遗嘱内容
>
> willRetain：保留标志，遗嘱信息也可以设置保留标志
>
> willQoS：遗嘱的服务质量，针对不同的服务质量级别，服务端会使用不同的服务质量来发布遗嘱信息

## 7.移植 MQTT 客户端库 API （同步模式）

```
#include<MQTTClient.h>		# 同步模式的头文件
```

### 7.1 MQTTClient_message

描述 MQTT 消息的负载和属性等相关信息

```c
typedef struct
{
	int payloadlen; 	//负载长度
	void* payload; 		//负载
	int qos; 			//消息的 qos 等级
	int retained; 		//消息的保留标志
	int dup; 			//dup 标志（重复标志）
	int msgid; 			//消息标识符，也就是前面说的 packetId
	......
} MQTTClient_message;

MQTTClient_message pubmsg = MQTTClient_message_initializer;			// 宏定义初始化
```

### 7.2 创建一个客户端对象

```c
int MQTTClient_create(MQTTClient *handle,
	const char *serverURI,
	const char *clientId,
	int persistence_type,
	void *persistence_context
);

handle： MQTT 客户端句柄；
serverURL： MQTT 服务器地址；
clientId： 客户端 ID；
persistence_type： 客户端使用的持久化类型：
	- MQTTCLIENT_PERSISTENCE_NONE：使用内存持久性。如果运行客户端的设备或系统出现故障或关闭，则任何传输中消息的当前状态都会丢失
	- MQTTCLIENT_PERSISTENCE_DEFAULT：使用默认的（基于文件系统）持久性机制
	- MQTTCLIENT_PERSISTENCE_USER：使用特定于应用程序的持久性实现
persistence_context：如果使用 MQTTCLIENT_PERSISTENCE_NONE 持久化类型，则该参数应设置为NULL。如果选择的是 MQTTCLIENT_PERSISTENCE_DEFAULT 持久化类型，则该参数应设置为持久化目录的位置，如果设置为 NULL，则持久化目录就是客户端应用程序的工作目录。
返回值： 成功返回 MQTTCLIENT_SUCCESS，失败将返回一个错误码
```

```c
MQTTClient client;
int rc;

/* 创建 mqtt 客户端对象 */
if (MQTTCLIENT_SUCCESS != (rc = MQTTClient_create(&client, "tcp://iot.ranye-iot.net:1883",
							"dt_mqtt_2_id", MQTTCLIENT_PERSISTENCE_NONE, NULL))) 
{
	printf("Failed to create client, return code %d\n", rc);	
    return EXIT_FAILURE;
}
```

### 7.3 连接服务端

客户端创建完毕后，便可以连接服务器

```c
int MQTTClient_connect(MQTTClient handle,
					MQTTClient_connectOptions *options
);
handle： 客户端句柄；
options：MQTTClient_connectOptions 结构体中包含了 keepAlive、 cleanSession 以及一个指向 MQTTClient_willOptions 结构体对象的指针
返回值： 成功返回 MQTTCLIENT_SUCCESS，是否返回错误码：
	- 1： 连接被拒绝。不可接受的协议版本，不支持客户端的 MQTT 协议版本
	- 2： 连接被拒绝：标识符被拒绝
	- 3： 连接被拒绝：服务器不可用
	- 4： 连接被拒绝：用户名或密码错误
	- 5： 连接被拒绝：未授权
	- 6-255： 保留以备将来使用
```

------

**MQTTClient_connectOptions 结构体**：连接客户端相关信息

```c
typedef struct
{
	int keepAliveInterval; 		//keepAlive
	int cleansession; 			//cleanSession
	MQTTClient_willOptions *will; 	//遗嘱相关
	const char *username; 			//用户名
	const char *password; 			//密码
	int reliable; 					//控制同步发布消息还是异步发布消息
	......
	......
} MQTTClient_connectOptions;
```

**MQTTClient_willOptions 结构体**：遗嘱相关信息

```c
typedef struct
{
	const char *topicName; 		//遗嘱主题
	const char *message; 		//遗嘱内容
	int retained;			 //遗嘱消息的保留标志
	int qos; 				//遗嘱消息的 QoS 等级
	......
	......
} MQTTClient_willOptions;
```

------

```c
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_willOptions will_opts = MQTTClient_willOptions_initializer;
......
/* 连接服务器 */
will_opts.topicName = "dt2914/willTopic"; 	//遗嘱主题
will_opts.message = "Abnormally dropped"; 	//遗嘱内容
will_opts.retained = 1; 			//遗嘱保留消息
will_opts.qos = 0; 					//遗嘱 QoS 等级

conn_opts.will = &will_opts;
conn_opts.keepAliveInterval = 30; 			//客户端 keepAlive 间隔时间
conn_opts.cleansession = 0; 				//客户端 cleanSession 标志
conn_opts.username = "dt_mqtt_2"; 			//用户名
conn_opts.password = "dt291444"; 		//密码

if (MQTTCLIENT_SUCCESS != (rc = MQTTClient_connect(client, &conn_opts))) 
{
	printf("Failed to connect, return code %d\n", rc);
	return EXIT_FAILURE;
}
```

### 7.4 设置回调函数

MQTTClient_setCallbacks 函数为应用程序设置回调函数 ；包括多个回调函数

- 断开连接时的回调函数 cl（当客户端检测到自己掉线时会执行该函数，如果将其设置为 NULL 表示应用程序不处理断线的情况 ）
- 接收消息的回调函数 ma（当客户端接收到服务端发送过来的消息时执行该函数，必须设置此函数否则客户端无法接收消息）
- 发布消息的回调函数 dc（当客户端发布的消息已经确认发送时执行该回调函数）

```c
int MQTTClient_setCallbacks(MQTTClient handle,
							void *context,
							MQTTClient_connectionLost *cl,
							MQTTClient_messageArrived *ma,
							MQTTClient_deliveryComplete *dc
);
handle： 客户端句柄；
context： 执行回调函数的时候，会将 context 参数传递给回调函数，
cl： 一个 MQTTClient_connectionLost 类型的函数指针
ma： 一个 MQTTClient_messageArrived 类型的函数指针
dc： 一个 MQTTClient_deliveryComplete 类型的函数指针
返回值： 成功返回 MQTTCLIENT_SUCCESS，失败返回 MQTTCLIENT_FAILURE。
```

```c
typedef void MQTTClient_connectionLost(void *context, char *cause);
cause：表示断线的原因，是一个字符串

typedef int MQTTClient_messageArrived(void *context, char *topicName,
		int topicLen, MQTTClient_message *message);
topicName：表示消息的主题名
topicLen：表示主题名的长度
message：指向一个 MQTTClient_message 对象，也就是客户端所接收到的消息

typedef void MQTTClient_deliveryComplete(void* context, MQTTClient_deliveryToken dt);
dt：表示 MQTT 消息的值，将其称为传递令牌
发布消息时（应用程序通过 MQTTClient_publishMessage 函数发布消息），MQTT 协议会返回给客户端应用程序一个传递令牌； 应用程序可以通过将调用 MQTTClient_publishMessage() 返回的传递令牌与传递给此回调的令牌进行匹配来检查消息是否已成功发布
```

```c
static void delivered(void *context, MQTTClient_deliveryToken dt)
{
	printf("Message with token value %d delivery confirmed\n", dt);
}

static int msgarrvd(void *context, char *topicName, int topicLen,
					MQTTClient_message *message)
{
    printf("Message arrived\n");
    printf("topic: %s\n", topicName);
    printf("message: <%d>%s\n", message->payloadlen, (char *)message->payload);
    MQTTClient_freeMessage(&message); //释放内存
    MQTTClient_free(topicName); //释放内存
    return 1;
}

static void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf(" cause: %s\n", cause);
}

int main(void)
{
    /* 设置回调 */
    if (MQTTCLIENT_SUCCESS != (rc = MQTTClient_setCallbacks(client, NULL, 
                                                            connlost, msgarrvd, delivered))) 
    {
		printf("Failed to set callbacks, return code %d\n", rc);
		return EXIT_FAILURE;
	}
}
```

### 7.5 发布消息

MQTTClient_publishMessage 来发布一个消息 

```c
int MQTTClient_publishMessage(MQTTClient handle,
							const char *topicName,
							MQTTClient_message *msg,
							MQTTClient_deliveryToken *dt
);
handle：客户端句柄；
topicName：主题名称。向该主题发布消息;
msg：指向一个 MQTTClient_message 对象的指针;
dt：返回给应用程序的传递令牌
返回值： 成功 MQTTCLIENT_SUCCESS，失败错误码
```

```c
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token;
......
/* 发布消息 */
pubmsg.payload = "online"; //消息内容
pubmsg.payloadlen = 6; //消息的长度
pubmsg.qos = 0; //QoS 等级
pubmsg.retained = 1; //消息的保留标志

if (MQTTCLIENT_SUCCESS !=
    (rc = MQTTClient_publishMessage(client, "dt2914/testTopic", &pubmsg, &token))) 
{
	printf("Failed to publish message, return code %d\n", rc);
	return EXIT_FAILURE;
}
```

### 7.6 订阅主题和取消订阅主题

客户端应用程序调用 MQTTClient_subscribe 函数来订阅主题： 

```c
int MQTTClient_subscribe(MQTTClient handle,
				const char *topic,
				int qos
);
handle：客户端句柄;
topic：主题名称。客户端订阅的主题;
qos：QoS 等级
返回值： 成功返回 MQTTCLIENT_SUCCESS，失败返回错误码
```

```c
if (MQTTCLIENT_SUCCESS !=
(rc = MQTTClient_subscribe(client, "dt2914/testTopic", 0))) 
{
	printf("Failed to subscribe, return code %d\n", rc);
	return EXIT_FAILURE;
}
```

------

取消订阅，可调用 MQTTClient_unsubscribe 函数 

```c
int MQTTClient_unsubscribe(MQTTClient handle,
						const char *topic
);
handle：客户端句柄；
topic：主题名称。取消订阅该主题；
返回值： 成功返回 MQTTCLIENT_SUCCESS，失败返回错误码
```

### 7.7 断开服务器连接

```c
int MQTTClient_disconnect(MQTTClient handle,int timeout);
handle：客户端句柄；
timeout：超时时间。 客户端将断开连接延迟最多 timeout 时间（以毫秒为单位）；
返回值： 客户端成功则返回 MQTTCLIENT_SUCCESS； 客户端无法与服务器断开连接，则返回错误代码。
```

## 8. demo

- 用户可通过手机或电脑远程控制开发板上的一颗 LED 灯
- 开发板客户端每个 30 秒向服务端发送 SoC 当前的温度值

```c
/* mqttClient.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "MQTTClient.h" 	//包含 MQTT 客户端库头文件

#define BROKER_ADDRESS "tcp://iot.ranye-iot.net:1883" 	//然也物联平台社区版 MQTT 服务器地址

/* 客户端 id、用户名、密码 */
#define CLIENTID "您的客户端 ID" 	//客户端 id
#define USERNAME "您的用户名" 		//用户名
#define PASSWORD "您的密码" 		//密码

#define WILL_TOPIC "dt_mqtt/will" 		//遗嘱主题
#define LED_TOPIC "dt_mqtt/led" 		//LED 主题
#define TEMP_TOPIC "dt_mqtt/temperature" 	//温度主题

static int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    if (!strcmp(topicName, LED_TOPIC)) 
    { //校验消息的主题
		if (!strcmp("2", message->payload)) //如果接收到的消息是"2"则设置 LED 为呼吸灯模式
			system("echo heartbeat > /sys/class/leds/sys-led/trigger");
		if (!strcmp("1", message->payload)) 
        { //如果是"1"则 LED 常量
			system("echo none > /sys/class/leds/sys-led/trigger");
			system("echo 1 > /sys/class/leds/sys-led/brightness");
		}
		else if (!strcmp("0", message->payload)) 
    	{//如果是"0"则 LED 熄灭
			system("echo none > /sys/class/leds/sys-led/trigger");
			system("echo 0 > /sys/class/leds/sys-led/brightness");
		}
		// 接收到其它数据 不做处理
	}
    /* 释放占用的内存空间 */
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    /* 退出 */
    return 1;
}

static void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf(" cause: %s\n", cause);
}

int main(int argc, char *argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_willOptions will_opts = MQTTClient_willOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    int rc;
    
    /* 创建 mqtt 客户端对象 */
    if (MQTTCLIENT_SUCCESS !=
        (rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, NULL))) 
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }
    
    /* 连接 MQTT 服务器 */
    will_opts.topicName = WILL_TOPIC; 		//遗嘱主题
    will_opts.message = "Unexpected disconnection"; 	//遗嘱消息
    will_opts.retained = 1; 		//保留消息
    will_opts.qos = 0; 				//QoS0
    
    conn_opts.will = &will_opts;
    conn_opts.keepAliveInterval = 30; 	//心跳包间隔时间
    conn_opts.cleansession = 0; 	//cleanSession 标志
    conn_opts.username = USERNAME; 	//用户名
    conn_opts.password = PASSWORD; 	//密码
    if (MQTTCLIENT_SUCCESS !=
    	(rc = MQTTClient_connect(client, &conn_opts))) 
    {
        printf("Failed to connect, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }
    
    printf("MQTT 服务器连接成功!\n");
    
    /* 发布上线消息 */
    pubmsg.payload = "Online"; 		//消息的内容
    pubmsg.payloadlen = 6; 			//内容的长度
    pubmsg.qos = 0; 			//QoS 等级
    pubmsg.retained = 1;		 //保留消息
    
    if (MQTTCLIENT_SUCCESS !=
        	(rc = MQTTClient_publishMessage(client, WILL_TOPIC, &pubmsg, NULL))) 
    {
        printf("Failed to publish message, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto disconnect_exit;
    }
    
    /* 订阅主题 dt_mqtt/led */
    if (MQTTCLIENT_SUCCESS !=
        	(rc = MQTTClient_subscribe(client, LED_TOPIC, 0))) 
    {
        printf("Failed to subscribe, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto disconnect_exit;
    }
    
    /* 向服务端发布芯片温度信息 */
    for ( ; ; ) 
    {
        MQTTClient_message tempmsg = MQTTClient_message_initializer;
        char temp_str[10] = {0};
        int fd;
        /* 读取温度值 */
        fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
        read(fd, temp_str, sizeof(temp_str));	//读取 temp 属性文件即可获取温度
        close(fd);
        
        /* 发布温度信息 */
        tempmsg.payload = temp_str; 	//消息的内容
        tempmsg.payloadlen = strlen(temp_str); 	//内容的长度
        tempmsg.qos = 0; 		//QoS 等级
        tempmsg.retained = 1; 		//保留消息
        if (MQTTCLIENT_SUCCESS !=
            (rc = MQTTClient_publishMessage(client, TEMP_TOPIC, &tempmsg, NULL))) 
        {
            printf("Failed to publish message, return code %d\n", rc);
            rc = EXIT_FAILURE;
            goto unsubscribe_exit;
        }
        
        sleep(30); //每隔 30 秒 更新一次数据
    }
unsubscribe_exit:
    if (MQTTCLIENT_SUCCESS !=
        (rc = MQTTClient_unsubscribe(client, LED_TOPIC))) 
    {
        printf("Failed to unsubscribe, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }
disconnect_exit:
    if (MQTTCLIENT_SUCCESS !=
        (rc = MQTTClient_disconnect(client, 10000))) 
    {
        printf("Failed to disconnect, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }
destroy_exit:
    MQTTClient_destroy(&client);
exit:
    return rc;
}
```

