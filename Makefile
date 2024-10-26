CC = gcc
CFLAGS = -Wall -g -fopenmp

OBJS = vehicleSpeed.o batteryVoltage.o main.o

all: dashboard

valgrind: dashboard
	valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./dashboard

dashboard: $(OBJS)
	$(CC) $(CFLAGS) -o dashboard $(OBJS) -lwiringPi 

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

vehicleSpeed.o: vehicleSpeed.c
	$(CC) $(CFLAGS) -c vehicleSpeed.c

batteryVoltage.o: batteryVoltage.c
	$(CC) $(CFLAGS) -c batteryVoltage.c

clean:
	rm -f $(OBJS) dashboard