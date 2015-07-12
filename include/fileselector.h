/*
** QpidR - Qpid recovery tool
** newslab.csie.ntu.edu.tw, Taiwan
** See Copyright Notice in common.h
*/

#ifndef FILESELECTOR_H_INCLIDED
#define FILESELECTOR_H_INCLIDED

#define GET_READY_FD_TIMEOUT (-2)
class FileSelector{
private:
	int regfd[400], numberofreg;
	unsigned timeout_s, timeout_us;
public:
	FileSelector(unsigned s, unsigned us);
	~FileSelector();

	void resetTimeout(unsigned s, unsigned us);

	//-2 if timeout, -1 if error
	int getReadReadyFD();
	void registerFD(int fd);
	void unregisterFD(int fd);
};

#endif
