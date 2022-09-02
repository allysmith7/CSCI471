// **************************************************************************************
// * Echo Strings (echo_s.cc)
// * -- Accepts TCP connections and then echos back each string sent.
// **************************************************************************************
#include "echo_s.h"
#include <cstring>
#include <string>
#include <unistd.h>

// **************************************************************************************
// * processConnection()
// * - Handles reading the line from the network and sending it back to the
// client.
// * - Returns 1 if the client sends "QUIT" command, 0 if the client sends
// "CLOSE".
// **************************************************************************************
int processConnection(int sockFd) {

  int quitProgram = 0;
  int keepGoing = 1;

  char buffer[MAX_LEN];
  while (keepGoing) {

    // clear the buffer
    bzero(buffer, MAX_LEN);
    // read from the socket to the buffer
    DEBUG << "Calling read(" << sockFd << ", buffer, " << MAX_LEN << ENDL;
    ssize_t bytesRead = read(sockFd, buffer, MAX_LEN);
    DEBUG << "Received message of length " << bytesRead << ENDL;

    // search for CLOSE or QUIT using cpp library functions
    std::string temp(buffer);
    size_t posClose = temp.find("CLOSE");
    size_t posQuit = temp.find("QUIT");

    // if either close or quit are found, handle this
    // if both are found, close is assumed
    if (posClose != std::string::npos) {
      // close fd and return 0
      DEBUG << "Input contained 'CLOSE'" << ENDL;
      close(sockFd);
      quitProgram = 0;
      break;
    } else if (posQuit != std::string::npos) {
      // close fd and return 1
      DEBUG << "Input contained 'QUIT'" << ENDL;
      close(sockFd);
      quitProgram = 1;
      break;
    }

    // write back to client, start listening again
    DEBUG << "Calling write(" << sockFd << ", buffer, " << bytesRead << ENDL;
    write(sockFd, buffer, bytesRead);
  }

  return quitProgram;
}

// **************************************************************************************
// * main()
// * - Sets up the sockets and accepts new connection until processConnection()
// returns 1
// **************************************************************************************

int main(int argc, char *argv[]) {

  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************
  boost::log::add_console_log(std::cout,
                              boost::log::keywords::format = "%Message%");
  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      boost::log::trivial::info);

  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************
  int opt = 0;
  while ((opt = getopt(argc, argv, "v")) != -1) {

    switch (opt) {
    case 'v':
      boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                          boost::log::trivial::debug);
      break;
    case ':':
    case '?':
    default:
      std::cout << "useage: " << argv[0] << " -v" << std::endl;
      exit(-1);
    }
  }

  // *******************************************************************
  // * Creating the inital socket is the same as in a client.
  // ********************************************************************
  int listenFd = -1;
  // Call socket() to create the socket you will use for lisening.
  listenFd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenFd == -1) {
    ERROR << "Failed to create listening socket, exiting..." << ENDL;
    exit(-1);
  }
  DEBUG << "Calling Socket() assigned file descriptor " << listenFd << ENDL;

  // ********************************************************************
  // * The bind() and calls take a structure that specifies the
  // * address to be used for the connection. On the cient it contains
  // * the address of the server to connect to. On the server it specifies
  // * which IP address and port to lisen for connections.
  // ********************************************************************
  struct sockaddr_in servaddr;
  srand(time(NULL));
  int port = (rand() % 10000) + 1024;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = PF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  // ********************************************************************
  // * Binding configures the socket with the parameters we have
  // * specified in the servaddr structure.  This step is implicit in
  // * the connect() call, but must be explicitly listed for servers.
  // ********************************************************************

  DEBUG << "Calling bind(" << listenFd << ", " << &servaddr << ", "
        << sizeof(servaddr) << ")" << ENDL;
  int bindStatus = -1;
  while (bindStatus != 0) {
    // You may have to call bind multiple times if another process is already
    // using the port your program selects.
    bindStatus = bind(listenFd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  }
  std::cout << "Using port " << port << std::endl;

  // ********************************************************************
  // * Setting the socket to the listening state is the second step
  // * needed to being accepting connections.  This creates a queue for
  // * connections and starts the kernel listening for connections.
  // ********************************************************************
  int listenQueueLength = 1;
  DEBUG << "Calling listen(" << listenFd << ", " << listenQueueLength << ")"
        << ENDL;
  int listenStatus = listen(listenFd, listenQueueLength);
  if (listenStatus != 0) {
    ERROR << "Failed to start listening, exiting..." << ENDL;
    exit(1);
  }

  // ********************************************************************
  // * The accept call will sleep, waiting for a connection.  When
  // * a connection request comes in the accept() call creates a NEW
  // * socket with a new fd that will be used for the communication.
  // ********************************************************************
  int quitProgram = 0;
  while (!quitProgram) {
    int connFd = 0;


    // The accept() call checks the listening queue for connection requests.
    // If a client has already tried to connect accept() will complete the
    // connection and return a file descriptor that you can read from and
    // write to. If there is no connection waiting accept() will block and
    // not return until there is a connection.

    DEBUG << "Calling accept(" << listenFd << ", NULL, NULL)." << ENDL;
    connFd = accept(listenFd, NULL, NULL);
    DEBUG << "We have recieved a connection on " << connFd << ENDL;

    // process the connection using our defined function
    quitProgram = processConnection(connFd);

    close(connFd);
  }

  close(listenFd);
}
