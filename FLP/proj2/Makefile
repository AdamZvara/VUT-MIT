all:
	swipl -q -g start -o flp23-log -c flp23-log.pl

run:
	./flp23-log < $(ARGS)

clean:
	rm -f flp23-log

.PHONY: all run clean