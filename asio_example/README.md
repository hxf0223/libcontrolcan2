# README

## tcp_serv_client 知识点

1. tcpClient使用 enable_shared_from_this ，被io service调用。在io service没有pending任务时，tcpClient实例将被销毁；
2. 使用 boost::stream_buffer 作为 receive/async_receive 实参时，需要在接收到数据后，使用 consume 消除 stream_buffer 中的已读数据。

## asio_demo_work_share_ptr 知识点

1. 使用 io_service::work 保持 io service 在没有pending任务时，依然保持不退出；
2. 使用 asio 异步发送数据时，使用持久数据，或使用 std::shared_ptr 创建数据；
