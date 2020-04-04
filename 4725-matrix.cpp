#include <iostream>
#include <pthread.h>
#include <fstream>
#include <time.h>
using namespace std;

int** matrix1, **matrix2, **output; // 2d arrays to hold matrix 1, 2 and output of multiplication
int row1,col1,row2,col2; // holds row, col numbers of each matrix
ifstream matrixFile; // matrix input file
ofstream outputFile; // matrix output file
clock_t timer; // timer variable to count time in milli seconds

struct Element // to hold arguments for threads of each element output of output matrix
{
    int row;
    int col;
};

struct Row{ // to hold arguments for threads of each row output of output matrix
    int row;
};

void matrixThreading();
void getMatricies();
int** scanMatrix(int*,int*);
void initializeOutput(Element*,Row*);
void elementOutput(Element*);
void rowOutput(Row*);
void* rowMultiplyElement(void* arg);
void* rowMultiplyRow(void* arg);
void printOutput();

int main()
{   
    // openning inpu file and checking that it exists, creating output file
    matrixFile.open("input.txt");
    outputFile.open("output.txt");
    if(!matrixFile || !outputFile)
    {
        cout << "Cant read/write to or from file";
        return 0;
    }
    matrixThreading(); // main method that is used for operation of matrix multiplication
    matrixFile.close(); // closing the matrix file
    outputFile.close(); // closing the output file
    return 0;
}

void matrixThreading()
{
    getMatricies();
    Element rowcol[col2 * row1];
    Row row[row1];
    initializeOutput(rowcol,row);
    clock_t start = clock();
    elementOutput(rowcol);
    clock_t end = clock() - start;
    printOutput();
    outputFile << "END1	"<< end << "ms" <<endl;
    start = clock();
    rowOutput(row);
    end = clock() - start;
    printOutput();
    outputFile << "END2	"<< end << "ms" <<endl;
}

void getMatricies()
{
    matrix1 = scanMatrix(&row1,&col1);
    matrix2 = scanMatrix(&row2,&col2);
    if(col1 != row2)
    {
        cout << "Invalid matrix multiplication, terminating" << endl;
        exit(1);
    }
}

int** scanMatrix(int* row, int* col)
{
    // cin >> *row;
    // cin >> *col;
    matrixFile >> *row;
    matrixFile >> *col;
    int** matrix = new int*[*row];
    for(int i = 0 ; i < *row ; ++i)
    {
        matrix[i] = new int[*col];
        for(int j = 0 ; j < *col ; ++j)
        {
            matrixFile >> matrix[i][j];
        }
    }
    return matrix;
}
// initializing the output array and struct arrays that holds the needed arguments for threads
// to be mutualy excluded 
void initializeOutput(Element* rowcol,Row* row)
{
    output = new int*[row1]; // creating 2d array that is ragged at the moment
    for(int i = 0,threadCounter = 0 ; i < row1 ; ++i)
    {
        output[i] = new int[col2]; // making it a rectangular array by giving same no. of columns to each element
        row[i].row = i; // initializing thread arguments of row outputs
        // initializing thread arguments of element outputs
        for(int j = 0 ; j < col2 ; ++j, ++threadCounter)
        {
            rowcol[threadCounter].row = i;
            rowcol[threadCounter].col = j;
        }
    }
}
// functio  that is responsible for creating threads for each element output in matrix multiplication output
void elementOutput(Element* rowcol)
{
    int threadCount = col2 * row1; // number of threads needed
    pthread_t elementsThreads[threadCount];
    int threadPtr = 0;
    // creating threads, passing arguments to them of each elements location in output array
    for(int colNum = 0 ; colNum < col2 ; ++colNum)
    {
        for(int rowNum = 0; rowNum < row1 ; ++rowNum, ++threadPtr)
        {
            pthread_create(&elementsThreads[threadPtr],NULL,rowMultiplyElement,&rowcol[threadPtr]);
        }
    }
    // waiting for all threads to finish before proceeding
    for(threadPtr = 0 ; threadPtr < threadCount ; ++threadPtr)
    {
         pthread_join(elementsThreads[threadPtr], NULL);
    }
}
void* rowMultiplyElement(void* arg)
{
    Element* rowcol = (Element*)arg; // getting the row col numbers for each element to be computed by thread
    int row = rowcol->row;
    int col = rowcol->col;
    int sum = 0;
    // doing matrix multiplication element output
    for(int i = 0 ; i < col1 ; ++i)
    {
        sum += matrix1[row][i] * matrix2[i][col];
    }
    output[row][col] = sum;
}
// function that is responsible for creating threads for row output of matrix multiplication
void rowOutput(Row* row)
{
    int threadCount = row1;
    pthread_t rowThread[threadCount];
    int threadPtr = 0;
    // creating threads for each row
    for(int rowNum = 0 ; rowNum < row1 ; ++rowNum,++threadPtr)
    {
        pthread_create(&rowThread[threadPtr],NULL,rowMultiplyRow,&row[rowNum]);
    }
    // waiting for threads to finish 
    for(threadPtr = 0 ; threadPtr < threadCount ; ++threadPtr)
    {
         pthread_join(rowThread[threadPtr], NULL);
    }
    
    
}
// function that is responsible for operation of row output thread
void* rowMultiplyRow(void* arg)
{
    Row* rowNum = (Row*)arg;
    int row = rowNum->row;
    int sum = 0;
    for(int i = 0 ; i < col2 ; ++i)
    {
        for(int j = 0 ; j < col1 ; ++j)
        {
            sum += matrix1[row][j] * matrix2[j][i];
        }
        output[row][i] = sum;
        sum = 0; 
    }
}

// outputs the result in file
void printOutput()
{
    for(int i = 0 ; i < row1 ; ++i)
    {
        for(int j = 0 ; j < col2 ; ++j)
            outputFile << output[i][j] << " ";
        outputFile << endl;
    }
}
