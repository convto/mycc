IMAGE_NAME=convto/mycc
IMAGE_VERSION=latest
RUN_CONTAINER=docker run -v `pwd`:/mycc -it --rm --cap-add=SYS_PTRACE --security-opt="seccomp=unconfined" $(IMAGE_NAME):$(IMAGE_VERSION)
CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

test-and-clean: test clean ## Run test and clean

mycc: $(OBJS) ## Build clang source code
	$(CC) -o mycc $(OBJS) $(LDFLAGS)

test: mycc ## Run test
	./test.sh

clean: ## Clean output files
	rm -f mycc *.o *~ tmp*

fmt: ## Format clang files
	clang-format -style=Google -i $(SRCS) mycc.h

help: ## Print help.
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z0-9_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

build-img: ## Build ubuntu image
	docker build -t $(IMAGE_NAME):$(IMAGE_VERSION) .

run-img: ## Run ubuntu image
	$(RUN_CONTAINER)

clean-img: ## Remove old docker img
	@docker system df
	@docker system prune -f
	@docker volume prune -f
	@docker system df
