#include "CenterlineExtraction.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Utility.hpp"
#include <unordered_set>
#include <stack>
#include "FAST/Exporters/MetaImageExporter.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

CenterlineExtraction::CenterlineExtraction() {
	createInputPort<Image>(0);
	createOutputPort<Mesh>(0, OUTPUT_DEPENDS_ON_INPUT, 0);

	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CenterlineExtraction/CenterlineExtraction.cl");
}

Image::pointer CenterlineExtraction::calculateDistanceTransform(Image::pointer input) {
	OpenCLDevice::pointer device = getMainDevice();
	cl::Program program = getOpenCLProgram(device);
	cl::CommandQueue queue = device->getCommandQueue();
	const int width = input->getWidth();
	const int height = input->getHeight();
	const int depth = input->getDepth();

	Image::pointer distance = Image::New();
	distance->create(input->getSize(), TYPE_INT16, 1);


	// First initialize
	cl::Kernel initializeKernel(program, "initialize");

	if(device->isWritingTo3DTexturesSupported()) {
		OpenCLImageAccess::pointer distanceAccess = distance->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
		initializeKernel.setArg(0, *inputAccess->get3DImage());
		initializeKernel.setArg(1, *distanceAccess->get3DImage());
	} else {
		OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
		OpenCLBufferAccess::pointer distanceAccess = distance->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		initializeKernel.setArg(0, *inputAccess->get3DImage());
		initializeKernel.setArg(1, *distanceAccess->get());
	}

	queue.enqueueNDRangeKernel(
			initializeKernel,
			cl::NullRange,
			cl::NDRange(width, height, depth),
			cl::NullRange
    );

	// Iteratively calculate distance
	Image::pointer distance2 = Image::New();
	distance2->create(input->getSize(), TYPE_INT16, 1);

	cl::Buffer changedBuffer(
			device->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(char)
    );

	// TODO no 3D write support
	cl::Kernel distanceKernel(program, "calculateDistance");
	distanceKernel.setArg(2, changedBuffer);
	int counter = 0;
	if(device->isWritingTo3DTexturesSupported()) {
		OpenCLImageAccess::pointer distanceAccess = distance->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		OpenCLImageAccess::pointer distance2Access = distance2->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		cl::Image3D* clDistance = distanceAccess->get3DImage();
		cl::Image3D* clDistance2 = distance2Access->get3DImage();

		queue.enqueueCopyImage(
				*clDistance,
				*clDistance2,
				createOrigoRegion(),
				createOrigoRegion(),
				createRegion(input->getSize())
		);

		while(true) {
			// Write 0 to changedBuffer
			char zero = 0;
			queue.enqueueWriteBuffer(changedBuffer,CL_FALSE,0,sizeof(char),&zero);

			if(counter % 2 == 0) {
				distanceKernel.setArg(0, *clDistance);
				distanceKernel.setArg(1, *clDistance2);
			} else {
				distanceKernel.setArg(1, *clDistance);
				distanceKernel.setArg(0, *clDistance2);
			}
			counter++;
			queue.enqueueNDRangeKernel(
				distanceKernel,
				cl::NullRange,
				cl::NDRange(width, height, depth),
				cl::NullRange
			);

			queue.enqueueReadBuffer(changedBuffer,CL_TRUE,0,sizeof(char),&zero);
			if(zero == 0) {
				break;
			}
		}
	} else {

		OpenCLBufferAccess::pointer distanceAccess = distance->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		OpenCLBufferAccess::pointer distance2Access = distance2->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		cl::Buffer* clDistance = distanceAccess->get();
		cl::Buffer* clDistance2 = distance2Access->get();

		queue.enqueueCopyBuffer(
				*clDistance,
				*clDistance2,
				0,
				0,
				input->getSize().x()*input->getSize().y()*input->getSize().z()*sizeof(short)
		);

		while(true) {
			// Write 0 to changedBuffer
			char zero = 0;
			queue.enqueueWriteBuffer(changedBuffer,CL_FALSE,0,sizeof(char),&zero);

			if(counter % 2 == 0) {
				distanceKernel.setArg(0, *clDistance);
				distanceKernel.setArg(1, *clDistance2);
			} else {
				distanceKernel.setArg(1, *clDistance);
				distanceKernel.setArg(0, *clDistance2);
			}
			counter++;
			queue.enqueueNDRangeKernel(
				distanceKernel,
				cl::NullRange,
				cl::NDRange(width, height, depth),
				cl::NullRange
			);

			queue.enqueueReadBuffer(changedBuffer,CL_TRUE,0,sizeof(char),&zero);
			if(zero == 0) {
				break;
			}
		}
	}
	reportInfo() << "Calculated distance in " << counter << " steps" << reportEnd();


	if(counter % 2 == 0) {
		return distance;
	} else {
		return distance2;
	}
}

inline uint linearPosition(Vector3i pos, Vector3i size) {
	return pos.x() + pos.y()*size.x() + pos.z()*size.x()*size.y();
}

inline Vector3i position3D(int pos, Vector3i size) {
	int z = floor(pos / (size.x()*size.y()));
	pos -= z*size.x()*size.y();
	int y = floor(pos / (size.x()));
	pos -= y*size.x();
	int x = pos;

	return Vector3i(x,y,z);
}

inline double solveQuadratic(const UniquePointer<double[]>& G, const UniquePointer<double[]>& speed, Vector3i pos, Vector3i size) {

	std::vector<double> abc = {
            std::min(G[linearPosition(pos + Vector3i(1,0,0), size)], G[linearPosition(pos - Vector3i(1,0,0), size)]),
            std::min(G[linearPosition(pos + Vector3i(0,1,0), size)], G[linearPosition(pos - Vector3i(0,1,0), size)]),
            std::min(G[linearPosition(pos + Vector3i(0,0,1), size)], G[linearPosition(pos - Vector3i(0,0,1), size)])
	};

	std::sort(abc.begin(), abc.end());
	double a = abc[2];
	double b = abc[1];
	double c = abc[0];

	double f = speed[linearPosition(pos, size)];

	if(f == 0) {
		std::cout << "speed is zero!" << std::endl;
		exit(0);
	}

	double u = c + 1.0 / f;
	if(u <= b) {
	    return u;
	} else {
		double sqrted = -b*b - c*c + 2.0*b*c + (2.0/(f*f));
		if(sqrted < 0) {
			sqrted = 0;
		} else {
			sqrted = sqrt(sqrted);
		}

	    u = (b + c + sqrted)/2.0;
	    if(u <= a) {
	        return u;
	    } else {
	    	sqrted = 4.0*(a + b + c)*(a + b + c) - 12.0*(a*a + b*b + c*c - 1/(f*f));
            if(sqrted < 0) {
            	sqrted = 0;
            } else {
                sqrted = sqrt(sqrted);
            }
	        return (2.0*(a + b + c) + sqrted)/6.0;
	    }
	}
}

inline void growFromPointsAdded(std::vector<Vector3i> points, const UniquePointer<double[]>& G, std::unordered_set<int>& Sc, std::unordered_set<int>& processed, Vector3i size) {

	std::stack<Vector3i> stack;
	stack.push(points[0]);

	std::vector<Vector3i> neighbors2;
    for(int a = -1; a <= 1;  ++a) {
    for(int b = -1; b <= 1;  ++b) {
    for(int c = -1; c <= 1;  ++c) {
    	if(a == 0 && b == 0 && c == 0)
    		continue;
    	neighbors2.push_back(Vector3i(a,b,c));
    }}}

	while(!stack.empty()) {
		Vector3i current = stack.top();
		stack.pop();
        std::unordered_set<int>::iterator it = Sc.find(linearPosition(current, size));
        if(it != Sc.end()) {
            Sc.erase(it);
        }

		// Add neighbors
        for(Vector3i neighbor : neighbors2) {
            Vector3i xn = current + neighbor;
            if(G[linearPosition(xn, size)] < G[linearPosition(current, size)]) {
            	if(processed.count(linearPosition(xn, size)) == 0) {
                    stack.push(xn);
                    processed.insert(linearPosition(xn, size));
            	}
            }
        }
	}
}

void CenterlineExtraction::execute() {
	Image::pointer input = getStaticInputData<Image>();
	Vector3f spacing = input->getSpacing();

	// Do distance transform
	Image::pointer distance = calculateDistanceTransform(input);
	/*
	MetaImageExporter::pointer exporter = MetaImageExporter::New();
	exporter->setInputData(distance);
	exporter->setFilename("/home/smistad/distance.mhd");
	exporter->update();
	*/

	// Extract candidate centerlines, and get max distance
	const Vector3i size = input->getSize().cast<int>();
	const int width = input->getWidth();
	const int height = input->getHeight();
	const int depth = input->getDepth();
	const int totalSize = width*height*depth;

	Image::pointer candidateCenterpointsImage = Image::New();
	candidateCenterpointsImage->create(size.cast<uint>(), TYPE_UINT8, 1);

	{
        OpenCLDevice::pointer device = getMainDevice();
        cl::Program program = getOpenCLProgram(device);
        cl::CommandQueue queue = device->getCommandQueue();
        OpenCLImageAccess::pointer distanceAccess = distance->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);

        // First initialize
        cl::Kernel candidateKernel(program, "findCandidateCenterpoints");
        candidateKernel.setArg(0, *inputAccess->get3DImage());
        candidateKernel.setArg(1, *distanceAccess->get3DImage());

        if(device->isWritingTo3DTexturesSupported()) {
			OpenCLImageAccess::pointer candidateAccess = candidateCenterpointsImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
			candidateKernel.setArg(2, *candidateAccess->get3DImage());
        } else {
			OpenCLBufferAccess::pointer candidateAccess = candidateCenterpointsImage->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
			candidateKernel.setArg(2, *candidateAccess->get());
        }

        queue.enqueueNDRangeKernel(
            candidateKernel,
            cl::NullRange,
            cl::NDRange(width, height, depth),
            cl::NullRange
        );
	}
	/*
	MetaImageExporter::pointer exporter2 = MetaImageExporter::New();
	exporter2->setInputData(candidateCenterpointsImage);
	exporter2->setFilename("/home/smistad/candidates.mhd");
	exporter2->update();
	*/

	ImageAccess::pointer inputAccess = input->getImageAccess(ACCESS_READ);
	ImageAccess::pointer distanceAccess = distance->getImageAccess(ACCESS_READ);
	ImageAccess::pointer candidateAccess = candidateCenterpointsImage->getImageAccess(ACCESS_READ);
	short* distanceArray = (short*)distanceAccess->get();
	uchar* inputArray = (uchar*)inputAccess->get();
	uchar* candidateArray = (uchar*)candidateAccess->get();
	int maxDistance = 0;
	int maxIndex = 0;
	UniquePointer<bool[]> isInL(new bool[totalSize]);
	std::unordered_set<int> Sc;
	for(int i = 0; i < totalSize; ++i) {
		if(inputArray[i] == 1) {
			// Inside object
			isInL[i] = false;

			// Get max distance
            if(candidateArray[i] == 1) {
                short distance = distanceArray[i];
                if(distance > maxDistance) {
                    maxDistance = distance;
                    maxIndex = i;
                }
                Sc.insert(i);
            }
		} else {
			isInL[i] = true;
		}
	}
	Vector3i maxPosition = position3D(maxIndex, size);
	reportInfo() << "Max position found at " << maxPosition.transpose() << " with value " << maxDistance << reportEnd();

	// Calculate speed term
	double beta = 1.0 / (0.02*maxDistance);
	UniquePointer<double[]> speed(new double[totalSize]);
	UniquePointer<double[]> G(new double[totalSize]);
	for(int i = 0; i < totalSize; ++i) {
		speed[i] = exp(beta*distanceArray[i]);
		G[i] = std::numeric_limits<double>::infinity();
	}

	G[linearPosition(maxPosition, size)] = 0;
	Vector3i neigbhors[6] = {
            {1, 0, 0},
            {-1, 0, 0},
            {0, 1, 0},
            {0, -1, 0},
            {0, 0, 1},
            {0, 0, -1}
	};
	std::vector<Vector3i> L;
	for(int i = 0; i < 6; ++i) {
		Vector3i nPos = maxPosition + neigbhors[i];
		L.push_back(nPos);
		isInL[linearPosition(nPos, size)] = true;
	}

	// Do fast marching
	const double threshold = 0;
	while(L.size() > 0) {
		std::vector<Vector3i> L2;
		for(Vector3i x : L) {
			double p = G[linearPosition(x, size)];
			double q = solveQuadratic(G, speed, x, size);
            G[linearPosition(x, size)] = q;
            if(fabs(p-q) <= threshold) {
                for(int i = 0; i < 6; ++i) {
                    Vector3i xn = x + neigbhors[i];
                    if(!isInL[linearPosition(xn, size)]) {
                        double p = G[linearPosition(xn, size)];
                        double q = solveQuadratic(G, speed, xn, size);
                        if(p > q) {
                            G[linearPosition(x, size)] = q;
                            L2.push_back(xn);
                            isInL[linearPosition(xn, size)] = true;
                        }
                    }
                }// For all neighbors
            } else {
                L2.push_back(x);
            } // fabs(p-q)
		} // for all x in L
		L = L2;
	} // while L.size() > 0
	reportInfo() << "Finished fast marching" << reportEnd();

	std::vector<Vector3i> neighbors2;
    for(int a = -1; a <= 1;  ++a) {
    for(int b = -1; b <= 1;  ++b) {
    for(int c = -1; c <= 1;  ++c) {
    	if(a == 0 && b == 0 && c == 0)
    		continue;
    	neighbors2.push_back(Vector3i(a,b,c));
    }}}

	std::vector<MeshVertex> vertices;
	std::vector<MeshLine> lines;
    std::unordered_set<int> refinedCenterline;
    std::unordered_set<int> processed;
	// Do backtrace
	while(Sc.size() > 0) {
		// Find candidate centerline point with highest G
		double maxG = 0.0;
		int maxLinearPosition = -1;
		for(int linearPosition : Sc) {
			if(G[linearPosition] > maxG && !std::isinf(G[linearPosition])) {
				maxG = G[linearPosition];
				maxLinearPosition = linearPosition;
			}
		}

		// Convert maxPosition to 3D
		Vector3i maxPosition = position3D(maxLinearPosition, size);
		reportInfo() << "Max position found at " << maxPosition.transpose() << " with G: " << maxG << reportEnd();

		if(maxLinearPosition == -1)
			break;

		std::vector<Vector3i> pointsToAdd;
		Vector3i current = maxPosition;
		Vector3i previous = Vector3i::Zero();
		Vector3i previous2 = Vector3i::Zero();
		while(true) {
			std::unordered_set<int>::iterator it = Sc.find(linearPosition(current, size));
			if(it != Sc.end()) {
                Sc.erase(it);
			}

			// Find neighbor point with min G
			double minG = std::numeric_limits<double>::infinity();
			Vector3i bestPos;
			Vector3i bestDPos;
			double maxD = -1;
			for(Vector3i n : neighbors2) {
				Vector3i xn = current + n;
				if(refinedCenterline.count(linearPosition(xn, size)) > 0) {
                    if(distanceArray[linearPosition(xn, size)] > maxD) {
                    	maxD = distanceArray[linearPosition(xn, size)];
                    	bestDPos = xn;
                    }
				}
				if(G[linearPosition(xn, size)] < minG) {
					minG = G[linearPosition(xn, size)];
					bestPos = xn;
				}
			}

			if(maxD > -1) {
                current = bestDPos;
			} else {
                current = bestPos;
			}
			pointsToAdd.push_back(current);

			// A failsafe (why is this needed?)
			if(previous2 == current) {
				break;
			}
			previous2 = previous;
			previous = current;

			// Stop conditions
			if(minG == 0) {
				break;
			}
			if(refinedCenterline.count(linearPosition(current,size)) > 0) {
				// Bifurcation
				break;
			}
		}

		if(pointsToAdd.size() > 10) { // minimum length
			growFromPointsAdded(pointsToAdd, G, Sc, processed, size);
			int counter = vertices.size();
            vertices.push_back(MeshVertex(pointsToAdd[0].cast<float>().cwiseProduct(spacing)));
            refinedCenterline.insert(linearPosition(pointsToAdd[0], size));
			for(int i = 1; i < pointsToAdd.size(); ++i) {
                refinedCenterline.insert(linearPosition(pointsToAdd[i], size));
				vertices.push_back(MeshVertex(pointsToAdd[i].cast<float>().cwiseProduct(spacing)));
				lines.push_back(MeshLine(counter, counter+1));
				counter += 1;
			}
		}
	}

	Mesh::pointer output = getStaticOutputData<Mesh>();
    output->create(vertices, lines);
	SceneGraph::setParentNode(output, input);
}



}
