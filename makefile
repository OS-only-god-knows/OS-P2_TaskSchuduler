job: job.c job.h error.c enq deq stat Demo Demo1 Demo2 Demo3 Demo4 Demo5 Demo6 Demo7 Demo8 Demo9
	cc -o job job.c job.h error.c
enq: enq.c job.h error.c
	cc -o enq enq.c job.h error.c
deq: deq.c job.h error.c
	cc -o deq deq.c job.h error.c
stat: stat.c job.h error.c
	cc -o stat stat.c job.h error.c
Demo: Demo.c
	cc -o Demo Demo.c
Demo1: Demo1.c
	cc -o Demo1 Demo1.c
Demo2: Demo2.c
	cc -o Demo2 Demo2.c
Demo3: Demo3.c
	cc -o Demo3 Demo3.c
Demo4: Demo4.c
	cc -o Demo4 Demo4.c
Demo5: Demo5.c
	cc -o Demo5 Demo5.c
Demo6: Demo6.c
	cc -o Demo6 Demo6.c
Demo7: Demo7.c
	cc -o Demo7 Demo7.c
Demo8: Demo8.c
	cc -o Demo8 Demo8.c
Demo9: Demo9.c
	cc -o Demo9 Demo9.c
clean:
	rm job enq deq stat Demo
