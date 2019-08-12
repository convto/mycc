IMAGE_NAME=convto/mycc
IMAGE_VERSION=latest

PONY:help

help: ## Print help.
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z0-9_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

build: ## Build ubuntu image
	docker build -t $(IMAGE_NAME):$(IMAGE_VERSION) .

run: ## Run ubuntu image
	docker run -v `pwd`:/mycc -it --rm $(IMAGE_NAME):$(IMAGE_VERSION)
