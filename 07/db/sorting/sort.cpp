#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <queue>

using namespace std;

typedef pair <uint64_t , int> ii;
// Priority queue used for sorting the elements.
priority_queue <ii, vector<ii>, greater<ii> > Q;
// Output array that will copy its elements into the output file.
uint64_t *output;
 // Data containing chunks of each temporary file that will be transfered to the output.
uint64_t **fileData;
// Counter containing for each temporary file how many elements have been already added to the output.
int *counter;

/* Writes chunkSize of data into the specified file. */
int writeToTempFile(uint64_t *data, int chunkSize, char *file) {
    int fd;
    if ((fd = open(file, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)) < 0) {
		cerr << "cannot open file '" << file << "': " << strerror(errno) << endl;
		return -1;
	}
    
    write(fd, data, sizeof(uint64_t)*chunkSize);
    return fd;
}

/* Returns in minVal the next minimal value from the nr-th temporary File. */
int getNext(uint64_t *minVal, int tempFile, int nr, int readLength, int memSize) {
    struct stat fileStat;
    fstat(tempFile, &fileStat);
    
    // We read all elements from this temporary file
    if(counter[nr] >= (fileStat.st_size/sizeof(uint64_t))) {
        close(tempFile);
        return 0;
    }
    
    // A new chunk of data needs to be read.
    if((counter[nr] % readLength) == 0) {
        read(tempFile, fileData[nr], readLength * sizeof(uint64_t));
    }
    
    // The returned next minimal value.
    *minVal = fileData[nr][(counter[nr]++)%readLength]; 
    return 1;
}

/* Returns in minVal the minimal element from all temporary Files.*/
int getMin(uint64_t *minVal, int nrFiles, int *tempFile, int readLength, int memSize) {
    // If the queue is empty we finished.
    if (Q.empty()) return 0;
    ii top = Q.top();
    Q.pop();
    
    //First element of the queue contains the smallest element.
    *minVal = top.first;
    
    uint64_t m;
    // Pushes the next read element to the priority queue.
    if (getNext(&m, tempFile[top.second], top.second, readLength, memSize))
        Q.push(make_pair(m, top.second));
    
    return 1;
}

void externalSort(int fdInput, unsigned long size, int fdOutput, unsigned long memSize) {
    int number = size/sizeof(uint64_t);
    int i = 0;
    
    // Size of elements that fit into RAM.
    int chunkSize = memSize/sizeof(uint64_t);
    // Data that are read.
    uint64_t data[chunkSize];
    // Number of files that are necessary for sorting the elements.
    int nrFiles = (int)ceil((float)size/memSize);
    // Array containing the temporary files descriptors.
    int tempFile[nrFiles];
    
    while(i < number) {
        int length = i + chunkSize > number ? length = number - i : length = chunkSize;
        int curi = i;
        // Read another chunk of data.
        for(; i < curi+length; ++ i) {
            read(fdInput, &data[i-curi], sizeof(uint64_t));
        }
        // Sort the elements.
        sort(data, data+length);
        
        // Write into temporary file and prepare file descriptor for the merging session.
        char fileName[16];
        sprintf(fileName, "temp/%d", curi/chunkSize);
        tempFile[curi/chunkSize] = writeToTempFile(data, length, fileName);
        close(tempFile[curi/chunkSize]);
        tempFile[curi/chunkSize] = open(fileName, O_RDONLY, S_IRUSR|S_IWUSR);
    }
    
    // Length of the reading chunks.
    int readLength = memSize/((nrFiles+3)*sizeof(uint64_t));
    cout<<readLength<<" "<<nrFiles<< endl;
    
    // Output array that will copy its elements into the output file.
    output = (uint64_t *)malloc(readLength * sizeof(uint64_t));
    // Counter containing for each temporary file how many elements have been already added to the output.
    counter = (int *)malloc(readLength * sizeof(int));
    memset(counter, 0, sizeof(counter));
    // Data containing chunks of each temporary file that will be transfered to the output.
    fileData = (uint64_t**)malloc(nrFiles * sizeof(uint64_t*));
    for(int i = 0; i < nrFiles; ++ i)
        fileData[i] = (uint64_t *)malloc(readLength * sizeof(uint64_t));

    // Initialize queue.
    uint64_t m;
    for (int i = 0; i < nrFiles; ++ i) {
        getNext(&m, tempFile[i], i, readLength, memSize);
        ii element = make_pair(m, i);
        Q.push(element);
    }
   
    int position = 0;
    while(getMin(&m, nrFiles, tempFile, readLength, memSize)) {
        output[position++] = m;
        if(position >= readLength) {
            position = 0;
            // Write to output.
            write(fdOutput, output, sizeof(uint64_t)*readLength);
        }
    }    
    
    //Write the remaining elements to output.
    if(position > 0)
        write(fdOutput, output, sizeof(uint64_t)*position);
    close(fdOutput);
}

int testing(char *file) {
    int fdOutput;
    //Open file.
    if ((fdOutput = open(file, O_RDONLY, S_IRUSR|S_IWUSR)) < 0) {
		cerr << "cannot open file '" << file << "': " << strerror(errno) << endl;
		return -1;
	}
	
	//Test that the elements are correctly sorted.
	uint64_t prev = 0, cur;
	while(read(fdOutput, &cur, sizeof(uint64_t))!=0) {
        if(cur < prev) return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    //Parse arguments.
    if(argc < 4) {
        cerr << "Not enough arguments." << endl;
        return -1;
    }
    unsigned memSize = atoi(argv[3]);
    //Parse arguments.
	if (memSize==0) {
		cerr << "Invalid length: " << argv[3] << endl;
		return -1;
	}
	
	//Parse arguments.
	int fdInput, fdOutput;
	if ((fdInput = open(argv[1], O_RDONLY, S_IRUSR|S_IWUSR)) < 0) {
		cerr << "cannot open file '" << argv[1] << "': " << strerror(errno) << endl;
		return -1;
	}
	if ((fdOutput = open(argv[2], O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) < 0) {
		cerr << "cannot open file '" << argv[2] << "': " << strerror(errno) << endl;
		return -1;
	}
    
    struct stat fileStat;
    fstat(fdInput, &fileStat);
    
    //Exercise 1.
    externalSort(fdInput, fileStat.st_size, fdOutput, memSize);
    
    //Exercise 2.
    if(testing(argv[2]) >= 0) cout<<"Sorting was correct :)..."<<endl;
    else cout<<"Sorting was incorrect :(..."<<endl;
    
    return 0;
}
