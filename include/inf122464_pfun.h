#ifndef INF122464_PFUN
#define INF122464_PFUN

bool quit;

void chooseDate(struct msgbuf* patient);
void requestForDoctorsList(int msgid, struct msgbuf* patient);
bool makeAnAppointment(int msgid, struct msgbuf* patient, int pid_registration, bool doctors_decision);
struct msgbuf registerNewPatient(int msgid);
struct msgbuf displayStatusOfVisit(int msgid, struct msgbuf patient);
void cancelVisit(int msgid, struct msgbuf* patient);
void changeDateOfVisit(int msgid, struct msgbuf* patient);
void displayInstruction();

#endif