namespace fast {

namespace Config {
     std::string getTestDataPath();
     std::string getKernelSourcePath();
     std::string getKernelBinaryPath();
     std::string getDocumentationPath();
     std::string getPipelinePath();
	 void setTestDataPath(std::string path);
	 void setKernelSourcePath(std::string path);
	 void setKernelBinaryPath(std::string path);
	 void setDocumentationPath(std::string path);
	 void setPipelinePath(std::string path);
     void setConfigFilename(std::string filename);
     void setBasePath(std::string path);
	 void loadConfiguration();
}

}