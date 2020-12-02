#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void randomArray(int* array, int size){
	srand(1);
    for (int i = 0; i<size; i++){
	    //srand(1);
        array[i] = rand() %1000;
        // printf("the integer in function %d\n",array[i]);
    }
}

void swap(int* array, int start, int end){
    int tmp = array[start];
    array[start] = array[end];
    array[end] = tmp;
}

int oddSort(int* array, int size){
    int flag = 0;
    for (int i = 0; i < size - 1; i += 2){
        if(array[i]> array[i+1]){
            swap(array, i, i+1);
            flag = 1;
        }
    }
    return flag;
}

int evenSort(int* array, int size){
    int flag = 0;
    for (int i = 1; i < size - 1; i += 2){
        if(array[i]> array[i+1]){
            swap(array, i, i+1);
            flag = 1;
        }
    }
    return flag;
}

void oddEvenSort(int* array, int size){
    int flag = 0;
    while (!flag){
        int oddFlag = oddSort(array, size);
        int evenFlag = evenSort(array, size);
        if (!oddFlag && !evenFlag) flag = 1;
    }
}

void printSeq(int* array, int size){
    for (int i = 0; i < size; i++){
        printf("%d\t",array[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]){
    time_t startTime;
    time_t endTime;
    int size = atoi(argv[1]);
    int* array = malloc(sizeof(int) * size);
    randomArray(array, size);  
    printf("\n, with %d processors and %d length arrat\n\n",1, size);
     printf(" Name: Yaqian Chen,\n Student ID: %d,\n Homework 1, Odd Even Sort, Sequential Implementation\n",117010032);
    // printf("the sequence before sort:\n");
    // printSeq(array,size);
    startTime = clock();
   // endTime = clock();
    oddEvenSort(array,size);
    endTime = clock();
    // printf("the sequence after sort:\n");
    // printSeq(array,size);
    printf(" runTime is %f\n",(double)(endTime - startTime) / CLOCKS_PER_SEC);
    return 0;
}

