
/* Author:  Keith Shomper
 * Date:    8 Nov 2016
 * Purpose: Supports HW4 for CS3320 */

#define MAX_HINT_SIZE 20
#define MAX_PWD_SIZE 20
#define MAX_HINT_USER 20

#define MAX_HINT_PWD_SIZE MAX_HINT_SIZE+1+MAX_PWD_SIZE+1

#define FORWARD       0
#define REVERSE       1

#define FALSE         0
#define TRUE          1

/* structure to hold the hint-password pairs                           */
struct hint_pwd {
   char hint[MAX_HINT_SIZE];
   char pwd[MAX_PWD_SIZE];
};

/* allows the hint-password pairs to be grouped into a linked list     */
struct hpw_list {
   struct hint_pwd  hpw;
   struct hpw_list *next;
   struct hpw_list *prev;
};

/* hold information about a list, including a pointer to the head */
struct hpw_list_h {
   int               total_hpw_pairs;
   int               num_hints;
   char              seek_hint[MAX_HINT_PWD_SIZE];
   struct hpw_list **data;
   struct hpw_list  *fp;
};

/* the password vault is essentially an array of hpw list head pointers */
struct pwd_vault {
   int                num_users;
   struct hpw_list_h *uhpw_data;
};

/* a typedefed function pointer for walking the data structure sequentially   */
typedef struct hpw_list*(*seq_func_ptr)(struct pwd_vault*,int,struct hpw_list*);

/*
 * Function prototypes follow
 */

/* initialize_vault:  initializes the pwd vault                              */
int initialize_vault (struct pwd_vault *v, int size);

/* dump_vault:  prints the contents of the vault to log for debugging         */
void dump_vault (struct pwd_vault *v, int dir);

/* finalize_vault:  releases the allocated memory for the vault               */
void finalize_vault (struct pwd_vault *v);

/* num_hints:  how many unique hints have been inserted by this user, 1-index */
int num_hints (struct pwd_vault *v, int uid);

/* rem_hints:  how many addtl unique hints may be inserted by user, 1-index   */
int rem_hints (struct pwd_vault *v, int uid);

/* num_pairs(int): how many hint-pwd pairs have been inserted by user, 1-index*/
int num_pairs (struct pwd_vault *v, int uid);

/* num_vhints:  how many unique hints have been inserted into the vault       */
int num_vhints (struct pwd_vault *v);

/* num_vpairs(void):  how many hint-pwd pairs have been inserted into vault   */
int num_vpairs (struct pwd_vault *v);

/* insert_pair: inserts hint-pwd pair for given uid (one-indexed) into vault  */
int insert_pair (struct pwd_vault *v, int uid, char *hint, char *pwd);

/* delete_pair: deletes hint-pwd pair for given uid (one-indexed) from vault  */
void delete_pair (struct pwd_vault *v, int uid, char *hint, char *pwd);

/* retrieve_pwd:  retrieves pwd(s) for hint for uid (one-indexed) to debug    */
int retrieve_pwd (struct pwd_vault *v, int uid, char *hint, 
                  char  pwd[MAX_HINT_USER][MAX_PWD_SIZE]);

/* find_hint:  finds the specified hint in the vault and returns a pointer to
 *             it, or returns NULL if the hint is not present; also sets
 *             hint_num to the sequential location of the hint in the vault 
 *             Note, uid is one-indexed.                                      */
struct hpw_list*  find_hint  (struct pwd_vault *v, int uid, char *hint, 
                              int *hint_num);

/* find_hint_pwd:  finds the specified hint-pwd pair and returns a pointer to
 *                 it, or returns NULL if the pair is not present.            */
struct hpw_list*  find_hint_pwd (struct pwd_vault *v, int uid, char *hint, 
                                 char  *pwd);

/* next_hint:  returns a pointer to the next hint in the current user's set,
 *             or NULL if there is no next hint.  uid is one-indexed.         */
struct hpw_list*  next_hint  (struct pwd_vault *v, int uid, struct hpw_list *l);

/* prev_key:  returns a pointer to the prev hint in the current user's set,
 *            or NULL if there is no prev hint.  uid is one-indexed.          */
struct hpw_list*  prev_hint  (struct pwd_vault *v, int uid, struct hpw_list *l);

/* get_last_in_list:  walks given list to last element and returns its ref    */
struct hpw_list*  get_last_in_list (struct hpw_list *l);

/* free_list:  releases any allocated memory in tail of incoming list         */
void free_list (struct hpw_list *l);

/* insert_in_list:  inserts the hint-pwd pair into list l                     */
int insert_in_list (struct hpw_list **l, char *hint, char *pwd);

/* delete_from_list: deletes the referenced hint-pwd pair from vault          */
void delete_from_list (struct hpw_list **l);

