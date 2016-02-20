#ifndef INF122464_ALL
#define INF122464_ALL

// odbiera rejestracja
#define PATIENT_TYPE        2
#define DOCTOR_TYPE         3
#define P_LOGIN_REQUEST     4
#define D_LOGIN_REQUEST     5
#define DOCTORS_REQUEST     6
#define APPOINTMENT_TYPE    7
#define LOGOUT              8
#define QUIT                9
#define VISITS_REQUEST      10
#define CHANGE_VISIT        11
#define DELETE_VISIT        12
#define NEW_VISIT           13
#define D_LEAVE             14

// odbiera lekarz
#define ACCEPT_0            20
#define ACCEPT_1            21
#define ACCEPT_2            22
#define ACCEPT_3            23
#define ACCEPT_4            24


#define MSGBUF_SIZE         136
#define MSGBUF_SIZE2        112
#define PASSWORD_SIZE       10
#define MAX_LOG_COUNTER     3

struct msgbuf {
        long typ;
        int index;                      //4
        int pid;                        //4     // doctor
        int pesel[11];	                //44    // patient
        char name[20];		            //20
        char surname[20];	            //20
        char password[PASSWORD_SIZE];   //10
        int time_of_visit;              //4
        time_t date_of_visit;           //8
        char test;                      //1
    };

time_t today;
time_t last_date;
char day[13][5];

struct msgbuf login(int msgid, int type);
int createMessageQueue(char string[30]);
void clearCalendar(char (*day2)[13][5]);
time_t improveDate(time_t date);
void setLastDate();
void readConfigurationFile(char file_name[30]);
void displayVisit(struct msgbuf* patient, struct msgbuf* visit);

#endif