#include "clients/server.hpp"
#include <sys/eventfd.h>

void Server::initSockConfigs(int client_id, const std::vector<std::pair<std::string, int>>& ip_port_pairs) {
    std::printf("[Server %d] Init socket configurations.\n", client_id);
    client_id_ = client_id;
    ip_port_pairs_ = ip_port_pairs;
    connect_sockfds_ = std::vector<int>(ip_port_pairs.size(), -1);
    accepted_sockfds_ = std::vector<int>{};
}

void Server::start() {
    std::printf("[Server %d] Start processing.\n", client_id_);
    keep_running_ = true;
    master_thread_ = std::thread(&Server::masterThreadFunc, this);
    for (int i = 0; i < num_workers_; ++i) {
        worker_threads_.emplace_back(&Server::workerThreadFunc, this);
    }
}

void Server::stop() {
    std::printf("[Server %d] Stop processing.\n", client_id_);
    keep_running_ = false;

    cond_var_.notify_all();

    // Join worker threads
    for (auto& worker : worker_threads_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    // Close all fds
    if (exit_eventfd_ != -1) {
        close(exit_eventfd_);
    }
    if (epoll_fd_ != -1) {
        close(epoll_fd_);
    }
    if (listen_sockfd_ != -1) {
        close(listen_sockfd_);
    }
    for (int sockfd : connect_sockfds_) {
        if (sockfd != -1) close(sockfd);
    }
    for (int sockfd : accepted_sockfds_) {
        if (sockfd != -1) close(sockfd);
    }
}

void Server::sendMsg(int target_client_id, std::string& str) {
    // Send message
    if (connect_sockfds_[target_client_id] == -1) {
        // ERROR not connected
        std::printf("\033[31m[Error][Server %d] Socket connection to server %d not set up.\033[0m\n", client_id_, target_client_id);
    }
    sleep(3);
    if (send(connect_sockfds_[target_client_id], str.c_str(), str.size(), 0) < 0) {
        // ERROR
        std::printf("\033[31m[Error][Server %d] Send RequestMsg to server %d fail, data: %s.\033[0m\n", client_id_, target_client_id, str.c_str());
    }
}

int Server::process(const std::string& str) {
    // Parse message based on its type
    std::string type = json::parse(str).at("type");
    auto parser = parser_factory_.createParser(type);
    std::unique_ptr<Msg> msg = parser->parse(str);

    // Process message based on its type
    auto processor = processor_factory_.createProcessor(type);
    int result;
    if ((result = processor->process(std::move(msg))) != 0) {
        return result;
    }
    return 0;
}

void Server::masterThreadFunc() {
    // Set up epoll
    epoll_fd_ = epoll_create1(0);

    // Add exit_eventfd_ to epoll for exit
    exit_eventfd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    addToEpoll(exit_eventfd_);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(ip_port_pairs_[client_id_].second);
    inet_pton(AF_INET, ip_port_pairs_[client_id_].first.c_str(), &(servaddr.sin_addr));

    // Create socket for passive listening
    listen_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sockfd_ < 0) {
        // Handle error
        std::printf("\033[31m[Error][Server %d][Server::masterThreadFunc] Fail to create socket.\033[0m\n", client_id_);
    }

    if (bind(listen_sockfd_, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        // Handle error
        close(listen_sockfd_);
        std::printf("\033[31m[Error][Server %d][Server::masterThreadFunc] Fail to bind to %s:%d.\033[0m\n", client_id_, ip_port_pairs_[client_id_].first.c_str(), ip_port_pairs_[client_id_].second);
    }

    if (listen(listen_sockfd_, 3) < 0) {
        // Handle error
        close(listen_sockfd_);
        std::printf("\033[31m[Error][Server %d][Server::masterThreadFunc] Fail to listen to %s:%d.\033[0m\n", client_id_, ip_port_pairs_[client_id_].first.c_str(), ip_port_pairs_[client_id_].second);
    }

    // Add listen_sockfd_ to epoll for passive listening
    addToEpoll(listen_sockfd_);
    std::printf("[Server %d] Listen socket on %s:%d.\n", client_id_, ip_port_pairs_[client_id_].first.c_str(), ip_port_pairs_[client_id_].second);

    sleep(3);
    bool connect_socket_fail = false;
    for (int i = 0; i < ip_port_pairs_.size(); i++) {
        if (i == client_id_) {
            continue;
        }
        struct sockaddr_in servaddr;
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(ip_port_pairs_[i].second);
        inet_pton(AF_INET, ip_port_pairs_[i].first.c_str(), &(servaddr.sin_addr));

        // Create socket for active connecting
        int connect_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (connect_sockfd < 0) {
            // Handle error
            close(connect_sockfd);
            connect_socket_fail = true;
            std::printf("\033[31m[Error][Server %d] Fail to connect socket on server %d with %s:%d.\033[0m\n", client_id_, i, ip_port_pairs_[i].first.c_str(), ip_port_pairs_[i].second);
            continue;
        }

        if (connect(connect_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            // Handle error
            close(connect_sockfd);
            connect_socket_fail = true;
            std::printf("\033[31m[Error][Server %d] Fail to connect socket on server %d with %s:%d.\033[0m\n", client_id_, i, ip_port_pairs_[i].first.c_str(), ip_port_pairs_[i].second);
            continue;
        }

        connect_sockfds_[i] = connect_sockfd;
    }

    if (!connect_socket_fail) {
        std::printf("[Server %d] Connect sockets on all %lu clients.\n", client_id_, ip_port_pairs_.size() - 1);
    }

    while (true) {
        struct epoll_event events[10];
        int nfds = epoll_wait(epoll_fd_, events, 10, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_sockfd_) {
                // Accept new connection
                int client_sock = accept(events[i].data.fd, nullptr, nullptr);
                if (client_sock >= 0) {
                    // Add server socket to epoll
                    addToEpoll(client_sock);
                }
                accepted_sockfds_.push_back(client_sock);
                if (accepted_sockfds_.size() == ip_port_pairs_.size() - 1) {
                    std::printf("[Server %d] Accept sockets from all %lu clients.\n", client_id_, ip_port_pairs_.size() - 1);
                }
            } else if (events[i].data.fd == exit_eventfd_) {
                uint64_t u;
                read(exit_eventfd_, &u, sizeof(uint64_t));
                stop();
                return;
            } else {
                // Handle server socket
                handleServer(events[i].data.fd);
                if (!keep_running_) {
                    return;
                }
            }
        }
    }
}

void Server::handleServer(int client_sock) {
    // Add task to the queue for worker threads
    std::unique_lock<std::mutex> lock(mutex_);

    std::string buffer; // Use std::string to hold incoming data
    char temp_buffer[1024]; // Temporary buffer for receiving data
    int bytes_received = recv(client_sock, temp_buffer, sizeof(temp_buffer) - 1, 0);
    if (bytes_received > 0) {
        temp_buffer[bytes_received] = '\0'; // Null-terminate the received data
        buffer = std::string(temp_buffer); // Assign to std::string
    }
    if (bytes_received > 0) {
        if (json::parse(buffer).at("type") == "ExitMsg") {
            lock.unlock();
            stop();
            return;
        }
        task_queue_.emplace([this, buffer] {
            this->process(buffer);
        });
    }
    lock.unlock();
    cond_var_.notify_one();
}

void Server::workerThreadFunc() {
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]{ return !keep_running_ || !task_queue_.empty(); });
        if (!keep_running_ && task_queue_.empty()) {
            return;
        }
        auto task = std::move(task_queue_.front());
        task_queue_.pop();
        lock.unlock();

        // Process task
        task();
    }
}

void Server::addToEpoll(int fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
}