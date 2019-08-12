IMAGE_NAME=convto/mycc
IMAGE_VERSION=latest
CFLAGS=-std=c11 -g -static
RUN_CONTAINER=docker run -v `pwd`:/mycc -it --rm $(IMAGE_NAME):$(IMAGE_VERSION)

test-and-clean: test clean ## Run test and clean

mycc: mycc.c

test: mycc ## Run test
	./test.sh

clean: ## Clean output files
	rm -f mycc *.o *~ tmp*

help: ## Print help.
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z0-9_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

build-img: ## Build ubuntu image
	docker build -t $(IMAGE_NAME):$(IMAGE_VERSION) .

run-img: ## Run ubuntu image
	$(RUN_CONTAINER)

