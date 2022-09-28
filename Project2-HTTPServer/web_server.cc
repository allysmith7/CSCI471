#include "web_server.h"
#include <cstdlib>
#include <cstring>
#include <regex>
#include <signal.h>
#include <string>
#include <unistd.h>

int sigCode = 0;
int connFD = -1, listenFD = -1;
void sigInterrupt(int s) {
  sigCode = s;
  DEBUG << "Received code " << s << ENDL;
  exitProgram(s);
}

void exitProgram(int s) {
  DEBUG << "Exiting" << ENDL;
  close(connFD);
  close(listenFD);

  exit(s);
}

// *****************************************************************************
// - Read the header
// - Return the appropriate status code and filename
// *****************************************************************************
int readRequest(int socketFD, std::string *filename) {
  // Set default return code to 400
  int returnStatus = 400;
  int numChars;
  std::cmatch result = std::cmatch();
  std::regex endlinePattern("\r\n");
  std::regex doubleEndlinePattern("\r\n\r\n");
  std::regex getPattern(
      "GET\\s+/(file\\d\\.html|image\\d\\.jpg)?\\s+HTTP/\\d\\.\\d\r\n");
  std::regex noFilePattern("GET \\s+/\\s+HTTP/\\d\\.\\d\r\n");
  std::regex filePattern("(image\\d\\.jpg|file\\d\\.html)");

  // Read everything into a temporary buffer so we can isolate the header
  char *buffer = (char *)malloc(MAX_LEN * sizeof(char));
  // read in new characters
  DEBUG << "Reading " << MAX_LEN << " bytes from socket " << socketFD << ENDL;
  int bytesRead = read(socketFD, buffer, MAX_LEN);
  if (bytesRead == 0 || bytesRead == -1) {
    ERROR << "Problem reading from socket";
    return 400;
  }

  // regex search for double endline
  if (!regex_search(buffer, result, doubleEndlinePattern)) {
    // no end of header, this is bad
    ERROR << "No double newline found";
    return 400;
  }
  // copy the double newline and nothing more
  numChars =
      result.prefix().length() + 4; // add 4 for the "\r\n\r\n" at the end
  DEBUG << "Double newline found (" << numChars << " characters)";

  char *header = (char *)malloc(numChars * sizeof(char));
  copyBuffer(header, buffer, numChars, 0, 0);

  // isolate first line
  regex_search(header, result, endlinePattern);
  numChars = result.prefix().length() + 2; // add 2 for "\r\n"
  char *firstLine = (char *)malloc(numChars * sizeof(char));
  copyBuffer(firstLine, header, numChars, 0, 0);

  // regex search first line for GET request
  DEBUG << "Searching first line for valid GET request" << ENDL;
  if (regex_search(firstLine, getPattern)) {
    DEBUG << "Valid GET request found, checking for filename";
    // check for file name
    if (!regex_search(firstLine, noFilePattern)) {
      // file name found, validate and isolate
      if (regex_search(firstLine, result, filePattern)) {
        returnStatus = 200;
        DEBUG << "Valid file found: " << result[0].str();
      } else {
        DEBUG << "Invalid file requested";
        returnStatus = 404;
      }
    } else {
      // no file name?
      DEBUG << "No filename found";
    }
  } else {
    DEBUG << "No GET found";
  }

  return returnStatus;
}

void copyBuffer(char *dest, char *source, int n, int offsetDest,
                int offsetSource) {
  for (int i = 0; i < n; i++) {
    dest[offsetDest + i] = source[offsetSource + i];
  }
}

// *****************************************************************************
// Send one line (including the line terminator <LF><CR>)
// *****************************************************************************
void sendLine(int socketFD, std::string line) {
  // convert string to char *
  int l = line.length();
  char buffer[l + 2];
  for (int i = 0; i < l; i++) {
    buffer[i] = line[i];
  }
  // append "\r\n"
  buffer[l] = 13;
  buffer[l + 2] = 10;
  // write to FD
  write(socketFD, buffer, l+2);
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

  // sendLine(socketFD, "HTTP/1.1 400 ");

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
      exitProgram(1);
    }
  }

  // Set up signals
  struct sigaction newact;
  struct sigaction oldact;
  newact.sa_handler = sigInterrupt;
  sigemptyset(&newact.sa_mask);
  newact.sa_flags = 0;

  sigaction(SIGHUP, &newact, &oldact);
  sigaction(SIGINT, &newact, &oldact);
  sigaction(SIGQUIT, &newact, &oldact);
  sigaction(SIGABRT, &newact, &oldact);
  sigaction(SIGKILL, &newact, &oldact);

  // test code here


  
  // exit(0);

  // Creating the inital socket is the same as in a client.
  listenFD = -1;
  // Call socket() to create the socket you will use for lisening.
  listenFD = socket(AF_INET, SOCK_STREAM, 0);
  if (listenFD == -1) {
    ERROR << "Failed to create listening socket, exiting..." << ENDL;
    exitProgram(1);
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
    exitProgram(1);
  }

  /* The accept call will sleep, waiting for a connection. When a connection
    request comes in the accept() call creates a NEW socket with a new fd that
    will be used for the communication. */
  int quitProgram = 0;
  while (!quitProgram && !sigCode) {
    connFD = 0;

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
