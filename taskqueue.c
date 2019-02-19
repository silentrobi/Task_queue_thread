
// Created by MOHAMMAD ABU MUSA RABIUL on 12-12-18.
// ID:  220201072

#include <time.h>
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include <string.h>
#include <pthread.h>

#define  false 0
#define  true  1


struct lst_node_s { // list node
    int data;
    struct lst_node_s *next;
};
struct tsk_node_s { // task node
    int task_num; //starting from 0
    int task_type; // insert:0, delete:1, search:2
    int value;
    struct tsk_node_s* next;
};

/* rename the type*/

typedef struct lst_node_s ListNode;
typedef struct tsk_node_s TaskNode;

/* List Node functions */

ListNode* createNode(int value); //Create and return a node with the given data
int isEmpty(ListNode *head); //checks if there is an element in the list or not
int insert(int value); //Add given node to the given index
int delete(int value); //Delete first ListNode where given data occurs
void printList(ListNode *head); //print the values of each node in the list
void sortedInsertion(ListNode** head, ListNode* newNode); //insert a node in sorted order
int search(int value); //search a value in the list
void deleteAll(ListNode** head); // delete all node list
/* Task queue functions */

void Task_enqueue(int task_num, int task_type, int value); //insert a new task into task queue
int Task_dequeue(); //take a task from task queue
void processTask(TaskNode * task); // process a task (insert/ search/ delete)

/* global head variables */
ListNode* listHead= NULL;
TaskNode* taskHead= NULL;



int checkNumber(char* s);
void precheck(char* s, int size);


// Define thread task
void* task_thread(void * param);


// Initialize mutex
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

// Initialize conditional variable
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;

int task_count=0;
int available= false;
int exit_status= false;


int main(int argc, char *argv[]){

    clock_t begin = clock();

    int THREAD_SIZE; // hold user define thread size
    int TASK_QUEUE_SIZE;  // hold user define task size

    precheck(argv[1],argc);// pre-check  thread size input
    THREAD_SIZE= atoi(argv[1]);
    precheck(argv[2],argc);// pre-check  task size input
    TASK_QUEUE_SIZE= atoi(argv[2]);

    /* parent thread main, creating child threads */
    pthread_t tid[THREAD_SIZE]; /* the thread identifier */
    pthread_attr_t attr[THREAD_SIZE]; /* set of thread attributes */
    int i;
    for(i=0; i<THREAD_SIZE; i++) {
        pthread_attr_init(&attr[i]); // get the thread default attributes
        if (pthread_create(&tid[i], &attr[i], task_thread,NULL)) {
            printf("ERROR OCCURED DURING THREAD CREATION\n");
            exit(EXIT_FAILURE);
        }
    }

    if ( TASK_QUEUE_SIZE == 0){
        printf("%s\n", "EXPECTED TASK QUEUE SIZE SHOULD BE GREATHER THAN ZERO !!!\n");
        exit(EXIT_FAILURE);
    }
    if ( THREAD_SIZE == 0){
        printf("%s\n", "EXPECTED THREAD SIZE SHOULD BE GREATHER THAN ZERO !!!\n");
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL)); // to get different seed
    while (task_count != TASK_QUEUE_SIZE){
        pthread_mutex_lock(&mut); //lock

        if (!available) {
            int randomType = rand() % 3; // random type
            int randomValue = rand() % 100;  // taking random value [0-99] Note: random value can be chosen in any range

            Task_enqueue(task_count, randomType, randomValue); //en-queuing task one by one into the task queue
            task_count++;
            available = true;

            printf("task_cout %d\n",task_count);

        }

        if (available){
            pthread_cond_signal(&cond);
        }

        pthread_mutex_unlock(&mut); //release
    }

    while(1){ //check if still any available task.
              //if exist then send signal until other thread process that task.
        pthread_mutex_lock(&mut); //lock
        if (available){
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mut); //unlock


        }

        if (!available) {

            pthread_cond_broadcast(&cond);
            exit_status =true;
            pthread_mutex_unlock(&mut);
            break;

        }


    }


    // wait for child threads to join

    for (i=0; i < THREAD_SIZE; i++){
        pthread_join(tid[i], NULL);
    }

    printf("Final List:\n");
    printList(listHead);
    deleteAll(&listHead); //deallocating all elements in the list

    clock_t end = clock();


    double ex_time= (double)(end - begin) / CLOCKS_PER_SEC;
    printf("\nExecution time: %lf\n", ex_time);
    return EXIT_SUCCESS;
}

/*List node functions */
//Create and return a node with the given data
ListNode* createNode(int value){
    ListNode *newNode = malloc(sizeof(ListNode));
    newNode->data = value;
    newNode->next = NULL;

    return newNode;
}

//checks if there is an element in the list or not
int isEmpty(ListNode *head){
    ListNode* temp = head;
    if (temp == NULL) return 1;
    return 0;

}

/* Inserts a new node on the front of the list. */

int insert(int value){

    ListNode** head= &listHead; // assigning global head value of List
    if (!search(value)) { // if the value is not present in the list
        ListNode* newNode = createNode(value);
        sortedInsertion(head,newNode);
        return 1;
    }
    return 0; // if present return false

}

//Delete first ListNode where given data occurs
int  delete( int value) // return the value which is deleted.
{
    ListNode ** head= &listHead; // assigning global head value of List
    ListNode* temp = *head, *prev= NULL; //Store head node

    // if list is empty
    if (*head == NULL) return 0; // 0 means couldn't find the value in the list to be deleted

    // If head node holds the date to be deleted
    if (temp != NULL && temp->data == value)
    {

        *head = temp->next;   // Changed head
        free(temp);   // free head
        return 1;
    }

    // Look for the  value to be deleted,
    //Also keep track of the  previous node because we have to change the 'prev->next'
    while (temp != NULL && temp->data != value)
    {
        prev = temp;
        temp = temp->next;
    }

    // If next node is empty
    if (temp == NULL) return 0; // 0 means couldn't find the value in the list to be deleted


    // Unlink the node from linked list if value of the node is found in the node list
    prev->next = temp->next;

    free(temp);  // Free memory
    return 1;
}

int search(int value){ // sequential search
    ListNode* temp = listHead; // assigning global head value of Node List
    while (!isEmpty(temp)){ // if not empty
        if (temp->data == value)
        {
            return 1; // return true

        }
        temp = temp->next;


    }

    return 0; // return false


}



//print the values of each node in the list
void printList(ListNode *head){
    ListNode* temp = head;
    if (temp == NULL){
        printf("%s\n", "LIST IS EMPTY");
        return;
    }
    while (temp != NULL) {
        printf("%d ", temp->data);
        temp=temp->next;
    }
}


/* function to insert a newNode in a list. Note that this
function expects a pointer to pointer to  head as it can modify the
head of the input linked list*/
void sortedInsertion(ListNode** head, ListNode* newNode) {

    /* special case for the head*/
    if (*head == NULL || (*head)->data > newNode->data) {
        newNode->next = *head;
        *head = newNode;
    } else {
        /* finding the node before insertion */
        ListNode *current = *head;
        while (current->next != NULL && current->next->data < newNode->data) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}
void deleteAll(ListNode** head)
{

    ListNode* current = *head;

    ListNode* next;
    if (isEmpty(*head)){

        printf("%s\n", "Can't delete empty list");
        return;
    }
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    *head=NULL; //making head value null
    printf("\n%s", "De-allocated all elements in the list\n" );
}


TaskNode * createTask(int task_num, int taskType, int value){ // create a task
    TaskNode *newTask = malloc(sizeof(TaskNode));
    newTask->value = value;
    newTask->task_num= task_num;
    newTask->task_type= taskType;
    newTask->next = NULL;

    return newTask;

}

void Task_enqueue(int task_num, int task_type, int value) {
    TaskNode ** head = &taskHead; //// assigning global head value of Task queue
    TaskNode *newTask = createTask(task_num, task_type, value);
    TaskNode *temp = *head;
    if (temp == NULL) { // task head is null
        *head = newTask;
        return;
    } else {
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newTask;
        return;
    }
}
void processTask(TaskNode* task){ // process a given Task

    int flag=0;

    if (task == NULL){
        printf("%s", "No Task to process");
        return;
    }
    else if (task->task_type == 0) { //insert: 0
        flag= insert(task->value);
        if (flag){
            printf("Task %d-insert %d: %d %s\n",task->task_num, task->value,task->value, "is inserted.");
        }
        else{
            printf("Task %d-insert %d: %d %s\n",task->task_num, task->value,task->value, "can not be inserted");
        }
    }
    else if (task->task_type == 1) {// delete: 1
        flag = delete(task->value);
        if (flag)  printf("Task %d-delete %d: %d %s\n",task->task_num, task->value,task->value, "is deleted.");
        else  printf("Task %d-delete %d: %d %s\n",task->task_num, task->value,task->value, "can not be deleted");
    }

    else if (task->task_type == 2) { //search: 2
        flag = search(task->value);
        if (flag) {
            printf("Task %d-search %d: %d %s\n",task->task_num, task->value,task->value, "is present.");
        }
        else printf("Task %d-search %d: %d %s\n",task->task_num, task->value,task->value, "is not found.");
    }else{
        printf("%s\n", "UNKNOWN ERROR OCCURRED");
    }



}


int Task_dequeue(){

    TaskNode* delete;
    if (taskHead == NULL){
        printf("%s", "No Task in the queue.");
        return 0;
    }
    else if (taskHead != NULL){
        delete= taskHead;

        taskHead = taskHead->next; // changing the head value.
        processTask(delete); // process the task before deleting.
        free(delete); // de-allocating the de-queued node.
        return 1;
    }
    else{
        printf("%s\n", "UNKNOWN ERROR OCCURRED DURING DE_QUEUE.");
    }


}

int checkNumber(char * s){
    int i=0;
    if(s[0]== '-'){
        for(i=1;i< strlen(s);i++){
            if (!isdigit(s[i])) return 0;
        }
        return 1;
    }
    else{
        for(;i< strlen(s);i++){
            if (!isdigit(s[i])) return 0;
        }
        return 1;
    }

}

void precheck(char* s, int size){
    if ( size<3){
        printf("%s\n","ERROR: too few arguments.");
        exit(1);
    }
    if (size >3){
        printf("%s\n","ERROR: too many arguments.");
        exit(1);

    }

    if(!checkNumber(s)) {

        printf("%s\n","ERROR: argument is not a number.");
        exit(1);

    }
    if( atoi(s) < 0 ){
        printf("%s\n","ERROR: argument is less than zero.");
        exit(1);
    }
}

void * task_thread(void * param){
    while (true) {   // each thread tries infinitely until gets a broatcast signal from main thread.
        pthread_mutex_lock(&mut); //lock

        while (!available && !exit_status) {

            pthread_cond_wait(&cond, &mut);

        }
        if (exit_status) {
            pthread_mutex_unlock(&mut);
            pthread_exit(NULL);
        }
        Task_dequeue();
        available = false;
        pthread_mutex_unlock(&mut);
    }
}

