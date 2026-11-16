#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>

int main()
{
  int port{};

  std::cout << "Server Port: ";
  std::cin >> port;

  int sock{ socket(AF_INET, SOCK_DGRAM, 0) };  //UDP socket
  if (sock < 0) {
    std::cerr << "Failed to create socket" << '\n';
    return 1;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0) {
    std::cerr << "Bind failed" << '\n';
    close(sock);
    return 1;
  }

  std::cout << "Server listening on port " << port << '\n';

  std::map<std::string, sockaddr_in> clients{};  //"ip:port" -> address

  char buff[1024];
  while (true) {
    sockaddr_in client{};
    socklen_t clientLen{ sizeof(client) };

    ssize_t n{ recvfrom(sock, buff, sizeof(buff) - 1, 0, (sockaddr *)&client, &clientLen) };
    if (n < 0) {continue;}
    buff[n] = '\0';

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client.sin_addr, ipStr, sizeof(ipStr));
    std::string id{ std::string(ipStr) + ":" + std::to_string(ntohs(client.sin_port)) };

    //first packet from a new address registers the client
    if (clients.find(id) == clients.end()) {
      clients[id] = client;
      std::cout << id << " joined" << '\n';
    }

    std::string msg{ id + ": " + buff };
    std::cout << msg << '\n';

    //relay to everyone except the sender
    for (auto &[otherId, otherAddr] : clients) {
      if (otherId == id) {continue;}
      sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr *)&otherAddr, sizeof(otherAddr));
    }
  }

  close(sock);
  return 0;
}
