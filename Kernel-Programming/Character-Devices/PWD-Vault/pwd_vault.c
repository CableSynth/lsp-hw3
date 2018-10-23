/* Author:  Keith Shomper
 * Date:    8 Nov 2016
 * Purpose: Supports HW4 for CS3320 */

#include <linux/slab.h>       /* for kmalloc*/
#include <linux/string.h>     /* for memset*/
#include "pwd_vault.h"

/* initialize_vault:  initializes the pwd vault */
int  initialize_vault (struct pwd_vault *v, int size) {

   /* allocate memory for the password vault */
   v->num_users = 0;
   v->uhpw_data = kmalloc(size*sizeof(struct hpw_list_h), GFP_KERNEL);
   memset(v->uhpw_data, 0, size*sizeof(struct hpw_list_h));

   /* if error with allocation, return FALSE */
   if (v->uhpw_data == NULL) return FALSE;

   /* otherwise, set the num_users field accordingly */
   v->num_users = size;
   return TRUE;
}

/* dump_vault:  prints the contents of the vault to the kernel log */
void dump_vault (struct pwd_vault *v, int dir) {
   struct hpw_list_h *udata = v->uhpw_data;
   seq_func_ptr       next  = (dir == FORWARD) ? next_hint : prev_hint;
   int                num   = v->num_users;
   int                u;
   
   /* print the hints for each user */
   for (u = 0; u < num; u++) {
      int  uid  = (dir == FORWARD) ? u : num-u-1; /* uid is zero-indexed */
      int  n    = udata[uid].num_hints;

      /* this user has no hints to print */
      if (n == 0) continue;

      /* references the hint-pwd pair to be printed */
      struct hpw_list *l;

      /* point l at the first (or last) hint in the user's set */
      if   (dir == FORWARD) l = udata[uid].data[0];
      else                  l = get_last_in_list(udata[uid].data[n-1]);

      /* print hints in FORWARD (or REVERSE) order until they are exhausted */
      while (l != NULL) {
         printk(KERN_WARNING "\t[%s %s]\n", l->hpw.hint, l->hpw.pwd);
         l = next(v, uid+1, l);
      }
   }
}

/* finalize_vault:  releases the allocated memory for the vault */
void finalize_vault (struct pwd_vault *v) {
   /* no data allocated, simply return */
   if (v->uhpw_data == NULL) return;

   /* release allocations for each user */
   int i;
   for (i = 0; i < v->num_users; i++) {
      
      /* if memory was allocated to this user, release it */
      if (v->uhpw_data[i].data != NULL) {

         /* free memory for each chain of linked-list passwords */
         int n = v->uhpw_data[i].num_hints;
         int k;
         for (k = 0; k < n; k++){
            free_list(v->uhpw_data[i].data[k]);
         }

         /* free the allocated memory for user's data */
         kfree (v->uhpw_data[i].data);
      }
   }

   /* free the array of user data */
   kfree (v->uhpw_data);
}

/* num_hints:  how many unique hints inserted by this; user uid 1-indexed */
int num_hints (struct pwd_vault *v, int uid) {
   if (uid < 1 || uid > v->num_users) return -1;
   
   return v->uhpw_data[uid-1].num_hints;
}

/* rem_hints:  how many additional unique hints may yet be inserted by user */
/* uid 1-indexed */
int rem_hints (struct pwd_vault *v, int uid) {
   if (uid < 1 || uid > v->num_users) return -1;
   
   return MAX_HINT_USER - v->uhpw_data[uid-1].num_hints;
}

/* num_pairs(int):  how many total hint-pwd pairs have been inserted by user */
/* uid 1-indexed */
int num_pairs (struct pwd_vault *v, int uid) {
   if (uid < 1 || uid > v->num_users) return -1;
   
   return v->uhpw_data[uid-1].total_hpw_pairs;
}

/* num_vhints(void):  how many unique hints have been inserted into vault */
int num_vhints (struct pwd_vault *v) {
   int sum = 0, uid;
   for (uid = 0; uid < v->num_users; uid++) {
      sum += num_hints(v, uid+1);
   }
   return sum;
}

/* num_vpairs(void):  how many hint-pwd pairs have been inserted into vault */
int num_vpairs (struct pwd_vault *v) {
   int sum = 0, uid;
   for (uid = 0; uid < v->num_users; uid++) {
      sum += num_pairs(v, uid+1);
   }
   return sum;
}

/* insert_pair: inserts hint-pwd pair for given uid (one-indexed) into vault */
int  insert_pair (struct pwd_vault *v, int uid, char *hint, char *pwd) {

#ifdef DEBUG
   printk(KERN_WARNING "insert_pair: v is %x\n", v);
#endif

   /* hint-pwd pairs not kept for this uid, return FALSE */
   if (uid < 1 || uid > v->num_users) return FALSE;

#ifdef DEBUG
   printk(KERN_WARNING "insert_pair: v->users is %x\n", v->num_users);
   printk(KERN_WARNING "insert_pair: v->uhpw_data is %x\n", v->uhpw_data);
#endif

   /* locate the given user's hint data */
   struct hpw_list_h *user = &v->uhpw_data[uid-1];

#ifdef DEBUG
   printk(KERN_WARNING "insert_pair: user->num_hints is %d\n", user->num_hints);
#endif

   /* if first hint for this user, then we need to allocate memory */
   if (user->data == NULL) {
#ifdef DEBUG
      printk(KERN_WARNING "insert_pair: first hint for user %d\n", uid);
      printk(KERN_WARNING "insert_pair: allocating %d hints for user\n", 
                           MAX_HINT_USER);
#endif

      user->data = kmalloc(MAX_HINT_USER*sizeof(struct hpw_list*), GFP_KERNEL);

      /* if allocation fails, then return false */
      if (user->data == NULL) {
#ifdef DEBUG
         printk(KERN_WARNING "insert_pair: allocation failed\n");
#endif
         return FALSE;
      }
      memset(user->data, 0, MAX_HINT_USER*sizeof(struct hpw_list*));

#ifdef DEBUG
      printk(KERN_WARNING "insert_pair: allocations succeeded\n");
#endif
   }
   
   /* scan this user's hints for duplicates */
   struct hpw_list **la = user->data;
   int i;
   for (i = 0; i < user->num_hints; i++) {
      /* duplicate hint found, exit loop */
#ifdef DEBUG
      printk(KERN_WARNING "insert_pair: searching duplicate among hint %d\n",i);
#endif
      if (strncmp(la[i]->hpw.hint, hint, MAX_HINT_SIZE) == 0) break;
   }

   /* no more new hints permitted for this user, return FALSE */
   if (i == MAX_HINT_USER) return FALSE;

#ifdef DEBUG
   if (i < user->num_hints) {
      printk(KERN_WARNING "insert_pair: hint %s is duplicate of hint %d\n",
             hint, i);
   } else {
      printk(KERN_WARNING "insert_pair: hint %s unique among stored hints\n",
             hint);
   }

   printk(KERN_WARNING "insert_pair: la[i] for i = %d is %x\n", i, la[i]);
#endif

   int rc = insert_in_list(&la[i], hint, pwd);

   /* hint was successfully inserted */
   if (rc) {
      user->total_hpw_pairs++;

      /* inserted hint was a new (non-duplicate) hint */
      if (i == user->num_hints) user->num_hints++;

#ifdef DEBUG
      printk(KERN_WARNING "insert_pair: success (%d, %d)\n", 
                           user->num_hints, user->total_hpw_pairs);
   /* or not */
   } else {
      printk(KERN_WARNING "insert_pair: %s %s failure\n", hint, pwd);
#endif
   }

   return rc;
}

/* delete_pair: deletes hint-pwd pair for given uid (one-indexed) from vault */
void delete_pair (struct pwd_vault *v, int uid, char *hint, char *pwd) {

   /* find the hint to delete */
   struct hpw_list *l = find_hint_pwd(v, uid, hint, pwd);

#ifdef DEBUG
   printk(KERN_WARNING "delete_pair:  deleting hint at position %x\n", l);
#endif

   /* hint-pwd pair is not present */
   if (l == NULL) return;

   /* determine if the hint is at the head of a list */
   int i;
   int num_hints = v->uhpw_data[uid-1].num_hints;
   for (i = 0; i < num_hints; i++) {
      if (v->uhpw_data[uid-1].data[i] == l) break;
   }

   /* test for head of list with only one element */
   int only_element_in_list = FALSE;
   int head_of_list = FALSE;
   if (i < num_hints) {
      head_of_list = TRUE;

#ifdef DEBUG
      printk(KERN_WARNING "delete_pair:  the hint is a head of list\n");
#endif
      
      only_element_in_list = (int) (l->next == NULL);
   }


   /* point the head pointer to the next element (could be NULL) */
   if (head_of_list) {
      v->uhpw_data[uid-1].data[i] = l->next;

#ifdef DEBUG
      printk(KERN_WARNING "delete_pair:  resetting head from %x to %x\n", 
         l, v->uhpw_data[uid-1].data[i]);
#endif
   }

   /* the hint-pwd pair about to be deleted is the last in its list */
   if (only_element_in_list) {
      struct hpw_list **la = v->uhpw_data[uid-1].data;

      printk(KERN_WARNING "delete_pair:  compacting to avoid holes\n");

      /* to avoid holes among list pointers, compact the list head pointers */
      int j;
      for (j = i; j < num_hints-1; j++) {

#ifdef DEBUG
         printk(KERN_WARNING "delete_pair: la[%d](%x) replaced by la[%d](%x)\n",
                              j, la[j], j+1, la[j+1]);
#endif
         la[j] = la[j+1];

      }
      v->uhpw_data[uid-1].num_hints--;

      /* NULL-terminate what was the head pointer to the last list */
      v->uhpw_data[uid-1].data[num_hints-1] = NULL;

#ifdef DEBUG
      printk(KERN_WARNING "delete_pair:  NULL-terminating list la[%d]\n",
                           num_hints-1);
#endif
   }

   delete_from_list(&l);

   /* reduce the total number for this uid */
   v->uhpw_data[uid-1].total_hpw_pairs--;

#ifdef DEBUG
   printk(KERN_WARNING "delete_pair:  hints reduced (%d, %d)\n",
                        v->uhpw_data[uid-1].num_hints,
                        v->uhpw_data[uid-1].total_hpw_pairs);
#endif
}

/* retrieve_pwd:  retrieves pwd(s) for hint for given uid (one-indexed) */
int  retrieve_pwd (struct pwd_vault *v, int uid, char *hint, 
                   char pwd[MAX_HINT_USER][MAX_PWD_SIZE]) {
   
   /* required parameter for find_hint, but not used in this function */
   int hint_num;

   /* get pointer to hint-pwd pair in vault */
   struct hpw_list *l = find_hint(v, uid, hint, &hint_num);

   /* if l is NULL, then the hint is not present */
   if (l == NULL) return 0;

   /* otherwise, hint was found, retrive cnt associated pwd(s) */
   int cnt = 0;
   while (l != NULL) {
      strncpy(pwd[cnt], l->hpw.pwd, MAX_PWD_SIZE);
      cnt++;
      l = l->next;
   }

   return cnt;
}

/* find_hint:  finds the specified hint in the vault and returns a pointer to
 *            it, or returns NULL if the hint is not present; also sets
 *            hint_num to the sequential location of the hint in the vault 
 *            Note, uid is one-indexed.
 */
struct hpw_list* find_hint (struct pwd_vault *v, int uid, char *hint, 
                          int *hint_num) {

   /* assume searched hint is not the last in the user's set */
   *hint_num = 0;

   /* hint-pwd pairs not kept for this uid, return NULL */
   if (uid < 1 || uid > v->num_users) return NULL;

   /* locate the given user's hint data */
   struct hpw_list_h *user = &v->uhpw_data[uid-1];
   
   /* scan this user's hints for match */
   struct hpw_list **la = user->data;
   int i;
   for (i = 0; i < user->num_hints; i++) {
      /* hint found, exit loop */
      if (strncmp(la[i]->hpw.hint, hint, MAX_HINT_SIZE) == 0) break;
   }

   /* if hint not found, return NULL */
   if (i == user->num_hints) return NULL;

   /* otherwise, set hint_num and return l as the pointer to the hpw_list */
   *hint_num = i;
   return la[i];
}

/* find_hint_pwd: finds the specified hint-pwd pair and returns a pointer to
 *                it, or returns NULL if the pair is not present. uid 1-index */
struct hpw_list*  find_hint_pwd (struct pwd_vault *v, int uid, char *hint, 
                                 char  *pwd) {

   int hint_num;  /* unused */

   /* find the appropriate list of hints (if present) */
   struct hpw_list *l = find_hint(v, uid, hint, &hint_num);

   /* if there is such a hint list, now search for the selected pwd */
   if (l != NULL) {

      /* loop while we have pwd to check and have not yet found the pwd */ 
      while (l != NULL && (strncmp(l->hpw.pwd, pwd, MAX_PWD_SIZE) != 0)) {
         l = l->next;
      }
   }

   /* return the outcome:  either NULL (not present) or pointer to the pair */
   return l;
}

/* next_hint:  returns a pointer to the next hint in the current user's set,
 *            or NULL if there is no next hint.  uid is one-indexed.
 */
struct hpw_list* next_hint (struct pwd_vault *v, int uid, struct hpw_list *l) {

   /* return NULL, if l is NULL*/
   if (l == NULL) return NULL;

   /* return the next hint in the current list, if present */
   if (l->next != NULL) return l->next;

   /* otherwise, find the first (perhaps ony) hint of this list */
   int hint_num;
   l = find_hint(v, uid, l->hpw.hint, &hint_num);

   /* if this hint is last in the array of hints for this user, return NULL */
   if (uid < 1 || uid > v->num_users || 
       hint_num == v->uhpw_data[uid-1].num_hints-1) return NULL;

   /* otherwise, return the next hint in the array */
   return v->uhpw_data[uid-1].data[hint_num+1];
}

/* prev_hint:  returns a pointer to the prev hint in the current user's set,
 *            or NULL if there is no prev hint.  uid is one-indexed.
 */
struct hpw_list* prev_hint (struct pwd_vault *v, int uid, struct hpw_list *l) {

   /* return the prev hint in the current list, if present */
   if (l->prev != NULL) return l->prev;

   /* if this hint is first overall hint for this user, return NULL */
   if (uid < 1 || uid > v->num_users || 
       l==v->uhpw_data[uid-1].data[0]) return NULL;

   /* otherwise, find first (perhaps ony) hint of list */
   int num_hint;
   l = find_hint(v, uid, l->hpw.hint, &num_hint);

   /* otherwise, return the last hint in the prev list */
   l = get_last_in_list(v->uhpw_data[uid-1].data[num_hint-1]);

   return l;
}

/* get_last_in_list:  walks given list to last element and returns its ref */
struct hpw_list*   get_last_in_list (struct hpw_list *l) { 

   /* return NULL if there is no list */
   if (l == NULL) return NULL;

   /* otherwise, walk list to end, returning a reference to the last item */
   while (l->next != NULL) l = l->next; 
   return l; 
}

/* free_list:  releases any allocated memory in tail of incoming list */
void free_list(struct hpw_list *l) {

   /* while there are list elements to release */
   while (l != NULL) {

      /* retain the tail of the list */
      struct hpw_list *tail = l->next;

      /* release the current list element */
      kfree (l);

      /* reset the head of the list */
      l = tail;
   }
}

/* insert_in_list:  inserts the hint-pwd pair into list l */
int  insert_in_list (struct hpw_list **lp, char *hint, char *pwd) {

   struct hpw_list *l;

   /* no hints allocated to this list */
   if (*lp == NULL) {

#ifdef DEBUG
      printk(KERN_WARNING "IIL: hint %s begins new list\n", hint);
#endif

      /* so allocate one */
      *lp = kmalloc(sizeof(struct hpw_list), GFP_KERNEL);
      memset(*lp, 0, sizeof(struct hpw_list));

      /* if kmalloc failed, return FALSE */
      if (*lp == NULL) return FALSE;

#ifdef DEBUG
      printk(KERN_WARNING "IIL: allocation of new list node succeeded\n");
#endif

      l = *lp;

#ifdef DEBUG
      printk(KERN_WARNING "IIL: l assigned to non-NULL\n");
#endif

   /* this this already has hints */
   } else {

#ifdef DEBUG
      printk(KERN_WARNING "IIL: hint %s belongs to existing list\n", hint);
#endif

      /* walk the list to the last element */
      l = *lp;
      while (l->next != NULL) l = l->next;

#ifdef DEBUG
      printk(KERN_WARNING "IIL: walk to end of list complete\n");
#endif

      /* add the new hint-pwd pair */
      l->next = kmalloc(sizeof(struct hpw_list), GFP_KERNEL);

      /* if kmalloc failed, return FALSE */
      if (l->next == NULL) return FALSE;

#ifdef DEBUG
      printk(KERN_WARNING "IIL: allocation of tail list node succeeded\n");
#endif

      /* set new elem's prev ptr, NULL-term next ptr, and adv l to new elem */
      l->next->prev = l;
      l->next->next = NULL;
      l = l->next;
   }

#ifdef DEBUG
   printk(KERN_WARNING "IIL: copying data to list node\n");
#endif

   /* copy the hint-pwd pair into the referenced list element */
   strncpy(l->hpw.hint, hint, MAX_HINT_SIZE);
   strncpy(l->hpw.pwd, pwd, MAX_PWD_SIZE);

#ifdef DEBUG
   printk(KERN_WARNING "IIL: copy complete\n");
#endif

   return TRUE;
}

/* delete_from_list: deletes the referenced hint-pwd pair from vault */
void delete_from_list (struct hpw_list **la) {
   struct hpw_list *l = *la;

   if (l == NULL) return;

   struct hpw_list *p = l->prev;
   struct hpw_list *n = l->next;

   /* cause previous element in list to reference what appears after l */
   if (p != NULL) {
      p->next = n;

#ifdef DEBUG
      printk(KERN_WARNING "DFL:  node at %x now references %x as next\n", p, n);
#endif
   }

   /* cause next element in list to reference what appears before l */
   if (n != NULL) {
      n->prev = p;

#ifdef DEBUG
      printk(KERN_WARNING "DFL:  node at %x now references %x as prev\n", n, p);
#endif
   }

#ifdef DEBUG
   printk(KERN_WARNING "DFL:  releasing %x\n", *la);
#endif

   kfree(*la);
   *la = NULL;

#ifdef DEBUG
   printk(KERN_WARNING "DFL:  released %x\n", *la);
#endif
}

