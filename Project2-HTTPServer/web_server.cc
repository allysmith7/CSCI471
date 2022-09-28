#include "web_server.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

// *****************************************************************************
// - Read the header
// - Return the appropriate status code and filename
// *****************************************************************************
int readRequest(int socketFD, std::string *filename) {
  /*
  - Set default return code to 400
  - Read everything up to and including the end of the header
  - Look at the first line of the header to see if it contains a valid GET
    - If there is a valid GET, find the filename.
    - If there is a filename, make sure it is a valid filename
      - If the filename is valid set the return code to 200
      - If the filename is invalid set the return code to 404 */
  return 0;
}

bool isEndline(char *buffer, size_t index) {
  if (index < 1)
    return false;
  return (buffer[index] == 10 && buffer[index - 1] == 13);
}

bool isDoubleEndline(char *buffer, size_t index) {
  if (index < 3)
    return false;
  return isEndline(buffer, index) && isEndline(buffer, index - 2);
}

// *****************************************************************************
// Send one line (including the line terminator <LF><CR>)
// *****************************************************************************
void sendLine(int socketFD, std::string line) {
  /* - Convert the std::string to an array that is 2 bytes longer than the
     string
     - Replace the last two bytes of the array with the <CR> and <LF>
     - Use write to send that array */

  return;
}

// *****************************************************************************
// Send a 404
// *****************************************************************************
void send404(int socketFD) {
  /* Using the sendLine() function, send the following:
      - Send a properly formatted HTTP response with the error code 404
      - Send the string, "content-type: text/html" to indicate we are sending a
        message
      - Send a blank line to terminate the header
      - Send a friendly message that indicates what the problem is (file not
        found or something like that)
      - Send a blank line to indicate the end of the message body */

  return;
}

// *****************************************************************************
// Send a 400
// *****************************************************************************
void send400(int socketFD) {
  /* Using the sendLine() function, send the following:
      - Send a properly formatted HTTP response with the error code 404
      - Send the string, "content-type: text/html" to indicate we are sending a
        message
      - Send a blank line to terminate the header
      - Send a friendly message that indicates what the problem is (file not
        found or something like that)
      - Send a blank line to indicate the end of the message body */

  return;
}

void sendFile(int socketFD, std::string filename) {}

// *****************************************************************************
// - Handles reading the line from the network and sending it back to the client
// - Returns 1 if the client sends "QUIT" command, 0 if the client sends "CLOSE"
// *****************************************************************************
int processConnection(int socketFD) {
  std::string filename = "";
  int returnCode = readRequest(socketFD, &filename);
  switch (returnCode) {
  case 400:
    send400(socketFD);
    break;
  case 404:
    send404(socketFD);
    break;
  case 200:
    sendFile(socketFD, filename);
    break;
  default:
    ERROR << "Unknown response code, skipping" << ENDL;
  }
  return 0;
}

// *****************************************************************************
// * - Sets up the sockets and accepts new connection until processConnection()
// returns 1
// *****************************************************************************
int main(int argc, char *argv[]) {
  boost::log::add_console_log(std::cout,
                              boost::log::keywords::format = "%Message%");
  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      boost::log::trivial::info);

  // Process the command line arguments
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

  // Creating the inital socket is the same as in a client.
  int listenFD = -1;
  // Call socket() to create the socket you will use for lisening.
  listenFD = socket(AF_INET, SOCK_STREAM, 0);
  if (listenFD == -1) {
    ERROR << "Failed to create listening socket, exiting..." << ENDL;
    exit(-1);
  }
  DEBUG << "Calling Socket() assigned file descriptor " << listenFD << ENDL;

  /* The bind() and calls take a structure that specifies the address to be used
    for the connection. On the cient it contains the address of the server to
    connect to. On the server it specifies which IP address and port to listen
    for connections. */
  struct sockaddr_in servAddr;
  srand(time(NULL));
  // int port = (rand() % 10000) + 1024;
  int port = 1024;
  bzero(&servAddr, sizeof(servAddr));
  servAddr.sin_family = PF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(port);

  /* Binding configures the socket with the parameters we have specified in the
    servaddr structure. This step is implicit in the connect() call, but must be
    explicitly listed for servers. You may have to call bind multiple times if
    another process is already using the port your program selects. */
  DEBUG << "Calling bind(" << listenFD << ", " << &servAddr << ", "
        << sizeof(servAddr) << ")" << ENDL;
  int bindStatus = -1;
  while (bindStatus != 0) {
    bindStatus = bind(listenFD, (struct sockaddr *)&servAddr, sizeof(servAddr));
  }
  std::cout << "Using port " << port << std::endl;

  /* Setting the socket to the listening state is the second step
    needed to being accepting connections. This creates a queue for
    connections and starts the kernel listening for connections. */
  int listenQueueLength = 1;
  DEBUG << "Calling listen(" << listenFD << ", " << listenQueueLength << ")"
        << ENDL;
  int listenStatus = listen(listenFD, listenQueueLength);
  if (listenStatus != 0) {
    ERROR << "Failed to start listening, exiting..." << ENDL;
    exit(1);
  }

  /* The accept call will sleep, waiting for a connection. When a connection
    request comes in the accept() call creates a NEW socket with a new fd that
    will be used for the communication. */
  int quitProgram = 0;
  while (!quitProgram) {
    int connFD = 0;

    // The accept() call checks the listening queue for connection requests.
    DEBUG << "Calling accept(" << listenFD << ", NULL, NULL)." << ENDL;
    connFD = accept(listenFD, NULL, NULL);
    DEBUG << "We have recieved a connection on " << connFD << ENDL;

    // process the connection using our defined function
    quitProgram = processConnection(connFD);

    close(connFD);
  }

  close(listenFD);
}
