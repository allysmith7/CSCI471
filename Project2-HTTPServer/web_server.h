// ********************************************************
// * A common set of system include files needed for socket() programming
// ********************************************************
#include <arpa/inet.h>
#include <cstddef>
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
bool isEndline(char *buffer, int index, int index_max);
int hasSingleEndline(char *buffer, int index_max);
int hasDoubleEndline(char *buffer, int index_max);
int countEndlines(char *buffer, int index, int index_max);
int nextEndline(char *buffer, int indStart, int index_max);
void copyBuffer(char *dest, char *source, int n, int offsetDest,
                int offsetSource);
void sigInterrupt(int s);
void exitProgram(int s);

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
#define BUF_LEN 32
