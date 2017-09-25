//   Request-reply client in C++
//   Connects REQ socket to tcp://localhost:5559
//   Sends "Hello" to server, expects "World" back
//
// Olivier Chamoux <olivier.chamoux@fr.thalesgroup.com>
#include "zhelpers.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

class NetworkDispatcher:public el::LogDispatchCallback {
  public:
// Setters
	void Init(std::string module,std::string proto,std::string host,int port,zmq::context_t * p_context){
		auto pos=module.find_last_of("/");
		m_module=module.substr(pos==std::string::npos?0:pos+1);
		m_host=host;
		m_port=port;
		m_proto=proto;
		m_context=p_context;
    	m_requester=new zmq::socket_t(*m_context, ZMQ_DEALER);
		connect();
	}	

	void connect(){
		std::stringstream connect_string;
		connect_string<<m_proto<<"://"<<m_host<<":"<<m_port;
    	m_requester->connect(connect_string.str());
		
	}
  protected:
    void handle(const el::LogDispatchData * data) noexcept override {
        m_data = data;
// Dispatch using default log builder of logger
        dispatch(m_data->logMessage()->logger()->logBuilder()->
                 build(m_data->logMessage(),
                       m_data->dispatchAction() ==
                       el::base::DispatchAction::NormalLog));
  	} 
  private:
    const el::LogDispatchData * m_data;
    std::string m_host;
    int m_port;
	std::string m_proto;
	std::string m_module;
	zmq::context_t* m_context;
	zmq::socket_t* m_requester;

    void dispatch(el::base::type::string_t && logLine) noexcept {
		std::stringstream log_string;
		log_string<<logLine.substr(0,logLine.length()-1)<<"|192.168.0.1|"<<m_module<<"_"<<getpid()<<"|";
        s_send(*m_requester,log_string.str());
    }
};


int main(int argc,char **argv)
{
    zmq::context_t context(1);
    el::Helpers::installLogDispatchCallback < NetworkDispatcher >
      ("NetworkDispatcher");
    NetworkDispatcher *dispatcher =
      el::Helpers::logDispatchCallback < NetworkDispatcher >
      ("NetworkDispatcher");
    dispatcher->setEnabled(true);
	dispatcher->Init(argv[0],"tcp","127.0.0.1",15560,&context);

	int count=0;
	while(true){
    	LOG(INFO) << "First network log "<<count++;
		sleep(1);
	}
}
