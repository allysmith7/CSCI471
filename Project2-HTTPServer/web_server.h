// ********************************************************
// * A common set of system include files needed for socket() programming
// ********************************************************
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// ********************************************************
// * The boost log facility makes setting log levels easy.
// * Students don't have to use this facility.
// ********************************************************
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>

// functions
int readRequest(int socketFD, std::string *filename);
bool isEndline(char *buffer, size_t index);
bool isDoubleEndline(char *buffer, size_t index);
void sendLine(int socketFD, std::string line);
void send404(int socketFD);
void send400(int socketFD);
void sendFile(int socketFD, std::string filename);

// ********************************************************
// * These don't really provide any improved functionality,
// * but IMHO they make the code more readable.
// ********************************************************
#define TRACE BOOST_LOG_TRIVIAL(trace)
#define DEBUG BOOST_LOG_TRIVIAL(debug)
#define INFO BOOST_LOG_TRIVIAL(info)
#define WARNING BOOST_LOG_TRIVIAL(warning)
#define ERROR BOOST_LOG_TRIVIAL(error)
#define FATAL BOOST_LOG_TRIVIAL(fatal)
#define ENDL " (" << __FILE__ << ":" << __LINE__ << ")"
#define MAX_LEN 1024
