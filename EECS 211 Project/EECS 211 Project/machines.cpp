#include <iostream>
#include <stdlib.h>
#include <string>

#include "machines.h"

//*****************************************//
// node class functions

node::node(string n, IPAddress a) {
    name = new string;
    *name = n;
    my_address = a;
}

node::~node() {
    delete name;
}

void node::display() {
    
    cout << "   Name: " << *name << "   IP address: ";
    my_address.display();
}

int node::amIThisComputer(IPAddress a) {
    if(my_address.sameAddress(a))
        return 1;
    else
        return 0;
}

int node::myType() {
    return 0;
}

IPAddress node::myAddress(){
    return my_address;
}

//*****************************************//
// laptop class functions

laptop::laptop(string n, IPAddress a) : node(n,a)  {
    incoming = outgoing = NULL;
    my_server.parse("0.0.0.0");
}

int laptop::myType() {
    return LAPTOP;
}

void laptop::initiateDatagram(datagram* d) {
    outgoing = d;
}

void laptop::receiveDatagram(datagram* d) {
    incoming = d;
}

int  laptop::canAcceptConnection(int machine_type) {
    if(machine_type!=SERVER) return 0;
    return my_server.isNULL(); //we can only connect if the server is empty
}

void laptop::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER) my_server = x;
}

void laptop::transferDatagram() {
    int i;
    extern node* network[MAX_MACHINES];
    
    if(outgoing==NULL) return;
    if(my_server.isNULL()) return;
    for (i = 0; i < MAX_MACHINES; i++)
    {
        if (network[i] != NULL)
        {
            if (network[i]->amIThisComputer(my_server))
                break;
        }
    }
    network[i]->receiveDatagram(outgoing);
    outgoing = NULL;
}

void laptop::display() {
    
    cout << "Laptop computer:  ";
    node::display();
    
    if(my_server.isNULL()) {
        cout << "\n    Not connected to any server.\n";
    }
    else {
        cout << "\nConnected to ";
        my_server.display();
        cout << "\n";
    }
    
    cout << "\n   Incoming datagram:  ";
    if(incoming==NULL) cout << "No datagram.";
    else               {cout << "\n";  incoming->display(); }
    
    cout << "\n   Outgoing datagram:  ";
    if(outgoing==NULL) cout << "No datagram.";
    else               {cout << "\n"; outgoing->display(); }
    
}

/**************new*************/
laptop::~laptop()
{
    if (incoming != NULL)
        delete incoming;
    if (outgoing != NULL)
        delete outgoing;
}
/**************new*************/

void laptop::consumeDatagram(){
    incoming = NULL;
}

int laptop::canReceiveDatagram(){
    if (incoming==NULL){
        return 1;
    }
    else{
        return 0;
    }
}

//*****************************************//
// server class functions

server::server(string n, IPAddress a) : node(n,a)  {
    number_of_laptops = number_of_wans = 0;
    dlist = new msg_list;
}


int server::myType() {
    return SERVER;
}

int  server::canAcceptConnection(int machine_type) {
    if(machine_type==LAPTOP)
        return (number_of_laptops<8);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    return 0;
}

void server::connect(IPAddress x, int machine_type) {
    if(machine_type==LAPTOP)
        laptop_list[number_of_laptops++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void server::receiveDatagram(datagram* d) {
    dlist->append(d);
}

void server::display() {
    int i;
    
    cout << "Server computer:  ";
    node::display();
    
    cout << "\n   Connections to laptops: ";
    if(number_of_laptops==0) cout << "    List is empty.";
    else for(i=0; i<number_of_laptops; i++) {
        cout << "\n      Laptop " << i+1 << ":   ";
        laptop_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

/**************new*************/
server::~server()
{
    dlist->deleteList();
}
/**************new*************/

void server::transferDatagram(){
    extern node* network[];
    msg_list* tmp;
    datagram *dg;
    dg = dlist->returnFront();
    while(dg!=NULL){
        if(dg->destinationAddress().firstOctad()==myAddress().firstOctad()){
            for(int i=0;i<number_of_laptops;i++){
                if(laptop_list[i].sameAddress(dg->destinationAddress())){
                    for(int j=0;j<MAX_MACHINES;j++){
                        if(network[j]->amIThisComputer(laptop_list[i])){
                            if(((laptop*)network[j])->canReceiveDatagram()){
                                network[j]->receiveDatagram(dg);
                            }
                            else{
                                tmp->append(dg);
                            }
                        }
                    }
                }
            }
        }
        else{
            if(number_of_wans==0){
                tmp->append(dg);
            }
            else{
                int reciever_number = abs(WAN_list[0].firstOctad()-dg->destinationAddress().firstOctad());
                int reciever = 0;
                for(int i=1;i<number_of_wans;i++){
                    if(reciever_number>abs(WAN_list[i].firstOctad()-dg->destinationAddress().firstOctad())){
                        reciever_number = abs(WAN_list[i].firstOctad()-dg->destinationAddress().firstOctad());
                        reciever = i;
                    }
                }
                for(int i;i<MAX_MACHINES;i++){
                    if (network[i]->amIThisComputer(WAN_list[reciever])) {
                        network[i]->receiveDatagram(dg);
                    }
                }
                
            }
            
        }
        dg = dlist->returnFront();
    }
    delete dlist;
    dlist = tmp;
}
//*****************************************//
// WAN class functions

WAN::WAN(string n, IPAddress a) : node(n,a)  {
    number_of_servers = number_of_wans = 0;
    dlist = new msg_list;
}

int WAN::myType() {
    return WAN_MACHINE;
}

int  WAN::canAcceptConnection(int machine_type) {
    if(machine_type==SERVER)
        return (number_of_servers<4);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    
    return 0;
}

void WAN::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER)
        server_list[number_of_servers++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void WAN::receiveDatagram(datagram* d) {
    dlist->append(d);
}


void WAN::display() {
    int i;
    
    cout << "WAN computer:  ";
    node::display();
    
    cout << "\n   Connections to servers: ";
    if(number_of_servers==0) cout << "    List is empty.";
    else for(i=0; i<number_of_servers; i++) {
        cout << "\n      Server " << i+1 << ":   ";
        server_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

/**************new*************/
WAN::~WAN()
{
    dlist->deleteList();
}
/**************new*************/

void WAN::transferDatagram(){
    extern node* network[];
    datagram *dg;
    msg_list* tmp;
    dg = dlist->returnFront();
    while(dg!=NULL){
        for(int i = 0;i<number_of_servers;i++){
            if(server_list[i].firstOctad()==dg->destinationAddress().firstOctad()){
                for(int i=0;i<MAX_MACHINES;i++){
                    if(network[i]->amIThisComputer(server_list[i])){
                        network[i]->receiveDatagram(dg);
                    }
                }
            }
            else if(number_of_wans!=0){
                int reciever_number = abs(WAN_list[0].firstOctad()-dg->destinationAddress().firstOctad());
                int reciever = 0;
                for(int i=1;i<number_of_wans;i++){
                    if(reciever_number>abs(WAN_list[i].firstOctad()-dg->destinationAddress().firstOctad())){
                        reciever_number = abs(WAN_list[i].firstOctad()-dg->destinationAddress().firstOctad());
                        reciever = i;
                    }
                }
                for(int i=0;i<MAX_MACHINES;i++){
                    if(network[i]->amIThisComputer(WAN_list[reciever])){
                        network[i]->receiveDatagram(dg);
                    }
                }
            }
            else{
                tmp->append(dg);
            }
        }
        dg = dlist->returnFront();
    }
    delete dlist;
    dlist = tmp;
}





















