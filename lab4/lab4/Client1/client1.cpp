#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#include "protocol.h"

using namespace std;

void send_header(int sock, Command cmd, uint32_t payload_size) {
    PacketHeader header;
    header.total_size = htonl(payload_size);
    header.command = htonl(cmd);
    if (send(sock, (char*)&header, sizeof(header), 0) < 0) {
        throw runtime_error("Failed to send header");
    }
}

int main() {
    int sock = 0;
    sockaddr_in serv_addr;

    try {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) throw runtime_error("Socket creation error");

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);

        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) throw runtime_error("Invalid address");

        cout << "[Client] Connecting to server..." << endl;
        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) throw runtime_error("Connection failed");

        send_header(sock, CMD_HELLO, 0);
        char hello_buffer[10] = { 0 };
        recv(sock, hello_buffer, 9, 0);
        cout << "[Client] Handshake response: " << hello_buffer << endl;

        uint32_t N = 1000;
        uint32_t threads = 10;

        ConfigPayload cfg = { htonl(N), htonl(threads) };
        send_header(sock, CMD_CONFIG, sizeof(cfg));
        send(sock, (char*)&cfg, sizeof(cfg), 0);

        char cfg_ack[7] = { 0 };
        recv(sock, cfg_ack, 6, 0);
        cout << "[Client] Config status: " << cfg_ack << endl;

        cout << "[Client] Generating and sending matrix..." << endl;
        send_header(sock, CMD_DATA, N * N * sizeof(int));
        for (uint32_t i = 0; i < N; ++i) {
            vector<int> row(N);
            for (uint32_t j = 0; j < N; ++j) row[j] = rand() % 10 + 1;
            send(sock, (char*)row.data(), N * sizeof(int), 0);
        }

        char data_ack[8] = { 0 };
        recv(sock, data_ack, 7, 0);
        cout << "[Client] Data status: " << data_ack << endl;

        send_header(sock, CMD_START, 0);
        char start_ack[8] = { 0 };
        recv(sock, start_ack, 7, 0);
        cout << "[Client] Computation " << start_ack << endl;

        string status = "";
        while (status != "READY") {
            send_header(sock, CMD_STATUS, 0);
            char status_buf[64] = { 0 };
            int bytes = recv(sock, status_buf, 63, 0);
            if (bytes > 0) status = string(status_buf, bytes);

            cout << "[Client] Progress: " << status << endl;
            if (status == "READY") break;

            this_thread::sleep_for(chrono::milliseconds(500));
        }

        cout << "[Client] Requesting final result..." << endl;
        send_header(sock, CMD_RESULT, 0);

        vector<vector<int>> result_matrix(N, vector<int>(N));
        for (uint32_t i = 0; i < N; ++i) {
            recv_full(sock, result_matrix[i].data(), N * sizeof(int));
        }

        cout << "[Client] Result received successfully!" << endl;
        cout << "Diagonal [0][0]: " << result_matrix[0][0] << endl;

    }
    catch (const exception& e) {
        cerr << "[Client Error] " << e.what() << endl;
    }

    if (sock != 0) close(sock);
    return 0;
}