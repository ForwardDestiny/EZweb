#ifndef REQ_HAND_
#define REQ_HAND_

#include "EZweb.h"
#include <fstream>
#include <map>

using std::fstream;
using std::map;

class requestHandle{
public:
	requestHandle(string root_dir, string index_page = "index.html"): root(root_dir), connect_fd(-1), index(index_page), file_fd(-1) { }
	
	inline void setConnFd(int fd)
	{
		connect_fd = fd;
		sockaddr_in peer_info;
		socklen_t length = sizeof(peer_info);

		if((getpeername(connect_fd, EZ_R_CAST(&peer_info, SA*), &length)) < 0)
		{
			EZ_ERR("setConnFd ERR\n");
			exit(0);
		}

		EZ_peer = sock_ntop(EZ_R_CAST(&peer_info, SA*), length);
		
	}

	virtual void execute(){

		int cnt;
		map<string, string> header;

		while((cnt = Readline(connect_fd, cur_text)) > 0)
		{
			if(cur_text == "\r\n")
			{
				//EZ_INFO("Head_Over\n");
				break;
			}
			string cmd_t, content_t;
			dispart(cmd_t, content_t, cur_text);
			header[cmd_t] = content_t;
			//cout << "cmd:" <<  cmd << " content:" << content << flush;
		}
		for(auto m:header)
		{
			processCmd(m.first, m.second);
		}
	//	EZ_INFO("Returned!\n");
		return;
	}

protected:
	void dispart(string& command, string& content, const string& text);

	virtual void processCmd(const string& command, const string& content);

	string extractFileDir(const string& content);

	int connect_fd;
	int file_fd;

	string root;
	string index;
	string cur_text;
	string EZ_peer;
};


#endif
