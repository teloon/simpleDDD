#include <iostream>

using namespace std;

struct strt1{
	char *str;
} myStrt1;

struct strt2{
	strt1* s1;
} myStrt2;

int main()
{
	myStrt1.str = new char[20];
	strcpy(myStrt1.str, "hello");
	cout << myStrt1.str << endl;
	myStrt2.s1 = &myStrt1;
/*	cout << (myStrt2.s1) << endl;
	cout << (&myStrt1) << endl;
*/	cout << myStrt2.s1->str << endl;

	return 0;
}
