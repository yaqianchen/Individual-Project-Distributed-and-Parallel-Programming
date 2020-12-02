#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void randomArray(int* array, int size){
    srand(1);
    for (int i = 0; i<size; i++){
        array[i] = rand() %1000;
        // printf("the integer in function %d\n",array[i]);
    }
}

void printSeq(int* array, int size){
    for (int i = 0; i < size; i++){
        printf("%d\t",array[i]);
    }
    printf("\n");
}

void printSeqId(int* array, int size, int myid, int mode){
    printf("in the rank %d:\t",myid);
    if (mode == 0){
        printf("in the mode odd sort\t");
    }
    else if (mode == 1){
        printf("in the mode even sort\t");
    }
    else {
        printf("in the mode boundary sort\t");
    }
    for (int i = 0; i < size; i++){
        printf("%d\t",array[i]);
    }
    printf("\n");
}

void swap(int* array, int start, int end){
    int tmp = array[start];
    array[start] = array[end];
    array[end] = tmp;
}

int oddSort(int* array, int size){
    // printf("size is %d in odd sort,=====================\n",size);
    // printf(" in odd sort\n");
    for (int i = 0; i < size - 1; i += 2){
        if(array[i]> array[i+1]){
            swap(array, i, i+1);
        }
    }
}

int evenSort(int* array, int size){
    // printf("size is %d in even sort,=====================\n",size);
    // printf("in even sort\n");
    for (int i = 1; i < size - 1; i += 2){
        if(array[i]> array[i+1]){
            swap(array, i, i+1);
        }
    }
}

int boundSort(int* array, int size, int myid, int numProcs, MPI_Comm comm){
    // printf("in boundary\n");
    int leftSeqArray = (myid - 1 + numProcs) % numProcs;
    int rightSeqArray = (myid + 1) % numProcs;
    int recvLeft, recvRight;
    //left bound
    if (myid != 0){
        // printf("myid is %d, size is %d, left %d, in left bound,=====================\n",myid, size, leftSeqArray);
        MPI_Send(&array[0], 1, MPI_INT, leftSeqArray, 0, comm);
        MPI_Recv(&recvLeft, 1, MPI_INT, leftSeqArray, 0, comm, MPI_STATUS_IGNORE);
        if (recvLeft > array[0]){
            array[0] = recvLeft;
        }
    }
    //right bound
    if (myid != (numProcs -1)){
        // printf("mtid is %d, size is %d, right %d, in right bound,=====================\n",myid,size, rightSeqArray); 
        MPI_Recv(&recvRight, 1, MPI_INT, rightSeqArray, 0, comm, MPI_STATUS_IGNORE);
        MPI_Send(&array[size - 1], 1, MPI_INT, rightSeqArray, 0, comm);
        if (recvRight < array[size -1]){
            array[size - 1] = recvRight;
        }
    }
}

void oddEvenSort(int* array, int size, int myid, int numProcs, int totalNumber, MPI_Comm comm){
    for (int i = 0; i < totalNumber/2; i++){
        // printf("myid is %d, size is %d in odd even sort,=====================\n",myid,size);
        oddSort(array, size);
        if ((sizeof(array)/sizeof(int))%2 != 0){
            boundSort(array, size, myid, numProcs, comm);
        }
        evenSort(array, size);
        boundSort(array, size, myid, numProcs, comm);
    }
}

int main(int argc, char* argv[]){
    // preparation
    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
    int myid, numProcs;
    int wholeNum = atoi(argv[1]);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);    
    int* wholeArray = malloc(sizeof(int) * wholeNum);
    int sepNum = wholeNum/numProcs;

    //array generation in rank0
    if (myid == 0){
        randomArray(wholeArray, wholeNum);
        printf("\nwith %d processors and %d length array\n\n",numProcs, wholeNum);
        printf("Name: Yaqian Chen,\n Student ID: %d,\n Homework 1, Odd Even Sort, MPI Implementation with balanced separation method\n",117010032);
        // printf("the sequence before sort:\n");
        // printSeq(wholeArray, wholeNum);
    }
    int* wholeNumList = malloc(sizeof(int) * numProcs);
    int* disNumList = malloc(sizeof(int) * numProcs);
    for (int i = 0; i < numProcs; i++){
        if (i < wholeNum%numProcs){
            wholeNumList[i] = sepNum + 1;
            disNumList[i] = i * (sepNum + 1);
        }
        else{
            wholeNumList[i] = sepNum;
            disNumList[i] = disNumList[i - 1] + wholeNumList[i - 1];
        }
    }
    // printf("the whole NumList\n");
    // printSeq(wholeNumList,numProcs);
    // printf("the disNumList\n");
    // printSeq(disNumList,numProcs);
    
    int recvCount = (myid < wholeNum%numProcs) ? sepNum + 1 : sepNum;
    int* sepArray = malloc(sizeof(int) * recvCount);
    // printf("in rank %d, recvcount is %d\n",myid, recvCount);
    // printf("in rank %d, length of array is %ld\n",myid, sizeof(sepArray)/sizeof(int));
    MPI_Scatterv(wholeArray, wholeNumList, disNumList, MPI_INT, sepArray, recvCount, MPI_INT, 0, comm);
    // printf("before oddeven sort\n");
    // printSeqId(sepArray,recvCount,myid, 0);
    time_t startTime = clock();
    oddEvenSort(sepArray, recvCount, myid, numProcs, wholeNum, comm);
    time_t endTime = clock();
    MPI_Gatherv(sepArray, recvCount, MPI_INT, wholeArray, wholeNumList, disNumList, MPI_INT, 0, comm);
    if (myid == 0){
        // printf("the sequence after sort:\n");
        // printSeq(wholeArray, wholeNum);
        printf("runTime is %f\n",(double)(endTime - startTime) / CLOCKS_PER_SEC);
    }
    MPI_Finalize();
    return 0;
}