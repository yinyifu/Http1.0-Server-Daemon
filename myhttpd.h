 #ifndef __MYHTTPD_H__
#define __MYHTTPD_H__

#include <queue>
#include <thread>
#include <sys/socket.h>

#include <unistd.h>
#include <unordered_map>

#include <netinet/in.h>
#include <arpa/inet.h>

#define	BUF_LEN	8192
#define CWDMAX 500
#define SERVER "myhttpd -v1.0"


enum schedule {FCFS, SJF};
enum file_type {OTHER, DIREC};

struct client_req{
	std::string request;
	char * ip;
	std::string req;
	int file_stat;
	int err;
	time_t date;
	time_t last_modified;
	file_type content_type;
	off_t content_length;
	std::string file_name;
	std::string protocol;
	int socket;
};



std::string usage_info[] = {
	"myhttpd - A daemonized command line simple server interface.",
	"\e[1mmyhttpd\e[22m [−d] [−h] [−l file] [−p port] [−r dir] [−t time] [−n threadnum] [−s sched]",    // Snynosis

 	"\e[1mmyhttpd\e[22m is a simple web server. It binds to a given port on the given address and waits for\
 incoming HTTP/1.0 requests. It serves content from the given directory. That is, any requests\
 for documents is resolved relative to this directory (the document root – by default,\
 the directory where the server is running).", //Description

	"-d", "Enter debugging mode. That is, do not daemonize, only accept one connection at a time and enable logging to stdout. Without this option, the web server should run as a daemon process in the background.",
	"-h", "Print a usage summary with all options and exit.",
	"−l file", "Log all requests to the given file. See LOGGING for details.",
	"−p port", "Listen on the given port. If not provided, myhttpd will listen on port 8080.",
	"-r dir", "Set the root directory for the http server to dir.",
	"−t time", "Set the queuing time to time seconds. The default should be 60 seconds.",
	"−n threadnum", "Set number of threads waiting ready in the execution thread pool to threadnum. The default should be 4 execution threads.",
	"−s sched", "Set the scheduling policy. It can be either FCFS or SJF. The default will be FCFS."
};

extern char * optarg;
char buf[BUF_LEN];
int retval;
std::vector<std::thread> worker_ts;
socklen_t sockaddrlen = sizeof(struct sockaddr_in);
typedef std::priority_queue<client_req, std::vector<client_req>, std::function<bool(struct client_req, struct client_req)> > clientpq;
fd_set ready;
Semaphore sem(0);			// a semaphore

std::unordered_map<int, bool> wcyifuyin21;	// -clients
std::mutex wclock;			// clients lock 
clientpq *qq944626604;		// -quests
std::mutex qqlock;			// qlock
bool loggin = false;		// -l
char fs[100];			// -l
static bool DEBUG = false;	// -d

int pipeline[2]; 			//self pipe trick
int done = 0;

// Worker thread for handling requests.
void execute_request(void);

// Helper function for setting up the server.
void setup_server(void);

//Timing thread wait for int time and start
void time_to_start(int);

//queueing thread, socker listener
void get_que(void);


bool FCFSCOM(const client_req& lhs, const client_req& rhs){
	return false;
}

bool SJFCOM(const client_req& lhs, const client_req& rhs){
	return lhs.content_length > rhs.content_length;	
}

void print_queue(clientpq * &cprcr){
	std::queue<client_req> newqueue;
	while(!cprcr->empty()) {
        std::cout << cprcr->top().req << " ";
        newqueue.push(cprcr->top());
        cprcr->pop();
    }
    std::cout << '\n';
    while(!newqueue.empty()) {
        cprcr->push(newqueue.front());
        newqueue.pop();
    }
}
void logga(struct client_req cr){

}
void logger_raw(char* ipaddr, time_t recv_t, time_t exe_t, const char *request, int status, size_t contlen){
	char printmsg[BUF_LEN];
	char recv[30];
	char exe[30];
	struct tm *dt = localtime(&recv_t);
	strftime(recv, sizeof(recv), "%d/%b/%Y:%H/%M/%S", dt);
	dt = localtime(&exe_t);
	strftime(exe, sizeof(exe), "%d/%b/%Y:%H/%M/%S", dt);
	sprintf(printmsg, "%s - [%s -0600] [%s -0600] %s %d %zu", ipaddr, recv, exe, request, status, contlen);
	if(DEBUG){
		std::cout << printmsg << std::endl;
	}else
	if(loggin){
		std::ofstream of(fs);
		of << printmsg << std::endl;
	}
}
void logger(struct client_req cr, time_t execute_time, int status){
	struct sockaddr_in addr;
	socklen_t size = sizeof(addr);
	getpeername(cr.socket, (struct sockaddr*)&addr, &size);
	char ipstr[20];
	int port;
	union {
		uint32_t addr;
		char bytes[4];
	} fromaddr;
	fromaddr.addr = ntohl(addr.sin_addr.s_addr);
				sprintf(ipstr, "%d.%d.%d.%d: ", 0xff & (unsigned int)fromaddr.bytes[3],
			    	0xff & (unsigned int)fromaddr.bytes[2],
			    	0xff & (unsigned int)fromaddr.bytes[1],
			    	0xff & (unsigned int)fromaddr.bytes[0]);
	logger_raw(ipstr, cr.date, execute_time, cr.request.c_str(), status, cr.content_length);
}


void disconnect(int socket){
	wcyifuyin21.find(socket)->second = false;
	write(pipeline[1], "hey\n", 16);
}
void sendafile(int, const char *);
bool is_binary(const void *data, size_t len)
{
    return memchr(data, '\0', len) != NULL;
}

#endif
