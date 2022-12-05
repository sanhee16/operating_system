## operating_system
- (multi, single) thread, virtual memory, scheduling, filesystem, shell 등의 os 개념을 simulator로 구현

### Skills  
- C, Linux

### Explanation
- 각 코드에 대한 상세 설명 및 목적은 각 폴더안에 들어있음.   



### Scheduling
- 스케쥴링 개념인 FIFO(First In First Out), RR(Round-Robin), SJF(Shortest Job First) 구현
- 같은 작업을 주고 어떤 스케쥴링 알고리즘을 사용했을때 output(FIFO.txt, FIFO2.txt, RR.txt, RR2.txt)이 어떻게 나오는지 비교  
- process를 만들어서 task를 수행하도록 함  

### Shell   
- shell 환경 구현  


### thread
1개의 producer(thread)는 task를 만들고, n개의 consumer(thread)는 task를 수행한다.  
producer가 만든 task가 모두 끝나면 프로그램 종료, 각각 걸린 시간을 비교한다. 
consumer들끼리는 메모리가 공유되기 때문에, deadLock과 busy waiting에 주의한다.  
- 1. single Thread: (1개의 producer, 1개의 consumer)
- 2. multi Thread: (100개의 producer, 100개의 consumer) 

### Virtual Memory  
실제 주어진 메모리보다 더 많이사용할 수 있도록 가상메모리 구현.
- One-level paging
- Two-level paging  
- swapping(clock, lru)
