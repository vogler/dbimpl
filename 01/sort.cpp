#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <algorithm>
//#include <queue>
#include <math.h>
#include <stdio.h>

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
	int fi, fo, ret;
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
	unsigned runs = ceil(fileSize*1.0/bufferSize);
	std::cout << "input file size: " << fileSize << " Byte -> " << runs << " run(s)" << std::endl;
	if(fileSize%sizeof(uint64_t) != 0){
		std::cerr << "file size has to be a multiple of 64 Bit" << std::endl;
		return -1;
	}
	
	for(int run=0; run<runs; run++){
		char filename[strlen(argv[2])+1+(int)ceil(log10(runs+1))];
		sprintf(filename, "%s_%d", argv[2], run);
		unsigned n = bufferSize/sizeof(uint64_t);
		uint64_t buffer[n];
		std::cout << "run " << run << ": starting to read " << n << " elements..." << std::endl;
		unsigned runSize = read(fi, &buffer, bufferSize);
		std::cout << "run " << run << ": read " << runSize << " Byte" << std::endl;
		//for (int i = 0; i < n; ++i) 
		//	std::cout << buffer[i] << ' ';
		//std::cout << "sorting..." << std::endl;
		std::sort(buffer, buffer + n); // sorts bufferSize-count empty elements for the last run -> doesn't really matter
		//for (int i = 0; i < n; ++i) 
		//	std::cout << buffer[i] << ' ';
		if ((fo = open(filename, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) < 0) {
			std::cerr << "cannot open output file '" << filename << "': " << strerror(errno) << std::endl;
			return -1;
		}
		if ((ret = posix_fallocate(fo, 0, runSize)) != 0){
			std::cerr << "warning: could not allocate file space: " << strerror(ret) << std::endl;
			return -1;
		}
		if (write(fo, &buffer, runSize) < 0) {
			std::cout << "error writing to " << filename << ": " << strerror(errno) << std::endl;
			return -1;
		}
		std::cout << "run " << run << ": wrote " << runSize << " Byte to " << filename << std::endl;
		close(fo);
	}
	close(fi);
	return 0;
}
