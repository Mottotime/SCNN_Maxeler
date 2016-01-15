/**
 * Document: Manager Compiler Tutorial (maxcompiler-manager-tutorial.pdf)
 * Chapter: 2      Example: 3      Name: Command Stream
 * MaxFile name: CmdStream
 * Summary:
 *     Writes the values for two input streams to LMem, sets up the correct
 *     data length and burst size for the computation and reads and checks the
 *     results from LMem.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "Maxfiles.h"
#include <MaxSLiCInterface.h>

void generateInputData(int size, int32_t *inA, int32_t *inB)
{
	srand(time(0));
	for (int i = 0; i < size; i++) {
		inA[i] = rand();
		inB[i] = rand();
	}
}

void CmdStreamCPU(int size, int32_t *inA, int32_t *inB, int32_t *outData)
{
	for (int i = 0; i < size; i++) {
		outData[i] = inA[i] + inB[i];
	}
}

int check(int size, int32_t *outData, int32_t *expected)
{
	int status = 0;
	for (int i = 0; i < size; i++) {
		if (outData[i] != expected[i]) {
			fprintf(stderr, "[%d] Verification error, out: %u != expected: %u\n",
				i, outData[i], expected[i]);
			status = 1;
		}
	}
	return status;
}

int main()
{
	const int size = 1920;
	int sizeBytes = size * sizeof(int32_t);
	int32_t *inA = malloc(sizeBytes);
	int32_t *inB = malloc(sizeBytes);

	generateInputData(size, inA, inB);

	max_file_t *maxfile = CmdStream_init();

	int burstLengthInBytes = max_get_burst_size(maxfile, "cmd_tolmem");

	printf("Loading DFE memory.\n");
	CmdStream_writeLMem(size, 0, inA);
	CmdStream_writeLMem(size, size, inB);

	printf("Running DFE.\n");
	CmdStream(size, burstLengthInBytes);

	int32_t *expected = malloc(sizeBytes);
	CmdStreamCPU(size, inA, inB, expected);

	printf("Reading DFE memory.\n");
	int32_t *outData = malloc(sizeBytes);
	CmdStream_readLMem(size, 2 * size, outData);

	int status = check(size, outData, expected);
	if (status)
		printf("Test failed.\n");
	else
		printf("Test passed OK!\n");

	return status;
}
