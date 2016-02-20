#ifndef INF122464_PFUN
#define INF122464_PFUN

bool quit;

void chooseDate(struct msgbuf* patient);
void requestForDoctorPossibleVisits(int msgid, struct msgbuf* patient);
struct msgbuf displayStatusOfVisit(int msgid, struct msgbuf* patient);
void requestForDoctorsList(int msgid, struct msgbuf* patient);
struct msgbuf registerNewPatient(int msgid);
void makeAnAppointment(int msgid, struct msgbuf* patient, int pid_registration);
void displayInstruction();
void cancelVisit(int msgid, struct msgbuf* patient);
    
#endif