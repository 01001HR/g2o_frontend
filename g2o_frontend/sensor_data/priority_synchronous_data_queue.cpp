/*
 * PrioritySynchronousdata_queue.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: jacopo
 */

#include "priority_synchronous_data_queue.h"

PrioritySynchronousDataQueue::PrioritySynchronousDataQueue() {

}

bool PrioritySynchronousDataQueue::empty() const {
	this->lock();
	bool isEmpty = _queue.empty();
	this->unlock();
	return(isEmpty);
}

g2o::HyperGraph::Data* PrioritySynchronousDataQueue::front() {
	SensorData* dataPtr = 0;
	{
		this->lock();
		if(!_queue.empty())
			dataPtr = (SensorData*)_queue.top();
		this->unlock();
	}
	return(dataPtr);
}

void PrioritySynchronousDataQueue::pop_front() {
	this->lock();
	_queue.pop();
	this->unlock();
}

g2o::HyperGraph::Data* PrioritySynchronousDataQueue::front_and_pop() {
  SensorData* dataPtr = 0;
  this->lock();
  if(!_queue.empty()){
    dataPtr = (SensorData*)PriorityDataQueue::front();
    PriorityDataQueue::pop_front();
  }
  this->unlock();
  return dataPtr;
}

void PrioritySynchronousDataQueue::insert(g2o::HyperGraph::Data* d) {
	this->lock();	
	PriorityDataQueue::insert(d);
	this->unlock();
}
