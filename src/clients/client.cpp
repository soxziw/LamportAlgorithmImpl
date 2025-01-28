#include "clients/client.hpp"

void Client::initSockConfigs(int client_id, const std::vector<std::pair<std::string, int>>& ip_port_pairs) {
    std::printf("[Client %d] Init socket configurations.\n", client_id);
    client_id_ = client_id;
    ip_port_pairs_ = ip_port_pairs;
    listen_sockfds_ = std::vector<int>(ip_port_pairs.size(), -1);
    connect_sockfds_ = std::vector<int>(ip_port_pairs.size(), -1);
    accepted_sockfds_ = std::vector<int>(ip_port_pairs.size(), -1);
}

void Client::start() {
    std::printf("[Client %d] Start processing.\n", client_id_);
    getKeepRunning() = true;
    master_thread_ = std::thread(&Client::masterThreadFunc, this);
    for (int i = 0; i < num_workers_; ++i) {
        worker_threads_.emplace_back(&Client::workerThreadFunc, this);
    }
}

void Client::stop() {
    std::printf("[Client %d] Stop processing.\n", client_id_);
    getKeepRunning() = false;

    // Join worker threads
    for (auto& worker : worker_threads_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    // Join master thread
    if (master_thread_.joinable()) {
        master_thread_.join();
    }
}

int Client::process(const std::string& str) {
    // Parse message based on its type
    std::string type = json::parse(str).at("type");
    auto parser = parser_factory_.createParser(type);
    std::unique_ptr<Msg> msg = parser->parse(str);

    // Process message based on its type
    std::printf("[Client %d] Processing %s, data: %s.\n", client_id_, type.c_str(), str.c_str());
    auto processor = processor_factory_.createProcessor(type);
    int result;
    if ((result = processor->process(std::move(msg))) != 0) {
        return result;
    }
    return 0;
}

void Client::masterThreadFunc() {
    // Set up epoll
    epoll_fd_ = epoll_create1(0);
    for (int i = 0; i < ip_port_pairs_.size(); i++) {
        if (i == client_id_) {
            continue;
        }
        struct sockaddr_in servaddr;
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(ip_port_pairs_[i].second);
        inet_pton(AF_INET, ip_port_pairs_[i].first.c_str(), &(servaddr.sin_addr));

        // Create socket for passive listening
        int listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sockfd < 0) {
            // Handle error
            continue;
        }

        if (bind(listen_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            // Handle error
            close(listen_sockfd);
            continue;
        }

        if (listen(listen_sockfd, 3) < 0) {
            // Handle error
            close(listen_sockfd);
            continue;
        }

        // Add listen_sockfd to epoll for passive listening
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = listen_sockfd;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_sockfd, &ev);
        listen_sockfds_[i] = listen_sockfd;
        std::printf("[Client %d] Listen socket on client %d with ip %s, port %d.\n", client_id_, i, ip_port_pairs_[i].first.c_str(), ip_port_pairs_[i].second);
    }

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
            continue;
        }

        if (connect(connect_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            // Handle error
            close(connect_sockfd);
            continue;
        }

        // Add connect_sockfd to epoll for active connecting
        struct epoll_event connect_ev;
        connect_ev.events = EPOLLIN;
        connect_ev.data.fd = connect_sockfd;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, connect_sockfd, &connect_ev);
        connect_sockfds_[i] = connect_sockfd;
        std::printf("[Client %d] Connect socket on client %d with ip %s, port %d.\n", client_id_, i, ip_port_pairs_[i].first.c_str(), ip_port_pairs_[i].second);
    }

    while (getKeepRunning()) {
        struct epoll_event events[10];
        int nfds = epoll_wait(epoll_fd_, events, 10, -1);
        for (int i = 0; i < nfds; ++i) {
            auto itr = listen_sockfds_.end();
            if ((itr = std::find(listen_sockfds_.begin(), listen_sockfds_.end(), events[i].data.fd)) != listen_sockfds_.end()) {
                // Accept new connection
                int client_sock = accept(events[i].data.fd, nullptr, nullptr);
                if (client_sock >= 0) {
                    // Add client socket to epoll
                    struct epoll_event client_ev;
                    client_ev.events = EPOLLIN;
                    client_ev.data.fd = client_sock;
                    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_sock, &client_ev);
                }
                int id = itr - listen_sockfds_.begin();
                accepted_sockfds_[id] = client_sock;
                close(*itr);
                *itr = -1;
                std::printf("[Client %d] Accept socket on client %d with ip %s, port %d.\n", client_id_, id, ip_port_pairs_[id].first.c_str(), ip_port_pairs_[id].second);
            } else {
                // Handle client socket
                handleClient(events[i].data.fd);
            }
        }
    }
    
    // Close all sockets
    for (int sockfd : listen_sockfds_) {
        if (sockfd != -1) close(sockfd);
    }
    for (int sockfd : connect_sockfds_) {
        if (sockfd != -1) close(sockfd);
    }
    for (int sockfd : accepted_sockfds_) {
        if (sockfd != -1) close(sockfd);
    }
}

void Client::handleClient(int client_sock) {
    // Add task to the queue for worker threads
    std::unique_lock<std::mutex> lock(mutex_);
    task_queue_.emplace([this, client_sock] {
        std::string buffer; // Use std::string to hold incoming data
        char temp_buffer[1024]; // Temporary buffer for receiving data
        ssize_t bytes_received = recv(client_sock, temp_buffer, sizeof(temp_buffer) - 1, 0);
        if (bytes_received > 0) {
            temp_buffer[bytes_received] = '\0'; // Null-terminate the received data
            buffer = std::string(temp_buffer); // Assign to std::string
            // Process the received data
            this->process(buffer);
        }
        // Handle error or disconnection
    });

    // Print where this message come from.
    auto itr = accepted_sockfds_.end();
    if ((itr = std::find(accepted_sockfds_.begin(), accepted_sockfds_.end(), client_sock)) != accepted_sockfds_.end()) {
        int id = itr - accepted_sockfds_.begin();
        std::printf("[Client %d] Receive message from client %d with ip %s, port %d.\n", client_id_, id, ip_port_pairs_[id].first.c_str(), ip_port_pairs_[id].second);
    } else {
        std::printf("[Client %d] Receive message from unknown client.\n", client_id_);
    }
    lock.unlock();
    cond_var_.notify_one();
}

void Client::workerThreadFunc() {
    while (getKeepRunning()) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]{ return !task_queue_.empty(); });
        auto task = std::move(task_queue_.front());
        task_queue_.pop();
        lock.unlock();

        // Process task
        task();
    }
}