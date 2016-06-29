#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "Slug8.h"

using namespace std;

/*
    ./test_slug8
*/

int main(int argc, char *argv[])
{
    string slug = Slug8::Slug(1000000000);

    cout<<"slug : "<<slug<<endl;

    unsigned long int id = Slug8::DeSlug("mlsnwsce");
    cout<<"id : "<<id<<endl;
    return 0;
}
