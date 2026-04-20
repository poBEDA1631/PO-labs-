#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <optional>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

// Конфігурація сервера
#define PORT 6769

#define ROOT_DIR "static"

// Повертає вміст файлу або nullopt якщо файл не знайдено
optional<string> read_file(const string& path) {
    ifstream file(path, ios::binary);
    if (!file.is_open()) return nullopt;
    ostringstream ss;
    ss << file.rdbuf();
    return ss.str(); // може бути порожнім рядком — це валідний вміст
}

// Визначення Content-Type за розширенням файлу
string get_content_type(const string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html; charset=utf-8";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css")  return "text/css";
    if (path.size() >= 3 && path.substr(path.size() - 3) == ".js")   return "application/javascript";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")  return "image/png";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg")  return "image/jpeg";
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".jpeg") return "image/jpeg";
    return "application/octet-stream";
}

// Санітизація URL-шляху: захист від path traversal (/../)
// Повертає false якщо шлях підозрілий
bool sanitize_path(const string& path) {
    // Забороняємо ".." у будь-якому місці шляху
    if (path.find("..") != string::npos) return false;
    // Шлях має починатися з '/'
    if (path.empty() || path[0] != '/') return false;
    return true;
}

// Функція для формування та відправки HTTP-відповіді
void send_response(int client_sock, const string& status,
                   const string& content, const string& content_type = "text/html; charset=utf-8") {
    ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n";
    response << "Content-Length: " << content.size() << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << content;

    string res_str = response.str();
    send(client_sock, res_str.c_str(), res_str.length(), 0);
}

// Обробка одного клієнта у окремому потоці
void handle_client(int client_sock) {
    try {
        char buffer[4096];
        ssize_t received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);

        if (received <= 0) {
            throw runtime_error("Empty request or connection closed");
        }

        buffer[received] = '\0';
        cout << "[Server] Request received on socket " << client_sock << endl;

        istringstream request(buffer);
        string method, path, version;
        request >> method >> path >> version;

        // Підтримуємо тільки GET
        if (method != "GET") {
            string body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
            send_response(client_sock, "405 Method Not Allowed", body);
        }
        else {
            // Нормалізація кореневого шляху
            if (path == "/") path = "/index.html";

            // Захист від path traversal
            if (!sanitize_path(path)) {
                cerr << "[Server] Blocked suspicious path: " << path << endl;
                string body = "<html><body><h1>400 Bad Request</h1></body></html>";
                send_response(client_sock, "400 Bad Request", body);
            }
            else {
                string file_path = string(ROOT_DIR) + path;
                auto content = read_file(file_path);

                if (!content.has_value()) {
                    // Файл справді не знайдено
                    cout << "[Server] 404 Not Found: " << path << endl;
                    string body = "<html><body><h1>404 Not Found</h1></body></html>";
                    send_response(client_sock, "404 Not Found", body);
                }
                else {
                    // Успіх (content може бути порожнім — це нормально)
                    cout << "[Server] 200 OK: " << path << endl;
                    send_response(client_sock, "200 OK", content.value(), get_content_type(file_path));
                }
            }
        }
    }
    catch (const exception& e) {
        cerr << "[Error] Socket " << client_sock << ": " << e.what() << endl;
    }

    // Закриття з'єднання (HTTP/1.1 без Keep-Alive для простоти)
    close(client_sock);
    cout << "[Server] Connection closed for socket " << client_sock << endl;
}

int main() {
    // Створення сокета
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Failed to create socket" << endl;
        return 1;
    }

    // Налаштування повторного використання адреси (уникнення "Address already in use")
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Налаштування адреси
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    // Прив'язка сокета
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "Bind failed. Port " << PORT << " might be busy." << endl;
        close(server_fd);
        return 1;
    }

    // Слухання вхідних з'єднань
    if (listen(server_fd, SOMAXCONN) < 0) {
        cerr << "Listen failed" << endl;
        close(server_fd);
        return 1;
    }

    cout << "HTTP Server started at http://localhost:" << PORT << endl;
    cout << "Serving files from directory: ./" << ROOT_DIR << endl;
    cout << "Working directory: run from project root so './" << ROOT_DIR << "/' is reachable." << endl;

    // Головний цикл прийому клієнтів
    while (true) {
        int client_sock = accept(server_fd, nullptr, nullptr);
        if (client_sock >= 0) {
            // Кожне з'єднання обробляється у окремому detached-потоці
            thread(handle_client, client_sock).detach();
        }
    }
}