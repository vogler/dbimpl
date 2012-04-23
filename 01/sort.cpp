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

int main(int argc, char* argv[]) {
	if (argc < 4) {
		std::cerr << "usage: " << argv[0] << " <inputFile> <outputFile> <memoryBufferInMB>" << std::endl;
		return -1;
	}
	unsigned maxBytes = atoi(argv[3])*1024;
	if (n<1024) {
		std::cerr << "invalid memory buffer size (min. 1MB): " << argv[3] << std::endl;
		return -1;
	}
	int fi, fo, ret;
	if ((fi = open(argv[1], O_RDONLY)) < 0) {
		std::cerr << "cannot open file '" << argv[1] << "': " << strerror(errno) << std::endl;
		return -1;
	}
	struct stat st;
	if(fstat(fi, &st) < 0){
		std::cerr << "fstat for file '" << argv[1] << "' failed: " << strerror(errno) << std::endl;
		return -1;
	}
	long long size = st.st_size;
	std::cout << "input file size: " << size << "Byte" << std::endl;
	if(size%8 != 0){
		std::cerr << "file size has to be a multiple of 64bit" << std:endl;
	}
	long buffer[maxBytes];
	int count;
	while(count = read(fi, buffer, maxBytes)){
		
	}
	if ((fo = open(argv[2], O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) < 0) {
		std::cerr << "cannot open file '" << argv[2] << "': " << strerror(errno) << std::endl;
		return -1;
	}
	if ((ret = posix_fallocate(fo, 0, n*sizeof(uint64_t))) != 0)
		std::cerr << "warning: could not allocate file space: " << strerror(ret) << std::endl;
	//for (unsigned i=0; i<n; ++i) {
	//	uint64_t x = rand.next();
	//	if (write(fd, &x, sizeof(uint64_t)) < 0) {
	//		std::cout << "error writing to " << argv[1] << ": " << strerror(errno) << std::endl;
	//	}
	//}
	close(fi);
	close(fo);
	return 0;
}
