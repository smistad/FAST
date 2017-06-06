#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_

#include "FASTExport.hpp"
#include <exception>
#include <string>
#include <sstream>

namespace fast {
inline std::string intToString(int number) {
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}
class FAST_EXPORT  Exception : public std::exception {
    public:
        Exception() {
            this->line = -1;
            this->message = "";
            this->file = "";
        };
        Exception(std::string message) {
            this->line = -1;
            this->message = message;
            this->file = "";
        };
        Exception(int line, const char * file) {
            this->line = line;
            this->message = "";
            this->file = file;
        };
        Exception(std::string message, int line, const char * file) {
            this->message = message;
            this->line = line;
            this->file = file;
        };
        virtual const char * what() const throw() {
            if(line > -1) {
                std::string string = message + "\nException thrown at line " +
                        intToString(line) + " in file " + std::string(file);
                return string.c_str();
            } else {
                return message.c_str();
            }
        };
        void setLine(int line) {
            this->line = line;
        };
        void setFile(const char * file) {
            this->file = file;
        };
        void setMessage(std::string message) {
            this->message = message;
        };
        ~Exception() throw() {};
    private:
        int line;
        const char * file;
        std::string message;
};

class FAST_EXPORT  FileNotFoundException : public Exception {
    public:
        FileNotFoundException(std::string filename) : Exception() {
            setMessage("Could not open the file " + filename);
        };
        FileNotFoundException() : Exception() {};
        FileNotFoundException(int line, const char * file) : Exception(line,file) {};
        FileNotFoundException(std::string message, int line, const char * file): Exception(message,line,file) {};
};

class FAST_EXPORT  OutOfBoundsException : public Exception {
    public:
        OutOfBoundsException() : Exception() {
            setMessage("Out of bounds.");
        };
        OutOfBoundsException(int line, const char * file) : Exception(line,file) {
            setMessage("Out of bounds.");
        };
        OutOfBoundsException(std::string message, int line, const char * file): Exception(message,line,file) {};
};

class FAST_EXPORT  NotImplementedException : public Exception {
    public:
        NotImplementedException() : Exception() {
            setMessage("Not implemented");
        };
        NotImplementedException(int line, const char * file) : Exception(line,file) {
            setMessage("Not implemented");

        };
};

class FAST_EXPORT  DoesNotExistException : public Exception {
    public:
        DoesNotExistException(std::string msg) : Exception(msg) {};
};

class FAST_EXPORT  ExistException : public Exception {
    public:
        ExistException(std::string msg) : Exception(msg) {};

};

} // end namespace fast


#endif /* EXCEPTIONS_HPP_ */
