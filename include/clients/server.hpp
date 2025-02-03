#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/epoll.h>
#include <functional>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <memory>
#include "msg.hpp"
#include "parsers/parserFactory.hpp"
#include "processors/processorFactory.hpp"

/**
 * Server - base class of clients.
 */
class Server : public std::enable_shared_from_this<Server> {
public:
    /**
     * initSockConfigs() - init socket configurations.
     *
     * @param client_id: id of current server.
     * @param ip_port_pairs: ip-port pairs for clients to build up socket connections.
     */
    void initSockConfigs(int client_id, const std::vector<std::pair<std::string, int>>& ip_port_pairs);
    
    /**
     * start() - start processing.
     *
     * Create master thread to build up socket with other clients;
     * create worker thread to process message received from other clients.
     */
    void start();
    
    /**
     * stop() - stop processing.
     *
     * Signal workers to stop and join threads.
     */
    void stop();

    /**
    * sendMsg() - sending message to target server.
    *
    * @param target_client_id
    * @param str
    */
    void sendMsg(int target_client_id, std::string& str);

    /**
     * process() - processing.
     *
     * @param str: message received from socket.
     */
    int process(const std::string& str);

    /**
     * masterThreadFunc() - function that run on master thread.
     */
    void masterThreadFunc();
    
    /**
     * handleServer() - handle message from other server.
     *
     * @param client_sock
     */
    void handleServer(int client_sock);

    /**
     * workerThreadFunc() - function that run on worker thread.
     */
    void workerThreadFunc();

    /**
     * addToEpoll() - add fd to epoll.
     */
    void addToEpoll(int fd);

    ParserFactory parser_factory_; /* factory of parser */
    ProcessorFactory processor_factory_; /* factory of processor */

    int client_id_;
    bool keep_running_; /* to terminate master and worker threads */
    int exit_eventfd_; /* to activate epoll to exit */

    std::vector<std::pair<std::string, int>> ip_port_pairs_;
    int listen_sockfd_; /* to build up socket connections */
    std::vector<int> connect_sockfds_; /* to send messages */
    std::vector<int> accepted_sockfds_; /* to receive messages */

    int epoll_fd_;
    int num_workers_ = 5; /* default number of workers */

    /**
     * Producer-consumer model for master and worker threads.
     */
    std::thread master_thread_; /* producer */
    std::vector<std::thread> worker_threads_; /* consumer */
    std::queue<std::function<void()>> task_queue_;

    std::mutex mutex_;
    std::condition_variable cond_var_;
};
