#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <algorithm>
#include <queue>
#include <math.h>
#include <stdio.h>

void printBuffer(uint64_t* buffer){
	for (int i = 0; i < sizeof(buffer)/sizeof(uint64_t); ++i) 
		std::cout << buffer[i] << ' ';
}

void externalSort(int fdInput, unsigned long size, int fdOutput, unsigned long memSize){
	// determine the number of required runs
	unsigned runs = ceil(size*1.0*sizeof(uint64_t)/memSize);
	std::cout << "input file size: " << size << " elements -> " << runs << " run(s)" << std::endl;
	unsigned n = memSize/sizeof(uint64_t);

	// sort each run and write it to files of size memSize
	char basename[] = "tmprun";
	int fo, ret;
	for(unsigned run=0; run<runs; run++){
		char filename[strlen(basename)+1+(int)ceil(log10(runs+1))];
		sprintf(filename, "%s_%d", basename, run);
		uint64_t buffer[n];
		std::cout << "run " << run << ": starting to read max. " << n << " elements..." << std::endl;
		unsigned runSize = read(fdInput, &buffer, memSize);
		std::cout << "run " << run << ": read " << runSize << " Byte" << std::endl;
		std::sort(buffer, buffer + n); // sorts bufferSize-count empty elements for the last run -> doesn't really matter
		if ((fo = open(filename, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) < 0) {
			std::cerr << "cannot open output file '" << filename << "': " << strerror(errno) << std::endl;
			return;
		}
		if ((ret = posix_fallocate(fo, 0, runSize)) != 0){
			std::cerr << "warning: could not allocate file space: " << strerror(ret) << std::endl;
			return;
		}
		if (write(fo, &buffer, runSize) < 0) {
			std::cout << "error writing to " << filename << ": " << strerror(errno) << std::endl;
			return;
		}
		std::cout << "run " << run << ": wrote " << runSize << " Byte to " << filename << std::endl;
		close(fo);
	}

	// open files of runs for reading
	int fdRuns[runs];
	for(unsigned run=0; run<runs; run++){
		char filename[strlen(basename)+1+(int)ceil(log10(runs+1))];
		sprintf(filename, "%s_%d", basename, run);
		if ((fdRuns[run] = open(filename, O_RDONLY)) < 0) {
			std::cerr << "cannot open input file '" << filename << "': " << strerror(errno) << std::endl;
			return;
		}
	}
		
	// read memSize/(runs+1) elements from each run at once, push it into a queue and write it to fdOutput
	unsigned long readElements = 0;
	while(readElements < size){
		std::priority_queue<uint32_t> q;
		for(unsigned run=0; run<runs; run++){
			// read into buffer
			unsigned m = n/(runs+1);
			uint64_t buffer[m];
			unsigned readSize = read(fdRuns[run], &buffer, sizeof(buffer));
			if(readSize < 0) continue;
			readElements += readSize/sizeof(uint64_t);
			// write to queue
			for(int i=0; i<m; i++) q.push(buffer[i]);
		}
		// write to fdOutput
		if (write(fdOutput, &q, sizeof(q)) < 0) {
			std::cout << "error writing to output file" << strerror(errno) << std::endl;
			return;
		}
		// clear queue

	}

	// close files of runs
	for(unsigned run=0; run<runs; run++){
		close(fdRuns[run]);
	}
}

int main(int argc, char* argv[]) {
	if (argc < 4) {
		std::cerr << "usage: " << argv[0] << " <inputFile> <outputFile> <memoryBufferInMB>" << std::endl;
		return -1;
	}
	unsigned bufferSize = atoi(argv[3])*1024;
	if (bufferSize < 1024) {
		std::cerr << "invalid memory buffer size (min. 1MB): " << argv[3] << std::endl;
		return -1;
	}
	int fi, fo;
	if ((fi = open(argv[1], O_RDONLY)) < 0) {
		std::cerr << "cannot open input file '" << argv[1] << "': " << strerror(errno) << std::endl;
		return -1;
	}
	struct stat st;
	if(fstat(fi, &st) < 0){
		std::cerr << "fstat for file '" << argv[1] << "' failed: " << strerror(errno) << std::endl;
		return -1;
	}
	long long fileSize = st.st_size;
	if(fileSize%sizeof(uint64_t) != 0){
		std::cerr << "file size has to be a multiple of 64 Bit" << std::endl;
		return -1;
	}
	if ((fo = open(argv[2], O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) < 0) {
		std::cerr << "cannot open output file '" << argv[2] << "': " << strerror(errno) << std::endl;
		return -1;
	}

	externalSort(fi, fileSize/sizeof(uint64_t), fo, bufferSize);

	close(fi);
	close(fo);
	return 0;
}
