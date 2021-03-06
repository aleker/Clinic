#ifndef INF122464_RFUN
#define INF122464_RFUN

# define APPOINTMENTS_LIST_SIZE 40
// APPOINTMENTS LIST
struct msgbuf appointments_list[APPOINTMENTS_LIST_SIZE];
int appointments_list_size;

// PATIENTS LIST
struct msgbuf patients_list[20];
int patients_list_size;

// DOCTORS LIST
struct msgbuf doctors_list[5];
int doctors_list_size;

// DOCTORS VACATION LIST
struct msgbuf vacation_list[5];

bool quit;

void answerForDoctorsLeave(int msgid);
void answerForChangeDateOfVisit(int msgid);
void deleteVisit(int msgid, int msgrcv_size);
void searchForOutdatedAppointments(int msgid);
void searchForNewAppointment(int msgid, int msgrcv_size);
void addPatient(int msgid, int msgrcv_size);
void answerForListOfDoctors(int msgid, int msgrcv_size);
void answerForDoctorLoginRequest(int msgid, int msgrcv_size);
void answerForLoginRequest(int msgid, int msgrcv_size);
void answerForListOfVisits(int msgid, int msgrcv_size);
void answerForDoctorListOfVisits(int msgid, int msgrcv_size);
bool checkExit(int msgid);
void deleteReservedVisits(int msgid, struct msgbuf leave);

#endif