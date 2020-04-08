#include "Config.hpp"
#include "Exception.hpp"
#include "Utility.hpp"
#include <fstream>

// Includes needed to get path of dynamic library
#ifdef _WIN32
#include <windows.h>
#else
#define __USE_GNU 1
#include <dlfcn.h>
#endif

namespace fast {

	namespace Config {
		namespace {
			// Initialize global variables, put in anonymous namespace to hide them
			bool mConfigurationLoaded = false;
			std::string mConfigFilename = "";
			std::string mBasePath = "";
			std::string mTestDataPath;
			std::string mKernelSourcePath;
			std::string mKernelBinaryPath;
			std::string mDocumentationPath;
			std::string mPipelinePath;
			std::string mLibraryPath;
			std::string mQtPluginsPath;
			StreamingMode m_streamingMode = STREAMING_MODE_PROCESS_ALL_FRAMES;
		}

		std::string getPath() {
			if (mBasePath != "")
				return mBasePath;
			std::string path = "";
			std::string slash = "/";
			// Find path of the FAST dynamic library
			// The fast_configuration.txt file should lie in the folder below
#ifdef _WIN32
			// http://stackoverflow.com/questions/6924195/get-dll-path-at-runtime
			HMODULE hm = NULL;

			if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
				GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCSTR)&getPath,
				&hm)) {
				int ret = GetLastError();
				throw Exception("Error reading dyanmic library address in getPath()");
			}
			char dlpath[MAX_PATH];
			GetModuleFileNameA(hm, dlpath, sizeof(dlpath));
			path = std::string(dlpath);
			path = replace(path, "\\", "/"); // Replace windows \ with /
			path = replace(path, "/FAST.dll", "");
#else
			// http://stackoverflow.com/questions/1681060/library-path-when-dynamically-loaded
			Dl_info dl_info;
			int ret = dladdr((void *)&getPath, &dl_info);
			if (ret == 0)
				throw Exception("Error reading dynamic library address in getPath()");
			const char* dlpath = dl_info.dli_fname;
			path = std::string(dlpath);
			// Remove lib name and lib folder
			int libPos = path.rfind(slash + "lib" + slash);
			path = path.substr(0, libPos) + slash + "bin";
#endif


			path = path + slash; // Make sure there is a slash at the end
			return path;
		}
		
		void loadConfiguration() {
			if (mConfigurationLoaded)
				return;

			// Set default paths
			mTestDataPath = getPath() + "../../data/";
			mKernelSourcePath = getPath() + "../../source/FAST/";
			mKernelBinaryPath = getPath() + "../kernel_binaries/";
			mDocumentationPath = getPath() + "../../doc/";
			mPipelinePath = getPath() + "../../pipelines/";
			mQtPluginsPath = getPath() + "../plugins/";

			std::string writeablePath = getPath();
#ifdef WIN32
			mLibraryPath = getPath();
			if(getPath().find("Program Files") != std::string::npos) {
				// Special case for windows: default install dir: program files is not writeable.
				// Use ProgramData folder instead.
				writeablePath = "C:/ProgramData/FAST/";
				// Copy pipelines (first time only)
				for(auto&& pipeline : getDirectoryList(mPipelinePath)) {
					if(!fileExists(writeablePath + "pipelines/" + pipeline)) {
						// Copy file from source folder
						std::ifstream  src(mPipelinePath + pipeline, std::ios::binary);
						std::ofstream  dst(writeablePath + "pipelines/" + pipeline, std::ios::binary);

						dst << src.rdbuf();
					}
				}
			}
#else
			mLibraryPath = getPath() + "/../lib/";
#endif

			// Read and parse configuration file
			// It should reside in the build folder when compiling, and in the root folder when using release
			std::string filename;
			if(mConfigFilename == "") {
				filename = getPath() + "fast_configuration.txt";
			}
			else {
				filename = mConfigFilename;
			}
			std::ifstream file(filename);
			if (!file.is_open()) {
				Reporter::warning() << "Unable to open the configuration file " << filename << ". Using defaults instead." << Reporter::end();
				mConfigurationLoaded = true;
				return;
			}
			Reporter::info() << "Loaded configuration file: " << filename << Reporter::end();


			do {
				std::string line;
				std::getline(file, line);
				trim(line);
				if (line[0] == '#' || line.size() == 0) {
					// Comment or empty line, skip
					continue;
				}
				std::vector<std::string> list = split(line, "=");
				if (list.size() != 2) {
					throw Exception("Error parsing configuration file. Expected key = value pair, got: " + line);
				}

				std::string key = list[0];
				std::string value = list[1];
				trim(key);
				trim(value);				

				if (key == "TestDataPath") {
					value = replace(value, "@ROOT@", getPath() + "/../");
					mTestDataPath = value;
				}
				else if (key == "KernelSourcePath") {
					value = replace(value, "@ROOT@", getPath() + "/../");
					mKernelSourcePath = value;
				}
				else if (key == "KernelBinaryPath") {
					value = replace(value, "@ROOT@", writeablePath);
					mKernelBinaryPath = value;
				}
				else if (key == "DocumentationPath") {
					value = replace(value, "@ROOT@", getPath() + "/../");
					mDocumentationPath = value;
				}
				else if (key == "PipelinePath") {
					value = replace(value, "@ROOT@", writeablePath);
					mPipelinePath = value;
				}
				else if (key == "QtPluginsPath") {
					value = replace(value, "@ROOT@", getPath() + "/../");
					mQtPluginsPath = value;
				}
				else if (key == "LibraryPath") {
					value = replace(value, "@ROOT@", getPath() + "/../");
					mLibraryPath = value;
				}
				else {
					throw Exception("Error parsing configuration file. Unrecognized key: " + key);
				}
			} while (!file.eof());
			file.close();

			Reporter::info() << "Test data path: " << mTestDataPath << Reporter::end();
			Reporter::info() << "Kernel source path: " << mKernelSourcePath << Reporter::end();
			Reporter::info() << "Kernel binary path: " << mKernelBinaryPath << Reporter::end();
			Reporter::info() << "Documentation path: " << mDocumentationPath << Reporter::end();
			Reporter::info() << "Pipeline path: " << mPipelinePath << Reporter::end();
			Reporter::info() << "Qt plugins path: " << mQtPluginsPath << Reporter::end();
            Reporter::info() << "Library path: " << mLibraryPath << Reporter::end();

			mConfigurationLoaded = true;
		}

		std::string getTestDataPath() {
			loadConfiguration();
			return mTestDataPath;
		}

		std::string getKernelSourcePath() {
			loadConfiguration();
			return mKernelSourcePath;
		}

		std::string getKernelBinaryPath() {
			loadConfiguration();
			return mKernelBinaryPath;
		}

		std::string getDocumentationPath() {
			loadConfiguration();
			return mDocumentationPath;
		}

		std::string getPipelinePath() {
			loadConfiguration();
			return mPipelinePath;
		}

		std::string getLibraryPath() {
		    loadConfiguration();
		    return mLibraryPath;
		}

		std::string getQtPluginsPath() {
			loadConfiguration();
			return mQtPluginsPath;
		}

		void setConfigFilename(std::string filename) {
			mConfigFilename = filename;
			loadConfiguration();
		}

		void setBasePath(std::string path) {
			mBasePath = path;
			if (mBasePath[mBasePath.size() - 1] != '/')
				mBasePath += "/";
			mTestDataPath = getPath() + "../data/";
			mKernelSourcePath = getPath() + "../source/FAST/";
			mKernelBinaryPath = getPath() + "kernel_binaries/";
			mDocumentationPath = getPath() + "../doc/";
			mPipelinePath = getPath() + "../pipelines/";
#ifdef WIN32
            mLibraryPath = getPath() + "../bin/";
#else
            mLibraryPath = getPath() + "../lib/";
#endif
			loadConfiguration();
		}

		void setTestDataPath(std::string path) {
			mTestDataPath = path;
		}

		void setKernelSourcePath(std::string path) {
			mKernelSourcePath = path;
		}

		void setKernelBinaryPath(std::string path) {
			mKernelBinaryPath = path;
		}

		void setDocumentationPath(std::string path) {
			mDocumentationPath = path;
		}

		void setPipelinePath(std::string path) {
			mPipelinePath = path;
		}

		void setStreamingMode(StreamingMode mode) {
		    m_streamingMode = mode;
		}

		StreamingMode getStreamingMode() {
		    return m_streamingMode;
		}

	} // end namespace Config

}; // end namespace fast
