#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <atomic>
#include <string>
#include <stdexcept>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "protocol.h"

using namespace std;

struct ClientState {
    vector<vector<int>> matrix;
    int num_threads = 1;
    atomic<bool> is_processing{ false };
    atomic<int> completed_threads{ 0 };
};

void process_task(int startRow, int endRow, ClientState* state) {
    for (int i = startRow; i < endRow; ++i) {
        int maxInRow = *max_element(state->matrix[i].begin(), state->matrix[i].end());
        state->matrix[i][i] = maxInRow;
    }
    state->completed_threads++;
}

void handle_client(int client_sock) {
    ClientState state;
    try {
        cout << "[Server] Handling new client on socket " << client_sock << endl;

        while (true) {
            PacketHeader header;
            if (recv(client_sock, (char*)&header, sizeof(header), 0) <= 0) break;
            
            // Перетворюємо дані з мережевого порядку байтів (Big-Endian) у хостовий (Little-Endian для x86)
            uint32_t cmd = ntohl(header.command);
            uint32_t payload_size = ntohl(header.total_size);

            switch (cmd) {
            case CMD_HELLO: {
                cout << "[Server] Handshake received." << endl;
                send(client_sock, "CONNECTED", 9, 0);
                break;
            }
            case CMD_CONFIG: {
                uint32_t cfg[2];
                recv_full(client_sock, cfg, sizeof(cfg));
                int N = ntohl(cfg[0]);
                state.num_threads = ntohl(cfg[1]);
                state.matrix.assign(N, vector<int>(N));
                cout << "[Server] Config: N=" << N << ", Threads=" << state.num_threads << endl;
                send(client_sock, "OK_CFG", 6, 0);
                break;
            }
            case CMD_DATA: {
                int N = state.matrix.size();
                for (int i = 0; i < N; ++i) {
                    recv_full(client_sock, state.matrix[i].data(), N * sizeof(int));
                }
                cout << "[Server] Data received." << endl;
                send(client_sock, "OK_DATA", 7, 0);
                break;
            }
            case CMD_START: {
                if (state.is_processing) throw runtime_error("Already processing");
                state.is_processing = true;
                state.completed_threads = 0;

                thread([&state]() {
                    vector<thread> workers;
                    int N = state.matrix.size();
                    for (int t = 0; t < state.num_threads; ++t) {
                        int start = t * N / state.num_threads;
                        int end = (t + 1) * N / state.num_threads;
                        workers.emplace_back(process_task, start, end, &state);
                    }
                    for (auto& w : workers) w.join();
                    state.is_processing = false;
                    }).detach();

                send(client_sock, "STARTED", 7, 0);
                break;
            }
            case CMD_STATUS: {
                string status = to_string(state.completed_threads.load()) + "/" +
                    to_string(state.num_threads) + " completed";
                if (!state.is_processing && state.completed_threads == state.num_threads) {
                    status = "READY";
                }
                send(client_sock, status.c_str(), status.size(), 0);
                break;
            }
            case CMD_RESULT: {
                if (state.is_processing) {
                    send(client_sock, "WAIT", 4, 0);
                }
                else {
                    for (auto& row : state.matrix) {
                        send(client_sock, (char*)row.data(), row.size() * sizeof(int), 0);
                    }
                }
                break;
            }
            default:
                throw runtime_error("Unknown command");
            }
        }
    }
    catch (const exception& e) {
        cerr << "[Error] Socket " << client_sock << ": " << e.what() << endl;
        string err_msg = "ERROR: " + string(e.what());
        send(client_sock, err_msg.c_str(), err_msg.size(), 0);
    }
    close(client_sock);
    cout << "[Server] Connection closed for socket " << client_sock << endl;
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT); // Перетворення номера порту у мережевий формат
    addr.sin_addr.s_addr = INADDR_ANY; // Сервер слухатиме всі доступні мережеві інтерфейси (localhost, Wi-Fi тощо)

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) return 1;
    listen(server_fd, 5);

    cout << "Server started on port 8080. Awaiting clients..." << endl;

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client >= 0) {
            thread(handle_client, client).detach();
        }
    }

    return 0;
}