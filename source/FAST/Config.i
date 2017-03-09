namespace fast {

class Config {
public:
    static std::string getTestDataPath();
    static std::string getKernelSourcePath();
    static std::string getKernelBinaryPath();
    static void setConfigFilename(std::string filename);
    static void setBasePath(std::string path);
};

}