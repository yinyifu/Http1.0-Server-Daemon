#define	BUF_LEN	8192

#include <fstream>
#include <unordered_set>
#include <sys/stat.h>
#include <sstream>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>

#include <iomanip>  // for put_time
#include "debug.h"
#include "semaphore.h"
#include "myhttpd.h"
#define ACCEPTMESSAGE "You are now logged in to the server, you may not send for than one request during waiting time or else you will break the server."


std::string port ("8080");	// -p
schedule sched = FCFS; 		// -s
char basecwd[CWDMAX];		// -r
int numthread = 4;			// -n
int wsec = 5;				// -t
//Starts when connection establish or start when server start?
std::condition_variable que_time;	// -t
std::mutex que_lock;				// -t
int main(int argc, char* argv[]){
	//Daemontize this process if d.option is not choosed;

	int retval;

	char *progname;
	if((progname = rindex(argv[0], '/')) == NULL)
	{	progname = argv[0];	}
	else
	{	progname += 1;	}

	char optc;
	int fpid;
	
	while ((optc = getopt(argc, argv, "dhl:p:r:t:n:s:")) != -1)
	{
		switch(optc) {
			case 'd':
				DEBUG = true;
				break;
			case 'l':
				loggin = true;
				strcpy(fs, optarg);
				break;
			case 'p':
				if(isdigit(*optarg) &&  strlen(optarg)<=5){
					port = optarg;	
				}else{
					usage(progname, usage_info, LENGTH(usage_info));
				}
				break;
			case 'r':
				chdir(optarg);
				strcpy(basecwd, optarg);
				break;
			case 't':
				ERRORIF(optarg==NULL || !isdigit(*optarg), "-t option wrong argument");
				wsec = atoi(optarg);
				break;
			case 'n':
				ERRORIF(optarg==NULL || !isdigit(*optarg), "-n option wrong argument");
				numthread = atoi(optarg);
				break;
			case 's':
				ERRORIF(optarg==NULL, "-h option wrong argument");
				if(strcmp(optarg, "FCFS")==0){
					sched = FCFS;
				}else if(strcmp(optarg, "SJF")==0){
					sched = SJF;
				}else{
					ERRORMSG("Schedule Option");
				}
				break;
			case 'h':
				usage(progname, usage_info, LENGTH(usage_info));
				break;
			case '?':
			default:
				std::cout<< "Use -h for help" << std::endl;
				exit(-1);
				break;
		}
	}

	if(!DEBUG){
		fpid = fork();
		if(fpid>0){
			exit(0);
		}
	}

	setup_server();

}

void execute_request(){
	std::unique_lock<std::mutex> queulk(que_lock);
	que_time.wait(queulk);
	while(!done){
		int status = 200;
		sem.P();
		time_t service_time;
		time(&service_time);
		qqlock.lock();

		//insurance 
		if(qq944626604->empty()){
			qqlock.unlock();
			continue;
		}

		struct client_req creq = qq944626604->top();
		qq944626604->pop();
		qqlock.unlock();
		if(creq.req.empty() || creq.protocol.empty() || creq.file_name.empty()){
			std::string errooo = "Bad Request... Use GET/HEAD [Filename] HTTP/1.0.\n"; 
			send(creq.socket, errooo.c_str(),strlen(errooo.c_str()), 0 );
			disconnect(creq.socket);
			status = 400;
			logger(creq, service_time, status);
			continue;
		}
		if(creq.protocol !="HTTP/1.0" ){
			std::string errooo = "Protocol unimplemented...Use HTTP/1.0\n"; 
			send(creq.socket, errooo.c_str(),strlen(errooo.c_str()), 0 );
			disconnect(creq.socket);
			status = 400;
			logger(creq, service_time, status);
			continue;
		}
		if(creq.file_stat < 0 ){
			char *errooo = strerror(creq.err); 
			send(creq.socket, errooo,strlen(errooo), 0 );
			send(creq.socket, "\n",1, 0 );
			disconnect(creq.socket);
			status = 404;
			logger(creq, service_time, status);
			continue;
		}
		
		if(creq.content_type==DIREC){
			const char* content;
			if(access(content = ((creq.file_name+((creq.file_name[creq.file_name.size()-1]=='/')?"index.html":"/index.html")).c_str()), F_OK ) == -1){
				DIR * directory = opendir(creq.file_name.c_str());
				if(!directory){
					closedir(directory);
					ERRORMSG("File close failed");
				}
				struct dirent *ep;
				while((ep = readdir(directory))){
					if(ep->d_name[0]!='.'){
						send(creq.socket, ep->d_name, strlen(ep->d_name), 0 );
						send(creq.socket, "\n", 1, 0 );
					}
				}
			}else{
				struct stat status2;
				stat(content, &status2);
				creq.last_modified = status2.st_mtime;
				creq.content_length = status2.st_size;
				sendafile(creq.socket, content);
			}
		}else
		if(creq.req =="HEAD" || creq.req == "GET"){
			
			
			if(creq.req == "GET"){
				sendafile(creq.socket, creq.file_name.c_str());
			}
		}else{
			char buf[50];
			sprintf(buf, "%s unimplemented...Instead use GET or HEAD.\n", creq.req.c_str()); 
			send(creq.socket, buf,strlen(buf), 0 );
		}
		logger(creq, service_time, status);
		disconnect(creq.socket);
	}
};
void sendahead(struct client_req &creq, time_t service_time){
	send(creq.socket, "Date\t\t:", 7, 0);
			
	char timestr[30];
	struct tm * dt = std::localtime(&service_time);
	strftime(timestr, sizeof(timestr), "%d/%b/%Y:%H/%M/%S %z", dt);
	send(creq.socket, timestr, strlen(timestr), 0);
	send(creq.socket, "\n", 1, 0);
	send(creq.socket, "Server\t\t:myhttpd -v1.0", 22, 0);
	send(creq.socket, "\n", 1, 0);
	
	send(creq.socket, "Last-Modified\t\t:", 15, 0);
	
	dt = std::localtime(&creq.last_modified);
	strftime(timestr, sizeof(timestr), "d/%b/%Y:%H/%M/%S %z", dt);
	send(creq.socket, timestr, strlen(timestr), 0);
	send(creq.socket, "\n", 1, 0);

	send(creq.socket, "Content-Type\t\t:", 14, 0);
	std::string str = "image/gif";
	send(creq.socket, str.c_str(), str.length(), 0);
	send(creq.socket, "\n", 1, 0);

	send(creq.socket, "Content-Length\t\t:", 16, 0);
	char nice_format[30];
	sprintf(nice_format, "%lld", creq.content_length);
	send(creq.socket, nice_format, strlen(nice_format), 0);
	send(creq.socket, " bytes\n", 7, 0);
}
void sendafile(int socket, const char * filename){
	//std::cout << "sending file";	 
	std::ifstream file(filename);
	if(!file){
		ERRORMSG("File open fault");
	}
	file.seekg(0, file.end);
	int length = file.tellg();
	file.seekg(0, file.beg);

	char buffer[length];
	file.read(buffer, length);
	send(socket, buffer, length, 0);
}
void setup_server(){
	for(int i = 0; i < numthread; i++){
		worker_ts.emplace_back(execute_request);
	}

	std::thread timer (time_to_start, wsec);
	
	struct sockaddr_in serv, remote;
	
	
	int s = socket(AF_INET, SOCK_STREAM, 0);
	ERRORIF(s < 0, "LocalHostSocket Creation Failed");
 
	memset(&serv, 0, sockaddrlen);
	
	serv.sin_family = AF_INET;
 
	if(port.empty())
	{
		serv.sin_port = htons(0);
	}
	else if (isdigit(port.at(0)))
	{
		serv.sin_port = htons(stoi(port));
	}
 
	retval = bind(s, (struct sockaddr *)&serv, sockaddrlen); 
	ERRORIF(retval < 0, "bind");
 
	retval = getsockname(s, (struct sockaddr *)&remote, &sockaddrlen);
	ERRORIF(retval < 0, "getsockname");
		/* DEBUG */
	ERRORIF(remote.sin_port == 0, "getname");
	ASSERT(sizeof(serv) == sockaddrlen);
	ASSERT(serv.sin_port == remote.sin_port);
	
	listen(s, 1);
	pipe(pipeline);
	int sock = accept(s, (struct sockaddr *) &remote, &sockaddrlen);


	union {
		uint32_t addr;
		char bytes[4];
	} fromaddr;
	fromaddr.addr = ntohl(remote.sin_addr.s_addr);
				fprintf(stderr, "%d.%d.%d.%d: ", 0xff & (unsigned int)fromaddr.bytes[3],
			    	0xff & (unsigned int)fromaddr.bytes[2],
			    	0xff & (unsigned int)fromaddr.bytes[1],
			    	0xff & (unsigned int)fromaddr.bytes[0]);
		wclock.lock();
		wcyifuyin21.insert(std::pair<int, bool> (sock, true));
		wclock.unlock();
	std::thread qq (get_que);
	while(!done){
		sock = accept(s, (struct sockaddr *) &remote, &sockaddrlen);
	
		send(sock, ACCEPTMESSAGE,strlen(ACCEPTMESSAGE),0);
		wclock.lock();
		wcyifuyin21.insert(std::pair<int, bool> (sock, true));
		wclock.unlock();
		write(pipeline[1], "hey\n", 16);
	}
}
 
void time_to_start(int time){
	sleep(time);
	que_time.notify_all();
}


void get_que(){
	fd_set ready;
	fd_set income;
	int bytes;
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	struct sockaddr_in msgfrom;
	qq944626604 = new clientpq(sched == FCFS? FCFSCOM : SJFCOM);
	while (!done) {
		FD_ZERO(&ready);
		FD_SET(fileno(stdin), &ready);		// stdin for quitting
		FD_SET(pipeline[0], &ready);		// self awaking pipe
		int sock;
		int clientslen = wcyifuyin21.size();
		// get all the fd_set ready
		wclock.lock();
		for(std::pair<int, bool> pair : wcyifuyin21){
			int sock_ = pair.first;
			bool connect = pair.second;
			if(connect){
				sock = std::max(sock, sock_);
				FD_SET(sock_, &ready);
			}else{
				close(sock_);
				wcyifuyin21.erase(sock_);
			}
		}
		wclock.unlock();

		// Wait for things to happen from fds in fd_set
		if (select(sock+1, &ready, 0, 0, 0) < 0) {
			perror("select");
			exit(1);
		}
		
		// Treat my input differently. As a selfish human being I listen to myself more than other people.
		if (FD_ISSET(fileno(stdin), &ready)) {
			read(fileno(stdin), buf, BUF_LEN);
			if (strncmp(buf, "q", 1)==0|| strncmp(buf, "quit", 4)==0 || strncmp(buf, "Quit", 4)==0 || strncmp(buf, "QUIT", 4)==0){
				done++;
			}
		}
		
		if (FD_ISSET(pipeline[0], &ready)) {
			read(pipeline[0], buf, BUF_LEN);
		}

		// send shit to my clients, they don't matter at all.
		for(std::pair<int, bool> pair : wcyifuyin21){
			int soc = pair.first;
			if (FD_ISSET(soc, &ready)) {
				memset(buf, 0, BUF_LEN);
				if ((bytes = recvfrom(soc, buf, BUF_LEN, 0, (struct sockaddr *)&msgfrom, &sockaddrlen) <= 0)) 
				{
					ASSERT(sockaddrlen == sizeof(msgfrom));
					done++;
				}
				struct client_req creq;
				struct stat status;
				char *point;

				std::string bufer(buf);
				creq.request = bufer;
				std::istringstream iss(bufer);
				iss >> creq.req;
				iss >> creq.file_name;
				iss >> creq.protocol;
				retval = stat(creq.file_name.c_str(), &status);
				creq.file_stat = retval;
				creq.err = errno;
				if(!S_ISREG(status.st_mode)){
					creq.content_type=DIREC;
				}else{
					creq.content_type = OTHER;
				}
				time(&creq.date);
		
				creq.last_modified = status.st_mtime;
				creq.content_length = status.st_size;				
				
				qqlock.lock();
				
				creq.socket = soc;
				
				qq944626604->push(creq);
				sem.V();
				qqlock.unlock();
			}
		}
	}
	delete qq944626604;
	for(int i = 0; i < numthread; i++){
		sem.V();
	}
	
	for(auto & ts: worker_ts){
		ts.join();
	}
	exit(-1);
}

