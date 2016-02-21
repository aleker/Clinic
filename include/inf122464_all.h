#ifndef INF122464_ALL
#define INF122464_ALL

// receiving by registration:
#define PATIENT_TYPE        2
#define DOCTOR_TYPE         3
#define P_LOGIN_REQUEST     4
#define D_LOGIN_REQUEST     5
#define DOCTORS_REQUEST     6
#define APPOINTMENT_TYPE    7
#define LOGOUT              8
#define QUIT                9
#define VISITS_REQUEST      10
#define D_VISITS_REQUEST    11
#define CHANGE_VISIT        12
#define DELETE_VISIT        13
#define NEW_VISIT           14
#define D_LEAVE             15

#define MSGBUF_SIZE         128 //136
#define PASSWORD_SIZE       10
#define MAX_LOG_COUNTER     3
#define SEC_COMFIRM         60

struct msgbuf {
        long typ;
        int index;
        int pid;
        int pesel[11];
        char name[20];
        char surname[20];
        char password[PASSWORD_SIZE];
        int time_of_visit;
        time_t date_of_visit;
        //char test;
    };
time_t today;
time_t last_date;
char day[13][5];

void displayVisit(struct msgbuf* patient, struct msgbuf* visit);
struct msgbuf login(int msgid, int type);
int createMessageQueue(char string[30]);
void clearCalendar(char (*day2)[13][5]);
time_t improveDate(time_t date);
void setLastDate();
void readConfigurationFile(char file_name[30]);

#endif