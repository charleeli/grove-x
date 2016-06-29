#include <iostream>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

using namespace std;

/*
    ./test_p26 1000
*/

string to_p26(unsigned long int n){
	string code="";
	vector<int> remainders;
	remainders.reserve(10);

	while (n!=0)
	{
		remainders.push_back(n%26);
		n=n/26;
	}

	//转换为26进制小写字符串,从高位到地位
	int size = remainders.size();
	for(int j=size-1;j>=0;j--)
	{
		code += (char)(remainders[j]+97) ;
	}

	return code;
}

int main(int argc, char *argv[])
{
    long int n = atoi( argv[1] );
    if(n <= 0){
    	cout<<"n can't be lt 0"<<endl;
    	return 0;
    }

    cout<<"26^4 = "<<to_string(pow(26,4))<<" p26: "<<to_p26(pow(26,4))<<endl;
    cout<<"26^5 = "<<to_string(pow(26,5))<<" p26: "<<to_p26(pow(26,5))<<endl;
    cout<<"26^7 = "<<to_string(pow(26,7))<<" p26: "<<to_p26(pow(26,7))<<endl;

    cout<<"300000 p26: "<<to_p26(300000)<<endl;
    cout<<"1000000000 p26: "<<to_p26(1000000000)<<endl;

    cout<<"-----------------------------"<<endl;
    cout<<"26^8 = "<<to_string(pow(26,8))<<" p26: "<<to_p26(pow(26,8))<<endl;
    cout<<"100000000000 p26: "<<to_p26(100000000000)<<endl;

    cout<<"-----------------------------"<<endl;
    cout<<to_p26(n)<<endl;

    return 0;
}
