#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP
#include <exception>
#include <cstdio>
namespace oul {

class Exception : public std::exception {
    public:
        Exception() {
            this->line = -1;
            this->message = "";
            this->file = "";
        };
        Exception(const char * message) {
            this->line = -1;
            this->message = message;
            this->file = "";
        };
        Exception(int line, const char * file) {
            this->line = line;
            this->message = "";
            this->file = file;
        };
        Exception(const char * message, int line, const char * file) {
            this->message = message;
            this->line = line;
            this->file = file;
        };
        virtual const char * what() const throw() {
            char * string = new char[255];
            if(line > -1) {
                sprintf(string, "%s \nException thrown at line %d in file %s", message, line, file);
                return string;
            } else {
                return message;
            }
        };
        void setLine(int line) {
            this->line = line;
        };
        void setFile(const char * file) {
            this->file = file;
        };
        void setMessage(const char * message) {
            this->message = message;
        };
    private:
        int line;
        const char * file;
        const char * message;
};

class NoPlatformsInstalledException : public Exception {
    public:
        NoPlatformsInstalledException() {
            setMessage("No platforms are installed on this system."
                "Check that OpenCL is installed and that ICD is registered properly");
        };
};

class NoValidPlatformsException : public Exception {
    public:
        NoValidPlatformsException() {
            setMessage("No valid OpenCL platforms were found. Check the device criteria.");
        };

};

} // end namespace oul
#endif
