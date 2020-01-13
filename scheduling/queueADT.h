typedef struct Node 
{
	int data;
	struct Node *next;
}Node;


typedef struct Queue 
{
	Node *front;
	Node *rear; 
	int count;
}Queue;

int fnum; 

void InitQueue(Queue *queue);
int IsEmpty(Queue *queue);
void Enqueue(Queue *queue, int data); 
int Dequeue(Queue *queue); 
int find(Queue *queue, int find_data);
int counting (Queue *queue);
int sorting(Queue *queue, int *ptr);

		
int sorting(Queue *queue, int *ptr){
	int i= 0;
	if (queue->count==0){return -1;}
	Node *now = queue ->front;
	while (now){
		*(ptr+i)=now -> data;
		now = now->next;
		i++;
	}
	return 0;
}

void InitQueue(Queue *queue){
	queue->front = queue->rear = NULL; 
	queue->count = 0;
}

int IsEmpty(Queue *queue)
{
	return queue->count == 0;   
}

void Enqueue(Queue *queue, int data)
{
	Node *now = (Node *)malloc(sizeof(Node)); 
	now->data = data;
	now->next = NULL;

	if (IsEmpty(queue))
	{
		queue->front = now;
	}
	else
	{
		queue->rear->next = now;
	}
	queue->rear = now;
	queue->count++;
}

int Dequeue(Queue *queue)
{
	int re = -1;
	Node *now;
	if (IsEmpty(queue))
	{
		return -1;
	}
	now = queue->front;
	re = now->data;
	queue->front = now->next;
	queue->count--;
	return re;
}
int counting (Queue *queue){
	return queue->count;
}	
int find(Queue *queue, int find_data){
	int re = -1;
	Node* now;
	fnum=0;
	Node *find = queue->front;
	Node *link = queue->front;
	
	int exist=0;
	if (IsEmpty(queue))
	{
		return re;
	}
	for(int a=0; a < queue->count; a++){
		fnum++;
		if(find->data == find_data){
			now = find;
			exist=1;
			break;
		}
		link = find;
		find = find->next;
	}
	if(exist==0){
		return -1;
	}
	else if(exist==1){
		if(fnum==1){
			queue->front = queue->front->next;
			find->next=NULL;
			queue->count--;
			return find->data;

		}
		else if(fnum==queue->count){
			queue->rear = link;
			queue->rear->next==NULL;
			queue->count--;
			return find->data;	
		}
		link->next = find->next;
		find->next=NULL;
		queue->count--;
		return find->data;
	}
}
