#pragma once
class IHandleObject
{

public:
	IHandleObject() {};
	~IHandleObject() {};
	virtual int read(char* b, size_t offset, size_t length) = 0;
	virtual int write(char* buffer, size_t offset, size_t length) = 0;

};


