#include<iostream>
#include<queue>
#include<string>
#include<cstdlib>

#include<boost/thread.hpp>
#include<boost/bind.hpp>
#include<boost/asio.hpp>
#include<boost/asio/ip/tcp.hpp>
#include<boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<string> string_ptr;
typedef boost::shared_ptr< queue<string_ptr> > messageQueue_ptr;

io_service service; // Boost Asio io_service
messageQueue_ptr messageQueue(new queue<string_ptr>); // Queue for producer consumer pattern
tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001); // TCP socket for connecting to server
const int inputSize = 256; // Maximum size for input buffer
string_ptr promptCpy; // Terminal prompt displayed to chat users

bool isOwnMessage(string_ptr);
void displayLoop(socket_ptr);
void inboundLoop(socket_ptr, string_ptr);
void writeLoop(socket_ptr, string_ptr);
string* buildPrompt();

int main(int argc, char** argv) 
{
	try
	{
		boost::thread_group threads;
		socket_ptr sock(new tcp::socket(service));

		string_ptr prompt( buildPrompt() );
		promptCpy = prompt;

		sock->connect(ep);

		cout << "Welcome to the ChatApplication\nType \"exit\" to quit" << endl;

		threads.create_thread(boost::bind(displayLoop, sock));
		threads.create_thread(boost::bind(inboundLoop, sock, prompt));
		threads.create_thread(boost::bind(writeLoop, sock, prompt));

		threads.join_all();
	}
	catch(std::exception& e)
	{
		cerr << e.what() << endl;
	}

	puts("Press any key to continue...");
	getc(stdin);
	return EXIT_SUCCESS;
}

string* buildPrompt()
{
	const int inputSize = 256;
	char inputBuf[inputSize] = {0};
	char nameBuf[inputSize] = {0};
	string* prompt = new string(": ");

	cout << "Please input a new username: ";
	cin.getline(nameBuf, inputSize);
	*prompt = (string)nameBuf + *prompt;
	boost::algorithm::to_lower(*prompt);

	return prompt;
}

void inboundLoop(socket_ptr sock, string_ptr prompt)
{
	int bytesRead = 0;
	char readBuf[1024] = {0};

	for(;;)
	{
		if(sock->available())
		{
			bytesRead = sock->read_some(buffer(readBuf, inputSize));
			string_ptr msg(new string(readBuf, bytesRead));

			messageQueue->push(msg);
		}

		boost::this_thread::sleep( boost::posix_time::millisec(1000));
	}
}

void writeLoop(socket_ptr sock, string_ptr prompt)
{
	char inputBuf[inputSize] = {0};
	string inputMsg;

	for(;;)
	{
		cin.getline(inputBuf, inputSize);
		inputMsg = *prompt + (string)inputBuf + '\n';

		if(!inputMsg.empty())
		{
			sock->write_some(buffer(inputMsg, inputSize));
		}

		// The string for quitting the application
		// On the server-side there is also a check for "quit" to terminate the TCP socket
		if(inputMsg.find("exit") != string::npos)
			exit(1); // Replace with cleanup code if you want but for this tutorial exit is enough

		inputMsg.clear();
		memset(inputBuf, 0, inputSize);
	}
}

void displayLoop(socket_ptr sock)
{
	for(;;)
	{
		if(!messageQueue->empty())
		{
			// Can you refactor this code to handle multiple users with the same prompt?
			if(!isOwnMessage(messageQueue->front()))
			{
				cout << "\n" + *(messageQueue->front());
			}

			messageQueue->pop();
		}

		boost::this_thread::sleep( boost::posix_time::millisec(1000));
	}
}

bool isOwnMessage(string_ptr message)
{
	if(message->find(*promptCpy) != string::npos)
		return true;
	else
		return false;
}
