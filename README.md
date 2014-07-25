*deprecated*

distribute log system based on gearman
bglogger :  worker of gearman
logclient:  client of gearman

pulog是一个基于gearman的异步分布式日志记录系统，关于gearman请参考http://gearman.org/
本项目没有什么高深的算法，仅仅是对gearman两个角色client和worker的封装，以解决在高并发下
日志记录影响性能以及不同程序日志过于分散不方便集中话管理的问题，优点如下：
1. 分布式,可以有多个后台logger同时工作，提高多核时的性能
2. 异步，日志被提交到队列服务器，异步写入文件，以避免日志记录造成的效率降低问题
3. 集中，日志被集中到日志服务器，便于管理及后续的分析
4. 语言无关，可以自定义实现日志记录的客户端，来适用于不同的语言环境，默认提供c++客户端
