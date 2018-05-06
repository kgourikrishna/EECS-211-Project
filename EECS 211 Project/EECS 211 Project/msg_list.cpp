#include <iostream>

#include "msg_list.h"

msg_list::msg_list(){
	front = back = NULL;
}

void msg_list::display() {
	msg_list_node *tmp;  int i;
	
	if(front==NULL) {
		cout << "** List is empty. **\n";
		return;
	}
	
	tmp = front;  i = 1;
	while(tmp!=NULL) {
		cout << "Datagram " << i++ << ":  \n";
		(tmp->d)->display();
		cout << "\n";
		tmp = tmp->next;
	}
	
}

void msg_list::append(datagram *x) {
	msg_list_node* tmp = new msg_list_node;
	tmp->next = NULL;
	tmp->d = x;
    
	if(front==NULL)
        front = tmp;
	else
        back->next = tmp;
	back = tmp;
}

datagram* msg_list::returnFront(){
    msg_list_node *currNode;
    datagram *data;
    if(front==NULL){
        return NULL;
    }
    if(front==back){
        currNode = front;
        front = NULL;
        back = NULL;
        data = currNode->d;
        delete currNode;
        return data;
    }
    currNode = front;
    front = currNode->next;
    data = currNode->d;
    delete currNode;
    return data;
    
}

void msg_list::deleteList(){
    msg_list_node* currNode;
    currNode = front;
    while(currNode){
        delete currNode->d;
        front = currNode->next;
        delete currNode;
        currNode = front;
    }
    front = NULL;
    back = NULL;
}
