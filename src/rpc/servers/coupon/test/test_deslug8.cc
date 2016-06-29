#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "Slug8.h"

using namespace std;

/*
    ./test_slug8 mlsnwsce
*/

int main(int argc, char *argv[])
{
    unsigned long int id = Slug8::DeSlug(string(argv[1]));
    cout<<"id : "<<id<<endl;
    return 0;
}
