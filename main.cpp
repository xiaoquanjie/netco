#include "netio/netio.hpp"
#include "coroutine/coroutine.hpp"
#include <iostream>
#include "base/thread.hpp"
#include "synccall/synccall.hpp"

using namespace std;

void synccall_server();
void synccall_client();
void synccall_test() {
	int i;
	cout << "select 1 is server or client:";
	cin >> i;
	if (i == 1)
		synccall_server();
	else
		synccall_client();
}

int main() {

	synccall_test();

	int pause_i;
	cin >> pause_i;
	return 0;
}