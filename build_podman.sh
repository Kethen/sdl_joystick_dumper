set -xe

IMAGE_TAG="sdl_joystick_dumper"

if [ "$REBUILD_IMAGE" == "true" ]
then
	podman image rm -f $IMAGE_TAG
fi

if ! podman image exists $IMAGE_TAG
then
	podman image build -t $IMAGE_TAG -f Dockerfile
fi

podman run \
	--rm -it \
	-v ./:/work_dir \
	-w /work_dir \
	--entrypoint /usr/bin/bash \
	$IMAGE_TAG \
	-c '

set -xe
bash build.sh
bash build_windows.sh

'
	
